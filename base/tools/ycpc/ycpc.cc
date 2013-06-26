/*
 * ycpc.cc
 *
 * YCP standalone bytecode compiler
 *
 * Authors: Klaus Kaempf <kkaempf@suse.de>
 *	    Stanislav Visnovsky <visnov@suse.cz>
 */

#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <utime.h>
#include <errno.h>
#include <sys/stat.h>

#include <fstream>
#include <list>
#include <map>

#include <YCP.h>
#include <ycp/YCode.h>
#include <ycp/Parser.h>
#include <ycp/Bytecode.h>
#include <ycp/Xmlcode.h>
#include <ycp/Import.h>
#include <ycp/y2log.h>
#include <../../libycp/src/parser.hh>
#include <ycp/pathsearch.h>
#include <y2/Y2Component.h>
#include <y2/Y2ComponentBroker.h>

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
static int verbose = 0;		// much output
static int no_std_path = 0;	// dont use builtin pathes
static int recursive = 0;	// recursively all files
static int parse = 0;		// just parse source code
static int compile = 0;		// just compile source to bytecode
static int to_xml = 0;		// output XML instead of bytecode
static int read_n_print = 0;	// read and print bytecode
static int read_n_run = 0;	// read and run bytecode
static int freshen = 0;		// freshen recompilation
static int force = 0;		// force recompilation
static int no_implicit_namespaces = 0;	// don't preload implicit namespaces
static const char *ui_name = 0;
#define UI_QT_NAME "qt"
#define UI_NCURSES_NAME "ncurses"

#define progress(...)	{ if (!quiet) fprintf (stderr, __VA_ARGS__); }

//---------------------------------------------------------------------------..

#define MAXPATHLEN 2048

/// directory recursion (ycpc)
typedef struct recurse_struct {
    struct recurse_struct *parent;
    DIR *d;			// opendir/readdir handle
    char *path;			// current directory path (points to parent->path if parent != 0)
    int length;			// if parent == 0: length of starting directory in path[], path[0...startlen-1] == path from recurseStart()
				// else: length of current subdir in path[], path[startlen...currentlen-1] == current recursion point
} recurseT;

static recurseT *recurseStart (const char *path);
static recurseT *recurseNext (recurseT *handle, struct stat *st = 0);
static void recurseEnd (recurseT *handle);


/**
 * recurse through directory
 *
 * start recursion at path, return recurseT handle
 *
 */

static recurseT *
recurseStart (const char *path)
{
    y2debug ("recurseStart(%s)", path);
    int pathlen = strlen (path);
    if (pathlen > MAXPATHLEN-2)			// save 2 chars for trailing slash and 0
    {
	fprintf (stderr, "Path too long (%d chars, max %d chars allowed\n", pathlen, MAXPATHLEN-2);
	return 0;
    }

    recurseT *handle = (recurseT *)malloc (sizeof (recurseT));
    if (handle == 0)
    {
	perror ("malloc");
	return 0;
    }
    handle->path = (char *)malloc (MAXPATHLEN);
    if (handle->path == 0)
    {
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

    if (handle->path[pathlen-1] != '/')
    {
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

static void
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

static recurseT *
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
	if (handle->length + dlength >= MAXPATHLEN-1)
	{
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
	    if (next == 0)
	    {
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

/// file dependency (ycpc)
class FileDep {
    private:
	std::string m_name;
	std::string m_path;
	bool m_is_module;	///< module or include
	bool m_have_source;
	time_t m_srctime;
	time_t m_bintime;
    public:
	FileDep () {};
	FileDep (const std::string & name, const std::string & path, bool is_module, bool have_source, time_t srctime, time_t bintime);
	const std::string & name () const;
	const std::string & path () const;
	void setPath (const std::string &path);
	bool is_module () const;
	time_t srctime () const;
	time_t bintime () const;
	std::string toString() const;
};


FileDep::FileDep (const std::string & name, const std::string & path, bool is_module, bool have_source, time_t srctime, time_t bintime)
    : m_name (name)
    , m_path (path)
    , m_is_module (is_module)
    , m_have_source (have_source)
    , m_srctime (srctime)
    , m_bintime (bintime)
{
}


const std::string &
FileDep::name () const
{
    return m_name;
}


const std::string &
FileDep::path () const
{
    return m_path;
}


time_t
FileDep::srctime () const
{
    return m_srctime;
}


time_t
FileDep::bintime () const
{
    return m_bintime;
}


void
FileDep::setPath (const std::string & path)
{
    m_path = path;
}


bool
FileDep::is_module () const
{
    return m_is_module;
}


std::string
FileDep::toString () const
{
    return string (m_is_module ? "module \"" : "include \"")
	    + m_name
	    + string ("\" at '")
	    + m_path + string ("'");
}

//-----------------------------------------------------------------------------

// try to resolve dependency to module/include file
// return false if name not found

FileDep
resolveDep (const char *name, bool as_module)
{
    if (verbose > 1) printf ("resolveDep (%s %s)", as_module ? "import" : "include", name);
    std::string srcpath;
    time_t srctime = 0;
    std::string binpath;
    time_t bintime = 0;
    bool have_source = true;
    struct stat st;

    if (as_module)
    {
	srcpath = YCPPathSearch::findModule (name, true);	// find source code for module
	if (srcpath.empty())
	{
	    have_source = false;
	}
	else if (stat (srcpath.c_str(), &st) == 0)
	{
	    srctime = st.st_mtime;
	}
	binpath = YCPPathSearch::findModule (name);		// find binary code for module
	if (!binpath.empty()
	    && stat (binpath.c_str(), &st) == 0)
	{
	    bintime = st.st_mtime;
	}
    }
    else
    {
	srcpath = YCPPathSearch::findInclude (name);
	if (!srcpath.empty()
	    && stat (srcpath.c_str(), &st) == 0)
	{
	    srctime = st.st_mtime;
	}
    }

    if (verbose > 1) printf (" -> '%s'\n", srcpath.c_str());

    return FileDep (name, have_source ? srcpath : binpath, as_module, have_source, srctime, bintime);
}

//-----------------------------------------------------------------------------
// compute dependencies of file
//
// return list of FileDep.
//  First list entry is the file itself,
//  further list entries are the dependencies (import or include) in order of appearance.
//
// if the list has a single entry only, the file is at the leaf of
// the dependency tree (does not import/include any other module/file)
//
// return empty list on error
//

std::list <FileDep>
parseFile (const char *path, const char *expected)
{
    if (verbose > 1) printf ("List dependencies for '%s'\n", path);

#define LBUFSIZE 8192
    char lbuf[LBUFSIZE];

    std::list <FileDep> deplist;

    FILE *f = fopen (path, "r");
    if (f == 0)
    {
	perror (path);
	return deplist;
    }

    char *lptr;
    int lcount = 0;

    bool have_module = false;

    while (fgets (lbuf, LBUFSIZE-1, f) != 0)
    {
	lcount++;
	lptr = lbuf;
	while (isblank (*lptr)) lptr++;

	if ((*lptr == 'm')
	    && (strncmp (lptr + 1, "odule", 5) == 0))		// module
	{
	    lptr += 6;
	    while (isblank (*lptr)) lptr++;
	    if (*lptr++ != '"') continue;
	    char *name = lptr;
	    while (*lptr)
	    {
		if (*lptr == '"')
		    break;
		lptr++;
	    }
	    *lptr++ = 0;
	    while (isblank (*lptr)) lptr++;
	    if (*lptr != ';') continue;

	    if (!deplist.empty())
	    {
		fprintf (stderr, "Bad file '%s':\nmodule statement at wrong postion\nLine %d:[%s]", path, lcount, lbuf);
		fprintf (stderr, "Have already: %s\n", deplist.front().toString().c_str());
		deplist.clear();
		return deplist;
	    }

	    if (*expected
		&& strcmp (name, expected) != 0)
	    {
		fprintf (stderr, "Module file %s does not have expected name '%s' but '%s'\n", path, expected, name);
		return deplist;
	    }
	    have_module = true;
	    deplist.push_back (FileDep (name, path, true, false, 0, 0));
	}
	else if (*lptr == 'i')
	{
	    if (strncmp (lptr + 1, "mport", 5) == 0)		// import
	    {
		lptr += 6;
		while (isblank (*lptr)) lptr++;
		if (*lptr++ != '"') continue;
		char *name = lptr;
		while (*lptr)
		{
		    if (*lptr == '"')
			break;
		    lptr++;
		}
		*lptr++ = 0;
		while (isblank (*lptr)) lptr++;
		if (*lptr != ';') continue;

		FileDep fd = resolveDep (name, true);
		if (fd.path().empty())
		{
		    fprintf (stderr, "*** Error: Can't find module %s\n", name);
		}
		else
		{
		    deplist.push_back (fd);
		}
	    }
	    else if (strncmp (lptr + 1, "nclude", 6) == 0)		// include
	    {
		lptr += 7;
		while (isblank (*lptr)) lptr++;
		if (*lptr++ != '"') continue;
		char *name = lptr;
		while (*lptr)
		{
		    if (*lptr == '"')
		    {
			break;
		    }
		    lptr++;
		}
		*lptr++ = 0;
		while (isblank (*lptr)) lptr++;
		if (*lptr != ';') continue;

		FileDep fd = resolveDep (name, false);
		if (fd.path().empty())
		{
		    fprintf (stderr, "*** Error: Can't find include %s\n", name);
		}
		else
		{
		    deplist.push_back (fd);
		}
	    }
	} // 'i'include / 'i'mport
    }

    fclose (f);

    // if no 'module' found -> generate fake include FileDep
    if (!have_module)
    {
	if (*expected == 0)
	{
	    fprintf (stderr, "%s is not a module\n", path);
	    exit (1);
	}
	deplist.push_front (FileDep (expected, path, false, false, 0, 0));
    }

    if (verbose > 1) printf ("parseFile (%s) done\n", path);
    return deplist;
}


//-----------------------------------------------------------------------------
// List all files in dir
//  @param path of directory
//  returns list of FileDep with path() filled for each .ycp file found
//

std::list <FileDep>
makeDirList (const char *dir)
{
    std::list <FileDep> deplist;

    struct stat st;

    if (lstat (dir, &st) == -1)				// not existant, not accessible
    {
	perror (dir);
	return deplist;
    }

    if (S_ISREG (st.st_mode))				// a single file
    {
	deplist.push_back (FileDep ("", dir, false, false, 0, 0));
	return deplist;
    }

    if (!S_ISDIR (st.st_mode))
    {
	fprintf (stderr, "Not a file or directory: %s\n", dir);
	return deplist;
    }

    if (verbose) printf ("List files below '%s'\n", dir);

    recurseT *handle, *next;

    handle = recurseStart (dir);
    if (handle == 0)
    {
	return deplist;
    }

    int extpos;

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
	    deplist.push_back (FileDep ("", handle->path, false, false, 0, 0));
	    if (verbose) printf ("%s\n", handle->path);
	}
    }

    if (verbose) printf ("directory %s listed, %zu files\n", dir, deplist.size());
    return deplist;
}

//-----------------------------------------------------------------------------
// create solved dependency map
//   @param dep	list of FileDep pathes
//
//   Parse all files listed in dep->path() and generate dependency map [name] -> list of FileDep
//

std::map <std::string, std::list <FileDep> >
makeDependMap (const std::list<FileDep> & dep)
{
    if (verbose) printf ("makeDependMap (), %zu files\n", dep.size());

    std::map <std::string, std::list <FileDep> > depmap;

    if (dep.empty())
    {
	fprintf (stderr, "makeDependMap, empty input!\n");
	return depmap;
    }

    std::list<FileDep>::const_iterator depit;
    std::list<FileDep> resolved;

    for (depit = dep.begin(); depit != dep.end(); depit++)
    {
	resolved = parseFile (depit->path().c_str(), depit->name().c_str());
	if (resolved.empty())
	{
	    fprintf (stderr, "Couldn't parse file '%s'\n", depit->path().c_str());
	    continue;
	}

	depmap[resolved.front().name()] = resolved;

	if (verbose > 2)
	{
	    printf ("Map %s: %zu entries\n", resolved.front().name().c_str(), resolved.size());
	}

	if (verbose > 1)
	{
	    std::list<FileDep>::iterator resit = resolved.begin();
	    printf ("%s needs ", resit->toString().c_str());
	    resit++;
	    if (resit == resolved.end())
	    {
		printf ("nothing");
	    }
	    else while (resit != resolved.end())
	    {
		printf ("\n\t%s", resit->toString().c_str());
		resit++;
	    }
	    printf ("\n");
	}
    }

    // now go through all map entries and resolve them
    std::map <std::string, std::list <FileDep> >::iterator mapit;
    bool found_unsolved = true;
    while (found_unsolved)
    {
	std::list<FileDep>::iterator depit;

	found_unsolved = false;

	if (depmap.empty())
	{
	    fprintf (stderr, "Empty map ?!\n");
	    break;
	}

	for (mapit = depmap.begin(); mapit != depmap.end(); mapit++)
	{
	    if (mapit->second.empty())
	    {
		fprintf (stderr, "Map entry %s empty !\n", mapit->first.c_str());
		break;
	    }
	    for (depit = mapit->second.begin(); depit != mapit->second.end(); depit++)
	    {
		if (depmap.find (depit->name()) == depmap.end())
		{
		    resolved = parseFile (depit->path().c_str(), depit->name().c_str());
		    if (resolved.empty())
		    {
			fprintf (stderr, "Couldn't parse file '%s'\n", depit->path().c_str());
			continue;
		    }
		    if (verbose > 1) printf ("Solved %s as %s\n", depit->name().c_str(), resolved.front().name().c_str());
		    depmap[resolved.front().name()] = resolved;
		    found_unsolved = true;
		    break;
		}
	    }
	    if (found_unsolved) break;
	}
    }

    return depmap;
}


//-----------------------------------------------------------------------------
//
// compute dependency tree from dependency map
// start at module, return list of dependencies
// if module empty, build tree over all module entries in depmap

std::list<FileDep>
depTree (std::string module, const std::map<std::string, std::list<FileDep> > & depmap)
{
    static std::map<std::string, int> seenmap;
    std::list<FileDep> ret;

    if (verbose > 1) printf ("depTree (%s)\n", module.c_str());

    if (module.empty())
    {
	if (verbose > 1) printf ("initialize\n");
	seenmap.clear();
	if (depmap.empty())
	{
	    fprintf (stderr, "No dependencies found\n");
	}
	return ret;
    }

    if (seenmap.find (module) != seenmap.end())		// already seen this module
    {
	if (verbose > 1) printf ("-- already seen %s\n", module.c_str());
	return ret;
    }

    seenmap[module] = 1;

    std::map<std::string, std::list<FileDep> >::const_iterator it = depmap.find (module);

    if (it == depmap.end())
    {
	fprintf (stderr, "Can't find '%s' in dependency map\n", module.c_str());
	return ret;
    }

    if (verbose > 1) printf ("\nCheck '%s': ", module.c_str()); fflush (stdout);

    std::list<FileDep> dl = it->second;		// get <mod> <imp> <imp> <imp> list

    if (dl.size() == 0)
    {
	fprintf (stderr, "%s not a module ?\n", module.c_str());
	return ret;
    }
    else if (dl.size() < 2)				// no further dependencies
    {
	if (verbose > 1) printf ("-- leaf\n");
	return ret;
    }

    // recurse dependencies

    // first push all dependencies for the current module

    std::list<FileDep>::iterator dli = dl.begin();
    dli++;						// skip initial module

    while (dli != dl.end())
    {
	std::string name = dli->name();
	if (seenmap.find (name) == seenmap.end())		// not seen this name before
	{
	    if (verbose > 1) printf ("%s -> %s: ", module.c_str(), name.c_str());
	    std::list<FileDep> l = depTree (name, depmap);	// recusion point
	    if (verbose > 1) printf ("%s -> %s done\n", module.c_str(), name.c_str());

	    std::list<FileDep>::iterator it;
	    for (it = l.begin(); it != l.end(); it++)
	    {
		ret.push_back (*it);
	    }

	    // then push the current name

	    ret.push_back (*dli);
	}

	dli++;
    }

    if (verbose > 1)
    {
	printf ("(%s:", module.c_str()); fflush (stdout);
	std::list<FileDep>::iterator rit;
	for (rit = ret.begin(); rit != ret.end(); rit++)
	{
	    printf (" %s", rit->name().c_str());
	}
	printf (")\n");
    }
    return ret;
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
 * "-" is stdin
 */
YCodePtr
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

    progress ("parsing '%s'\n", infname);

    parser->setInput (infile, infname);
    parser->setBuffered();

    if (verbose > 2) SymbolTableDebug = 1;

    YCodePtr c = NULL;

    c = parser->parse ();
    
    int ln = parser->scanner ()->lineNumber ();
    
    if (! parser->atEOF () && parser->scanner ()->yylex () != END_OF_FILE)
    {
	YaST::ee.setFilename(parser->scanner()->filename());
	YaST::ee.setLinenumber(ln);
	ycperror ("Unreachable code at the end of file");
	c = NULL;
    }

    fclose (infile);

    if (c == NULL || c->isError ())
    {
	progress ("Error\n");
	return NULL;
    }

    progress ("done\n");

    return c;
}

/**
 * Compile one file
 * infname: "-" is stdin
 * outfname: if NULL it is created from infname by replacing .ycp by .ybc
 * return:
 * 0 - success
 * 1 - writing failed
 * 2 - parsing failed
 */

int
compilefile (const char *infname, const char *outfname)
{
    string ofname = infname;

    if (outfname != NULL)
    {
	ofname = outfname;
    }
    else
    {
	int len = ofname.size ();
	if (len > 4 && ofname.substr (len-4, 4) == ".ycp")
	{
	    ofname = ofname.replace (len-4, 4, to_xml ? ".xml" : ".ybc");
	}
	else
	{
	    ofname += to_xml ? ".xml" : ".ybc";
	}
    }
    progress ("compiling to '%s'\n", ofname.c_str ());

    YCodePtr c = parsefile (infname);

    if (c != NULL )
    {
	progress ("saving ...\n");
	int result = 0;
	if (to_xml) {
	    result = Xmlcode::writeFile (c, ofname);
	}
	else {
	    result = Bytecode::writeFile (c, ofname);
	}
	return result ? 0 : 1;
    }

    return 2;
}

// this is a template function just because I couldn't look at the
// copy & paste programming

/**
 * Print the code to the given file
 * 0 is success
 */
template <typename toStringAble>
int printcode (const char *outfname, const toStringAble & c)
{
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

    progress ("Parsed:\n");
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

/**
 * Process one file according to the command:
 * compile, check syntax
 */

int
processfile (const char *infname, const char *outfname)
{
    if (compile)
    {
	return compilefile (infname, outfname);
    }

    if (parse)
    {
	YCodePtr c = parsefile (infname);

	if (c == NULL) return 1;

	if (quiet) return 0;

	return printcode (outfname, c);
    }

    if (read_n_print)
    {
	progress ("Reading: %s\n", infname);
	YCodePtr c = Bytecode::readFile (infname);
	if (c == 0)
	{
	    progress ("Bytecode read failed\n");
	    return 1;
	}

	progress ("Result:\n");

	return printcode (outfname, c);
    }

    if (read_n_run)
    {
	progress ("Reading:\n");
	YCodePtr c = Bytecode::readFile (infname);
	progress ("Running:\n");

	Y2Component *server = Y2ComponentBroker::createServer (ui_name);
	if (!server)
	{
	    fprintf (stderr, "Can't start UI component '%s'", ui_name);
	    return 1;
	}

	YCPValue result = c->evaluate();
	progress ("Result:\n");

	return printcode (outfname, result);
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
    if (handle == 0)
    {
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
#define opt_fmt "\t%-25s %s\n"
    printf ("Usage:\n");
    printf ("  %s [-h] [--help]\n", name);
    printf ("  %s [-v] [--version]\n", name);
    printf ("  %s <command> [<option>]... <filename>...\n", name);
    printf ("  %s\n", "Commands:");
    printf (opt_fmt, "-c, --compile", "compile to bytecode");
    printf (opt_fmt, "-E, --fsyntax-only", "check syntax and print (unless -q)");
    printf (opt_fmt, "-f, --freshen", "freshen .ybc files");
    printf (opt_fmt, "-p, --print", "read and print bytecode");
    printf (opt_fmt, "-r, --run", "read and run bytecode");
    printf ("  Options:\n");
    printf (opt_fmt, "-l, --log-file <name>", "log file, - means stderr");	// common
    printf (opt_fmt, "-q, --quiet", "no output");
    printf (opt_fmt, "-t, --test", "more output (-tt, -ttt)");
    printf ("\n");
    printf (opt_fmt, "-d, --no-implicit-imports", "don't preload implicit namespaces");
    printf (opt_fmt, "-F, --Force", "force recompilation of all dependant files");
    printf (opt_fmt, "-I, --include-path", "where to find include files");
    printf (opt_fmt, "-M, --module-path", "where to find module files");
    printf (opt_fmt, "--no-std-includes", "drop all built-in include paths");
    printf (opt_fmt, "--no-std-modules", "drop all built-in module paths");
    printf (opt_fmt, "-n, --no-std-paths", "no standard paths");
    printf (opt_fmt, "-x, --xml", "for -c, produce a XML parse tree instead of YBC");
    printf (opt_fmt, "-o, --output", "output file for -c, -E, -p, -r");
    printf (opt_fmt, "-R, --recursive", "operate recursively");
    printf (opt_fmt, "-u, --ui {ncurses|qt}", "UI to start in combination with 'r'");
//    printf (opt_fmt, "-, --", "");
#undef opt_fmt
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

    // list of -I / -M pathes
    //   will be pushed to YCPPathSearch later to keep correct order
    //   (the last added path to YCPPathSearch will be searched first)
    std::list<std::string> modpathes;
    std::list<std::string> incpathes;

    for(;;)
    {
	int option_index = 0;

	static struct option options[] =
	{
	    {"compile", 0, 0, 'c'},			// compile to bytecode
	    {"no-implicit-imports", 0, 0, 'd'},		// don't preload implicit namespaces
	    {"fsyntax-only", 0, 0, 'E'},		// parse only
	    {"freshen", 0, 0, 'f'},			// freshen .ybc files
	    {"Force", 0, 0, 'F'},			// force recompile of all dependant files
	    {"help", 0, 0, 'h'},			// show help and exit
	    {"include-path", 1, 0, 'I'},		// where to find include files
	    {"module-path", 1, 0, 'M'},			// where to find module files
	    {"no-std-includes", 0, 0, 257},		// drop all built-in include pathes
	    {"no-std-modules", 0, 0, 258},		// drop all built-in module pathes
	    {"nostdincludes", 0, 0, 257},		// drop all built-in include pathes
	    {"nostdmodules", 0, 0, 258},		// drop all built-in module pathes
	    {"log-file", 1, 0, 'l'},			// specify log file
	    {"logfile", 1, 0, 'l'},			// specify log file
	    {"no-std-path", 0, 0, 'n'},			// no standard pathes
	    {"no-std-paths", 0, 0, 'n'},		// no standard pathes
	    {"output", 1, 0, 'o'},			// output file
	    {"print", 0, 0, 'p'},			// read & print bytecode
	    {"run", 0, 0, 'r'},				// read & run bytecode
	    {"quiet", 0, 0, 'q'},			// no output
	    {"recursive", 0, 0, 'R'},			// recursively
	    {"test", 0, 0, 't'},			// lots of output
	    {"ui", 1, 0, 'u' },				// UI to start in combination with 'r'
	    {"version", 0, 0, 'v'},			// show version and exit
	    {"xml", 0, 0, 'x'},				// output XML insteaf of Bytecode
	    {0, 0, 0, 0}
	};

	int c = getopt_long (argc, argv, "h?vxVnpqrtRdEcFfI:M:o:l:u:", options, &option_index);
	if (c == EOF) break;

	switch (c)
	{
	    case 257:
		YCPPathSearch::clearPaths (YCPPathSearch::Include);
	    break;
	    case 258:
		YCPPathSearch::clearPaths (YCPPathSearch::Module);
	    break;
	    case 'h':
	    case '?':
		print_help ("ycpc");
		exit (0);
	    case 'v':
	    case 'V':
		print_version ();
		exit (0);
	    case 'r':
		read_n_run = 1;
		break;
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
		freshen = 1;
		break;
	    case 'F':
		force = 1;
		break;
	    case 'I':
		incpathes.push_front (string (optarg));		// push to front so first one is last in list
		break;
	    case 'M':
		modpathes.push_front (string (optarg));		// dito
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
	    case 'n':
		no_std_path = 1;
		break;
	    case 't':
		verbose += 1;
		break;
	    case 'p':
		read_n_print = 1;
		break;
	    case 'u':
		ui_name = strdup (optarg);
		break;
	    case 'x':
		to_xml = 1;
		break;
	    default:
		fprintf (stderr, "Try `%s -h' for more information.\n", argv[0]);
		exit(1);
	}
    }

    // setup log
    if (!log_set)
    {
	// user didn't setup log, use stdout
	set_log_filename("-");
    }
    set_log_simple_mode (true);

    // add include and module pathes to YCPPathSearch so that the argument order is kept

    std::list<std::string>::iterator pathit;
    for (pathit = incpathes.begin(); pathit != incpathes.end(); pathit++)
    {
	YCPPathSearch::addPath (YCPPathSearch::Include, pathit->c_str());
    }
    for (pathit = modpathes.begin(); pathit != modpathes.end(); pathit++)
    {
	YCPPathSearch::addPath (YCPPathSearch::Module, pathit->c_str());
    }

    // register builtins
    SCR scr;
    WFM wfm;

    if ((compile == parse)		// both are zero
	&& (compile == freshen)
	&& (compile == read_n_print)
	&& (compile == read_n_run))
    {
	fprintf (stderr, "-c, -E, -f, -p or -r must be given\n");
	exit (1);
    }

    if (optind == argc)
    {
	fprintf (stderr, "No input file or directory given\n");
	exit (1);
    }

    if (read_n_run			// if no explicit UI name given for run
	&& ui_name == 0)		//   try to determine one according to $DISPLAY
    {
	char *display = getenv ("DISPLAY");
	if (display == 0
	    || *display == 0)
	{
	    ui_name = UI_NCURSES_NAME;
	}
	else
	{
	    ui_name = UI_QT_NAME;
	}
    }

    std::list <FileDep> deplist;

    for (i = optind; i < argc;i++)
    {
	if (freshen)
	{
	    std::list <FileDep> depdir = makeDirList (argv[i]);

	    if (depdir.empty())
	    {
		fprintf (stderr, "Can't check dependencies below %s\nNot a directory ?\n", argv[i]);
		return 1;
	    }

	    std::list<FileDep>::iterator it;

	    for (it = depdir.begin(); it != depdir.end(); it++)
	    {
		if (verbose > 1) printf ("Add %s to deplist\n", it->path().c_str());
		deplist.push_back (*it);
	    }
	}
	else if (recursive)
	{
	    if (!compile)
	    {
		if (recurse (argv[i]))
		{
		    ret = 1;
		}
	        continue;
	    }
	    
	    ret = compilefile (argv[i], 0);

	    if (ret == 1)
	    {
		fprintf (stderr, "Compilation of '%s' failed: %s\n", argv[i], strerror (errno));
		break;
	    }
	    else if (ret == 2)
	    {
		fprintf (stderr, "Compilation of '%s' failed\n", argv[i]);
		break;
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

    if (freshen)
    {
	std::map <std::string, std::list <FileDep> > depmap = makeDependMap (deplist);

	if (verbose) printf ("Map generated, %zu entries, computing order\n", depmap.size());

	if (depmap.empty())
	{
	    fprintf (stderr, "No files or dependencies found\n");
	    return 1;
	}

	// loop over all modules and generated a fake module "*" which depends on all other modules

	std::map <std::string, std::list <FileDep> >::iterator modit;
	deplist.clear();
	for (modit = depmap.begin(); modit != depmap.end(); modit++)
	{
	    if (verbose > 2) printf ("Entry %s ", modit->first.c_str());
	    if (modit->second.empty())
	    {
		fprintf (stderr, "Entry '%s' has no description\n", modit->first.c_str());
		continue;
	    }
	    if (verbose > 2) printf (" -> %s\n", modit->second.front().toString().c_str());
	    if (modit->second.front().is_module())
	    {
		deplist.push_back (modit->second.front());
		if (verbose > 1) printf ("Module %s\n", modit->first.c_str());
	    }
	}
	depmap["*"] = deplist;

	deplist = depTree ("*", depmap);

	if (deplist.empty())
	{
	    fprintf (stderr, "No depencies found\n");
	    exit (1);
	}
	std::list <FileDep>::iterator depit;

	depit = deplist.end();
	depit--;					// get last element, existance is guaranteed
	time_t starttime = depit->bintime() == 0 ? depit->srctime() : depit->bintime();
	string startname = depit->name();

	for (depit = deplist.begin(); depit != deplist.end(); depit++)
	{
	    if (depit->srctime() > starttime)
	    {
		printf ("%s is newer than %s\n", depit->toString().c_str(), startname.c_str());
		time (&starttime);
	    }

	    if (!depit->is_module())
	    {
		continue;
	    }
	    if (compile)
	    {
		errno = 0;
		
		ret = compilefile (depit->path().c_str(), NULL);
		if (ret == 1)
		{
		    fprintf (stderr, "Compilation failed for %s: %s\n", depit->path().c_str(), strerror (errno));
		    break;
		}
		else if (ret == 2)
		{
		    fprintf (stderr, "Compilation failed for %s\n", depit->path().c_str());
		    break;
		}

		ret = 0;
	    }
	    else
	    {
		printf ("%s\n", depit->name().c_str());
	    }
	}
    }
    else
    {
        if (verbose) printf ("Done\n");
    }
    return ret;
}
