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
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
/*
 * main function common to all Y2 components
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE		// needed for vasprintf below
#endif

#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <sstream>

#include <ycp/y2log.h>
#include <ycp/ExecutionEnvironment.h>
#include "Y2Component.h"
#include "Y2ErrorComponent.h"
#include "Y2ComponentBroker.h"
#include <YCP.h>
#include <ycp/Parser.h>
#include <ycp/pathsearch.h>

ExecutionEnvironment ee;

static const int YCP_ERROR = 16;

static const char *progname = "genericfrontend";

static void print_usage ();
static void print_help ();
static void print_error (const char*, ...) __attribute__ ((format (printf, 1, 2)));
static bool is_ycp_value (const char* arg);

void
signal_handler (int sig)
{
    signal (sig, SIG_IGN);
    fprintf (stderr, "YaST got signal %d at YCP file %s:%d\n",
	     sig, ee.filename ().c_str (), ee.linenumber ());
    y2error ("got signal %d at YCP file %s:%d",
	     sig, ee.filename ().c_str (), ee.linenumber ());
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
    if (!argv[arg]) {
	print_usage ();
	exit (1);
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
		exit (5);
	    }

	    if (!option->isList())
	    {
		print_error ("Client option -s: Parsed YCP value is NOT a YCPList");
		exit (5);
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
	else if (is_ycp_value (argv[arg]))	// option is a YCP value -> parse it directly
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
	exit (5);
    }

    // now create server
    if (!argv[arg]) {
	print_usage ();
	exit (1);
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
	else if (is_ycp_value (argv[arg]))	// option is a YCP value -> parse it directly
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
	exit (5);
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
	exit (1);
    }

    if (!strcmp (argv[1], "-h") || !strcmp (argv[1], "--help")) {
	print_help ();
	exit (0);
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
	    exit (5);
	}
	*pos = 0;
	Y2ComponentBroker::registerNamespaceException (argv[arg], pos+1);
	*pos = '=';
	arg++;
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

    Y2ComponentBroker::registerNamespaceException ("UI", server_name);
    Y2Component *server = Y2ComponentBroker::getNamespaceComponent ("UI");
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
	print_usage ();
	exit (5);
    }
    if (dynamic_cast<Y2ErrorComponent *>(client))
    {
	print_error ("Error while creating client module %s", client_name);
	exit (5);
    }


    // The environment variable YAST_IS_RUNNING is checked in rpm
    // post install scripts. Might be useful for other scripts as
    // well.
    if ((strcmp (client_name, "installation") == 0 ||
	 strcmp (client_name, "installation.ycp") == 0) &&
	arglist->contains (YCPString ("initial")))
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

    if( !result.isNull () && result->isBoolean() )
	exit( result->asBoolean()->value() ? 0 : YCP_ERROR );

    exit (EXIT_SUCCESS);
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
	     "ClientOptions are:\n"
	     "    -s                        : Get options as one YCPList from stdin\n"
	     "    -f FileName               : Get YCPValue(s) from file\n"
	     "    '(any YCPValue)'          : Parameter _IS_ a YCPValue\n"
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
is_ycp_value (const char* arg)
{
    return arg[0] == '(' && arg[strlen (arg) - 1] == ')';
}
