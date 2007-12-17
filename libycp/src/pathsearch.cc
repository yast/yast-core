/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	pathsearch.cc

   Search for YaST2 files at different paths.

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
		Martin Vidner <mvidner@suse.cz>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include <ycp/y2log.h>

#include "pathsearch.h"

// watch out, duplicated in YCPPathSearch::initialize
static const char *paths[] =
{
    "/y2update",		// Necessary during installation, but
				// can be achieved using -I and -M in
				// the yast2 start script
				// And more unimplemented stuff for scrconf...
    "Y2DIR",			// replace with env. var. Y2DIR
    "HOME",			// replace with users home dir + /.yast2
    YAST2DIR
};


static const int NUM_LEVELS = sizeof (paths) / sizeof (paths[0]);


int
Y2PathSearch::numberOfComponentLevels ()
{
    return NUM_LEVELS;
}


string
Y2PathSearch::searchPath (WHAT what, int level)
{
    static string *my_paths = 0;

    if (!my_paths)
    {
	// note: never deleted
	my_paths = new string [NUM_LEVELS];

	const char *home = getenv ("HOME");
	const char *y2dir = getenv ("Y2DIR");

	for (int i = 0; i < NUM_LEVELS; i++)
	{
	    // #330965, avoid publicly writable dirs in search path
	    // (we return a nonexistent dir because the API does not
	    // allow us to say Skip, and a cleanup patch to fix that
	    // would be too large)
	    static const char * not_there = YAST2DIR "/not-there";
	    if (strcmp (paths[i], "HOME") == 0)
	    {
	      if (home)
		my_paths[i] = string (home) + "/.yast2";
	      else
		my_paths[i] = string (not_there);
	    }
	    else if (strcmp (paths[i], "Y2DIR") == 0)
	    {
	      if (y2dir
		     && (strcmp (YAST2DIR, y2dir) != 0))		// prevent path duplication
		my_paths[i] = string (y2dir);
	      else
		my_paths[i] = string (not_there);
	    }
	    else
	    {
		my_paths[i] = string (paths[i]);
	    }
	}
    }

    switch (what)
    {
	case EXECCOMP:
	    if (level == NUM_LEVELS - 1) // FIXME
	    {
		return EXECCOMPDIR;
	    }
	    else
	    {
		return my_paths[level];
	    }
	break;

	case PLUGIN:
	    if (level == NUM_LEVELS - 1) // FIXME
	    {
		return PLUGINDIR;
	    }
	    else
	    {
		return my_paths[level] + "/plugin";
	    }
	break;

	default:
	break;
    }
    return my_paths[level];
}


// globsubst ("YaST::Foo::UI", "::", "/") == "YaST/Foo/UI"
string Y2PathSearch::globsubst (const string& where,
				const string& oldstr, const string& newstr)
{
    string ret;
    string::size_type olen = oldstr.length ();
    string::size_type p = 0, q;	// interval between occurences of oldstr
    for (;;)
    {
	q = where.find (oldstr, p); // find oldstr
	ret.append (where, p, q - p); // copy what is before it
	if (q == string::npos)
	{
	    break;
	}
	ret.append (newstr);
	p = q + olen;
    }
    return ret;
}


string
Y2PathSearch::completeFilename (const string& fname)
{
    string slashes = globsubst (fname, "::", "/");
    char* cfn = canonicalize_file_name (slashes.c_str ());
    string ret = cfn ? string (cfn) : fname;
    free (cfn);
    return ret;
}


string
Y2PathSearch::findy2 (string filename, int mode, int level)
{
    for (int i = 0; i < NUM_LEVELS; i++)
    {
	// for level == -1, all levels are scanned
	if ((level >= 0) && (i != level))
	{
	    continue;
	}

	string pathname (searchPath (GENERIC, i));
	if (pathname[pathname.length()-1] != '/') pathname += "/";
	pathname = completeFilename (pathname + filename);
	if (access (pathname.c_str(), mode) == 0)
	{
	    return pathname;
	}
    }
    return "";
}


string
Y2PathSearch::findy2exe (string root, string compname, bool server,
			 bool non_y2, int level)
{
    string subdir;
    if (server)
    {
	subdir = (non_y2 ? "/servers_non_y2/" : "/servers/");
    }
    else
    {
	subdir = (non_y2 ? "/clients_non_y2/" : "/clients/");
    }
    string pathname = root + searchPath (EXECCOMP, level) + subdir + compname;
    
    y2debug ("Trying file %s", pathname.c_str ());
    
    struct stat buf;
    if (stat (pathname.c_str (), &buf) == 0)
    {
	// Check at least if it is executable (for others) and
	// if it is a regular file.
	if (S_ISREG (buf.st_mode) && buf.st_mode & S_IXOTH == S_IXOTH)
	{
	    return pathname;
	}
    }

    return "";
}


string
Y2PathSearch::findy2plugin (string name, int level)
{
    // All plugins must follow this naming convention.
    string filename = searchPath (PLUGIN, level) + "/libpy2" + name + ".so.2";
    
    y2debug ("Testing existence of plugin %s", filename.c_str ());

    // Check if it is a regular file.
    struct stat buf;
    if (stat (filename.c_str (), &buf) == 0)
    {
	if (S_ISREG (buf.st_mode))
	{
	    return filename;
	}
    }
    return "";
}


int
Y2PathSearch::defaultComponentLevel ()
{
    for (int i = 0; i < NUM_LEVELS; i++)
    {
	if (searchPath (GENERIC, i) == YAST2DIR)
	{
	    return i;
	}
    }
    /* NOTREACHED */
    return NUM_LEVELS - 1;
}


int
Y2PathSearch::currentComponentLevel ()
{
    // Determine current component level
    int current_level = defaultComponentLevel ();
    char *levelstring = getenv ("Y2LEVEL");
    if (levelstring)
    {
	current_level = atoi (levelstring);
    }
    return current_level;
}


std::list<string> YCPPathSearch::searchList[YCPPathSearch::num_Kind];

bool YCPPathSearch::initialized = false;

// watch out, duplicated in char *paths[]
void
YCPPathSearch::initialize (Kind kind, const char *suffix)
{
    const char *home = getenv ("HOME");
    const char *y2dir = getenv ("Y2DIR");

    addPath (kind, string (YAST2DIR) + suffix);
    if (home)
    {
	string homey2 = string (home) + "/.yast2";
	addPath (kind, homey2 + suffix);
    }
    if (y2dir)
    {
	addPath (kind, string (y2dir) + suffix);
    }
    addPath (kind, string ("/y2update") + suffix);
}


void
YCPPathSearch::initialize ()
{
    if (! initialized)
    {
	initialize (Client, "/clients");
	initialize (Include, "/include");
	initialize (Module, "/modules");
	initialized = true;
    }
}


string
YCPPathSearch::find (Kind kind, const string& name)
{
    if (name[0] == '.'
	&& name[1] == '/')
    {
	if (access (name.c_str(), R_OK) == 0)
	{
	    return name;
	}
	return "";
    }

    initialize ();

    std::list<string>& kindList = searchList[kind];
    std::list<string>::iterator i = kindList.begin (), e = kindList.end ();
    while (i != e)
    {
	string pathname = completeFilename (*i + '/' + name);
	y2debug ("trying %s", pathname.c_str ());
	if (access (pathname.c_str(), R_OK) == 0)
	{
	    y2debug ("... success");
	    return pathname;
	}

	++i;
    }
    return "";
}


string
YCPPathSearch::findInclude (const string& name)
{
    return find (Include, name);
}


string
YCPPathSearch::findModule (string name, bool the_source)
{
    // TODO: more efficiently, do not search the whole string#
    string extension = (the_source ? ".ycp" : ".ybc");
    if (name.rfind (extension) == string::npos)
    {
	name += extension;
    }
    return find (Module, name);
}


void
YCPPathSearch::addPath (Kind kind, const string& path)
{
    std::list<std::string>::iterator it = searchList[kind].begin();
    while (it != searchList[kind].end())
    {
	if (*it == path)
	{
	    return;					// already in list, drop duplicate
	}
	it++;
    }
    searchList[kind].push_front (path);
}


void
YCPPathSearch::clearPaths (Kind kind)
{
    searchList[kind].clear();
}

std::list<string>::const_iterator
YCPPathSearch:: searchListBegin (Kind kind)
{
    return searchList[kind].begin ();
}

std::list<string>::const_iterator
YCPPathSearch:: searchListEnd (Kind kind)
{
    return searchList[kind].end ();
}

string
YCPPathSearch::bytecodeForFile (string filename)
{
    y2debug ("Testing existence of bytecode for %s", filename.c_str() );
    string ybc;
    
    // check the YCP extension
    if (filename.find_last_of ("ycp") == filename.length ()-1)
    {
	ybc = filename;
	ybc[ filename.length()-2 ] = 'b';
	ybc[ filename.length()-1 ] = 'c';
    }
    else
    {
	// no extension, no ybc lookup
	return "";
    }
    
    // check the modification times
    struct stat file_stat;
    if (stat (ybc.c_str (), &file_stat) != 0 || ! S_ISREG(file_stat.st_mode))
    {
	// return empty file
	return "";
    }
    time_t ybc_time = file_stat.st_mtime;
    
    if (stat (filename.c_str (), &file_stat) != 0 || ! S_ISREG(file_stat.st_mode))
    {
	// return empty file
	return "";
    }
    
    time_t ycp_time = file_stat.st_mtime;
    
    // compare the times
    if (ycp_time > ybc_time)
    {
	// too new
	return "";
    }
    
    return ybc;
}


// EOF
