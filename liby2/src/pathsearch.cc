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

/-*/

#define _GNU_SOURCE	// for get_current_dir_name ()
#include <sys/stat.h>
#include <unistd.h>

#include <ycp/y2log.h>

#include "pathsearch.h"


static const char *paths[] =
{
    "/media/floppy",
    "/y2update",
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
    // y2debug ("complete_filename (%s)", fname.c_str());
    if (fname.empty() || (fname[0] == '/'))
	return fname;

    char* tmp1 = get_current_dir_name ();
    string cwd = tmp1;
    free (tmp1);

    const char *fptr = fname.c_str();
    // y2debug ("complete_filename cwd (%s)", cwd.c_str());

    // scan filename, stripping leading ./ and ../
    // for every ../, remove a trailing path from cwd

    while (*fptr == '.')
    {
	fptr++;
	if (*fptr == '/')
	{
	    // skip "./"
	    fptr++;
	    continue;
	}
	else if (*fptr == '.')
	{
	    // handle ".."
	    fptr++;
	    if (*fptr != '/')
	    {
		// .. without /
		fptr--;
		y2error ("Bad filename prefix (%s)", fptr);
		break;
	    }

	    fptr++;	// skip '/'

	    // handle "../", adapt cwd accordingly
	    string::size_type slashpos = cwd.rfind ('/');
	    if (slashpos == string::npos)
	    {
		fptr--;
		y2error ("Filename (%s) doesn't match cwd (%s)", fptr, cwd.c_str());
		break;
	    }
	    cwd.resize (slashpos);
	}
	else
	{
	    // just leading dot, keep it.
	    break;
	}
    }

    if (cwd == "/")
    {
	cwd = "";
    }

    // y2debug ("complete_filename -> %s/%s", cwd.c_str(), fptr);

    return cwd + "/" + string (fptr);
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
