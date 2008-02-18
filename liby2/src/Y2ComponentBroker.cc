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
#include <ycp/pathsearch.h>
#include <y2util/y2log.h>


vector<const Y2ComponentCreator *> *Y2ComponentBroker::creators[Y2ComponentBroker::MAX_ORDER]
= { 0, 0, 0, 0, 0 };

bool Y2ComponentBroker::stop_register = false;

map<const char*, const Y2Component*, Y2ComponentBroker::ltstr> Y2ComponentBroker::namespaces;
map<string, string> Y2ComponentBroker::namespace_exceptions;


void Y2ComponentBroker::registerComponentCreator(const Y2ComponentCreator *c, order_t order, bool force)
{
    initializeLists();

    // The "force" flag is needed by the UI component creator: It resides in a
    // plug-in (libpy2UI), so it can register itself only when that plug-in is
    // loaded, by which time registration is already closed. While stopping
    // registration might make sense for most plug-ins, in this special case it
    // does not: The UI component creator can create the "UI" name space as
    // well as the "qt", "ncurses", "gtk" UI servers.
    //
    // 2008-02-18 sh@suse.de
    
    if (!stop_register || force)
    {
	// y2debug( "Registering component creator at %p - force: %d", c, (int) force );
        creators[order]->push_back(c);
    }
}


Y2Component*
Y2ComponentBroker::createComponent (const char* name, bool look_for_clients)
{
    initializeLists();

    stop_register = true;

    y2debug ("Creating component \"%s\" as %s)", name, (look_for_clients ? "client" : "server"));

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
			creator->createInLevel (name, level,
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


Y2Component*
Y2ComponentBroker::getNamespaceComponent (const char* name)
{
    stop_register = true;

    y2debug ("getNamespace (%s)", name);

    map<const char*, const Y2Component*, ltstr>::iterator ci = namespaces.find (name);
    if ( ci != namespaces.end () )
    {
	const Y2Component *c = ci->second;
	y2debug ("namespace already registered by %p", c );
	return const_cast<Y2Component*>(c);
    }

    // first, try to lookup exception
    map<string, string>::iterator exi = namespace_exceptions.find (name);
    if (exi != namespace_exceptions.end ())
    {
	string comp_name = exi->second;

	// the ui stuff is a mess, must prefer servers
	Y2Component *comp = createServer (comp_name.c_str ());
	if (! comp) 
	{
	    // no server component, try client as well
	    comp = Y2ComponentBroker::createClient (comp_name.c_str ());
	}

	if (comp)
	{
	    y2debug ("Component %p used for namespace %s as an exception", comp, name);
	    return comp;
	}
	else
	{
	    y2warning ("Cannot create component based on exception list for namespaces!!!");
	}
    }
    
    
// uselessly repeats if it failed
//    for (int level = 0; level < Y2PathSearch::numberOfComponentLevels ();
//	 level++)
    {
	for (int order = 0; order < MAX_ORDER; order++)
	{
	    for (unsigned int i = 0; i < creators[order]->size(); i++)
	    {
		const Y2ComponentCreator *ccreator = (*creators[order])[i];
		Y2ComponentCreator *creator = const_cast<Y2ComponentCreator *> (ccreator);

		Y2Component *component =
		    creator->provideNamespace (name);

		if (component)
		{
		    // FIXME: Y2PathSearch::GENERIC is not correct (must depend on order)
		    y2debug ("Component %p used for namespace %s", component, name);
		    return component;
		}
	    }
	}
    }

    return 0;
}

bool Y2ComponentBroker::registerNamespaceException(const char* name_space, const char* component_name)
{
    map<const char*, const Y2Component*, ltstr>::iterator ci = namespaces.find (name_space);
    if ( ci != namespaces.end () )
    {
        const Y2Component *c = ci->second;
        y2error ("namespace %s already instantiated by %p", name_space, c );
        return false;
    }

    namespace_exceptions.insert ( std::pair<string,string>(name_space,component_name) );
    
    return true;
}


void Y2ComponentBroker::initializeLists ()
{
    if (creators[0] == 0) {
	for (int order=0; order<MAX_ORDER; order++) {
	    creators[order] = new vector<const Y2ComponentCreator *>;
	}
    }
}
