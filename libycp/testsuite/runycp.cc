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
#include <ycp/Parser.h>
#include <ycp/y2log.h>
#include <ycp/pathsearch.h>
#include <ycp/YBlock.h>
#include <ycp/Bytecode.h>
#include <ycp/ExecutionEnvironment.h>

#include <y2/Y2Component.h>
#include <y2/Y2ComponentCreator.h>

extern ExecutionEnvironment ee;

class TestY2Component : public Y2Component {
    virtual Y2Namespace *import (const char* name, const char* timestamp = NULL)
    {
        Y2Namespace *block = Bytecode::readModule (name, timestamp != NULL ? timestamp : "");
        if (block == 0 )
        {
            y2debug ("Cannot import module");
            return NULL;
        }

        return block;
    }
    virtual string name () const { return "test";}

} TestComponent;


class TestY2CC : public Y2ComponentCreator
{
public:
    TestY2CC() : Y2ComponentCreator(Y2ComponentBroker::SCRIPT) {}
    virtual  Y2Component *provideNamespace(const char *name)
    {
        y2debug ("Test can handle this" );
        return &TestComponent;
    }
    virtual bool isServerCreator () const { return true;};
} cc;

extern int yydebug;
extern int SymbolTableDebug;

int
main (int argc, const char *argv[])
{
    const char *fname = 0;
    FILE *infile = stdin;
    bool make_depends = false;

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
		set_log_filename (argv[argp]);
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
	    else if ((argv[argp][0] == '-')
	        && (argv[argp][1] == '-')
	        && (strcmp (argv[argp] + 2, "depends") == 0))
	    {
		make_depends = true;	
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
    if (make_depends)
	parser->setDepends();
	
    parser->setPreloadNamespaces (false);

    ee.setFilename (string (fname));

    YCode *code = 0;

    SymbolTableDebug = 0;

    for (;;)
    {
	y2debug ("\n------------------------------------------- parsing");
	code = parser->parse ();
	if (parser->atEOF())
	{
	    break;
	}

	if (code == 0)
	{
	    fprintf (stderr, "runycp: parser error\n");
	    break;
	}

	fprintf (stderr, 
	    "Parsed:\n"
	    "----------------------------------------------------------------------\n"
	    "%s\n"
	    "----------------------------------------------------------------------\n",
	    code->toString().c_str());

	y2debug ("\n------------------------------------------- running");
	YCPValue value = code->evaluate ();

	y2debug ("\n------------------------------------------- done");
	printf ("(%s)\n", value.isNull() ? "nil" : value->toString().c_str());

    }

    delete parser;

    if (infile != stdin)
    {
	fclose (infile);
    }
    return 0;
}
