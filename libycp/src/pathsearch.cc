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
#include <boost/algorithm/string.hpp>

#include <ycp/y2log.h>

#include "pathsearch.h"


bool Y2PathSearch::searchPrefixWarn = true;


vector<string>
Y2PathSearch::getPaths()
{
    vector<string> ret;

    ret.push_back(string("/y2update"));

    const char* y2dir = getenv("Y2DIR");
    if (y2dir)
    {
	vector<string> y2dirs;
	boost::split(y2dirs, y2dir, boost::is_any_of(":"));
	for (vector<string>::const_iterator it = y2dirs.begin(); it != y2dirs.end(); ++it)
	{
	    if (*it != YAST2DIR)		// prevent path duplication
		ret.push_back(*it);
	}
    }

    const char* home = getenv("HOME");
    if (home)
    {
	ret.push_back(string(home) + "/.yast2");
    }

    ret.push_back(string(YAST2DIR));

    y2debug("getPaths %s", boost::join(ret, " ").c_str());

    return ret;
}


vector<string> Y2PathSearch::paths;


void
Y2PathSearch::initializePaths()
{
    if (paths.empty())
	paths = getPaths();
}


int
Y2PathSearch::numberOfComponentLevels()
{
    initializePaths();
    return paths.size();
}


string
Y2PathSearch::searchPath (WHAT what, int level)
{
    initializePaths();

    int levels = paths.size();

    switch (what)
    {
	case EXECCOMP:
	    if (level == levels - 1) // FIXME
	    {
		return EXECCOMPDIR;
	    }
	    else
	    {
		return paths[level];
	    }
	break;

	case PLUGIN:
	    if (level == levels - 1) // FIXME
	    {
		return PLUGINDIR;
	    }
	    else
	    {
		return paths[level] + "/plugin";
	    }
	break;

	default:
	break;
    }
    return paths[level];
}


string
Y2PathSearch::completeFilename (const string& fname)
{
    string slashes = boost::replace_all_copy(fname, "::", "/");
    char* cfn = canonicalize_file_name (slashes.c_str ());
    string ret = cfn ? string (cfn) : fname;
    free (cfn);
    return ret;
}


string
Y2PathSearch::findy2 (string filename, int mode, int level)
{
    initializePaths();

    int levels = paths.size();
    for (int i = 0; i < levels; i++)
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
	    // FIXME: this check is different for clients and for modules - see find
	    if( searchPrefixWarn && i != levels - 1 )
	    {
		y2warning( "Using special search prefix '%s' for '%s'",searchPath (GENERIC, i).c_str(), pathname.c_str() );
	    }
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
	if (S_ISREG (buf.st_mode) && (buf.st_mode & S_IXOTH == S_IXOTH))
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
    initializePaths();

    int levels = paths.size();
    for (int i = 0; i < levels; i++)
    {
	if (searchPath (GENERIC, i) == YAST2DIR)
	{
	    return i;
	}
    }
    /* NOTREACHED */
    return levels - 1;
}


int
Y2PathSearch::currentComponentLevel ()
{
    // Determine current component level
    int current_level = defaultComponentLevel ();
    const char* levelstring = getenv ("Y2LEVEL");
    if (levelstring)
    {
	current_level = atoi (levelstring);
    }
    return current_level;
}


std::list<string> YCPPathSearch::searchList[YCPPathSearch::num_Kind];

bool YCPPathSearch::initialized = false;


void
YCPPathSearch::initialize (Kind kind, const char *suffix)
{
    searchPrefixWarn = (getenv ("Y2SILENTSEARCH") == NULL);

    vector<string> paths = getPaths();
    for (vector<string>::const_reverse_iterator it = paths.rbegin(); it != paths.rend(); ++it)
	addPath(kind, *it + suffix);
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
	    // FIXME: this check is different for clients and for modules - see findy2
	    if( searchPrefixWarn && *i != kindList.back () ) {
		y2warning( "Using special search prefix '%s' for '%s'", i->c_str(), pathname.c_str() );
	    }
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
    string extension = (the_source ? ".ycp" : ".ybc");
    if (!boost::ends_with(name, extension))
    {
	name += extension;
    }
    return find (Module, name);
}


void
YCPPathSearch::addPath (Kind kind, const string& path)
{
    std::list<string>& l = searchList[kind];
    if (std::find(l.begin(), l.end(), path) == l.end())
	l.push_front(path);
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
