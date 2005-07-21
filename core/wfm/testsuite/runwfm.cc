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

   File:       runwfm.cc

   Author:     Klaus Kaempf (kkaempf@suse.de)
   Maintainer: Klaus Kaempf (kkaempf@suse.de)

/-*/

#include <stdio.h>

#include <y2/Y2StdioComponent.h>
#include <y2/Y2ComponentBroker.h>
#include "Y2WFMComponent.h"
#include <ycp/Parser.h>
#include <ycp/YCPCode.h>
#include <ycp/y2log.h>
#include <WFM.h>

extern int yydebug;

int
main (int argc, char *argv[])
{
    const char *fname = 0;
    FILE *infile = stdin;

    if (argc > 1)
    {
	int argp = 1;
	while (argp < argc)
	{
	    if ((argv[argp][0] == '-')
	        && (argv[argp][1] == 'l')
	        && (argp+1 < argc))
	    {
		argp++;
		set_log_filename (argv[argp]);
	    }
	    else if (fname == 0)
	    {
		fname = argv[argp];
	    }
	    else
	    {
		fprintf (stderr, "Bad argument '%s'\nUsage: runwfm [ name.ycp ]\n", argv[argp]);
	    }
	    argp++;
	}
    }

    Parser *parser;
    parser = new Parser ();

    if (!parser)
    {
	fprintf (stderr, "Failed to create Parser\n");
	return 1;
    }

    // create stdio as UI component, disable textdomain calls

    Y2Component *user_interface = new Y2StdioComponent (false, false, true);
    if (!user_interface)
    {
	fprintf (stderr, "Failed to create Y2StdioComponent\n");
	delete parser;
	return 1;
    }

    // create the component "wfm", the Workflowmanager to do the work.
    Y2WFMComponent *workflowmanager = dynamic_cast<Y2WFMComponent*>(Y2ComponentBroker::createClient("wfm"));

    if (!workflowmanager)
    {
        y2error ("Failed to create component wfm (Workflowmanager)");
	delete parser;
        return 1;
    }

    if (fname != 0)
    {
	infile = fopen (fname, "r");
	if (infile == 0)
	{
	    fprintf (stderr, "Failed to open '%s'\n", fname);
	    return 1;
	}
    }
    else
    {
	fname = "stdin";
    }

    parser->setInput (infile, fname);
    parser->setBuffered();
    
    // register builtins;
    WFM wfm;

    YCodePtr value = 0;

    for (;;)
    {
	value = parser->parse();
	if (value == 0)
	{
	    break;
	}
	
	// Prepare the arguments. It has the form [script, [clientargs...]]
	YCPList wfm_arglist;
	wfm_arglist->add (YCPCode (value));
	wfm_arglist->add (YCPString ("testing"));
	wfm_arglist->add (YCPString ("testing-fullname"));
	wfm_arglist->add (YCPList());
	
	workflowmanager->setupComponent ( "testing", "testing-fullname:"
	    , YCPCode (value));

	// Let the wfm do the work
	YCPValue res = workflowmanager->doActualWork(YCPList(), user_interface);

	printf ("(%s)\n", res.isNull()?"nil":res->toString().c_str());
    }

    delete parser;

    if (infile != stdin)
    {
	fclose (infile);
    }

    return 0;
}
