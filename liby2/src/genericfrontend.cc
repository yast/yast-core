/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	genericfrontend.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE		// needed for vasprintf below
#endif

/**
 * \file
 * Generic \ref main function handler for all YaST2 applications.
 */

/**
 * \mainpage Welcome to YaST2 Core System
 *
 * \section intro Introduction
 *
 * This is the main page of the YaST2 Core documentation. YaST Core is a generic component-based
 * system to provide the infrastructure for implementing functionality of the YaST
 * installation and configuration tool.
 *
 * \section whatis What is YaST2 Core?
 *
 * YaST Core is a collection of libraries to provide an infrastructure for rapid
 * development of configuration tools. It is a component-based, cross-language
 * system.
 *
 * Some of the general topics are covered by the following pages:
 *
 * - Component architecture
 * 	- \ref components
 * 	- \ref componentbroker
 *	- \ref componentsearch
 *
 * - Handling of error codes: \ref exitcodes
 *
 * - Libraries
 * 	- \ref liby2
 * 	- \ref libycp
 *	- \ref libscr
 *
 * - Generic components
 *	- \ref SCR
 * 	- \ref WFM
 *
 * \section community Join the community
 *
 * Visit our web site at : http://en.opensuse.org/YaST or #yast on irc.freenode.net
 *
 * \section License
 *
 * YaST is licensed under GPL v2.
 *
 */
 
/**
 * \page exitcodes YaST2 Exit Codes
 *
 * All applications using liby2 library share a common \ref main function. The function handles the exit codes in the following way.
 * 
 * The exit codes are described in \ref exitcodes.h header file. A special handling is applied to the value returned from the client.
 *  - If a value is \ref YCPNull or \ref YCPVoid, exitcode \ref YAST_OK will be used. 
 *  - If the value is \ref YCPBoolean, \ref YAST_OK will be returned for true, \ref YAST_CLIENTRESULT for false. 
 *  - If the value is \ref YCPInteger, the value will be added to \ref YAST_CLIENTRESULT and the resulting
 * integer will be the process exitcode. 
 *  - If the value is \ref YCPSymbol, for names defined by \ref ycp_error_exit_symbols \ref YAST_CLIENTRESULT
 * will be returned, otherwise \ref YAST_OK.
 */
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sstream>
#include <iomanip>
#include <string>

#include <ycp/y2log.h>
#include <ycp/ExecutionEnvironment.h>
#include "Y2Component.h"
#include "Y2ErrorComponent.h"
#include "Y2ComponentBroker.h"
#include <YCP.h>
#include <ycp/Parser.h>
#include <ycp/pathsearch.h>
#include "exitcodes.h"

/// number of symbols that are handled as error codes
#define MAX_YCP_ERROR_EXIT_SYMBOLS	2

/// symbol names that are handled as error codes when returned by the client
const char* ycp_error_exit_symbols[MAX_YCP_ERROR_EXIT_SYMBOLS] = { 
    "abort",
    "cancel"
};

using std::string;
ExecutionEnvironment ee;

/// fallback name of the program
static const char *progname = "genericfrontend";

static void print_usage ();
static void print_help ();
static void print_error (const char*, ...) __attribute__ ((format (printf, 1, 2)));
static bool has_parens (const char* arg);

int signal_log_fd;		// fd to use for logging in signal handler

static
void
signal_log_to_fd (int fd, const char * cs)
{
    ssize_t n = strlen (cs);
    while (true) {
       ssize_t w = write(fd, cs, n);
       if (w == n)
	   break;              // success
       else if (w == -1) {
	   if (errno == EINTR) {
	       // perror("gotcha"); // bnc#470645
	   }
	   else {
	       perror("write"); // other cases
	       break;
	   }
       }
       else {
	   errno = 0;
	   cs += w;
	   n -= w;
       }
    }

}

static
void
signal_log (const char * cs)
{
    signal_log_to_fd (signal_log_fd, cs);
}

static
bool
signal_log_ss (const string & s)
{
    signal_log (s.c_str ());
    return true;
}


static
void
signal_log_timestamp ()
{
    char buffer[200];
    time_t time_time;
    struct tm tm_time;

    time_time = time(NULL);
    localtime_r (&time_time, &tm_time);

    if (strftime(buffer, sizeof(buffer), "=== %F %T %z ===\n", &tm_time) != 0)
    {
	signal_log (buffer);
    }
}

// fate#302166 "cache yast debugging logs in case of failure"
static
void signal_log_stored_debug ()
{
    signal_log ("Liberating suppressed debugging messages:\n");
    blanik.for_each (signal_log_ss);
    signal_log ("End of suppressed debugging messages\n");
}

// FATE 302167, info '(libc) Backtraces'
void
signal_log_backtrace ()
{
    static const int N = 100;
    void *frames[N];
    size_t size = backtrace (frames, N);
    // demangling is not signal safe
    signal_log ("Backtrace: (use c++filt to demangle)\n");
    backtrace_symbols_fd (frames, size, signal_log_fd);
}

void
signal_log_open ()
{
    signal_log_fd = -1;

    const char * logfns[] = {
	"/var/log/YaST2/signal",
	"y2signal.log",
	NULL,			// sentinel
    };

    for (const char ** logfn_p = &logfns[0]; *logfn_p != NULL; ++logfn_p)
    {
	signal_log_fd = open (*logfn_p, O_WRONLY | O_CREAT | O_APPEND, 0600);
	if (signal_log_fd != -1)
	    break;
    }
}

void
signal_handler (int sig)
{
    signal (sig, SIG_IGN);

    // bnc#493152#c19 only signal-safe functions are allowed
    char buffer[200];
    int n = snprintf (buffer, sizeof(buffer),
		      "YaST got signal %d at YCP file %s:%d\n",
		      sig, ee.filename ().c_str (), ee.linenumber ());
    if (n >= (int)sizeof(buffer) || n < 0)
	strcpy (buffer, "YaST got a signal.\n");
    signal_log_to_fd (STDERR_FILENO, buffer);

    signal_log_open ();
    if (signal_log_fd == -1)
    {
	signal_log_to_fd (STDERR_FILENO, "Could not open log file.\n");
    }
    else
    {
	signal_log_timestamp ();
	signal_log (buffer);
	signal_log_stored_debug ();
	signal_log_backtrace ();

	if (close (signal_log_fd) == -1)
	    perror ("log close");
    }

    // bye
    signal (sig, SIG_DFL);
    kill ( getpid (), sig);
}


#include <blocxx/BLOCXX_config.h>
#include <blocxx/Logger.hpp>
#include <blocxx/LogMessage.hpp>
#if BLOCXX_LIBRARY_VERSION >= 5
#include <blocxx/LogAppender.hpp>
#else
#include <blocxx/LogConfig.hpp>
#endif

/**
 * The YaST logger for LiMaL framework. It's just a wrapper around
 * the standard YaST logging mechanism.
 */
#if BLOCXX_LIBRARY_VERSION >= 5
class YaSTLogger : public blocxx::LogAppender
#else
class YaSTLogger : public blocxx::Logger
#endif
{
public:
	virtual ~YaSTLogger() {};

	/**
	 * Constructor. YaST will try to log every message level,
	 * because we filter on our own.
	 */
#if BLOCXX_LIBRARY_VERSION >= 5
	YaSTLogger() : blocxx::LogAppender(
			blocxx::LogAppender::ALL_COMPONENTS,
			blocxx::LogAppender::ALL_CATEGORIES,
			blocxx::LogAppender::STR_TTCC_MESSAGE_FORMAT
		)
	{}
#else
        YaSTLogger() : blocxx::Logger ("YaST",blocxx::E_ALL_LEVEL) {}
#endif

	/**
	 * The logging message processing. The method converts the
	 * Blocxx LogMessage into a YaST logger. Unfortunatelly,
	 * the log level can be received only as a string, so
	 * we have to do a conversion to loglevel_t used by YaST.
	 *
	 * @param m	the message to be logged
	 */
#if BLOCXX_LIBRARY_VERSION >= 5
        virtual void doProcessLogMessage(const blocxx::String    & /*f*/,
	                                 const blocxx::LogMessage& m) const
#else
        virtual void doProcessLogMessage(const blocxx::LogMessage& m) const
#endif
	{
	    loglevel_t level = LOG_DEBUG;
	    if (m.category == blocxx::Logger::STR_FATAL_CATEGORY
		|| m.category == blocxx::Logger::STR_ERROR_CATEGORY)
	    {
		level = LOG_ERROR;
#if BLOCXX_LIBRARY_VERSION >= 5
	    } else if (m.category == blocxx::Logger::STR_WARNING_CATEGORY)
	    {
		level = LOG_WARNING;
#endif
	    } else if (m.category == blocxx::Logger::STR_INFO_CATEGORY)
	    {
		level = LOG_MILESTONE;
	    }

	    y2_logger(level,m.component.c_str ()
		,m.filename,m.fileline,m.methodname,"%s", m.message.c_str ());
	}
#if BLOCXX_LIBRARY_VERSION <= 4
	/**
	 * Clone this logger - create a new instance as a copy
	 * of this one and return a reference to the instance.
	 */
	virtual blocxx::LoggerRef doClone() const
	{
    	    return blocxx::LoggerRef(new YaSTLogger(*this));
	}
#endif
};

// YaSTLogger redirection factory for BloCxx/LiMaL
struct logger_initializer
{
	logger_initializer()
	{
#if BLOCXX_LIBRARY_VERSION >= 5
		blocxx::LogAppender::setDefaultLogAppender(
			blocxx::LogAppenderRef(new YaSTLogger())
		);
#else
		blocxx::Logger::setDefaultLogger(
			blocxx::LoggerRef(new YaSTLogger())
		);
#endif
	}
};
static logger_initializer initialize_logger;

void
parse_client_and_options (int argc, char ** argv, int& arg, char  *& client_name, YCPList& arglist)
{
    bool args_are_ycp = true;

    if (!argv[arg]) {
	print_usage ();
	exit (YAST_FEWARGUMENTS);
    }

    client_name = argv[arg];
    arg++;   // next argument (first client option)

    // Prepare client options
    while (arg < argc)
    {
	if (!strcmp(argv[arg], "-l") || !strcmp(argv[arg], "--logfile"))
	{
	    // Logfile already done at program start --> ignore here
	    arg++;   // skip filename
	}
	else if (!strcmp(argv[arg], "-c") || !strcmp(argv[arg], "--logconf"))
	{
	    // Logfile already done at program start --> ignore here
	    arg++;   // skip filename
	}
	else if (!strcmp(argv[arg], "-S"))
	{
	    args_are_ycp = false;
	}
	else if (!strcmp(argv[arg], "-s"))	// Parse one value (YCPList of options) from stdin
	{
	    Parser parser (0, "<stdin>");	// set parser to stdin
	    YCodePtr pc = parser.parse ();

	    YCPValue option = YCPNull ();
	    if (pc)
	    {
		option = pc->evaluate(true);	// get one value (should be a YCPList)
	    }

	    if (option.isNull())
	    {
		print_error ("Client option -s: Couldn't parse valid YCP value from stdin");
		exit (YAST_OPTIONERROR);
	    }

	    if (!option->isList())
	    {
		print_error ("Client option -s: Parsed YCP value is NOT a YCPList");
		exit (YAST_OPTIONERROR);
	    }

	    arglist = option->asList();	  // the option read _IS_ arglist
	}
	else if (!strcmp(argv[arg], "-f"))	// Parse values from file
	{
	    arg++;   // switch to filename

	    if (arg >= argc)
	    {
		print_error ("Client option -f is missing an argument");
		print_usage();
		exit(5);
	    }

	    FILE *file = fopen (argv[arg], "r");
	    if (!file)
	    {
		print_error ("Client option -f: Couldn't open %s for reading", argv[arg]);
		exit(5);
	    }

	    bool one_value_parsed = false;

	    while (!feof(file))	  // Parse all values until EOF
	    {
		Parser parser(file, argv[arg]);   // set parser to file

		YCodePtr pc = parser.parse ();

		if ( pc )
		{
		    arglist->add( pc->evaluate(true) );
		    one_value_parsed = true;
		}
	    }

	    fclose (file);

	    if (!one_value_parsed)
	    {
		print_error ("Client option -f: Couldn't parse valid YCP value from file %s",
			     argv[arg]);
		exit(5);
	    }
	}
	else if (has_parens (argv[arg]))	// client args
	{
	  if (args_are_ycp)	// bnc#382883
	  {
	    Parser parser (argv[arg]);	// set parser to option

	    YCodePtr pc = parser.parse ();

	    if (!pc )
	    {
		print_error ("Client option %s is not a valid YCP value", argv[arg]);
		exit(5);
	    }

	    arglist->add( pc->evaluate (true));   // add to arglist
	  }
	  else
	  {
	      string value(argv[arg] + 1, strlen (argv[arg]) - 2);
	      arglist->add (YCPString (value));
	  }
	}
	else break;   // must be server name

	arg++;	      // switch to next argument
    } // Parsing client options
}

void
parse_server_and_options (int argc, char ** argv, int& arg, char *& server_name, YCPList& preload)
{
    if (arg >= argc)
    {
	fprintf(stderr, "No server module given\n");
	print_usage ();
	exit (YAST_OPTIONERROR);
    }

    // now create server
    if (!argv[arg]) {
	print_usage ();
	exit (YAST_FEWARGUMENTS);
    }

    server_name = argv[arg];
    arg++;   // next argument (first server option)

    // Prepare server and general options

    while (arg < argc)
    {
	if (!strcmp(argv[arg], "-l") || !strcmp(argv[arg], "--logfile"))
	{
	    // Logfile already done at program start --> ignore here
	    arg++;   // skip filename
	}
	else if (!strcmp(argv[arg], "-p"))   // preload
	{
	    arg++;   // switch to filename

	    if (arg >= argc)
	    {
		print_error ("Server option -p is missing an argument");
		print_usage();
		exit(5);
	    }

	    FILE *file = fopen (argv[arg], "r");
	    if (!file)
	    {
		print_error ("Client option -p: Couldn't open %s for reading", argv[arg]);
		exit(5);
	    }

	    bool one_value_parsed = false;

	    while (!feof(file))	  // Parse all values until EOF
	    {
		Parser parser(file, argv[arg]);   // set parser to file

		YCodePtr pc = parser.parse ();

		if (pc)
		{
		    preload->add(pc->evaluate (true));   // add to preload list
		    one_value_parsed = true;
		}
	    }

	    fclose (file);

	    if (!one_value_parsed)
	    {
		print_error ("Server option -p: Couldn't parse a valid YCP value from file %s",
			     argv[arg]);
		exit(5);
	    }
	}
	else if (has_parens (argv[arg]))	// option is a YCP value -> parse it directly
	{
	    Parser parser (argv[arg]);	// set parser to option

	    YCodePtr pc = parser.parse ();

	    if (!pc)
	    {
		print_error ("Server option %s is not a valid YCP value", argv[arg]);
		exit(5);
	    }

	    preload->add (pc->evaluate (true));	// add to preload list
	}
	else break; // specific server options

	arg++;	    // switch to next argument
    }	// parsing server options

}

int
main (int argc, char **argv)
{
    if (!argv[0])
    {
	fprintf (stderr, "Missing argv[0]. It is a NULL pointer.");
	exit (YAST_OPTIONERROR);
    }

    progname = basename (argv[0]);	// get program name

    // Ignore SIGPIPE. No use in signals. Signals can't be assigned to
    // components
    signal(SIGPIPE, SIG_IGN);

    // Give some output for the SIGSEGV
    // and other signals too, #238172
    // Note that USR1 and USR2 are handled by the logger.
    signal (SIGHUP,  signal_handler);
    signal (SIGINT,  signal_handler);
    signal (SIGQUIT, signal_handler);
    signal (SIGILL , signal_handler);
    signal (SIGABRT, signal_handler);
    signal (SIGFPE,  signal_handler);
    signal (SIGSEGV, signal_handler);
    signal (SIGTERM, signal_handler);

    if (argc < 2) {
	fprintf (stderr, "\nToo few arguments");
	print_usage();
	exit (YAST_FEWARGUMENTS);
    }

    if (!strcmp (argv[1], "-h") || !strcmp (argv[1], "--help")) {
	print_help ();
	exit (YAST_OK);
    }

    // client _AND_ server must be given
    if (argc < 3)
    {
	fprintf (stderr, "\nPlease give client and server as arguments");
	print_usage();
	exit(5);
    }

    // Scan all options for -l/--logfile. They must be honored BEFORE
    // the logger is used the first time.
    for (int arg = 1; arg < argc; arg++)
    {
	if (!strcmp(argv[arg], "-l") || !strcmp(argv[arg], "--logfile"))
	{
	    arg++;   // switch to filename

	    if (arg >= argc)
	    {
		print_error ("Option %s is missing an argument", argv[arg-1]);
		exit(5);
	    }

	    set_log_filename( argv[arg] );   // set logfile given in command line
	}
	if (!strcmp(argv[arg], "-c") || !strcmp(argv[arg], "--logconf"))
	{
	    arg++;   // switch to filename

	    if (arg >= argc)
	    {
		print_error ("Option %s is missing an argument", argv[arg-1]);
		exit(5);
	    }

	    set_log_conf( argv[arg] );
	}
    }

    // set a defined umask
    umask (0022);

    YCPPathSearch::initialize();

    ostringstream argdump;
    for (int arg = 1; arg < argc; arg++)
    {
	argdump << " '" << argv[arg] << "'";
    }

    y2milestone ("Launched YaST2 component '%s'%s", progname, argdump.str().c_str());

    // Now evaluate command line options in sequence

    int arg = 1;

    // The first argument might be the log option or MUST be the client name
    if (!strcmp(argv[arg], "-l") || !strcmp(argv[arg], "--logfile"))
    {
	// Logfile already done at program start --> ignore here
	arg+=2;	  // skip over logfilename
    }
    if (!strcmp(argv[arg], "-c") || !strcmp(argv[arg], "--logconf"))
    {
	// Logfile already done at program start --> ignore here
	arg+=2;	  // skip over logfilename
    }
    // The first argument might be the log option or MUST be the client name
    if (!strcmp(argv[arg], "-l") || !strcmp(argv[arg], "--logfile"))
    {
	// Logfile already done at program start --> ignore here
	arg+=2;	  // skip over logfilename
    }


    // Check for namespace exceptions registration
    while (!strcmp(argv[arg], "-n"))
    {
	arg++;
	char *pos = index (argv[arg], '=');
	if (pos == NULL)
	{
	    print_error ("Option %s argument must be in format namespace=component", argv[arg-1]);
	    exit (YAST_OPTIONERROR);
	}
	*pos = 0;
	Y2ComponentBroker::registerNamespaceException (argv[arg], pos+1);
	*pos = '=';
	arg++;
    }

// FIXME the whole option parsing sucks **** !

    // list of -I / -M pathes
    //   will be pushed to YCPPathSearch later to keep correct order
    //   (the last added path to YCPPathSearch will be searched first)
    std::list<std::string> modpaths;
    std::list<std::string> incpaths;

    // include paths
    while (!strcmp(argv[arg], "-I"))
    {
	arg++;
	incpaths.push_front (string (argv[arg])); // push to front so first one is last in list
	arg++;
    }

    while (!strcmp(argv[arg], "-M"))
    {
	arg++;
	modpaths.push_front (string (argv[arg])); // push to front so first one is last in list
	arg++;
    }

    // add include and module pathes to YCPPathSearch so that the argument order is kept

    std::list<std::string>::iterator pathit;
    for (pathit = incpaths.begin(); pathit != incpaths.end(); pathit++)
    {
	YCPPathSearch::addPath (YCPPathSearch::Include, pathit->c_str());
    }
    for (pathit = modpaths.begin(); pathit != modpaths.end(); pathit++)
    {
	YCPPathSearch::addPath (YCPPathSearch::Module, pathit->c_str());
    }

    // "arg" and these two are output params
    char * client_name;
    YCPList arglist;
    parse_client_and_options (argc, argv, arg, client_name, arglist);

    // "arg" and these two are output params
    char * server_name;
    YCPList preload;		       // prepare preload files from option -p
    parse_server_and_options (argc, argv, arg, server_name, preload);

    // now create server

#if 0
    Y2ComponentBroker::registerNamespaceException ("UI", server_name);
#endif
    Y2ComponentBroker::getNamespaceComponent( "UI" );
    y2debug( "Creating server \"%s\"", server_name );
    Y2Component *server = Y2ComponentBroker::createServer( server_name );
    if (!server) {
	print_error ("No such server module %s", server_name);
	print_usage();
	exit(5);
    }

    // Put argument into a nice new array and give them to the server
    char **server_argv = new char *[argc-arg+2];
    server_argv[0] = strdup (server_name);
    for (int i = arg; i < argc; i++)
	server_argv[i-arg+1] = argv[i];
    argv[argc] = NULL;

    // set the server options directly
    server->setServerOptions(argc-arg+1, server_argv);

    // Preload server with scripts from -p and directly given YCPValues
    for (int i = 0; i < preload->size(); i++)
    {
	server->evaluate(preload->value(i));
    }

    // now create client

    Y2Component *client = Y2ComponentBroker::createClient (client_name);
    if (!client)
    {
	print_error ("No such client module %s", client_name);

	std::list<string>::const_iterator
	    i = YCPPathSearch::searchListBegin(YCPPathSearch::Client),
	    e = YCPPathSearch::searchListEnd(YCPPathSearch::Client);
	fprintf (stderr, "The search path follows. It does not include the current directory.\n");
	for (; i != e; ++i)
	    fprintf (stderr, "  %s\n", i->c_str());

	print_usage ();
	exit (YAST_OPTIONERROR);
    }
    if (dynamic_cast<Y2ErrorComponent *>(client))
    {
	print_error ("Error while creating client module %s", client_name);
	exit (YAST_OPTIONERROR);
    }


    // The environment variable YAST_IS_RUNNING is checked in rpm
    // post install scripts. Might be useful for other scripts as
    // well.
    if (strcmp (client_name, "live-installer") == 0 // bnc#389099
	|| (strcmp (client_name, "installation") == 0
	    && arglist->contains (YCPString ("initial"))))
    {
	setenv ("YAST_IS_RUNNING", "instsys", 1);
    }
    else
    {
	setenv ("YAST_IS_RUNNING", "yes", 1);
    }

    y2milestone ("YAST_IS_RUNNING is %s", getenv ("YAST_IS_RUNNING"));


    // Now start communication
    YCPValue result = client->doActualWork(arglist, server);   // give arglist collected above

    // get result
    server->result(result);

    // Cleanup
    delete server;
    delete[] server_argv;
    delete client;

    // might be useful in tracking segmentation faults
    y2milestone ("Finished YaST2 component '%s'", progname);

    if( result.isNull () )
	exit (YAST_OK);

    y2milestone( "Exiting with client return value '%s'", result->toString ().c_str ());

    if( result->isBoolean () )
    {
	exit( result->asBoolean()->value() ? YAST_OK : YAST_CLIENTRESULT );
    }
	
    if( result->isInteger () )
	exit( YAST_CLIENTRESULT + result->asInteger ()->value () );

    // if it is one of error symbols, return it as error
    if( result->isSymbol () )
    {
	string symbol = result->asSymbol()->symbol();
	for( int i = 0 ; i < MAX_YCP_ERROR_EXIT_SYMBOLS; i++ )
	    if( symbol == ycp_error_exit_symbols[i] )
		exit( YAST_CLIENTRESULT );
    }
    
    // all other values
    exit (YAST_OK);
}


static void
print_usage()
{
    fprintf (stderr,
	     "\nRun 'yast2 -h' for help on usage\n");
}


static void
print_help()
{
    fprintf (stderr, "\n"
	     "Usage: %s [LogOpts] Client [ClientOpts] Server [Generic ServerOpts] "
	     "[Specific ServerOpts]\n",
	     progname);

    fprintf (stderr,
	     "LogOptions are:\n"
	     "    -l | --logfile LogFile    : Set logfile\n"
	     "    -c | --logconf ConfFile   : Configure logging\n"
	     "    -n Namespace=Component    : Override component for namespace\n"
	     "    -I Path                   : Add include search path\n"
	     "    -M Path                   : Add module search path\n"
	     "ClientOptions are:\n"
	     "    -s                        : Get options as one YCPList from stdin\n"
	     "    -f FileName               : Get YCPValue(s) from file\n"
	     "    -S                        : Parameters are strings, not YCP to be parsed\n"
	     "    '(any YCPValue)...'       : Parameter _IS_ a YCPValue\n"
	     "                                -S '(t1)' '(\\t2)' is equivalent to '(\"t1\")' '(\"\\\\t2\")'\n"
	     "Generic ServerOptions are:\n"
	     "    -p FileName               : Evaluate YCPValue(s) from file (preload)\n"
	     "    '(any YCPValue)'          : Parameter _IS_ a YCPValue to be evaluated\n"
	     "Specific ServerOptions are any options passed on unevaluated.\n\n"
	     "Examples:\n"
	     "y2base installation qt\n"
	     "    Start binary y2base with intallation.ycp as client and qt as server\n"
	     "y2base installation '(\"test\")' qt\n"
	     "    Provide YCPValue '\"test\"' as parameter for client installation\n"
	     "y2base installation qt -geometry 800x600\n"
	     "    Provide geometry information as specific server options\n");
}


static void
print_error (const char* format, ...)
{
    char* msg;

    va_list ap;
    va_start (ap, format);
    vasprintf (&msg, format, ap);
    va_end (ap);

    fprintf (stderr, "%s\n", msg);
    y2error ("%s", msg);

    free (msg);
}


static bool
has_parens (const char* arg)
{
    return arg[0] == '(' && arg[strlen (arg) - 1] == ')';
}
