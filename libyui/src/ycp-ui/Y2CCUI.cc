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

   File:       Y2CCUI.cc

   Author:     Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Component Creator that executes access to UI
 *
 * Author: Stanislav Visnovsky <visnov@suse.cz>
 */


#include <ycp/y2log.h>
#include <y2/Y2Component.h>
#include "Y2CCUI.h"
#include "YUIComponent.h"

Y2Component* Y2CCDummyUI::provideNamespace(const char* name)
{
    y2debug ("UI library namespace provider tries for '%s'", name);
    // implementation shortcut: we only provide the UI namsepace and the UI component
    return create(name);
}

Y2Component* Y2CCDummyUI::create(const char* name) const
{
    if (strcmp (name, "UI") == 0)
    {
	Y2Component* ret = YUIComponent::uiComponent ();
	if (ret == 0)
	{
	    y2milestone ("Creating UI library component");
	    ret = new YUIComponent;
	}
	
	return ret;
    }
    else
    {
	return NULL;
    }
}

// Create global variable to register this component creator
Y2CCDummyUI g_y2ccUI;
