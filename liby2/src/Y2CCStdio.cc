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

   File:       Y2CCStdio.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
/*
 * Component Creator that creates stdio components
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "Y2CCStdio.h"
#include "Y2StdioComponent.h"

#include <ycp/y2log.h>

Y2CCStdio::Y2CCStdio (bool creates_servers, bool to_stderr)
    : Y2ComponentCreator (Y2ComponentBroker::BUILTIN),
      creates_servers (creates_servers),
      to_stderr (to_stderr)
{
}


bool Y2CCStdio::isServerCreator () const
{
    return creates_servers;
}


Y2Component *Y2CCStdio::create (const char *name) const
{
    if (strcmp (name, "stdio") == 0 && !to_stderr)
	return new Y2StdioComponent (creates_servers, false);

    if (strcmp (name, "stderr") == 0 && to_stderr)
	return new Y2StdioComponent (creates_servers, true);

    if (strcmp (name, "testsuite") == 0 && to_stderr)
	return new Y2StdioComponent (creates_servers, false, true);

    return 0;
}


Y2Component*
Y2CCStdio::provideNamespace(const char*)
{
    y2debug ("Y2StdioComponent cannot import namespaces");
    return NULL;
}


Y2CCStdio g_y2ccstdio0 (false, false);
Y2CCStdio g_y2ccstdio1 (true, false);
Y2CCStdio g_y2ccstdio2 (false, true);
Y2CCStdio g_y2ccstdio3 (true, true);
