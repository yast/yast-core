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

   File:	Y2ComponentBroker.cc

   Purpose:	Find Y2 components

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include <stdio.h>
#include <dlfcn.h>

#include "Y2ComponentBroker.h"
#include "Y2ComponentCreator.h"
#include "pathsearch.h"
#include <ycp/y2log.h>


vector<const Y2ComponentCreator *> *Y2ComponentBroker::creators[Y2ComponentBroker::MAX_ORDER]
= { 0, 0, 0, 0, 0 };

bool Y2ComponentBroker::stop_register = false;


void Y2ComponentBroker::registerComponentCreator(const Y2ComponentCreator *c, order_t order)
{
    initializeLists();

    if (!stop_register)
        creators[order]->push_back(c);
}


Y2Component*
Y2ComponentBroker::createComponent (const char* name, bool look_for_clients)
{
    initializeLists();

    stop_register = true;

    y2debug ("createComponent (%s, %s)", name, (look_for_clients ? "Client" : "Server"));

    for (int level = 0; level < Y2PathSearch::numberOfComponentLevels ();
	 level++)
    {
	for (int order = 0; order < MAX_ORDER; order++)
	{
	    for (unsigned int i = 0; i < creators[order]->size(); i++)
	    {
		const Y2ComponentCreator *creator = (*creators[order])[i];

		if ( ( look_for_clients && creator->isClientCreator ()) ||
		     (!look_for_clients && creator->isServerCreator ()) )
		{
		    Y2Component *component =
			creator->create (name, level,
					 Y2PathSearch::currentComponentLevel ());

		    if (component)
		    {
			// FIXME: Y2PathSearch::GENERIC is not correct (must depend on order)
			y2debug ("Component %s (%s) created in level = %i (%s), order = %i",
				 name, look_for_clients ? "client" : "server", level,
				 Y2PathSearch::searchPath (Y2PathSearch::GENERIC, level).c_str (), order);
			return component;
		    }
		}
	    }
	}
    }

    return 0;
}


Y2Component *Y2ComponentBroker::createClient (const char *name)
{
    return createComponent (name, true);
}


Y2Component *Y2ComponentBroker::createServer (const char *name)
{
    return createComponent (name, false);
}


void Y2ComponentBroker::initializeLists ()
{
    if (creators[0] == 0) {
	for (int order=0; order<MAX_ORDER; order++) {
	    creators[order] = new vector<const Y2ComponentCreator *>;
	}
    }
}
