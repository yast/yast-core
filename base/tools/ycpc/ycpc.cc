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

#include <UI.h>
#include <scr/SCR.h>
#include <WFM.h>

#include "config.h"

extern int yydebug;

extern int SymbolTableDebug;

static Parser *parser = NULL;

static char *outname = NULL;

static int quiet = 0;		// no output
static int recursive = 0;	// recursively all files
static int parse = 0;		// just parse source code
static int compile = 0;		// just compile source to bytecode
static int no_implicit_namespaces = 0;	// don't preload implicit namespaces

#define progress(text,param)	{ if (!quiet) printf ((text),(param)); }

/**
 * parse file and return corresponding YCode or NULL for error
 */
YCode*
parsefile (const char *infname)
{
    if (infname == NULL)
    {
	fprintf (stderr, "No input file given\n");
	return NULL;
    }
    
    if (parser == NULL)
    {
	parser = new Parser();

	if (parser == NULL)
	{
	    fprintf (stderr, "Can't create parser\n");
	    return NULL;
	}
    }
    
    FILE *infile = fopen (infname, "r");
    if (infile == NULL)
    {
	fprintf (stderr, "Can't read '%s'\n", infname);
	return NULL;
    }
    
    YSImport::flushActiveModules ();

    progress ("parsing '%s'\n", infname);
    
    parser->setInput (infile, infname);
    parser->setBuffered();
    parser->setPreloadNamespaces (no_implicit_namespaces == 0);
    SymbolTableDebug = 1;

    YCode *c = NULL;

    c = parser->parse ();

    fclose (infile);
    
    if (c == NULL || c->isError ())
    {
	progress ("Error\n", 0);
	return NULL;
    }

    progress ("done\n", 0);

    return c;
}

/**
 * Compile one file
 */

int
compilefile (const char *infname, const char *outfname)
{
    string ofname = infname;
    
    if (outfname != NULL) 
    {
	ofname = outfname;
	progress ("compiling to '%s'\n", ofname.c_str ());
    } 
    else 
    {
	int len = ofname.size ();
	if (len > 4 && ofname.substr (len-4, 4) == ".ycp")
	{
	    ofname = ofname.replace (len-4, 4, ".ybc");
	}
	else
	{
	    ofname += ".ybc";
	}
	progress ("compiling to '%s'\n", ofname.c_str ());
    }
    
    YCode *c = parsefile (infname);
    
    if (c != NULL ) {
	progress ("saving ...\n", 0);
	return Bytecode::writeFile (c, ofname);
    }
    
    return 1;
}

 
/**
 * Process one file
 */

int
processfile (const char *infname, const char *outfname)
{
    if (compile) {
	return compilefile (infname, outfname);
    }
    
    if (parse) {
	YCode *c = parsefile (infname);
	
	if (c == NULL) return 1;
	
	if (quiet) return 0;

	std::ofstream outstream;

	// print out the result
	if (outfname != NULL
	    && *outfname != '-')
	{
	    outstream.open (outfname, std::ios::out);
	}

	if (!outstream.is_open ())
	{
	    if ((outfname != NULL) && (*outfname != '-'))
	    {
		fprintf (stderr, "Can't write ''%s''\n", (outfname==0)?"<unknown>":outfname);
		return 1;
	    }
	}

	progress ("Parsed:\n", 0);
	if (outstream.is_open())
	{
	    outstream << c->toString();
	}
	else
	{
	    std::cout << c->toString();
	}

	return 0;
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
    printf ("  %s [-q] [-R] {-d} {-I include-path} {-M module-path} {-l logfile} {-c|-E|-p} {-o output} <filename>\n", name);
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
	    {"logfile", 1, 0, 'l'},			// specify log file
	    {"no-implicit-imports", 0, 0, 'd'},		// don't preload implicit namespaces
	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "h?vVpqrRdEcI:M:o:l:", options, &option_index);
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
		if (outname != NULL)
		{
		    fprintf (stderr, "-%c mutually exclusive with -c\n", c);
		    exit (1);
		}
		recursive = 1;
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
		break;
	    case 'o':
		if (recursive)
		{
		    fprintf (stderr, "-o mutually exclusive with -r\n");
		    exit (1);
		}
		outname = strdup (optarg);
		break;
	    case 'd':
		no_implicit_namespaces = 1;
		break;
	    default:
		fprintf (stderr,"Try `%s -h' for more information.\n",argv[0]);
		exit(1);
	}
    }

    // register builtins
    SCR scr;
    WFM wfm;
    UI ui;

    if (compile == parse)		// both are zero
    {
	// FIXME: what about -p only
	fprintf (stderr, "-E or -c must be given\n");
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
