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

#include "WFMInterpreter.h"

#include <y2/Y2StdioComponent.h>
#include <ycp/YCPParser.h>
#include <ycp/y2log.h>

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

    YCPParser *parser;
    parser = new YCPParser ();

    if (!parser)
    {
	fprintf (stderr, "Failed to create YCPParser\n");
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

    WFMInterpreter *interpreter = new WFMInterpreter (user_interface, user_interface,
						      "runwfm", "unknown", YCPList ());
    if (!interpreter)
    {
	fprintf (stderr, "Failed to create WFMInterpreter\n");
	delete user_interface;
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

    YCPValue value = YCPVoid();

    for (;;)
    {
	value = parser->parse();
	if (value.isNull())
	{
	    break;
	}
	value = interpreter->evaluate(value);

	if (!value.isNull() && (value->isError()))
	{
	    // triggers error log in interpreter
	    value = interpreter->evaluate(value);
	}
	printf ("(%s)\n", value.isNull()?"(NULL)":value->toString().c_str());
    }

    delete interpreter;
    delete parser;

    if (infile != stdin)
    {
	fclose (infile);
    }

    return 0;
}
