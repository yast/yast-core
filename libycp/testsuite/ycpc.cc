/* ycpc.cc
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
#include <ycp/Bytecode.h>
#include <ycp/y2log.h>
#include <ycp/pathsearch.h>

#include "config.h"

ExecutionEnvironment ee;

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
processfile (const char *infname, const char *outfname)
{
    FILE *infile = stdin;

    std::ifstream instream;
    std::ofstream outstream;

    if (infname == 0)
    {
	fprintf (stderr, "No input file given\n");
	return 1;
    }

    if (parse || compile)
    {
	if (!quiet) printf ("parsing '%s'\n", infname);
	infile = fopen (infname, "r");
	if (infile == 0)
	{
	    fprintf (stderr, "Can't read '%s'\n", infname);
	    return 1;
	}
    }

    if (compile
	&& outfname == 0)
    {
	int len = strlen (infname);
	if (len > 4
	    && strcmp (infname + len - 4, ".ycp") == 0)
	{
	    char *ofname = strdup (infname);
	    ofname[len-2] = 'b';	// .ycp -> ybc
	    ofname[len-1] = 'c';
	    if (!quiet) printf ("compiling to '%s'\n", ofname);
	    outstream.open (ofname, std::ios::out);
	}
    }

    if (run || print)	// neither parse, nor compile -> run
    {
	instream.open (infname, std::ios::in);
	if (!instream.is_open ())
	{
	    fprintf (stderr, "Failed to read ''%s''\n", infname);
	    return 1;
	}

	if (run)
	{
	    if (!quiet) printf ("running '%s'\n", infname);
	}
    }


    if (outfname != 0
	&& *outfname != '-'
	&& !outstream.is_open ())
    {
	outstream.open (outfname, std::ios::out);
    }

    if (!outstream.is_open ())
    {
	if (compile
	    || (outfname != 0)
		&& (*outfname != '-'))
	{
	    fprintf (stderr, "Can't write ''%s''\n", (outfname==0)?"<unknown>":outfname);
	    return 1;
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
	parser->setInput (infile, infname);
	parser->setBuffered();
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
	c = Bytecode::readCode (instream);
    }
    if (!quiet) printf ("done\n");

    if (c == 0)
    {
	fprintf (stderr, "Fail: '%s'\n", infname);
	return 1;
    }
    else if (c->isError ())
    {
	printf ("ERROR !\n");
	return 1;
    }
    else if (parse || print)
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
	c->toStream (outstream);
    }
    else	// run
    {
	if (!quiet) printf ("running ...\n");
	YCPValue value = c->evaluate ();

	if (!value.isNull() && (value->isError()))
	{
	    // triggers error log
	    value = value->asError()->evaluate ();
	}
	string result = value.isNull() ? "(NULL)" : value->toString();
	if (outstream.is_open())
	{
	    outstream << result << std::endl;
	}
	else
	{
	    std::cout << result << std::endl;
	}
    }

    if (infile
	&& infile != stdin)
    {
	fclose (infile);
    }

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
    int ret = 0;

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
	    strcat (pp, "/");
	strcat (pp, dent->d_name);
	if (lstat (pp, &st) == -1)
	{
	    perror (pp);
	    ret = 1;
	    continue;
	}
	if (S_ISLNK (st.st_mode))
	    continue;
	if (S_ISDIR (st.st_mode))
	{
	    if (recurse (pp))
		ret = 1;
	}
	if (S_ISREG (st.st_mode))
	{
	    /* fprintf(stderr,"FILE: %s%c%s\n",path,'/',dent->d_name); */
	    snprintf (name, sizeof (name), "%s/%s", path, dent->d_name);

	    if (strlen (name) > 4
		&& !strcmp (name+strlen(name)-4, ".ycp"))
	    {
		if (processfile (name, NULL))
		    ret = 1;
	    }
	}
	free (pp);
    }
    closedir (d);
    return ret;
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
    printf ("ycpc (libycp %s)\n", VERSION);
    printf ("Standalone YCP bytecode compiler\n");
}

/**
 * main() function
 */
int main(int argc, char *argv[])
{
    int i = 0, ret = 0;

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
		print_help ("ycpc");
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
		y2setLogfileName (optarg);
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
	    if (recurse (argv[i]))
		ret = 1;
	}
	else
	{
	    if (processfile (argv[i], outname))
		ret = 1;
	}
    }
    return ret;
}
