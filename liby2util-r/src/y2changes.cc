/* y2changes.cc
 *
 * YaST2: Core system
 *
 * YaST2 user-level logging implementation (based on y2log.cc)
 *
 * Authors: Mathias Kettner <kettner@suse.de>
 *          Thomas Roelz <tom@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *          Arvin Schnell <arvin@suse.de>
 *          Martin Vidner <mvidner@suse.cz>
 *	    Stanislav Visnovsky <visnov@suse.cz>
 */

#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits>
#include <list>

#include "y2util/y2changes.h"
#include "y2util/stringutil.h"
#include "y2util/PathInfo.h"
#include <syslog.h>

/* Defines */

#define _GNU_SOURCE 1				/* Needed for vasprintf below */

#define Y2CHANGES_DATE	"%Y-%m-%d %H:%M:%S"	/* The date format */

// 1 timestamp, 2 level, 3 hostname
#define Y2CHANGES_FORMAT	"%s <%s> %s "
// 1 level, 2 common part
#define Y2CHANGES_SYSLOG	"<%d>%s "

#define Y2CHANGES_MAXSIZE	10* 1024 * 1024		/* Maximal logfile size */
#define Y2CHANGES_MAXNUM	10			/* Maximum logfiles number */

#define LOGDIR		"/var/log/YaST2"

#define Y2CHANGES_ROOT	LOGDIR "/y2changes"
#define Y2CHANGES_USER	"/.y2changes"		/* Relative to $HOME */
#define Y2CHANGES_FALLBACK	"/y2changes"

#define Y2CHANGES_VAR_SIZE	"Y2MAXLOGSIZE"
#define Y2CHANGES_VAR_NUM	"Y2MAXLOGNUM"

#define Y2CHANGES_FACILITY	"yast2"

static bool did_set_logname = false;

static const char *logname;

static off_t maxlogsize;
static int   maxlognum;

static bool log_to_file = true;
static bool log_to_syslog = false;

static FILE *Y2CHANGES_STDERR = stderr;		/* Default output */

/* static prototypes */
static void do_log_syslog( const char* logmessage );
static void do_log_yast( const char* logmessage );
static void shift_log_files(string filename);

static const char *log_messages[] = {
    "item",
    "note",
};

/**
 * y2changes must use a private copy of stderr, esp. in case we're always logging
 * to it (option "-l -"). Some classes like liby2(ExternalProgram) redirect
 * stderr in order to redirect an external programs error output. As a side
 * effect Y2CHANGES output done after the redirection would show up in the external
 * programs output file instead of yast2's stderr.
 */
static int dup_stderr()
{
    int dupstderr = dup( 2 );
    if ( dupstderr != -1 ) {
	FILE * newstderr = fdopen( dupstderr, "a" );

	if ( newstderr == NULL ) {
	    fprintf( Y2CHANGES_STDERR, "y2log: Can't fdopen new stderr: %s.\n", strerror (errno) );
	}
	else {
	    fcntl (fileno (newstderr), F_SETFD, fcntl (fileno (newstderr), F_GETFD) | FD_CLOEXEC);
	    Y2CHANGES_STDERR = newstderr;
	}
    }
    else {
	fprintf( Y2CHANGES_STDERR, "y2log: Can't dup stderr: %s.\n", strerror (errno) );
    }
    return 1;
}
static int variable_not_used = dup_stderr();

static FILE * open_logfile()
{
    FILE *logfile = Y2CHANGES_STDERR;
    if (*logname != '-') {
	logfile = fopen (logname, "a");
	// try creating the directory if it may be missing
	if (!logfile && errno == ENOENT) {
	    PathInfo::assert_dir (Pathname::dirname(logname), 0700);
	    // and retry
	    logfile = fopen (logname, "a");
	}
	if (!logfile) {
	    fprintf (Y2CHANGES_STDERR, "y2log: Error opening logfile '%s': %s.\n",
		     logname, strerror (errno));
	    return NULL;
	}
    }
    return logfile;
}

string y2_changesfmt_prefix (logcategory_t category)
{
    /* Prepare the host name */
    char hostname[1024];
    if (gethostname(hostname, 1024))
	strncpy(hostname, "unknown", 1024);

    // just 1 second precision
    time_t timestamp = time (NULL);
    struct tm *brokentime = localtime (&timestamp);
    char date[50];		// that's big enough
    strftime (date, sizeof (date), Y2CHANGES_DATE, brokentime);

    char * result_c = NULL;
    asprintf (&result_c, Y2CHANGES_FORMAT, date, log_messages[category], hostname );
    string result = result_c;
    free (result_c);

    return result;
}

/**
 * The universal logger function
 */
void y2changes_function (logcategory_t category, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    /* Prepare the log text */
    char *logtext = NULL;
    vasprintf(&logtext, format, ap); /* GNU extension needs the define above */
    string common = logtext;
    common += '\n';
    free (logtext);

    if(log_to_syslog) {
	syslog (LOG_NOTICE, Y2CHANGES_SYSLOG, category, common.c_str ());
    }

    if(log_to_file) {
	string tolog = y2_changesfmt_prefix (category) + common;
	do_log_yast (tolog.c_str ());
    }

    va_end(ap);
}


static
void do_log_syslog( const char* logmessage )
{
    syslog (LOG_NOTICE, "%s", logmessage);
}

/**
 * Logfile name initialization
 */
static void set_log_filename (string fname)
{
    did_set_logname = true;

    if(log_to_syslog) openlog("yast2", LOG_PID, LOG_DAEMON);
    if(!log_to_file) return;

    struct passwd *pw = getpwuid( geteuid() );
    const char *filename = fname.c_str();

    char *env_maxlogsize = getenv(Y2CHANGES_VAR_SIZE);
    if ( env_maxlogsize ) {
      stringutil::strtonum( env_maxlogsize, maxlogsize );
      // prevent overflow (#156149)
      const off_t limit = std::numeric_limits<off_t>::max();
      const off_t limit_k = limit / 1024;
      if (maxlogsize <= limit_k)
	  maxlogsize *= 1024;
      else
	  maxlogsize = limit;
    } else 
      maxlogsize = Y2CHANGES_MAXSIZE;

    char *env_maxlognum = getenv(Y2CHANGES_VAR_NUM);
    maxlognum = env_maxlognum ? atoi(env_maxlognum) : Y2CHANGES_MAXNUM;

    /* Assign logfile name */

    if ((filename == 0) || (*filename == 0))
    {		/* No filename --> use defaults */
	if (geteuid()) {			/* Non root */
	    if (!pw)
	    {
		fprintf( Y2CHANGES_STDERR,
			 "Cannot read pwd entry of user id %d. Logfile will be '%s'.\n",
			 geteuid(), Y2CHANGES_FALLBACK );

		logname = Y2CHANGES_FALLBACK;
	    }
	    else
	    {
		// never freed
		char * my_logname = (char *)malloc (strlen (pw->pw_dir) + strlen (Y2CHANGES_USER) + 1);
		strcpy (my_logname, pw->pw_dir);
		logname = strcat (my_logname, Y2CHANGES_USER);
	    }
	}
	else		    /* Root */
	    logname = Y2CHANGES_ROOT;
    }
    else
	logname = strdup (filename);  /* Explicit assignment */
}


static
void do_log_yast( const char* logmessage )
{
    /* Prepare the logfile name */
    if(!did_set_logname) set_log_filename("");

    /* Prepare the logfile */
    shift_log_files (string (logname));

    FILE *logfile = open_logfile ();
    if (!logfile)
	return;

    fprintf (logfile, "%s", logmessage);

    /* Clean everything */
    if (logfile && logfile != Y2CHANGES_STDERR)
	fclose (logfile);
    else
	fflush (logfile);
}

static string old (const string & filename, int i, const char * suffix) {
    char numbuf[8];
    sprintf (numbuf, "%d", i);
    return filename + "-" + numbuf + suffix;
}

/**
 * Maintain logfiles
 * We do all of this ourselves because during the installation
 * logrotate does not run
 */
static void shift_log_files(string filename)
{
    struct stat buf;

    if( stat(filename.c_str(), &buf) )
	return;

    if( buf.st_size <= maxlogsize )
	return;

    static const char * gz = ".gz";
    // Delete the last logfile
    remove (old (filename, maxlognum - 1, ""   ).c_str());
    remove (old (filename, maxlognum - 1, gz).c_str());

    // rename existing ones
    for( int f = maxlognum-2; f > 0; f-- )
    {
	rename (old (filename, f, "").c_str(), old (filename, f+1, "").c_str());
	rename (old (filename, f, gz).c_str(), old (filename, f+1, gz).c_str());
    }

    // rename and compress first one
    rename( filename.c_str(), old (filename, 1, "").c_str() );
    // fate#300637: compress!
    // may fail, but so what
    system( ("nice -n 20 gzip " + old (filename, 1, "") + " &").c_str());
}


/* EOF */
