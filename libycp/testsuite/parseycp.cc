/* parseycp.cc
 *
 * YCP standalone parser
 *
 * Authors: Klaus Kaempf <kkaempf@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>

#include <YCP.h>
#include <ycp/YCode.h>
#include <ycp/Parser.h>
#include <ycp/y2log.h>
#include <ycp/pathsearch.h>

#include "config.h"

extern int yydebug;

extern int SymbolTableDebug;

static Parser parser;
int quiet = 0;
int recursive = 0;

/**
 * Parse one file
 */
int
parsefile(const char *fname)
{
    int ret = 0;
    FILE *infile = stdin;

    if (fname)
    {
	if ((infile = fopen (fname, "r")))
	    parser.setInput (infile, fname);
	else
	    parser.setInput (fname);
    }
    
    parser.setBuffered();
    YCode *c = 0;

    SymbolTableDebug = 1;

    for (;;)
    {
	c = parser.parse ();

	if (c == 0)
	{
	    if (parser.atEOF())
	    {
		if (!quiet)
		    printf("EOF\n");
		break;
	    }
	    if (ret == 0)		// first failure
	    {
		fprintf (stderr, "Fail: %s\n", fname);
		ret = 1;
	    }
	}
	else if (c->isError ())
	{
	    printf ("ERROR !\n");
	}
	else if (!quiet)
	{
	    printf ("--> %s\n", c->toString().c_str());
	}
    }

    if (infile && infile != stdin)
	fclose (infile);

    return ret;
}

/**
 * Recurse through directories
 */
int recurse(const char *path)
{
    DIR *d;
    struct dirent *dent;
    struct stat st;
    char *pp;
    char name[1024];
    int ret = 0;

    if(!(d=opendir(path)))
    {
	perror(path);
	return 1;
    }

    /* fprintf(stderr,"PATH: %s\n",path); */
    while ((dent = readdir(d)))
    {
	if (!(strcmp(dent->d_name,".")&&strcmp(dent->d_name,"..")))
	    continue;
	pp= (char*)malloc (strlen(path) + strlen(dent->d_name)+30);
	if (!pp)
	{
	    perror("malloc");
	    return 1;
	}
	strcpy (pp,path);
	if (pp[strlen(pp)-1] != '/')
	    strcat (pp, "/");
	strcat (pp,dent->d_name);
	if (lstat (pp,&st) == -1)
	{
	    perror (pp);
	    ret = 1;
	    continue;
	}
	if (S_ISLNK(st.st_mode))
	    continue;
	if (S_ISDIR(st.st_mode))
	{
	    if (recurse(pp))
		ret = 1;
	}
	if (S_ISREG(st.st_mode))
	{
	    /* fprintf(stderr,"FILE: %s%c%s\n",path,'/',dent->d_name); */
	    snprintf (name,sizeof(name),"%s/%s", path, dent->d_name);
	    if (strlen(name) > 4
		&& !strcmp (name+strlen(name)-4,".ycp"))
	    {
		if (parsefile(name))
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
void print_help(const char *name)
{
    printf ("Usage:\n");
    printf ("  %s [-h] [--help]\n", name);
    printf ("  %s [-v] [--version]\n", name);
    printf ("  %s [-q] [-R] {-I include-path} {-M module-path} <ycpvalue|filename>\n", name);
}

/**
 * Display version
 */
void print_version()
{
    printf ("parseycp (libycp %s)\n", VERSION);
    printf ("Standalone YCP parser\n");
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
            {"help", 0, 0, 'h'},
            {"version", 0, 0, 'v'},
            {"recursive", 0, 0, 'R'},
            {"quiet", 0, 0, 'q'},
            {"include-path", 1, 0, 'I'},
            {"module-path", 1, 0, 'M'},
            {"file", 1, 0, 'f'},
            {0, 0, 0, 0}
        };

        int c = getopt_long (argc,argv,"h?vVqrRI:M:",options,&option_index);
        if (c == EOF) break;

        switch (c)
	{
            case 'h':
            case '?':
                print_help(argv[0]);
                exit(0);
            case 'v':
            case 'V':
                print_version();
                exit(0);
            case 'r':
            case 'R':
                recursive = 1;
                break;
            case 'q':
                quiet = 1;
                break;
            case 'I':
		YCPPathSearch::addPath (YCPPathSearch::Include, optarg);
                break;
            case 'M':
		YCPPathSearch::addPath (YCPPathSearch::Module, optarg);
                break;
            default:
                fprintf (stderr,"Try `%s -h' for more information.\n",argv[0]);
                exit(1);
        }
    }

    if (optind == argc)
    {
	if (!recursive)
	{
	    parser.setInput(stdin);
	    return parsefile(0);
	}
        else
	    return recurse(".");
    }
	
    for (i = optind; i < argc;i++)
    {
	if (recursive)
	{
	    if (recurse(argv[i]))
		ret = 1;
	}
	else
	{
	    if (parsefile(argv[i]))
		ret = 1;
	}
    }
    return ret;
}
