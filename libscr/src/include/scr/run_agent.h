// -*- c++ -*-

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <stdio.h>
#include <unistd.h>

#include <ycp/y2log.h>
#include <ycp/Parser.h>
#include <y2/Y2StdioComponent.h>
#include <scr/SCRAgent.h>
#include <scr/SCR.h>


/**
 * Function to run an agent. Only use in testsuites for agents.
 */

template <class Agent> inline void
run_agent (int argc, char* argv[], bool load_scr)
{
    // create Agent
    SCRAgent* agent = new Agent ();
    if (!agent)
    {
	fprintf (stderr, "Failed to create Agent\n");
	exit (EXIT_FAILURE);
    }

    run_agent_instance (argc, argv, load_scr, agent);

    delete agent;
    exit (EXIT_SUCCESS);
}

/**
 * finds filename, sets logging
 */
const char*
process_options (int argc, char* argv[])
{
    const char* fname = 0;

    if (argc > 1)
    {
	int argp = 1;
	while (argp < argc) {
	    if ((argv[argp][0] == '-')
		&& (argv[argp][1] == 'l')
		&& (argp + 1 < argc)) {
		argp++;
		set_log_filename (argv[argp]);
	    } else if ((argv[argp][0] == '-')
		&& (argv[argp][1] == 'c')
		&& (argp + 1 < argc)) {
		argp++;
		set_log_conf (argv[argp]);
	    } else if (fname == 0) {
		fname = argv[argp];
	    } else {
		fprintf (stderr, "Bad argument '%s'\nUsage: %s [name.ycp]\n",
			 argv[0], argv[argp]);
	    }
	    argp++;
	}
    }

    return fname;
}

// alternate entry point, useful for testing eg. ag_ini where
// we need to use the ScriptingAgent and pass its constructor a parameter
void
run_agent_instance (int argc, char* argv[], bool load_scr, SCRAgent* agent)
{
    const char* fname = process_options (argc, argv);
    
    // fill in SCR builtins
    SCR scr;

    // create parser
    Parser* parser = new Parser ();
    if (!parser)
    {
	fprintf (stderr, "Failed to create Parser\n");
	exit (EXIT_FAILURE);
    }
    
    // do not try to load implicit imports, like Pkg
    parser->setPreloadNamespaces (false);

    // create stdio as UI component, disable textdomain calls
    Y2Component* user_interface = new Y2StdioComponent (false, true);
    if (!user_interface)
    {
	fprintf (stderr, "Failed to create Y2StdioComponent\n");
	exit (EXIT_FAILURE);
    }

    // load config file (if existing)
    if (fname && load_scr)
    {
	int len = strlen (fname);
	if (len > 5 && strcmp (&fname[len-4], ".ycp") == 0) {
	    char* cname = strdup (fname);
	    strcpy (&cname[len-4], ".scr");
	    if (access (cname, R_OK) == 0) {
		YCPValue confval = SCRAgent::readconf (cname);
		if (confval.isNull () || !confval->isTerm ()) {
		    fprintf (stderr, "Failed to read '%s'\n", cname);
		    fprintf (stderr, "Read result: %s\n", confval->toString().c_str());
		    exit (EXIT_FAILURE);
		}
		YCPTerm term = confval->asTerm();
		for (int i = 0; i < term->size (); i++)
		    agent->otherCommand (term->value (i)->asTerm ());
	    }
	}
    }

    // open ycp script
    FILE* infile = stdin;
    if (fname != 0)
    {
	infile = fopen (fname, "r");
	if (infile == 0) {
	    fprintf (stderr, "Failed to open '%s'\n", fname);
	    exit (EXIT_FAILURE);
	}
    }
    else
    {
	fname = "stdin";
    }

    // evaluate ycp script
    parser->setInput (infile, fname);
    parser->setBuffered ();
    YCode* value = 0;
    while (true)
    {
	value = parser->parse ();
	if (value == 0)
	    break;
	YCPValue result = value->evaluate ();
	printf ("(%s)\n", result->toString ().c_str ());
	fflush (0);
	delete value;
    }

    if (infile != stdin)
	fclose (infile);

    if( value != 0 ) delete value;
    delete user_interface;
    delete parser;

}
