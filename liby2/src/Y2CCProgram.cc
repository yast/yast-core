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

   File:       Y2CCProgram.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
/*
 * Component Creator for external program components
 */

#include <config.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Y2CCProgram.h"
#include "Y2ProgramComponent.h"
#include <ycp/pathsearch.h>


Y2CCProgram::Y2CCProgram (bool server, bool non_y2)
    : Y2ComponentCreator (Y2ComponentBroker::EXTERNAL_PROGRAM),
      creates_servers (server),
      creates_non_y2 (non_y2)
{
}


bool Y2CCProgram::isServerCreator() const
{
    return creates_servers;
}


Y2Component *
Y2CCProgram::create (const char *name, int level, int current_level) const
{
    string root;

    if (strncmp (name, "chroot=", 7) == 0)
    {
	const char *p = index (name, ':');

	if (p) {
	    root = string (name, 7, p - name - 7);
	    name = p + 1;
	}
    }

    string file;

    if (index (name, '/') == NULL)	// no slash found
    {
	file = Y2PathSearch::findy2exe (root, name, creates_servers,
					creates_non_y2, level);
	if (file.empty ())		// not found
	    return 0;
    }
    else				// yes: found slash
    {
	if (creates_servers)
	    return 0;			// only clients may have full paths
	file = name;
    }

    struct stat buf;
    if (stat (file.c_str (), &buf) == 0)
    {
	// Check at least if it is executable (for others) and
	// if it is a regular file.
	if (S_ISREG (buf.st_mode) && buf.st_mode & S_IXOTH == S_IXOTH) {
	    if (!root.empty ())
		file = file.substr (root.length ());
	    return new Y2ProgramComponent (root, file.c_str (), name,
					   creates_non_y2, level);
	}
    }

    return 0;		// no such file or no read rights for directory
}


Y2CCProgram g_y2ccprogram0 (true, true);
Y2CCProgram g_y2ccprogram1 (true, false);
Y2CCProgram g_y2ccprogram2 (false, true);
Y2CCProgram g_y2ccprogram3 (false, false);
