/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                          Copyright (c) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	Import.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/
// -*- c++ -*-

#include <libintl.h>

#include "ycp/Import.h"
#include "ycp/YBlock.h"

#include "ycp/y2log.h"

#include "y2/Y2Component.h"
#include "y2/Y2ComponentBroker.h"


//-------------------------------------------------------------------
// import

// static Import member
Import::module_map Import::m_active_modules;

Import::Import (const string &name, Y2Namespace *preloaded_namespace, bool from_stream)
    : m_name (name)
{
    y2debug ("Import::Import (%s), preloaded_namespace %p, from_stream %d", name.c_str(), preloaded_namespace, from_stream);
    m_module = m_active_modules.find (m_name);

    SymbolTable *table;

    if (m_module == m_active_modules.end())
    {
	const char *cname = name.c_str();
	Y2Namespace* name_space = preloaded_namespace;

	if (name_space == 0)
	{
	    y2debug ("Loading module '%s'", cname);
	    Y2Component* comp = Y2ComponentBroker::getNamespaceComponent (cname);	// find component for name

	    if (comp == 0)
	    {
		ycp2error ("Loading module '%s' failed", cname);
		y2error ("No matching component found");
		return;
	    }

	    name_space = comp->import (cname);						// do the bytecode import
	    if (name_space == NULL)
	    {
		ycp2error ("Loading module '%s' failed", cname);
		return;
	    }
	}

	table = name_space->table();
	if (table == 0)
	{
	    ycp2error ("No table associated to module '%s'", cname);
	    return;
	}

	SymbolEntry *constructor = 0;							// look for constructor
	TableEntry *tentry = table->find (cname, SymbolEntry::c_function);
	if (tentry != 0)
	{
	    constructor = tentry->sentry();
	}
	module_entry me = { name_space, constructor, false };
	m_active_modules.insert (std::make_pair (m_name, me));				// insert to list of known modules
	y2debug ("Module '%s' loaded, name_space @%p, table @%p, constructor @%p", cname, name_space, table, constructor);

	m_module = m_active_modules.find (m_name);

	table->startUsage();
    }
    else
    {
	table = m_module->second.name_space->table();
	y2debug ("Module '%s' already loaded, name_space %p, table %p", m_name.c_str(), m_module->second.name_space, table);
	if (table == 0)
	{
	    fprintf (stderr, "Oops, no table for already loaded module '%s'\n", m_name.c_str());
	    exit (1);
	}
    }
}


Import::~Import ()
{
}


string
Import::name () const
{
    return m_name;
}


Y2Namespace *
Import::nameSpace () const
{
    if (m_module == m_active_modules.end ())
	return NULL;
    else
	return m_module->second.name_space;
}


// EOF
