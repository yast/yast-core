/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       runycp.cc

   Author:     Klaus Kaempf (kkaempf@suse.de)
   Maintainer: Klaus Kaempf (kkaempf@suse.de)

/-*/

#include <stdio.h>
#include <locale.h>

#include <ycp/YCPParser.h>
#include <ycp/YCPInterpreter.h>
#include <ycp/y2log.h>

extern int yydebug;

int
main (int argc, const char *argv[])
{
    setlocale (LC_ALL, "");

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
		y2setLogfileName (argv[argp]);
	    }
	    else if (fname == 0)
	    {
		fname = argv[argp];
	    }
	    else
	    {
		fprintf (stderr, "Bad argument '%s'\nUsage: runycp [ name.ycp ]\n", argv[argp]);
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

    YCPInterpreter *interpreter;
    interpreter = new YCPInterpreter();

    if (!interpreter)
    {
	fprintf (stderr, "Failed to create YCPInterpreter\n");
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

    interpreter->current_file = string (fname);

    YCPValue value = YCPVoid();

    for (;;)
    {
	value = parser->parse();
	if (value.isNull())
	{
	    break;
	}
	fprintf (stderr,
	    "Parsed:\n"
	    "----------------------------------------------------------------------\n"
	    "%s\n"
	    "----------------------------------------------------------------------\n",
	    value->toString().c_str());

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
