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

   File:       YCPScope.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * symbol scope handler for YCP
 *
 */

#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include <stdarg.h>

#include "YCPScopeInstance.h"
#include "YCPScope.h"
#include "y2log.h"

void YCPScope::scopeDebug(const char *message, ...) const
{
    if (scopeDebugEnabled != 0)
    {
	// Prepare info text
	va_list ap;
	va_start(ap, message);
	Y2Logging::y2_vlogger (LOG_MILESTONE, "scope", current_file.c_str(), current_line,"", message, ap);
	va_end(ap);
    }
    return;
}


//-------------------------------------------------------------------
// YCPScope

YCPScope::YCPScope()
    : scopeDebugEnabled (0)
    , current_line (0)
{
    if (getenv ("SCOPEDEBUG") != 0)
    {
	scopeDebugEnabled = 1;
    }
    scopeLevel = -1;			// next level is 0
    scopeTop = 0;
    instances = 0;

    // ensure existance of global scope
    openScope();
    scopeGlobal = new YCPScopeInstance;	// new scope at top
    assert (scopeGlobal != 0);
    scopeGlobal->container = 0;
    scopeGlobal->outer = 0;
    scopeGlobal->inner = 0;
    scopeGlobal->level = scopeLevel;
    scopeTop = scopeGlobal;
}


YCPScope::~YCPScope()
{
    while (scopeTop != 0)
    {
	YCPScopeInstance *next = scopeTop->outer;
	delete scopeTop;
	scopeTop = next;
    }

    if (instances != 0)
    {
	YCPScopeInstanceContainer::const_iterator var;

	for (var = instances->begin(); var != instances->end(); ++var)
	{
	    delete var->second;
	}

	delete instances;
    }

    instances = 0;
}


// create a new named instance
//

bool YCPScope::createInstance(const string& scopename)
{
    scopeDebug ("createInstance(%s)", scopename.c_str());

    if (instances == 0)
    {
	instances = new YCPScopeInstanceContainer;
	assert (instances != 0);
    }

    YCPScopeInstanceContainer::iterator it = instances->find(scopename);
    if (it == instances->end())
    {
	YCPScopeInstance *instance = new YCPScopeInstance;
	// insert new definition
	(*(instances))[scopename] = instance;

	scopeDebug ("switching global was %p now %p", scopeGlobal, instance);
	global_stack.push_back(scopeGlobal);
	scopeGlobal = instance;
	scopeDebug ("switching top was %p now %p", scopeTop, instance);
	top_stack.push_back(scopeTop);
	scopeTop = scopeGlobal;
	level_stack.push_back(scopeLevel);
	return true;
    }
    return false;
}


// open a new named instance
//

bool YCPScope::openInstance(const string& scopename)
{
    scopeDebug ("openInstance(%s)", scopename.c_str());

    if (instances == 0)
    {
	ycp2error (current_file.c_str(), current_line, "No instances known, missing 'import %s' ?", scopename.c_str());
	return false;
    }

    YCPScopeInstanceContainer::iterator it = instances->find(scopename);
    if (it == instances->end())
    {
	ycp2error (current_file.c_str(), current_line, "Instance '%s' not known, , missing 'import' ?", scopename.c_str());
	return false;
    }
    else
    {
	YCPScopeInstance *instance = it->second;

	scopeDebug ("switching global was %p now %p", scopeGlobal, instance);
	global_stack.push_back(scopeGlobal);
	scopeGlobal = instance;
	if (instance->inner != 0)
	    instance = instance->inner;
	scopeDebug ("switching top was %p now %p", scopeTop, instance);
	top_stack.push_back(scopeTop);
	scopeTop = instance;
	level_stack.push_back(scopeLevel);
	scopeLevel = scopeTop->level;
    }
    return true;
}


// close named instance
//

void YCPScope::closeInstance(const string& scopename)
{
    scopeDebug ("closeInstance(%s)", scopename.c_str());

    if (instances != 0)
    {
	YCPScopeInstanceContainer::iterator it = instances->find(scopename);
	if (it == instances->end())
	{
	    ycp2error (current_file.c_str(), current_line, "Instance '%s' not found", scopename.c_str());
	}
	else
	{
	    YCPScopeInstance *module_instance = it->second;
	    if (module_instance == 0
		|| module_instance->empty())
	    {
		ycp2error (current_file.c_str(), current_line, "Empty module '%s'", scopename.c_str());
	    }
	    else
	    {
		// check if module scope has local declarations
		// if not, delete the local scope

		YCPScopeInstance *local_instance = module_instance->inner;
		if (local_instance != 0
		    && local_instance->empty())
		{
		    delete local_instance;
		    module_instance->inner = 0;
		}
	    }

	    scopeGlobal = global_stack.back();
	    global_stack.pop_back();
	    scopeTop = top_stack.back();
	    top_stack.pop_back();
	    scopeLevel = level_stack.back();
	    level_stack.pop_back();
	    scopeDebug ("restoring global/top to %p/ %p", scopeGlobal, scopeTop);
	}
    }

    return;
}


// delete named instance
//

void YCPScope::deleteInstance(const string& scopename)
{
    scopeDebug ("deleteInstance(%s)", scopename.c_str());

    if (instances != 0)
    {
	YCPScopeInstanceContainer::iterator it = instances->find(scopename);
	if (it != instances->end())
	{
	    if (it->second->inner)
		delete it->second->inner;
	    delete it->second;
	    instances->erase (it);
	}
    }
    return;
}


// open new scope
// just increase level here, create ScopeInstance on first declareSymbol()

void YCPScope::openScope()
{
    scopeLevel++;
    scopeDebug ("openScope(%d@%p), scopeLevel %d", (scopeTop?scopeTop->level:-1), this, scopeLevel);
}


// close scope
// check if a symbol was declared and do a real close
// decrease level

void YCPScope::closeScope()
{
    scopeDebug ("closeScope(%d), scopeLevel %d", (scopeTop?scopeTop->level:-1), scopeLevel);

    // never destroy the topmost scope

    YCPScopeInstance *top = scopeTop;

    // only destroy if top is defined and has the current level
    //   N.B. top usually has a lower level if no symbols were
    //	      defined since the last openScope()

    if (top && top->level == scopeLevel)
    {
	if (top->outer != 0)
	{
	    scopeTop = top->outer;
	    scopeTop->inner = 0;
	    delete top;
	}
	scopeDebug ("destroyScope (%d), top @ %d !", scopeLevel, scopeTop->level);

    }
    scopeLevel--;

    assert (scopeLevel >= 0);
    return;
}


// declare symbol, create container

void YCPScope::declareSymbol (const string& s, const YCPDeclaration& d, const YCPValue& v,
			      bool globally, bool as_const, bool do_warn)
{
    scopeDebug ("declareSymbol (%d:%s%s%s)@%p", scopeTop?scopeTop->level:-1, (globally?"global ":""), (as_const?"const ":""), s.c_str(), this);

    YCPScopeInstance *instance = scopeTop;
    bool new_level = false;

    if (globally)
    {
	// warn if new global symbol shadows non-global symbol

	if (!symbolDeclaredGlobally(s)
	    && symbolDeclared(s))
	{
	    ycp2warning (current_file.c_str(), current_line, "Global symbol '%s' shadows local symbol\n", s.c_str());
	}
	instance = scopeGlobal;
	assert (instance != 0);
    }
    else if (scopeTop->level < scopeLevel)
    {
	scopeTop = new YCPScopeInstance;		// new scope at top
	assert (scopeTop != 0);
	scopeTop->container = 0;
	scopeTop->outer = instance;
	instance->inner = scopeTop;
	scopeTop->level = scopeLevel;
	scopeDebug ("create scope, old %p@%d, new %p@%d", instance, instance->level, scopeTop, scopeTop->level);
	instance = scopeTop;
	new_level = true;
    }

    if (instance == 0)
    {
	ycp2error (current_file.c_str(), current_line, "FATAL: No environment found");
    }
    else
    {
	if (do_warn
	    && !globally
	    && !scopeGlobal->lookupValue(s).isNull())
	{
	    ycp2warning (current_file.c_str(), current_line, "Local symbol '%s' shadows global symbol\n", s.c_str());
	}

	int old_line = instance->declareSymbol (s, d, v, as_const, current_line);
	if (!globally && old_line > 0)
	{
	    ycp2error (current_file.c_str(), current_line, "Symbol '%s' already declared at line %d\n", s.c_str(), old_line);
	}
    }

    return;
}


// remove symbol from innermost scope
// look for scope which defines symbol
// return on first successful remove
//
void YCPScope::removeSymbol (const string& s)
{
    scopeDebug ("removeSymbol (%d:%s)", scopeTop?scopeTop->level:-1, s.c_str());

    YCPScopeInstance *instance = scopeTop;

    while (instance)
    {
	if (instance->removeSymbol(s))
	    break;
	instance = instance->outer;
    }
    return;
}


const YCPValue YCPScope::dumpScope(const YCPList& args) const
{
    scopeDebug ("dumpScope(%s)", args->toString().c_str());

    /**
     * @builtin _dump_scope() -> void
     * Use this for debugging. It will dump a list of all definitions
     * in all scopes to stderr.
     */
    YCPScopeInstance *instance = scopeTop;

    if ((args->size() == 1)
	&& args->value(0)->isString())
    {
	string dumpname = args->value(0)->asString()->value();
	scopeDebug ("dumpname (%s)", dumpname.c_str());
	if (instances)
	{
	    YCPScopeInstanceContainer::iterator it = instances->find(dumpname);
	    if (it != instances->end())
	    {
		scopeDebug ("found !");
		instance = it->second;
		instance->dumpScope();
		if (instance->inner)
		    instance->inner->dumpScope();
		instance = 0;
	    }
	    else
	    {
		scopeDebug ("dumpScope(%s) not found", dumpname.c_str());
	    }
	}
	else
	{
	    scopeDebug ("dumpScope(%s) no instances", dumpname.c_str());
	}
	return YCPVoid();
    }

    while (instance)
    {
	instance->dumpScope ();
	if (args->size() == 0)
	    break;
	instance = instance->outer;
    }

    return YCPVoid();
}


const YCPValue YCPScope::lookupValue (const string& symbolname, const string& scopename) const
{
    scopeDebug ("lookupValue (%d#%s::%s)", (scopeTop?scopeTop->level:-1), scopename.c_str(), symbolname.c_str());

    const YCPScopeInstance *instance = scopeTop;

    if (!scopename.empty())
    {
	if (scopename == "_")
	    instance = scopeGlobal;
	else
	    instance = lookupInstance (scopename);
    }

    while (instance)
    {
	YCPValue value = instance->lookupValue(symbolname);
	if (!value.isNull())
	{
	    return value;
	}
	instance = instance->outer;
    }
    return YCPNull();
}


const YCPDeclaration YCPScope::lookupDeclaration (const string& symbolname, const string& scopename) const
{
    const YCPScopeInstance *instance = scopeTop;

    if (!scopename.empty())
    {
	if (scopename == "_")
	    instance = scopeGlobal;
	else
	    instance = lookupInstance (scopename);
    }

    scopeDebug ("lookupDeclaration %d@%p ('%s')", (scopeTop?scopeTop->level:-1), instance, symbolname.c_str());

    while (instance)
    {
	YCPDeclaration declaration = instance->lookupDeclaration(symbolname);

	if (!declaration.isNull())
	    return declaration;
	instance = instance->outer;
    }

    return YCPNull();
}


// lookup in current scope only

bool YCPScope::symbolDeclaredLocally (const string& symbolname) const
{
    scopeDebug ("symbolDeclaredLocally(%s), top %d", symbolname.c_str(), scopeLevel);

    // check if any scope opened so far
    if (scopeTop == 0)
	return false;

    // check if current scope is open (i.e. has declared symbols)
    if (scopeTop->level < scopeLevel)
	return false;

    if (scopeTop->lookupValue(symbolname).isNull())
	return false;

    return true;
}


// lookup in global scope only

bool YCPScope::symbolDeclaredGlobally (const string& symbolname) const
{
    scopeDebug ("symbolDeclaredGlobally(%s)", symbolname.c_str());

    // check if any scope opened so far
    if (scopeGlobal == 0)
	return false;

    if (scopeGlobal->lookupValue(symbolname).isNull())
	return false;

    return true;
}


// recursively traverse scope stack up

bool YCPScope::symbolDeclared (const string& symbolname, const string& scopename) const
{
    if (lookupValue (symbolname, scopename).isNull())
	return false;

    return true;
}


const YCPValue YCPScope::assignSymbol(const string &symbolname, const YCPValue &newvalue, const string& scopename)
{
    scopeDebug ("assignSymbol (%d:[%s::]%s)", scopeTop?scopeTop->level:-1, scopename.c_str(), symbolname.c_str());

    YCPScopeInstance *instance;

    if (scopename.empty())
    {
	instance = scopeTop;
    }
    else
    {
	instance = lookupInstance (scopename);
    }

    YCPValue value = YCPNull();

    while (instance)
    {
	value = instance->assignSymbol(symbolname, newvalue);
	if (!value.isNull())	// symbol found
	    break;
	instance = instance->outer;
    }

    return value;
}


/**
 * PRIVATE
 * find instance by name
 */

YCPScopeInstance *YCPScope::lookupInstance (const string& scopename) const
{
    scopeDebug ("lookupInstance (%s)", scopename.c_str());
    if (instances != 0)
    {
	YCPScopeInstanceContainer::iterator it = instances->find(scopename);
	if (it != instances->end())
	{
	    scopeDebug ("found at %p", it->second);
	    return it->second;
	}
    }
    scopeDebug ("not found");
    return 0;
}


/* EOF */
