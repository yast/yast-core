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
#include <ycp/YCode.h>
#include <ycp/YCPError.h>
#include <ycp/Parser.h>
#include <ycp/y2log.h>
#include <ycp/pathsearch.h>

ExecutionEnvironment ee;

extern int yydebug;
extern int SymbolTableDebug;

int
main (int argc, const char *argv[])
{
    const char *fname = 0;
    FILE *infile = stdin;

    YCPPathSearch::initialize ();

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
	    // TODO - this is getting messy, use getopt_long
	    else if ((argv[argp][0] == '-')
	        && (argv[argp][1] == 'I'))
	    {
		const char *path = argv[argp] + 2;
		if ((*path == 0)
		    && (argp+1 < argc))
		{
		    argp++;
		    path = argv[argp];
		}
		if (*path == 0)
		{
		    fprintf (stderr, "missing argument to '-I'\n");
		}
		else
		{
		    YCPPathSearch::addPath (YCPPathSearch::Include, path);
		}
	    }
	    else if ((argv[argp][0] == '-')
	        && (argv[argp][1] == 'M'))
	    {
		const char *path = argv[argp] + 2;
		if ((*path == 0)
		    && (argp+1 < argc))
		{
		    argp++;
		    path = argv[argp];
		}
		if (*path == 0)
		{
		    fprintf (stderr, "missing argument to '-M'\n");
		}
		else
		{
		    YCPPathSearch::addPath (YCPPathSearch::Module, path);
		}
	    }
	    else if ((argv[argp][0] != '-')
		     && fname == 0)
	    {
		fname = argv[argp];
	    }
	    else
	    {
		fprintf (stderr, "Bad argument '%s'\nUsage: runycp [-l log] {-I include-path} {-M module-path} [name.ycp]\n", argv[argp]);
		return 1;
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

    ee.setFilename (string (fname));

    YCode *code = 0;

    SymbolTableDebug = 0;

    for (;;)
    {
	code = parser->parse();
	if (parser->atEOF())
	{
	    break;
	}

	if (code == 0)
	{
	    fprintf (stderr, "runycp: parser error\n");
	    continue;
	}

	fprintf (stderr, 
	    "Parsed:\n"
	    "----------------------------------------------------------------------\n"
	    "%s\n"
	    "----------------------------------------------------------------------\n",
	    code->toString().c_str());

	YCPValue value = code->evaluate ();

	if (!value.isNull() && (value->isError()))
	{
	    // triggers error log
	    value = value->asError()->evaluate ();
	}
	printf ("(%s)\n", value.isNull() ? "(NULL)" : value->toString().c_str());

    }

    delete parser;

    if (infile != stdin)
    {
	fclose (infile);
    }
    return 0;
}
