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

   File:       runpkg.cc

   Author:     Stefan Schubert ( schubi@suse.de )
   Maintainer: Stefan Schubert ( schubi@suse.de )

/-*/

#include <stdio.h>
#include <PKG.h>
#include <ycp/y2log.h>

extern int yydebug;

int
main (int argc, const char *argv[])
{
#if 0    
    const char *fname = 0;
    FILE *infile = stdin;

    if (argc > 1) {
	int argp = 1;
	while (argp < argc) {
	    if ((argv[argp][0] == '-')
	        && (argv[argp][1] == 'l')
	        && (argp+1 < argc)) {
		argp++;
		y2setLogfileName (argv[argp]);
	    }
	    else if (fname == 0) {
		fname = argv[argp];
	    } else {
		fprintf (stderr, "Bad argument '%s'\nUsage: runscr [ name.ycp ]\n", argv[argp]);
	    }
	    argp++;
	}
    }

    if (infile != stdin)
	fclose (infile);
#endif

    return 0;
}
