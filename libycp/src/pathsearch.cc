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
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/stat.h>
#include <unistd.h>

#include <ycp/y2log.h>

#include "pathsearch.h"

static const char *paths[] =
{
    // "/media/floppy",		// Was a substitute for broken /y2update
    // "/y2update",		// Necessary during installation, but
				// can be achieved using -I and -M in
				// the yast2 start script
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
	    if (strcmp (paths[i], "HOME") == 0 && home)
		my_paths[i] = string (home) + "/.yast2";
	    else if (strcmp (paths[i], "Y2DIR") == 0 && y2dir)
		my_paths[i] = string (y2dir);
	    else
		my_paths[i] = string (paths[i]);
	}
    }

    switch (what)
    {
	case EXECCOMP:
	    if (level == NUM_LEVELS - 1) // FIXME
		return EXECCOMPDIR;
	    else
		return my_paths[level];
	    break;

	case PLUGIN:
	    if (level == NUM_LEVELS - 1) // FIXME
		return PLUGINDIR;
	    else
		return my_paths[level] + "/plugin";
	    break;

	default:
	    return my_paths[level];
    }
}


string
Y2PathSearch::completeFilename (const string& fname)
{
    char* cfn = canonicalize_file_name (fname.c_str ());
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
	    return pathname;
    }
    return "";
}


string
Y2PathSearch::findy2exe (string root, string compname, bool server,
			 bool non_y2, int level)
{
    string subdir;
    if (server)
	subdir = (non_y2 ? "/servers_non_y2/" : "/servers/");
    else
	subdir = (non_y2 ? "/clients_non_y2/" : "/clients/");

    string pathname = root + searchPath (EXECCOMP, level) + subdir + compname;

    struct stat buf;
    if (stat (pathname.c_str (), &buf) == 0)
    {
	// Check at least if it is executable (for others) and
	// if it is a regular file.
	if (S_ISREG (buf.st_mode) && buf.st_mode & S_IXOTH == S_IXOTH)
	    return pathname;
    }

    return "";
}


string
Y2PathSearch::findy2plugin (string name, int level)
{
    // All plugins must follow this naming convention.
    string filename = searchPath (PLUGIN, level) + "/libpy2" + name + ".so.2";

    // Check if it is a regular file.
    struct stat buf;
    if (stat (filename.c_str (), &buf) == 0)
	if (S_ISREG (buf.st_mode))
	    return filename;

    return "";
}


int
Y2PathSearch::defaultComponentLevel ()
{
    for (int i = 0; i < NUM_LEVELS; i++)
    {
	if (searchPath (GENERIC, i) == YAST2DIR)
	    return i;
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
	current_level = atoi (levelstring);
    return current_level;
}


std::list<string> YCPPathSearch::searchList[YCPPathSearch::num_Kind];


void
YCPPathSearch::initialize (Kind kind, const char *suffix)
{
    const char *home = getenv ("HOME");
    const char *y2dir = getenv ("Y2DIR");

    string homey2 = string (getenv ("HOME")) + "/.yast2"; //TODO what if unset?

    addPath (kind, string (YAST2DIR) + suffix);
    if (home)
    {
	addPath (kind, homey2 + suffix);
    }
    if (y2dir)
    {
	addPath (kind, string (y2dir) + suffix);
    }
}


void
YCPPathSearch::initialize ()
{
    initialize (Client, "/clients");
    initialize (Include, "/include");
    initialize (Module, "/modules");
}


string
YCPPathSearch::find (Kind kind, const string& name)
{
    if (name[0] == '.'
	&& name[1] == '/')
    {
	if (access (name.c_str(), R_OK) == 0)
	    return name;
	return "";
    }

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
YCPPathSearch::findModule (string name)
{
    // TODO: more efficiently, do not search the whole string
    if (name.rfind (".ybc") == string::npos)
    {
	name += ".ybc";
    }
    return find (Module, name);
}


void
YCPPathSearch::addPath (Kind kind, const string& path)
{
    searchList[kind].push_front (path);
}
