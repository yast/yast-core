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

   File:       YCPScopeInstance.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * symbol scope handler for YCP
 *
 */

#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>

#include "ycp/YCPScopeInstance.h"
#include "ycp/y2log.h"

static int scopeDebugEnabled = 0;

static void scopeDebug(const char *message, ...)
{
    if (scopeDebugEnabled != 0)
    {
	// Prepare info text
	va_list ap;
	va_start(ap, message);
	Y2Logging::y2_vlogger (LOG_MILESTONE, "instance", "",0,"", message, ap);
	va_end(ap);
    }
    return;
}

//-------------------------------------------------------------------

// YCPScopeInstance

YCPScopeInstance::YCPScopeInstance()
{
    level = 0;
    container = 0;
    outer = 0;
    inner = 0;
    if (getenv ("SCOPEDEBUG") != 0)
    {
	scopeDebugEnabled = 1;
    }
}


YCPScopeInstance::~YCPScopeInstance()
{
    if (container != 0)
    {
	YCPScopeContainer::const_iterator var;

	for (var = container->begin(); var != container->end(); ++var)
	{
	    delete var->second;
	}

	delete container;
    }

    container = 0;
}


bool YCPScopeInstance::empty() const
{
    return (((container==0) || container->empty())?true:false);
}

// declare symbol in specific scope , create container

int YCPScopeInstance::declareSymbol (const string& s, const YCPDeclaration& d,
				     const YCPValue& v, bool as_const, int line)
{
    scopeDebug ("declareSymbol %d@%p (%s%s|%s = %s)",
	level,
	this,
	(as_const?"const ":""),
	d->toString().c_str(),
	s.c_str(),
	v->toString().c_str());

    // if this is the first declaration for this level,
    // do the scope initialization now.

    if (container == 0)
    {
	scopeDebug ("create map");
	container = new YCPScopeContainer;
	assert (container != 0);
    }

    int old_line = 0;

    YCPScopeContainer::iterator it = container->find(s);
    if (it != container->end())
    {
	old_line = it->second->get_line();

	// OPTION: only allow overwrite in global instance

	// overwrite old definition
	delete it->second;
	it->second = new YCPScopeElement (d, v, as_const, line);
	if ((old_line > 0) && (old_line == line))
	{
	    old_line = 0;	// no warning if same line
	}
    }
    else
    {
	// insert new definition
	(*(container))[s] = new YCPScopeElement (d, v, as_const, line);
    }

    return old_line;
}


// Removes a symbol from the instance.

bool YCPScopeInstance::removeSymbol (const string& s)
{
    scopeDebug (":removeSymbol(%d)", level);
    if (container)
    {
	YCPScopeContainer::iterator it = container->find(s);
	if (it != container->end())
	{
	    delete it->second;
	    container->erase (it);
	    return true;
	}
    }
    return false;
}

#if 0
int YCPScopeInstance::symbolLine (const string& symbolname) const
{
    scopeDebug (":symbolLine(%d@%s)", level, symbolname.c_str());
    if (container)
    {
	YCPScopeContainer::iterator it = container->find(s);
	if (it != container->end())
	{
	    return it->second->get_line();
	}
    }
    return 0;
}
#endif

void YCPScopeInstance::dumpScope () const
{
    scopeDebug ("dumpScope(%d@%p)", level, this);

    if (container == 0)
	return;

    YCPScopeContainer::const_iterator var;

    fprintf (stderr, "-- level %d --\n", level);
    for (var = container->begin(); var != container->end(); ++var)
    {
	YCPDeclaration decl = var->second->get_type();
	fprintf (stderr, "   %-8s %-15s = %s\n",
		decl->toString().c_str(),
		(decl->isDeclTerm()?"":var->first.c_str()),
		var->second->get_value()->toString().c_str());
	    fprintf (stderr, "\n");
    }
    fprintf (stderr, "-- end --\n");

    return;
}


YCPValue YCPScopeInstance::lookupValue (const string& symbolname) const
{
    scopeDebug ("lookupValue (%d)", level);
    if (container)
    {
	YCPScopeContainer::const_iterator it = container->find(symbolname);
	if (it != container->end())
	{
	    scopeDebug ("lookupValue found");
	    return it->second->get_value();
	}
    }
    scopeDebug ("lookupValue NOT found");
    return YCPNull();
}


YCPDeclaration YCPScopeInstance::lookupDeclaration (const string& symbolname) const
{
    scopeDebug ("lookupDeclaration @%p (%d:'%s')", this, level, symbolname.c_str());
    if (container)
    {
	YCPScopeContainer::const_iterator it = container->find(symbolname);
	if (it != container->end())
	{
	    if (it->second->get_type()->isDeclaration())
	    {
		scopeDebug ("lookupDeclaration found");
		return it->second->get_type();
	    }
	    else
	    {
		scopeDebug ("lookupDeclaration found a non-declaration");
	    }
	}
	else
	{
	    scopeDebug ("symbol not defined in scope");
	}
    }
    else
    {
	scopeDebug ("scope is empty");
    }

    scopeDebug ("lookupDeclaration NOT found");
    return YCPNull();
}


// returns:
// Null -> not found
// Void -> ok
// Declaration -> bad type

YCPValue YCPScopeInstance::assignSymbol (const string &symbolname, const YCPValue &newvalue)
{
    scopeDebug ("assignSymbol (%d:%s)", level, symbolname.c_str());

    if (container == 0)		// undeclared symbol
	return YCPNull();

    YCPScopeContainer::const_iterator symdef = container->find(symbolname);

    if (symdef != container->end())	// found a symbol with that name
    {
	YCPDeclaration declaration = symdef->second->get_type();
	if (newvalue->isVoid () && declaration->isDeclType())
	{
	    // assigning nil to any value is allowed !
	    symdef->second->set_value (YCPVoid ());
	}
	else if (!declaration->allows(newvalue))
	{
	    return declaration;
	}
	else
	{
	    symdef->second->set_value (newvalue);
	}

	return YCPVoid();
    }
    return YCPNull();
}

/* EOF */
