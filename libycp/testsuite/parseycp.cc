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
#include <ycp/YCPParser.h>
#include <ycp/y2log.h>

#include "config.h"

extern int yydebug;
extern int yyeof_reached;

static YCPParser parser;
int quiet = 0;
int normalize = 0;		// parse and print just the value
int recursive = 0;

/**
 * Parse one file
 */

int
parsefile (const char *fname)
{
    int ret = -1;
    FILE *infile = stdin;

    if (fname)
    {
	if ((infile = fopen(fname,"r")) != 0)
	{
	    parser.setInput(infile,fname);
	}
	else
	{
	    parser.setInput(fname);
	}
    }
    
    parser.setBuffered();
    YCPValue v = YCPVoid ();
    yyeof_reached = 0;

    for (;;)
    {
	if (yyeof_reached)
	{
	    fprintf (stderr, "yyeof before parse\n");
	    break;
	}
	v = parser.parse();
	if (yyeof_reached)
	{
	    fprintf (stderr, "yyeof after parse\n");
	    break;
	}

	if (v.isNull())
	{
	    if (!quiet && !normalize)
	    {
		printf("NULL\n");
	    }
	    if (ret == -1)
	    {
		if (quiet)
		{
		    fprintf(stderr,"Parse error: %s\n",fname);
		}
		ret = 1;
	    }
	    break;
	}
	if (!quiet)
	{
	    if (normalize)
	    {
		printf ("%s\n", v->toString().c_str());
	    }
	    else
	    {
		if(v->isString())
		{
		    printf("--> %s\n", v->asString()->value_cstr());
		}
		else
		{
		    printf("--> %s\n", v->toString().c_str());
		}
	    }
	}
	ret = 0;
    }

    if (infile
	&& infile != stdin)
    {
	fclose (infile);
    }

    return ret;
}

/**
 * Recurse through directories
 */
int
recurse(const char *path)
{
    DIR *d;
    struct dirent *dent;
    struct stat st;
    char *pp;
    char name[1024];
    int ret = 0;

    if (!(d=opendir(path)))
    {
	perror(path);
	return 1;
    }

    /* fprintf(stderr,"PATH: %s\n",path); */
    while ((dent=readdir(d)))
    {
	if (!(strcmp(dent->d_name,".")&&strcmp(dent->d_name,"..")))
	    continue;
	pp = (char *)malloc(strlen(path)+strlen(dent->d_name)+30);
	if (!pp)
	{
	    perror("malloc");
	    return 1;
	}
	strcpy(pp,path);
	if(pp[strlen(pp)-1]!='/')
	    strcat(pp,"/");
	strcat(pp,dent->d_name);
	if (lstat(pp,&st)==-1)
	{
	    perror(pp);
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
	    snprintf(name,sizeof(name),"%s/%s", path, dent->d_name);
	    if (strlen(name) > 4
		&& !strcmp(name+strlen(name)-4, ".ycp"))
		if (parsefile(name))
		    ret = 1;
	}
	free(pp);
    }
    closedir(d);
    return ret;
}

/**
 * Display help text
 */
void
print_help(const char *name)
{
    printf ("Usage:\n");
    printf ("  %s [-h] [--help]\n", name);
    printf ("  %s [-v] [--version]\n", name);
    printf ("  %s [-q] [-n] [-R] <ycpvalue|filename>\n", name);
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
int
main(int argc, char *argv[])
{
    int i = 0, ret = 0;

    for(;;)
    {
        int option_index = 0;

        static struct option options[] = {
            {"help", 0, 0, 'h'},
            {"version", 0, 0, 'v'},
            {"recursive", 0, 0, 'R'},
            {"quiet", 0, 0, 'q'},
            {"normalize", 0, 0, 'n'},
            {"file", 1, 0, 'f'},
            {0, 0, 0, 0}
        };

        int c = getopt_long(argc,argv,"h?vVqrRn",options,&option_index);
        if(c==EOF) break;

        switch(c) {
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
            case 'n':
                normalize = 1;
                break;
            default:
                fprintf(stderr,"Try `%s -h' for more information.\n",argv[0]);
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
	
    for (i=optind; i<argc;i++)
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
