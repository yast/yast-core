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

   File:	pathsearch.h

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
		Martin Vidner <mvidner@suse.cz>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

/*
 * Search for YaST2 files at different paths.
 */

#include <stdio.h>
#include <fcntl.h>

#include <string>
#include <list>

using std::string;


class Y2PathSearch
{

public:

    /**
     * Returns the number of search paths.
     */
    static int numberOfComponentLevels ();

    /**
     * This enum is used as the first agrument for searchPath. It's needed
     * since some components need a special path treatment ("yast2dir" is
     * not valid for those).
     */
    enum WHAT { GENERIC, EXECCOMP, PLUGIN };

    /**
     * Returns the n-th searchpath (starting at 0). No range check is
     * performed.
     */
    static string searchPath (WHAT what, int level);

    /**
     * Determines the current component level by consulting the
     * environment variable Y2LEVEL
     */
    static int currentComponentLevel ();

    /**
     * Find the given file sequentially searched in the all search paths or
     * only in the one specified by level. Return complete path on success or
     * empty string on failure.
     */
    static string findy2 (string filename, int mode = R_OK, int level = -1);

    /**
     * Searchs in one of a list of paths for a yast2 executable. Give the
     * name compname relative to YAST2DIR. The flag server specifies, whether
     * a server or a client component should be searched for. The flag non_y2
     * specifies, whether a ycp program or a non ycp program like a shell
     * script should be searched for. Returns an empty string if no such
     * component was found. Otherwise returns a complete absolute path. The
     * level argument gives the number of the path to search in. Use
     * numberOfComponentLevels () to get the number of allowed paths.
     */
    static string findy2exe (string root, string compname, bool server,
			     bool non_y2, int level);

    /**
     * Search for a plugin in the specified level. On success return
     * the absolute filename of the plugin, otherwise a empty string.
     */
    static string findy2plugin (string name, int level);

    //! globsubst ("YaST::Foo::UI", "::", "/") == "YaST/Foo/UI"
    static string globsubst (const string& where,
			     const string& oldstr, const string& newstr);
    /**
     * Complete filename to start with "/". Will take the current working
     * directory into account.
     */
    static string completeFilename (const string& fname);

private:

    static int defaultComponentLevel ();

};

/**
 * A straightforward path search.
 * It should eventually replace the complex interplay of Y2PathSearch
 * and Y2ComponentBroker, Y2ComponentCreator.
 */
class YCPPathSearch : public Y2PathSearch
{
public:
    enum Kind
    {
	// would like to use all uppercase,
	// but the parser tokens are the allmighty #defines...
	Client,
	Include,
	Module,
	// all others: scrconf, menuentry, plugin, agent... ?
	num_Kind // last
    };

    /**
     * @param name the shortest should suffice..
     */
    static string find (Kind kind, const string& name);
    /**
     * @param name the shortest should suffice..
     */
    static string findInclude (const string& name);

    /**
     * @param name, name of module to find
     * @param the_source, if false and name does not end with ".ybc", it will be appended
     *                  , if true and name does not end with ".ycp", it will be appended
     * TODO module nesting (:: -> /)
     */
    static string findModule (string name, bool the_source = false);

    /**
     * prepends a path to the search list
     * @param kind, kind of files to find in path
     * @param path, string path to directory
     */
    static void addPath (Kind kind, const string& path);

    /**
     * clears the specified search list
     * @param kind, kind fo pathes to clear
     * used for '--nostdmodules' and '--nostdincludes' in ycpc
     */
    static void clearPaths (Kind kind);

    /**
     * Initializes the search paths if not done already.
     * Called automatically in each find.
     */
    static void initialize ();
    
    /**
     * Search for a YBC file corresponding to the given YCP file. It also
     * checks, if the YBC file is newer than the YCP.
     *
     * @param file the YCP file for which YBC file should be found
     * @return YBC file name or empty string if no YBC file was found
     */
    static string bytecodeForFile (string file);

private:
    static bool initialized;
    static std::list<string> searchList[num_Kind];
    static void initialize (Kind kind, const char *suffix);
};
