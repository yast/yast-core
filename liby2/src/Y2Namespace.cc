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

   File:       Y2Namespace.cc

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Base class of all Y2 namespaces providing a default implementations
 *
 */


#include <ycp/y2log.h>
#include <ycp/SymbolTable.h>
#include <ycp/SymbolEntryPtr.h>
#include <ycp/SymbolEntry.h>
#include <ycp/Scanner.h>

#define DO_DEBUG 1

#include "Y2Namespace.h"
#include "Y2NamespaceCPP.h"
#include "Y2Function.h"

Y2Namespace::Y2Namespace ()
    : m_table (0)
    , m_symbolcount (0)
    , m_initialized (false)
{}


Y2Namespace::~Y2Namespace ()
{
#if DO_DEBUG
    y2debug ("Y2Namespace::~Y2Namespace [%p]", this);
#endif
    if (m_table)
    {
	delete m_table;
    }
}


const string
Y2Namespace::name () const
{
    return "";
}


unsigned int
Y2Namespace::symbolCount () const
{
    return m_symbols.size();
}


string
Y2Namespace::toString () const
{
    if (m_table == 0)
    {
	return "<empty>";
    }

    SymbolTable* t = m_table;
    string s = t->toString ();

    return s + symbolsToString ();
}


string
Y2Namespace::symbolsToString () const
{
    string s;

    map<unsigned int, SymbolEntryPtr>::const_iterator it;
    for (unsigned int p = 0; p < m_symbolcount; p++)
    {
	it = m_symbols.find (p);
	if (it != m_symbols.end())
	{
	    if (!it->second->isFilename())
	    {
		s += "\n    // ";
		s += it->second->toString();
	    }
	}
	else
	{
	    s += "\n    // ";
	    s += "<released>";
	}
    }

    return s;
}


SymbolEntryPtr 
Y2Namespace::symbolEntry (unsigned int position) const
{
    map<unsigned int, SymbolEntryPtr>::const_iterator it = m_symbols.find (position);
    if (it == m_symbols.end())
    {
	return 0;
    }
    return it->second;
}


// find symbol by pointer
// return index if found, -1 if not found
int
Y2Namespace::findSymbol (const SymbolEntryPtr sentry) const
{
    map<unsigned int, SymbolEntryPtr>::const_iterator it;
    for (it = m_symbols.begin(); it != m_symbols.end(); it++)
    {
	if (it->second == sentry)
	{
	    return it->first;
	}
    }
    return -1;
}


// add symbol to namespace, it now belongs here
// returns the index into m_symbols
//
// this is used for blocks with a local environment but no table
unsigned int
Y2Namespace::addSymbol (SymbolEntryPtr sentry)
{
#if DO_DEBUG
    y2debug ("addSymbol #%d:'%s'", m_symbolcount, sentry->toString().c_str());
#endif
    m_symbols[m_symbolcount] = sentry;
    return m_symbolcount++;
}


// lookup symbol in m_symbols
SymbolEntryPtr
Y2Namespace::lookupSymbol (const char *name) const
{
    // check duplicates
    map<unsigned int, SymbolEntryPtr>::const_iterator it;
    for (it = m_symbols.begin(); it != m_symbols.end(); it++)
    {
	if ((strcmp (it->second->name(), name) == 0)
            && !it->second->likeNamespace())			// allow symbol if namespace of same name already declared
        {
            return it->second;
        }
    }
    return 0;
}


// add symbol _and_ enter into table for lookup
//
// this is used for namespaces with a global environment and a table

void
Y2Namespace::enterSymbol (SymbolEntryPtr sentry, Point *point )
{
    addSymbol (sentry);
    if (m_table == 0)
    {
	m_table = new SymbolTable (-1);
    }

    m_table->enter (sentry->name(), sentry, point);

    return;
}


void
Y2Namespace::releaseSymbol (unsigned int position)
{
    map<unsigned int, SymbolEntryPtr>::iterator it = m_symbols.find (position);
    if (it != m_symbols.end())
    {
	it->second->setNamespace (0);
	m_symbols.erase (it);
    }
    return;
}


void
Y2Namespace::releaseSymbol (SymbolEntryPtr sentry)
{
    int p = findSymbol (sentry);
    if (p >= 0)
    {
	releaseSymbol (p);
    }
    return;
}


void
Y2Namespace::finish ()
{
#if 0
    // ATM, we only reorder sentries, so global ones are on top of the table
    // it is important to allow a bytecode ignore changes in local symbols - the indexes for globals
    // are not changed
    if ( m_count == 0 ) return;
    
#if DO_DEBUG
    y2debug ("Going to reorder");
#endif

    SymbolEntry** new_environment = (SymbolEntry **)calloc (sizeof (SymbolEntry *), m_count);

    int next_index = 0;

    // globals first
    for (uint i = 0 ; i < m_count ; i++)
    {
	if (m_senvironment[i]->isGlobal ())
	{
	    new_environment[next_index] = m_senvironment[i];
	    new_environment[next_index]->setPosition (next_index);
	    next_index++;
	}
    }
    
    // then locals
    for (uint i = 0 ; i < m_count ; i++)
    {
	if (! m_senvironment[i]->isGlobal ())
	{
	    new_environment[next_index] = m_senvironment[i];
	    new_environment[next_index]->setPosition (next_index);
	    next_index++;
	}
    }
    free (m_senvironment);
    m_senvironment = new_environment;
#endif
#if DO_DEBUG
    y2debug ("Reorder done");
#endif

    return;
}


void
Y2Namespace::pushToStack ()
{
    map<unsigned int, SymbolEntryPtr>::const_iterator it;
    for (it = m_symbols.begin(); it != m_symbols.end(); it++)
    {
	if (it->second->isVariable())
	{
	    it->second->push();
	}
    }
    return;
}


void
Y2Namespace::popFromStack ()
{
    map<unsigned int, SymbolEntryPtr>::const_iterator it;
    for (it = m_symbols.begin(); it != m_symbols.end(); it++)
    {
	if (it->second->isVariable())
	{
	    it->second->pop();
	}
    }
    return;
}


SymbolTable *
Y2Namespace::table () const
{
    return m_table;
}

void
Y2Namespace::createTable ()
{
    if (m_table == 0)
    {
	m_table = new SymbolTable (-1);
    }
}

void
Y2Namespace::initialize ()
{
    if (m_initialized)
    {
	// we are already initialized
	return;
    }
    
    // avoid recursion
    m_initialized = true;

    evaluate ();
    
    if (table ())
    { 
	SymbolTable* t = table ();
	t->disableUsage ();
	if (t->find (name ().c_str ()))
	{
	    Y2Function* c = createFunctionCall (name ());
	    if (c)
	    {
		c->evaluateCall ();
	    }
	}
	t->enableUsage ();
    }
}

// ************************** Y2NamespaceCPP.h helpers ***********************************

Y2CPPFunction::Y2CPPFunction (Y2Namespace* parent, string name, Y2CPPFunctionCallBase* call_impl)
    : YFunction (new YBlock ((Point *)0))
    , m_name (name)
    , m_parent (parent)
    , m_impl (call_impl)
{
    call_impl->registerParameters (declaration ());
    
    setDefinition (call_impl);
}

SymbolEntryPtr
Y2CPPFunction::sentry (unsigned int position)
{
    SymbolEntryPtr morefun = new SymbolEntry (m_parent, position, Scanner::doStrdup (m_name.c_str()), SymbolEntry::c_global, 
            Type::fromSignature ( m_impl->m_signature ), this );
    morefun->setCategory (SymbolEntry::c_function); 
    return morefun;
}

void
Y2CPPFunctionCallBase::newParameter (YBlockPtr decl, unsigned pos, constTypePtr type)
{
    string name;
    
    // FIXME: do it nicer
    switch (pos)
    {
	case 1: name = "param1"; break;
	case 2: name = "param2"; break;
	case 3: name = "param3"; break;
	case 4: name = "param4"; break;
    }
    SymbolEntryPtr param = decl->newEntry ( Scanner::doStrdup (name.c_str()), SymbolEntry::c_global, type, 0)->sentry ();
    param->setCategory (SymbolEntry::c_variable);
    
    switch (pos)
    {
	case 1: m_param1 = param; break;
	case 2: m_param2 = param; break;
	case 3: m_param3 = param; break;
	case 4: m_param4 = param; break;
    }
}
