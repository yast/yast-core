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

Import::Import ()
{
}


Import::Import (const string &name, Y2Namespace *preloaded_namespace)
{
    if (import (name, preloaded_namespace) != 0)
    {
	m_name = "";			// mark as failed import
    }
}


Import::~Import ()
{
}


int
Import::import (const string &name, Y2Namespace *preloaded_namespace)
{
    if (!m_name.empty())
    {
	ycp2error ("Import::import(%s) called again but already initialized with '%s'", name.c_str(), m_name.c_str());
	return -1;
    }

    y2debug ("Import::import (%s), preloaded_namespace %p", name.c_str(), preloaded_namespace);

    m_name = name;

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
		return -1;
	    }

	    name_space = comp->import (cname);						// do the bytecode import
	    if (name_space == NULL)
	    {
		ycp2error ("Loading module '%s' failed", cname);
		return -1;
	    }
	}

	table = name_space->table();
	if (table == 0)
	{
	    ycp2error ("No table associated to module '%s'", cname);
	    return -1;
	}

	m_active_modules.insert (std::make_pair (m_name, name_space));			// insert to list of known modules
	y2debug ("Module '%s' loaded, name_space @%p, table @%p", cname, name_space, table);

	m_module = m_active_modules.find (m_name);

	table->startUsage();
    }
    else
    {
	table = m_module->second->table();
	y2debug ("Module '%s' already loaded, name_space %p, table %p", m_name.c_str(), m_module->second, table);
	if (table == 0)
	{
	    fprintf (stderr, "Oops, no table for already loaded module '%s'\n", m_name.c_str());
	    exit (1);
	}
    }

    if (m_disable_tracking)
    {
	table->disableUsage();
	m_table_stack.push (std::make_pair (m_name, table));
    }

    return 0;
}


int Import::m_disable_tracking = 0;
std::stack <std::pair <string, SymbolTable *> > Import::m_table_stack;


void
Import::disableTracking ()
{
    m_disable_tracking++;
    y2debug ("Import::disableTracking (%d)", m_disable_tracking);
    return;
}


void
Import::enableTracking ()
{
    m_disable_tracking--;
    y2debug ("Import::enableTracking (%d)", m_disable_tracking);
    if (m_disable_tracking > 0)
    {
	return;
    }
    while (!m_table_stack.empty())
    {
	SymbolTable *table = m_table_stack.top().second;
	y2debug ("enableUsage (%s:%p)", m_table_stack.top().first.c_str(), table);
	table->enableUsage();
	m_table_stack.pop();
    }
    return;
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
	return m_module->second;
}


// EOF
