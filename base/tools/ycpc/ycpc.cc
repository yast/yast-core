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
#include <utime.h>
#include <sys/stat.h>

#include <fstream>
#include <list>
#include <map>

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

// name of timestamp file
#define STAMPNAME "/.stamp"
#define DEPENDNAME "/.depend"

extern int yydebug;

extern int SymbolTableDebug;

static Parser *parser = NULL;

static char *outname = NULL;

static int quiet = 0;		// no output
static int recursive = 0;	// recursively all files
static int parse = 0;		// just parse source code
static int compile = 0;		// just compile source to bytecode
static int force = 0;		// force recompilation
static int no_implicit_namespaces = 0;	// don't preload implicit namespaces

#define progress(text,param)	{ if (!quiet) fprintf (stderr,(text),(param)); }

//---------------------------------------------------------------------------..

#define MAXPATHLEN 2048

typedef struct recurse_struct {
    struct recurse_struct *parent;
    DIR *d;			// opendir/readdir handle
    char *path;			// current directory path (points to parent->path if parent != 0)
    int length;			// if parent == 0: length of starting directory in path[], path[0...startlen-1] == path from recurseStart()
				// else: length of current subdir in path[], path[startlen...currentlen-1] == current recursion point
} recurseT;

recurseT *recurseStart (const char *path);
recurseT *recurseNext (recurseT *handle, struct stat *st = 0);
void recurseEnd (recurseT *handle);


/**
 * recurse through directory
 *
 * start recursion at path, return recurseT handle
 *
 */

recurseT *
recurseStart (const char *path)
{
    y2debug ("recurseStart(%s)", path);
    int pathlen = strlen (path);
    if (pathlen > MAXPATHLEN-2) {	// save 2 chars for trailing slash and 0
	fprintf (stderr, "Path too long (%d chars, max %d chars allowed\n", pathlen, MAXPATHLEN-2);
	return 0;
    }

    recurseT *handle = (recurseT *)malloc (sizeof (recurseT));
    if (handle == 0) {
	perror ("malloc");
	return 0;
    }
    handle->path = (char *)malloc (MAXPATHLEN);
    if (handle->path == 0) {
	perror ("malloc");
	free (handle);
	return 0;
    }
    handle->parent = 0;

    strcpy (handle->path, path);

    if(!(handle->d = opendir (path)))
    {
	perror (path);
	free (handle->path);
	free (handle);
	return 0;
    }

    if (handle->path[pathlen-1] != '/') {
	handle->path[pathlen++] = '/';
	handle->path[pathlen] = 0;
    }
    handle->length = pathlen;


    return handle;
}


/**
 * end recurse through directory
 *
 * clean up handle
 *
 */

void
recurseEnd (recurseT *handle)
{
    closedir (handle->d);
    free (handle);
    return;
}


/**
 * recurse through directory
 *
 * get first/next path from handle->path
 * NULL at error or end-of-dir
 *
 */

recurseT *
recurseNext (recurseT *handle, struct stat *st)
{
    struct dirent *dent;
    int dlength;

    while ((dent = readdir (handle->d)) != 0)
    {
	if (!(strcmp (dent->d_name, ".")
	    && strcmp (dent->d_name, "..")))
	{
	    continue;
	}

	y2debug ("recurseNext(%s:%s)", handle->path, dent->d_name);
	dlength = strlen (dent->d_name);
	if (handle->length + dlength >= MAXPATHLEN-1) {
	    handle->path[handle->length] = 0;
	    fprintf (stderr, "Path too long [%s/%s]\n", handle->path, dent->d_name);
	    continue;
	}

	strcpy (handle->path + handle->length, dent->d_name);	// build complete path

	struct stat st1;

	if (st == 0) st = &st1;

	if (lstat (handle->path, st) == -1)
	{
	    perror (handle->path);
	    continue;
	}

	if (S_ISLNK (st->st_mode))
	{
	    continue;
	}

	if (S_ISDIR (st->st_mode))
	{
	    recurseT *next = (recurseT *)malloc (sizeof (recurseT));
	    if (next == 0) {
		perror ("malloc");
		return 0;
	    }
	    next->parent = handle;
	    next->path = handle->path;
	    next->length = handle->length + dlength;

	    if(!(next->d = opendir (next->path)))
	    {
		perror (next->path);
		free (next);
		continue;
	    }

	    next->path[next->length++] = '/';
	    next->path[next->length] = 0;

	    return recurseNext (next, st);
	}

	if (S_ISREG (st->st_mode))
	{
	    return handle;
	}
    }

    if (handle->parent)
    {
	recurseT *next = handle->parent;
	recurseEnd (handle);
	return recurseNext (next, st);
    }

    return 0;
}


//-----------------------------------------------------------------------------


/**
 * Recurse through directories, checking times against '.depend'
 *
 * return empty char * if no recompile needed
 * return NULL if error
 * return pathname of .depend file if recompile needed (.ybc missing, .ybc older than .ycp, any file newer than .depend) 
 *   in this case, the pathname is malloc'ed and should be free'd
 */

char *
dependCheck (const char *path)
{
    char *name = (char *)malloc (strlen (path) + strlen (STAMPNAME) + 1);
    if (name == 0) {
	perror ("malloc");
	return NULL;
    }
    strcpy (name, path);
    strcat (name, STAMPNAME);

    struct stat st;
    time_t dependtime;

    if (lstat (name, &st) != 0) {			// check existance of .depend
	printf ("No .depend found, recompiling\n");
	return name;					// -> must recompile
    }

    dependtime = st.st_mtime;

    recurseT *handle, *next;

    handle = recurseStart (path);
    if (handle == 0) {
	return NULL;
    }

    int ret = 0;
    int extpos;

    while (handle) {
	next = recurseNext (handle, &st);
	if (next == 0)
	{
	    recurseEnd (handle);
	    handle = 0;
	    break;
	}
	handle = next;

	// compute position of extension
	extpos = handle->length + strlen (handle->path + handle->length) - 4;
	if (extpos > 0)
	{
	    if (strcmp (handle->path + extpos, ".ycp") == 0)	// .ycp existing, check .ybc
	    {
		time_t ycptime = st.st_mtime;

		strcpy (handle->path + extpos, ".ybc");
		if (lstat (handle->path, &st) != 0)				// .ybc probably not existing
		{
		    handle->path[extpos] = 0;
		    printf ("%s.ycp is not compiled\n", handle->path);
		    ret = 1;
		    break;
		}
		handle->path[extpos] = 0;

		if (ycptime > st.st_mtime)
		{
		    printf ("%s.ycp newer than compiled .ybc\n", handle->path);
		    ret = 1;
		    break;
		}

		st.st_mtime = ycptime;
	    }
	    else
	    {
		st.st_mtime = dependtime;					// don't care about any other files
	    }

	    if (dependtime < st.st_mtime) {
		printf ("%s.ycp newer than .depend\n", handle->path);
		ret = 1;
	        break;
	    }
	}
    }

    if (handle)
    {
	recurseEnd (handle);
    }

    if (ret == 0) return "";
    return name;
}

//-----------------------------------------------------------------------------
// compute dependencies of file
//
// return list as 'module', 'import1', 'import2', ...
// the first list element is always the name of the module. 
// if the list is empty, the file is not a module
// if the list only has a single entry, the module is at the leaf of
// the dependency tree (does not import any other module)
//

std::list<std::string>
checkDependFile (char *file)
{
//    printf ("Compute dependencies for '%s'\n", file);

#define LBUFSIZE 8192
    char lbuf[LBUFSIZE];

    std::list<std::string> deplist;

    FILE *f = fopen (file, "r");
    if (f == 0)
    {
	perror (file);
	return deplist;
    }

    char *lptr;

    while (fgets (lbuf, LBUFSIZE-1, f) != 0)
    {
	lptr = lbuf;
	while (isblank (*lptr)) lptr++;

	    if (deplist.empty())		// nothing yet, look for 'module' first
	    {
		if ((*lptr == 'm')
		    && (strncmp (lptr + 1, "odule", 5) == 0))		// module
		{
		    lptr += 6;
		    while (isblank (*lptr)) lptr++;
		    if (*lptr++ != '"') break;
		    char *name = lptr;
		    while (*lptr)
		    {
			if (*lptr == '"')
			    break;
			lptr++;
		    }
		    *lptr++ = 0;
		    while (isblank (*lptr)) lptr++;
		    if (*lptr != ';') break;

		    deplist.push_back (name);
		}
	    }
	    else
	    {
		if (*lptr == 'i')
		{
		    if (strncmp (lptr + 1, "mport", 5) == 0)		// import
		{
		    lptr += 6;
		    while (isblank (*lptr)) lptr++;
		    if (*lptr++ != '"') break;
		    char *name = lptr;
		    while (*lptr)
		    {
			if (*lptr == '"')
			    break;
			lptr++;
		    }
		    *lptr++ = 0;
		    while (isblank (*lptr)) lptr++;
		    if (*lptr != ';') break;

		    deplist.push_back (name);
		}
		else if (strncmp (lptr + 1, "nclude", 6) == 0)		// include
		{
		    lptr += 7;
		    while (isblank (*lptr)) lptr++;
		    if (*lptr++ != '"') break;
//		    char *name = lptr;
		    while (*lptr)
		    {
			if (*lptr == '"')
			    break;
			lptr++;
		    }
		    *lptr++ = 0;
		    while (isblank (*lptr)) lptr++;
		    if (*lptr != ';') break;

		    // deplist.push_back (name);
		}
		}
	    }
    }

    fclose (f);
    return deplist;
}


//
// compute dependency tree from dependency map
// start at module, return list of 
// if module empty, start at depmap.begin()

std::list<std::string>
depTree (std::string module, const std::map<std::string, std::list<std::string> > & depmap)
{
    static std::map<std::string, int> seenmap;
    std::list<std::string> ret;

    if (module.empty())
    {
	seenmap.clear();
	if (depmap.empty())
	{
	    fprintf (stderr, "No dependencies found\n");
	    return ret;
	}
    }

    if (seenmap.find (module) != seenmap.end())		// already seen this module
    {
//	printf ("-- already seen %s\n", module.c_str());
	return ret;
    }

    seenmap[module] = 1;

    std::map<std::string, std::list<std::string> >::const_iterator it = depmap.find (module);

    if (it == depmap.end())
    {
	fprintf (stderr, "Can't find module '%s' in dependency map\n", module.c_str());
	return ret;
    }

//    printf ("\nCheck %s: ", module.c_str()); fflush (stdout);

    std::list<std::string> dl = it->second;		// get <mod> <imp> <imp> <imp> list

    if (dl.size() == 0)
    {
	fprintf (stderr, "%s not a module ?\n", module.c_str());
	return ret;
    }
    else if (dl.size() < 2)				// no further dependencies
    {
//	printf ("-- leaf\n");
	ret.push_front (module);
	return ret;
    }


    // recurse dependencies

    std::list<std::string>::iterator dli = dl.begin();
    dli++;						// skip initial module

    while (dli != dl.end())
    {
	std::list<std::string> l = depTree (*dli, depmap);
	std::list<std::string>::iterator it;
	for (it = l.begin(); it != l.end(); it++)
	{
	    ret.push_back (*it);
	}
	dli++;
    }

    if (!module.empty())
	ret.push_back (module);

//    printf ("(%s:", module.c_str()); fflush (stdout);
//	    std::list<std::string>::iterator rit;
//	    for (rit = ret.begin(); rit != ret.end(); rit++)
//	    {
//		printf (" %s", rit->c_str());
//	    }
//    printf (")\n");
    return ret;
}


// compute dependencies of all files in dir

std::list<std::string>
makeDependDir (char *dir)
{
    std::list<std::string> deplist;

    printf ("Compute dependencies below '%s'\n", dir);

    recurseT *handle, *next;

    handle = recurseStart (dir);
    if (handle == 0) {
	return deplist;
    }

    string depname (dir);
    depname += DEPENDNAME;

    FILE *depend = fopen (depname.c_str(), "w+");
    if (depend == 0)
    {
	fprintf (stderr, "Can't open %s: %s\n", depname.c_str(), strerror (errno));
	recurseEnd (handle);
	return deplist;
    }

    int extpos;
    struct stat st;

    std::map<std::string, std::list<std::string> > depmap;
    std::list<std::string> all;
    all.push_back ("*");

    while (handle)
    {
	next = recurseNext (handle, &st);
	if (next == 0)
	{
	    recurseEnd (handle);
	    handle = 0;
	    break;
	}
	handle = next;
	extpos = handle->length + strlen (handle->path + handle->length) - 4;
	if (strcmp (handle->path + extpos, ".ycp") == 0)
	{
	    deplist = checkDependFile (handle->path);
	    if (deplist.empty())
	    {
		fprintf (stderr, "Not a module: %s\n", handle->path);
	    }
	    else
	    {
		std::list<std::string>::iterator it = deplist.begin();
		depmap[*it] = deplist;
		all.push_back (*it);
		fprintf (depend, "%s:", it->c_str());
		while (++it != deplist.end())
		{
		    fprintf (depend, " %s", it->c_str());
		}
		fprintf (depend, "\n");
	    }
	}
    }

    depmap[""] = all;

    if (handle)
    {
	recurseEnd (handle);
    }

    deplist.clear();

    // now compute list from depmap

    deplist = depTree ("", depmap);

    return deplist;
}


/*
 * recompile all modules
 * deplist = ordered list of modules
 * depend = path to .depend file
 *
 */

int
recompileAll (const std::list<std::string> & deplist, const char *depend)
{
    return 0;
}

//-----------------------------------------------------------------------------


/**
 * parse file and return corresponding YCode or NULL for error
 */
YCode *
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
    
    FILE *infile;
    if (!strcmp (infname, "-"))
    {
	infile = stdin;
    }
    else
    {
	infile = fopen (infname, "r");
    }

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
	    outstream << c->toString() << endl;
	}
	else
	{
	    std::cout << c->toString() << endl;
	}

	return 0;
    }
    
    return 0;
}


/**
 * Recurse through directories, processing 
 */

int
recurse (const char *path)
{
    recurseT *handle, *next;

    handle = recurseStart (path);
    if (handle == 0) {
	return -1;
    }

    struct stat st;
    int extpos;			// position of file extension
    int ret = 0;

    while (handle)
    {
	next = recurseNext (handle, &st);
	if (next == 0)
	{
	    recurseEnd (handle);
	    handle = 0;
	    break;
	}
	handle = next;

	extpos = handle->length + strlen (handle->path + handle->length) - 4;
	if (strcmp (handle->path + extpos, ".ycp") == 0)
	{
	    if (processfile (handle->path, NULL))
		ret = 1;
	}
    }

    if (handle)
    {
	recurseEnd (handle);
    }

    return ret;
}


//-----------------------------------------------------------------------------

/**
 * Display help text
 */
void print_help (const char *name)
{
    printf ("Usage:\n");
    printf ("  %s [-h] [--help]\n", name);
    printf ("  %s [-v] [--version]\n", name);
    printf ("  %s [-q] [-R] {-d} {-I include-path} {-M module-path} {-l logfile} {-c|-E|-C} {-o output} <filename>\n", name);
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
    
    bool log_set = false;

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
	    {"force", 0, 0, 'f'},			// force recompilation
	    {"logfile", 1, 0, 'l'},			// specify log file
	    {"no-implicit-imports", 0, 0, 'd'},		// don't preload implicit namespaces
	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "h?vVpqrRdEcfI:M:o:l:", options, &option_index);
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
	    case 'f':
		force = 1;
		break;
	    case 'I':
		YCPPathSearch::addPath (YCPPathSearch::Include, optarg);
		break;
	    case 'M': {
		YCPPathSearch::addPath (YCPPathSearch::Module, optarg);
#if 0
		char *depend = dependCheck (optarg);
		if (depend || *depend != 0)
		{
		    list<string> deplist = makeDependDir (optarg);
		    recompileAll (deplist, depend);
		    free (depend);
		}
#endif
	    }
		break;
	    case 'l':
		set_log_filename (optarg);
		log_set = true;
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
    
    // setup log
    if (!log_set)
    {
	// user didn't setup log, use stdout
	set_log_filename("-");
    }
    set_simple (true);

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
	    if (!compile)
	    {
		if (recurse (argv[i]))
		{
		    ret = 1;
		}
	        continue;
	    }

	    // recursively compile

	    if (!force)
	    {
		continue;
	    }

	    char *depend = dependCheck (argv[i]);
	    if (depend != 0
		&& *depend == 0)
	    {
		continue;
	    }

	    list<string> deplist = makeDependDir (argv[i]);
	    if (deplist.empty())
	    {
		fprintf (stderr, "Can't check dependencies below %s\nNot a directory ?\n", argv[i]);
		return 1;
	    }

	    std::list<std::string>::iterator it;

	    for (it = deplist.begin(); it != deplist.end(); it++)
	    {
		string path = (string)argv[i] + (string)"/" + *it + (string)".ycp";
		if (compilefile (path.c_str(), 0))
		{
		    fprintf (stderr, "Compilation of '%s' failed: %s\n", it->c_str(), strerror (errno));
		}
	    }
	    ret = 0;
	}
	else
	{
	    if (processfile (argv[i], outname))
	    {
		ret = 1;
	    }
	}
    }
    return ret;
}
