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


#include <y2util/y2log.h>
#include <ycp/SymbolTable.h>


#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "Y2Namespace.h"
#include "Y2Function.h"
#include "SymbolEntry.h"

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
	// reset namespace of the symbol
	symbols_t::const_iterator it;
	for (it = m_symbols.begin(); it != m_symbols.end(); it++)
	{
	    (*it)->setNamespace(0);
	}
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

    symbols_t::const_iterator it;
    for (unsigned int p = 0; p < m_symbolcount; p++)
    {
	if ( m_symbols[p] )
	{
	    if (!m_symbols[p]->isFilename ())
	    {
		s += "\n    // ";
		s += m_symbols[p]->toString();
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
    if (position >= m_symbolcount)
    {
	return 0;
    }
    return m_symbols[position];
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
    m_symbols.push_back(sentry);
    return m_symbolcount++;
}


// lookup symbol in m_symbols
SymbolEntryPtr
Y2Namespace::lookupSymbol (const char *name) const
{
    for (unsigned int p = 0; p < m_symbolcount; p++)
    {
	if ( m_symbols[p] && (strcmp (m_symbols[p]->name(), name) == 0)
            && !m_symbols[p]->likeNamespace())			// allow symbol if namespace of same name already declared
        {
            return m_symbols[p];
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
    if (position < m_symbolcount)
    {
	m_symbols[position]->setNamespace (0);
	m_symbols[position] = 0;
    }
}


#if 0
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
#endif


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
    for (unsigned int p = 0; p < m_symbolcount; p++)
    {
	if ( m_symbols[p] && m_symbols[p]->isVariable() )
        {
            m_symbols[p]->push ();
        }
    }
}


void
Y2Namespace::popFromStack ()
{
    for (unsigned int p = 0; p < m_symbolcount; p++)
    {
	if ( m_symbols[p] && m_symbols[p]->isVariable() )
        {
            m_symbols[p]->pop ();
        }
    }
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
	    Y2Function* c = createFunctionCall (name (), 0);
	    if (c)
	    {
		c->evaluateCall ();
	    }
	    delete c;
	}
	t->enableUsage ();
    }
}

