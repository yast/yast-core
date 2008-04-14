/* y2log.cc
 *
 * YaST2: Core system
 *
 * YaST2 logging implementation
 *
 * Authors: Mathias Kettner <kettner@suse.de>
 *          Thomas Roelz <tom@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *          Arvin Schnell <arvin@suse.de>
 *          Martin Vidner <mvidner@suse.cz>
 *
 * $Id$
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

#include "y2util/y2log.h"
#include "y2util/miniini.h"
#include "y2util/stringutil.h"
#include "y2util/PathInfo.h"
#include <syslog.h>

/* Defines */

#define _GNU_SOURCE 1				/* Needed for vasprintf below */

#define Y2LOG_DATE	"%Y-%m-%d %H:%M:%S"	/* The date format */

// 1 component, 2 file, 3 func, 4 line, 5 logtext, 6 eol
#define Y2LOG_COMMON	"%s %s%s:%d %s%s"
#define Y2LOG_SIMPLE	"%2$s%3$s:%4$d %1$s %5$s%6$s"	/* this is GNU gettext parameter reordering */

// 1 timestamp, 2 level, 3 hostname, 4 pid
#define Y2LOG_FORMAT	"%s <%d> %s(%d)"
// 1 level, 2 common part
#define Y2LOG_SYSLOG	"<%d>%s"

#define Y2LOG_MAXSIZE	10* 1024 * 1024		/* Maximal logfile size */
#define Y2LOG_MAXNUM	10			/* Maximum logfiles number */

// FIXME
#define LOGDIR		"/var/log/YaST2"

#define Y2LOG_ROOT	LOGDIR "/y2log"
#define Y2LOG_USER	"/.y2log"		/* Relative to $HOME */
#define Y2LOG_FALLBACK	"/y2log"

#define Y2LOG_CONF	"log.conf"	/* Relative to $HOME or /etc/YaST2 */

#define Y2LOG_VAR_DEBUG	"Y2DEBUG"
#define Y2LOG_VAR_ALL	"Y2DEBUGALL"
#define Y2LOG_VAR_ONCRASH "Y2DEBUGONCRASH"
#define Y2LOG_VAR_SIZE	"Y2MAXLOGSIZE"
#define Y2LOG_VAR_NUM	"Y2MAXLOGNUM"

#define Y2LOG_FACILITY	"yast2"

inisection logconf;

static bool did_set_logname = false;
static bool did_read_logconf = false;

static const char *logname;

static off_t maxlogsize;
static int   maxlognum;

static bool log_debug = false;
static bool log_to_file = true;
static bool log_to_syslog = false;

static bool log_all_variable = false;
static bool log_simple = false;

static FILE *Y2LOG_STDERR = stderr;		/* Default output */

/* static prototypes */
static void do_log_syslog( const char* logmessage );
static void do_log_yast( const char* logmessage );
static void shift_log_files(string filename);

static const char *log_messages[] = {
    "debug",
    "milestone",
    "warning",
    "error",
    "error",
    "error",
    "error",
};

/**
 * y2log must use a private copy of stderr, esp. in case we're always logging
 * to it (option "-l -"). Some classes like liby2(ExternalProgram) redirect
 * stderr in order to redirect an external programs error output. As a side
 * effect y2log output done after the redirection would show up in the external
 * programs output file instead of yast2's stderr.
 */
static int dup_stderr()
{
    int dupstderr = dup( 2 );
    if ( dupstderr != -1 ) {
	FILE * newstderr = fdopen( dupstderr, "a" );

	if ( newstderr == NULL ) {
	    fprintf( Y2LOG_STDERR, "y2log: Can't fdopen new stderr: %s.\n", strerror (errno) );
	}
	else {
	    fcntl (fileno (newstderr), F_SETFD, fcntl (fileno (newstderr), F_GETFD) | FD_CLOEXEC);
	    Y2LOG_STDERR = newstderr;
	}
    }
    else {
	fprintf( Y2LOG_STDERR, "y2log: Can't dup stderr: %s.\n", strerror (errno) );
    }
    return 1;
}
static int variable_not_used = dup_stderr();

static FILE * open_logfile()
{
    FILE *logfile = Y2LOG_STDERR;
    if (*logname != '-') {
	logfile = fopen (logname, "a");
	// try creating the directory if it may be missing
	if (!logfile && errno == ENOENT) {
	    PathInfo::assert_dir (Pathname::dirname(logname), 0700);
	    // and retry
	    logfile = fopen (logname, "a");
	}
	if (!logfile && !log_simple) {
	    fprintf (Y2LOG_STDERR, "y2log: Error opening logfile '%s': %s.\n",
		     logname, strerror (errno));
	    return NULL;
	}
    }
    return logfile;
}

/**
 * The universal logger function
 */
void y2_logger_function(loglevel_t level, const char *component, const char *file,
	  const int line, const char *func, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    y2_vlogger_function(level, component, file, line, func, format, ap);
    va_end(ap);
}

void y2_logger_blanik(loglevel_t level, const char *component, const char *file,
	  const int line, const char *func, const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    y2_vlogger_blanik(level, component, file, line, func, format, ap);
    va_end(ap);
}

/**
 * Formats the common part
 */
string y2_logfmt_common(bool simple, const char *component, const char *file,
	   const int line, const char *function, const char *format, va_list ap)
{
    /* Prepare the log text */
    char *logtext = NULL;
    vasprintf(&logtext, format, ap); /* GNU extension needs the define above */

    /* Prepare the component */
    string comp = component;
    if (!comp.empty ())
	comp = " [" + comp + "]";

    /* Prepare the file, strip rooted path  */
    if(*file == '/')		     // rooted path
    {
	char *slashptr = strrchr (file, '/');
	if (slashptr > file)		    // last slash is second slash
	{
	    char *slashptr2 = slashptr-1;

	    // find last but one slash

	    while ((slashptr2 > file) && (*slashptr2 != '/'))
	    {
		slashptr2--;
	    }
	    // if last but one slash exists, use this as start
	    if (slashptr2 != file)
		file = slashptr2 + 1;
	}
    }

    /* Prepare the function */
    string func = function;
    if (!func.empty ())
	func = "(" + func + ")";

    /* do we need EOL? */
    bool eol = false;
    size_t len = strlen(logtext);
    if ((len==0) || ((len>0) && (logtext[len-1]!='\n')))
	eol = true;

    char * result_c;
    asprintf(&result_c, simple? Y2LOG_SIMPLE: Y2LOG_COMMON,
	     comp.c_str (), file, func.c_str (), line, logtext, eol?"\n":"");
    string result = result_c;
    free (result_c);

    free (logtext);
    return result;
}

string y2_logfmt_prefix (loglevel_t level)
{
    /* Prepare the PID */
    pid_t pid = getpid();

    /* Prepare the host name */
    char hostname[1024];
    if (gethostname(hostname, 1024))
	strncpy(hostname, "unknown", 1024);

    /* Prepare the date */
#if 1
    // just 1 second precision
    time_t timestamp = time (NULL);
    struct tm *brokentime = localtime (&timestamp);
    char date[50];		// that's big enough
    strftime (date, sizeof (date), Y2LOG_DATE, brokentime);
#else
    // 1 millisecond precision (use only for testing)
    timeval time;
    gettimeofday (&time, NULL);
    time_t timestamp = time.tv_sec;
    struct tm *brokentime = localtime (&timestamp);
    char tmp1[50], date[50];	// that's big enough
    strftime (tmp1, sizeof (tmp1), Y2LOG_DATE, brokentime);
    snprintf (date, sizeof (date), "%s.%03ld", tmp1, time.tv_usec / 1000);
#endif

#if 0
    // print "bytes from system" and "bytes in use" per thread (use only
    // for testing)
    struct mallinfo mi;
    mi = mallinfo ();
    char tmp2[50];
    snprintf (tmp2, sizeof (tmp2), " [%5dkB %5dkB]", mi.arena >> 10,
	      mi.uordblks >> 10);
    strcat (date, tmp2);
#endif

    char * result_c = NULL;
    asprintf (&result_c, Y2LOG_FORMAT, date, level, hostname, pid);
    string result = result_c;
    free (result_c);

    return result;
}


void y2_vlogger_function(loglevel_t level, const char *component, const char *file,
	   const int line, const char *function, const char *format, va_list ap)
{
    string common = y2_logfmt_common (log_simple,
				      component, file, line, function,
				      format, ap);

    if(log_to_syslog) {
	syslog (LOG_NOTICE, Y2LOG_SYSLOG, level, common.c_str ());
    }

    if(log_to_file) {
	string tolog;
	if (log_simple)
	    tolog = common;
	else
	    tolog = y2_logfmt_prefix (level) + common;
	do_log_yast (tolog.c_str ());
    }
}

void y2_vlogger_blanik(loglevel_t level, const char *component, const char *file,
	   const int line, const char *function, const char *format, va_list ap)
{
    string common = y2_logfmt_common (log_simple,
				      component, file, line, function,
				      format, ap);

    if(log_to_syslog || log_to_file) {
	string tolog;
	if (log_simple || (log_to_syslog > log_to_file))
	    tolog = common;
	else
	    tolog = y2_logfmt_prefix (level) + common;
	// store the message for worse times
	blanik.push_back (tolog);
    }
}

void y2_logger_raw( const char* logmessage )
{
    if(log_to_syslog) {
	do_log_syslog (logmessage);
    }

    if(log_to_file) {
	do_log_yast (logmessage);
    }
}

static
void do_log_syslog( const char* logmessage )
{
    syslog (LOG_NOTICE, "%s", logmessage);
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
    if (logfile && logfile != Y2LOG_STDERR)
	fclose (logfile);
    else
	fflush (logfile);
}

/**
 * Logfile name initialization
 */
void set_log_filename (string fname)
{
    did_set_logname = true;

    if(!did_read_logconf) set_log_conf("");

    if(log_to_syslog) openlog("yast2", LOG_PID, LOG_DAEMON);
    if(!log_to_file) return;

    struct passwd *pw = getpwuid( geteuid() );
    const char *filename = fname.c_str();

    char *env_maxlogsize = getenv(Y2LOG_VAR_SIZE);
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
      maxlogsize = Y2LOG_MAXSIZE;

    char *env_maxlognum = getenv(Y2LOG_VAR_NUM);
    maxlognum = env_maxlognum ? atoi(env_maxlognum) : Y2LOG_MAXNUM;

    /* Assign logfile name */

    if ((filename == 0) || (*filename == 0))
    {		/* No filename --> use defaults */
	if (geteuid()) {			/* Non root */
	    if (!pw)
	    {
		fprintf( Y2LOG_STDERR,
			 "Cannot read pwd entry of user id %d. Logfile will be '%s'.\n",
			 geteuid(), Y2LOG_FALLBACK );

		logname = Y2LOG_FALLBACK;
	    }
	    else
	    {
		// never freed
		char * my_logname = (char *)malloc (strlen (pw->pw_dir) + strlen (Y2LOG_USER) + 1);
		strcpy (my_logname, pw->pw_dir);
		logname = strcat (my_logname, Y2LOG_USER);
	    }
	}
	else		    /* Root */
	    logname = Y2LOG_ROOT;
    }
    else
	logname = strdup (filename);  /* Explicit assignment */
}


/**
 * Logfile name reporting
 */
string get_log_filename()
{
    if(!did_set_logname) set_log_filename("");
    return string (logname);
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


/**
 * Signal handler: re-read log configuration
 * The variable is checked in the main logger function
 */
static void signal_handler (int signum)
{
    if (signum == SIGUSR2)
    {
	did_read_logconf = false;
    }
    else if (signum == SIGUSR1)
    {
	log_debug = !log_debug;
    }
}

static void set_signal_handler (int signum)
{
    struct sigaction a;
    a.sa_handler = signal_handler;
    a.sa_flags = 0;
    sigemptyset (&a.sa_mask);

    int ret = sigaction (signum, &a, NULL);
    if (ret != 0)
    {
	fprintf (stderr, "Could not set handler of signal #%d for y2log: %d\n",
		 signum, ret);
    }
}

/**
 * Parse the log.conf
 */
void set_log_conf(string confname) {

    did_read_logconf = true;

    set_signal_handler (SIGUSR2);
    set_signal_handler (SIGUSR1);

    string logconfname = confname;
    if(logconfname == "") {
	/* Read the logconf.ycp */
	struct passwd *pw = getpwuid(geteuid());
	if(!pw) {
	    fprintf (stderr, "Cannot read pwd entry for user "
		    "id %d. No logconf, using defaults.\n", geteuid ());
	    return;
	}
	if(getuid())
	    logconfname = string (pw->pw_dir) + "/.yast2/" + Y2LOG_CONF;
	else
	    logconfname = "/etc/YaST2/" Y2LOG_CONF;
    }

    if(getenv(Y2LOG_VAR_ALL)) log_all_variable = true;

    /* We have to remember the errno. Otherwise a call of
     * y2error ("error: %m") can display a wrong message. */
    int save_errno = errno;
    inifile i = miniini(logconfname.c_str());
    logconf = i["Debug"];

    log_to_file = i["Log"]["file"] != "false";
    log_to_syslog = i["Log"]["syslog"] == "true";
    log_debug = (i["Log"]["debug"] == "true") || getenv(Y2LOG_VAR_DEBUG);

    if(i["Log"]["filename"] != "")
	logname = strdup(i["Log"]["filename"].c_str());

    errno = save_errno;
}


/**
 * Test if we should create a log entry
 */
bool should_be_logged (int loglevel, string componentname) {

    if(log_simple && !log_debug) return loglevel > 1;

    /* Only debug level is controllable */
    if(loglevel > 0) return true;

    /* Prepare the logfile name. */
    if(!did_set_logname)
	set_log_filename("");

    /* Read log configuration. */
    if(!did_read_logconf)
	set_log_conf("");

    /* Everything should be logged */
    if(log_all_variable) return true;

    /* Specific component */
    if(logconf.find(componentname) != logconf.end())
	return logconf[componentname] == "true";

    return log_debug;
}


/**
 * Set (or reset) the simple mode
 */
void set_log_simple_mode(bool simple) {
    log_simple = simple;
}

void set_log_debug(bool on)
{
	log_debug = on;
}

bool get_log_debug()
{
	return log_debug;
}

// buffer the debugging log and show it only if yast crashes
// fate#302166

bool should_be_buffered ()
{
    return getenv (Y2LOG_VAR_ONCRASH) != NULL;
}

// stores a few strings. can append one. can return all. old are forgotten.
class LogTail::Impl {
    size_t m_size;
    size_t m_max_size;
    std::list<Data> m_items;
public:
    Impl(size_t max_size = 42)
    : m_size (0)
    , m_max_size (max_size)
	{}

    void push_back (const Data &d) {
	if (m_size >= m_max_size)
	    m_items.pop_front ();
	else
	    ++m_size;

	m_items.push_back (d);
    }

    void for_each (Consumer c) {
	std::list<Data>::iterator i;
	for (i = m_items.begin (); i != m_items.end (); ++i)
	    if (! c(*i))
		break;
    }
};

LogTail::LogTail (size_t max_size)
{
    m_impl = new Impl (max_size);
}

LogTail::~LogTail ()
{
    delete m_impl;
}

void LogTail::push_back (const Data &d)
{
    m_impl->push_back (d);
}
void LogTail::for_each (LogTail::Consumer c)
{
    m_impl->for_each (c);
}

// define the singleton
LogTail blanik = LogTail ();

/* EOF */
