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

   File:       runc.cc

   Author:     Klaus Kaempf (kkaempf@suse.de)
   Maintainer: Klaus Kaempf (kkaempf@suse.de)

/-*/

#include <stdio.h>
#include <ycp/YCode.h>
#include <ycp/YCPError.h>
#include <ycp/y2log.h>
#include <ycp/pathsearch.h>
#include <ycp/Bytecode.h>

#include <fstream>

ExecutionEnvironment ee;

extern int yydebug;
extern int SymbolTableDebug;

int
main (int argc, const char *argv[])
{
    const char *fname = 0;
    std::ifstream infile;

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
		fprintf (stderr, "Bad argument '%s'\nUsage: runc [-l log] {-I include-path} {-M module-path} [name.ybc]\n", argv[argp]);
		return 1;
	    }
	    argp++;
	}
    }

    if (fname == 0)
    {
	fprintf (stderr, "Input file missing\n");
	return 1;
    }

    char *dotp = strrchr (fname, '.');
    if ((dotp == 0)
	|| (strcmp (dotp, ".ybc") != 0))
    {
	dotp = (char *)malloc (strlen (fname) + 5);
	strcpy (dotp, fname);
	strcat (dotp, ".ybc");
	fname = dotp;
    }

    infile.open (fname, std::ios::in);
    if (!infile.is_open ())
    {
	fprintf (stderr, "Failed to open '%s'\n", fname);
	return 1;
    }

    YCode *code = Bytecode::readCode (infile);

    if (code == 0)
    {
	fprintf (stderr, "Error reading bytecode\n");
	return 1;
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
    printf ("(%s)\n", value.isNull()?"(NULL)":value->toString().c_str());

    return 0;
}
