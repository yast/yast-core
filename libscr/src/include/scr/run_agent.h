// -*- c++ -*-

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <stdio.h>
#include <unistd.h>

#include <ycp/y2log.h>
#include <ycp/YCPParser.h>
#include <y2/Y2StdioComponent.h>
#include <scr/SCRInterpreter.h>
#include <scr/SCRAgent.h>


/**
 * Function to run an agent. Only use in testsuites for agents.
 */

template <class Agent> inline void
run_agent (int argc, char* argv[], bool load_scr)
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
	    } else if (fname == 0) {
		fname = argv[argp];
	    } else {
		fprintf (stderr, "Bad argument '%s'\nUsage: %s [name.ycp]\n",
			 argv[0], argv[argp]);
	    }
	    argp++;
	}
    }

    // create parser
    YCPParser* parser = new YCPParser ();
    if (!parser)
    {
	fprintf (stderr, "Failed to create YCPParser\n");
	exit (EXIT_FAILURE);
    }

    // create stdio as UI component, disable textdomain calls
    Y2Component* user_interface = new Y2StdioComponent (false, true);
    if (!user_interface)
    {
	fprintf (stderr, "Failed to create Y2StdioComponent\n");
	exit (EXIT_FAILURE);
    }

    // create Agent
    SCRAgent* agent = new Agent ();
    if (!agent)
    {
	fprintf (stderr, "Failed to create Agent\n");
	exit (EXIT_FAILURE);
    }

    // create interpreter
    SCRInterpreter* interpreter = new SCRInterpreter (agent);
    if (!interpreter)
    {
	fprintf (stderr, "Failed to create SCRInterpreter\n");
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
		    exit (EXIT_FAILURE);
		}
		YCPTerm term = confval->asTerm();
		for (int i = 0; i < term->size (); i++)
		    interpreter->evaluate (term->value (i));
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
    YCPValue value = YCPVoid ();
    while (true)
    {
	value = parser->parse ();
	if (value.isNull ())
	    break;
	value = interpreter->evaluate (value);
	printf ("(%s)\n", value->toString ().c_str ());
	fflush (0);
    }

    if (infile != stdin)
	fclose (infile);

    delete interpreter;
    delete agent;
    delete user_interface;
    delete parser;

    exit (EXIT_SUCCESS);
}
