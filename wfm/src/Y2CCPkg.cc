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

   File:       Y2CCPkg.cc

   Author:     Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Component Creator that executes access to packagemanager
 *
 * Author: Stanislav Visnovsky <visnov@suse.cz>
 */


#include <ycp/y2log.h>
#include <y2/Y2Component.h>
#include "Y2CCPkg.h"
#include "Y2PkgComponent.h"

Y2Component *Y2CCPkg::createInLevel(const char *name, int level, int) const
{
    if (strcmp (name, "pkg") == 0)
    {
	return Y2PkgComponent::instance ();
    }
    else
    {
	return NULL;
    }
}

bool Y2CCPkg::isServerCreator() const
{
    return false;
}

Y2Component* Y2CCPkg::provideNamespace(const char* name)
{
    if (strcmp (name, "Pkg") == 0)
    {
	return Y2PkgComponent::instance ();
    }
    else
    {
	return NULL;
    }
}

// Create global variable to register this component creator

Y2CCPkg g_y2ccPkg;
