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

   File:       Y2SystemNamespace.cc

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Base class of all Y2 namespaces providing a default implementations
 *
 */


#include <y2util/y2log.h>
#include <ycp/SymbolTable.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPString.h>

#include <y2/Y2ProgramComponent.h>

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "Y2StdioFunction.h"
#include "Y2SystemNamespace.h"
#include "Y2SystemFunction.h"
#include "ycp/SymbolTable.h"

Y2SystemNamespace::Y2SystemNamespace (Y2Namespace* local_ns)
    : Y2Namespace()
    , m_local_ns (local_ns)
{
    createTable ();
    local_ns->table ()->tableCopy (this);
}


Y2SystemNamespace::~Y2SystemNamespace ()
{
    m_table = 0;
    // FIXME: what about the m_localns ?
}


const string
Y2SystemNamespace::name () const
{
    return m_name;
}

const string
Y2SystemNamespace::filename () const
{
    return "NO FILE";
}

YCPValue
Y2SystemNamespace::evaluate (bool cse) 
{
    // run the local constructor
    m_local_ns->evaluate (cse);

    return YCPVoid ();
}


Y2Function* 
Y2SystemNamespace::createFunctionCall (const string name, constFunctionTypePtr type)
{
    TableEntry *func_te = table()->find (name.c_str (), SymbolEntry::c_function);
    
    // can't find the function definition
    if (!func_te->sentry ()->isFunction ())
        return 0;

    Y2Function* local_func = m_local_ns->createFunctionCall (name, type);
    if (local_func == 0)
    {
	return 0;
    }
    
    y2milestone ("allocating new Y2SystemFunction %s", name.c_str ());

    Y2SystemFunction* fnc = new Y2SystemFunction (local_func, type);
    m_functions.push_back (fnc);

    return fnc;
}


void
Y2SystemNamespace::useRemote (Y2ProgramComponent* sender)
{
    y2milestone ("redirecting to %s", sender->name ().c_str ());
    for ( vector<Y2SystemFunction*>::iterator it = m_functions.begin ();
	it != m_functions.end () ; ++it )
    {
	y2milestone ("Redirected: %s", (*it)->name ().c_str ());
	(*it)->useRemote (new Y2StdioFunction (
	    m_local_ns->name ()
	    , (*it)->name ()
	    , (*it)->type ()
	    , sender));
    }
}


void
Y2SystemNamespace::useLocal ()
{
    y2milestone ("redirecting to local");
    for ( vector<Y2SystemFunction*>::iterator it = m_functions.begin ();
	it != m_functions.end () ; ++it )
    {
	y2milestone ("Redirected: %s", (*it)->name ().c_str ());
	(*it)->useLocal ();
    }
}
