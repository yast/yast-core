/* runc.cc
 *
 * YCP standalone bytecode compiler
 *
 * Authors: Klaus Kaempf <kkaempf@suse.de>
 *
 * $Id$
 */

#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>

#include <fstream>

#include <YCP.h>
#include <ycp/YCode.h>
#include <ycp/Parser.h>
#include <ycp/YBlock.h>
#include <ycp/Bytecode.h>
#include <ycp/y2log.h>
#include <ycp/pathsearch.h>

#include <y2/Y2Component.h>
#include <y2/Y2ComponentCreator.h>

#include "config.h"

class TestY2Component : public Y2Component {
    virtual Y2Namespace *import (const char* name, const char* timestamp = NULL)
    {
	Y2Namespace* block = Bytecode::readModule (name, timestamp != NULL ? timestamp : "");
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

static Parser *parser = 0;

static char *outname = 0;

static int quiet = 0;		// no output
static int recursive = 0;	// recursively all files
static int parse = 0;		// just parse source code
static int print = 0;		// just read and print bytecode
static int compile = 0;		// just compile source to bytecode
static int run = 0;		// just read and print bytecode

/**
 * Process one file
 */

int
processfile (const char *infname, char *outfname)
{
    if (infname == 0)
    {
	fprintf (stderr, "No input file given\n");
	return 1;
    }

    if (parse || compile)
    {
	if (!quiet) printf ("parsing '%s'\n", infname);
    }

    if (compile
	&& outfname == 0)
    {
	int len = strlen (infname);
	if (len > 4
	    && strcmp (infname + len - 4, ".ycp") == 0)
	{
	    outfname = strdup (infname);
	    outfname[len-2] = 'b';	// .ycp -> ybc
	    outfname[len-1] = 'c';
	    if (!quiet) printf ("compiling to '%s'\n", outfname);
	}
    }

    if (run || print)	// neither parse, nor compile -> run
    {
	if (run)
	{
	    if (!quiet) printf ("running '%s'\n", infname);
	}
    }


    if (parse || compile)
    {
	if (parser == 0)
	{
	    parser = new Parser();
	}
	if (parser == 0)
	{
	    fprintf (stderr, "Can't create parser\n");
	    return 1;
	}
    }
    
    if (parser != 0)
    {
	FILE *infile = fopen (infname, "r");
	if (infile == 0)
	{
	    fprintf (stderr, "Can't  open \"%s\"\n", infname);
	    return 1;
	}
	parser->setInput (infile, infname);
	parser->setBuffered();
	parser->setPreloadNamespaces (false);
	SymbolTableDebug = 1;
    }

    YCode *c = 0;

    if (parse || compile)
    {
	if (!quiet) printf ("parsing ...\n");
	c = parser->parse ();
    }
    else
    {
	if (!quiet) printf ("loading ...\n");
	c = Bytecode::readFile (infname);
    }
    if (!quiet) printf ("done\n");

    if (c == 0)
    {
	fprintf (stderr, "Fail: '%s'\n", infname);
	return 1;
    }

    if (c->isError ())
    {
	printf ("ERROR !\n");
	c->evaluate();
	return 1;
    }

    std::ofstream outstream;

    if (outfname != 0
	&& *outfname != '-'
	&& !outstream.is_open ())
    {
	outstream.open (outfname, std::ios::out);
    }

    if (parse || print)
    {
	if (!quiet) printf ("Parsed:\n");
	if (outstream.is_open())
	{
	    outstream << c->toString();
	}
	else
	{
	    std::cout << c->toString();
	}
    }
    else if (compile)
    {
	if (!quiet) printf ("saving ...\n");
	Bytecode::writeFile (c, string (outfname));
    }
    else	// run
    {
	if (!quiet) printf ("running ...\n");
	YCPValue value = c->evaluate ();

	string result = value.isNull() ? "nil" : value->toString();
	if (outstream.is_open())
	{
	    outstream << result << std::endl;
	}
	else
	{
	    std::cout << result << std::endl;
	}
    }
    
    if (!quiet) printf ("done\n");

    return 0;
}


/**
 * Recurse through directories
 */

int
recurse (const char *path)
{
    DIR *d;
    struct dirent *dent;
    struct stat st;
    char *pp;
    char name[1024];

    if(!(d = opendir (path)))
    {
	perror (path);
	return 1;
    }

    /* fprintf (stderr,"PATH: '%s'\n",path); */
    while ((dent = readdir(d)))
    {
	if (!(strcmp (dent->d_name, ".")
	    && strcmp (dent->d_name, "..")))
	{
	    continue;
	}
	pp = (char*)malloc (strlen (path) + strlen (dent->d_name) + 30);
	if (!pp)
	{
	    perror ("malloc");
	    return 1;
	}
	strcpy (pp,path);
	if (pp[strlen(pp)-1] != '/')
	{
	    strcat (pp, "/");
	}
	strcat (pp, dent->d_name);
	if (lstat (pp, &st) == -1)
	{
	    perror (pp);
	    continue;
	}
	if (S_ISLNK (st.st_mode))
	{
	    continue;
	}
	if (S_ISDIR (st.st_mode))
	{
	    recurse (pp);
	}
	if (S_ISREG (st.st_mode))
	{
	    /* fprintf(stderr,"FILE: %s%c%s\n",path,'/',dent->d_name); */
	    snprintf (name, sizeof (name), "%s/%s", path, dent->d_name);

	    if (strlen (name) > 4
		&& !strcmp (name+strlen(name)-4, ".ycp"))
	    {
		if (processfile (name, NULL))
		{
		    break;
		}
	    }
	}
	free (pp);
    }
    closedir (d);
    return 0;
}

/**
 * Display help text
 */
void print_help (const char *name)
{
    printf ("Usage:\n");
    printf ("  %s [-h] [--help]\n", name);
    printf ("  %s [-v] [--version]\n", name);
    printf ("  %s [-q] [-R] {-I include-path} {-M module-path} {-l logfile} {-c|-E|-p} {-o output} <filename>\n", name);
}

/**
 * Display version
 */
void print_version()
{
    printf ("runc (libycp %s)\n", VERSION);
    printf ("Standalone YCP bytecode compiler\n");
}

/**
 * main() function
 */
int main (int argc, char *argv[])
{
    int i = 0;

    YCPPathSearch::initialize ();

    for(;;)
    {
	int option_index = 0;

	static struct option options[] =
	{
	    {"help", 0, 0, 'h'},			// show help and exit
	    {"version", 0, 0, 'v'},			// show version and exit
	    {"recursive", 0, 0, 'R'},			// recursively
	    {"quiet", 0, 0, 'q'},			// no output
	    {"include-path", 1, 0, 'I'},		// where to find include files
	    {"module-path", 1, 0, 'M'},			// where to find module files
	    {"fsyntax-only", 0, 0, 'E'},		// parse only
	    {"output", 1, 0, 'o'},			// output file
	    {"compile", 0, 0, 'c'},			// compile to bytecode
	    {"print", 0, 0, 'p'},			// print bytecode
	    {"logfile", 1, 0, 'l'},			// print bytecode
	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "h?vVpqrREcI:M:o:l:", options, &option_index);
	if (c == EOF) break;

	switch (c)
	{
	    case 'h':
	    case '?':
		print_help ("runc");
		exit (0);
	    case 'v':
	    case 'V':
		print_version ();
		exit (0);
	    case 'r':
	    case 'R':
		if (outname != 0)
		{
		    fprintf (stderr, "-%c mutually exclusive with -c\n", c);
		    exit (1);
		}
		recursive = 1;
		break;
	    case 'p':
		print = 1;
		break;
	    case 'q':
		quiet = 1;
		break;
	    case 'E':
		parse = 1;
		if (compile != 0)
		{
		    fprintf (stderr, "-E mutually exclusive with -c\n");
		    exit (1);
		}
		break;
	    case 'c':
		compile = 1;
		if (parse != 0)
		{
		    fprintf (stderr, "-c mutually exclusive with -E\n");
		    exit (1);
		}
		break;
	    case 'I':
		YCPPathSearch::addPath (YCPPathSearch::Include, optarg);
		break;
	    case 'M':
		YCPPathSearch::addPath (YCPPathSearch::Module, optarg);
		break;
	    case 'l':
		set_log_filename (optarg);
		fclose (stderr);
		stderr = fopen (optarg, "a+");
		break;
	    case 'o':
		if (recursive)
		{
		    fprintf (stderr, "-o mutually exclusive with -r\n");
		    exit (1);
		}
		outname = strdup (optarg);
		break;
	    default:
		fprintf (stderr,"Try `%s -h' for more information.\n",argv[0]);
		exit(1);
	}
    }

    if (compile == parse)		// both are zero -> run
    {
	run = 1;
    }

    if (optind == argc)
    {
	fprintf (stderr, "No input file or directory given\n");
	exit (1);
    }
	
    for (i = optind; i < argc;i++)
    {
	if (recursive)
	{
	    recurse (argv[i]);
	}
	else
	{
	    processfile (argv[i], outname);
	}
    }

    // this is for testsuite purpose only, so do not return a nonzero
    // exit status. dejagnu will handle it by checking the error output

    return 0;
}
