/*----------------------------------------------------------------------\
|									|
|		      __   __    ____ _____ ____			|
|		      \ \ / /_ _/ ___|_   _|___ \			|
|		       \ V / _` \___ \ | |   __) |			|
|			| | (_| |___) || |  / __/			|
|			|_|\__,_|____/ |_| |_____|			|
|									|
|				core system				|
|							  (C) SuSE GmbH |
\-----------------------------------------------------------------------/

   File:	SymbolTable.cc
		hash table class

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include <stdlib.h>
#include "ycp/y2log.h"
#include "ycp/SymbolTable.h"
#include "ycp/SymbolEntry.h"
#include "ycp/Bytecode.h"

// make debugging switchable so initial setup of StaticDeclaration
//   doesn't clobber debug output
int SymbolTableDebug = 0;

// -*- c++ -*-

//--------------------------------------------------------------
// TableEntry

TableEntry::TableEntry (const char *key, SymbolEntry *entry, int line, const SymbolTable *table)
    : m_prev (0)
    , m_next (0)
    , m_inner (0)
    , m_outer (0)
    , m_key (key)
    , m_entry (entry)
    , m_line (line)
    , m_table (table)
{
}


TableEntry::TableEntry (std::istream & str)
    : m_prev (0)
    , m_next (0)
    , m_inner (0)
    , m_outer (0)
    , m_key (0)
    , m_entry (0)
    , m_line (0)
    , m_table (0)
{
    m_entry = Bytecode::readEntry (str);
    m_key = m_entry->name();
    m_line = Bytecode::readInt32 (str);
}


TableEntry::~TableEntry ()
{
}


const char *
TableEntry::key() const
{
    return m_key;
}


SymbolEntry *
TableEntry::sentry() const
{
    return m_entry;
}


TableEntry *
TableEntry::next() const
{
    return m_next;
}


int
TableEntry::line() const
{
    return m_line;
}


const SymbolTable *
TableEntry::table() const
{
    return m_table;
}


string
TableEntry::toString() const
{
    static char lbuf[16];
    sprintf (lbuf, "%d", m_line);
    string s = string ("TEntry (") + m_key
	+ "@" + lbuf
	+ ":" + m_entry->toString()
	+ ((m_entry->category() == SymbolEntry::c_function) ? "()" : "")
	+ ")";
    return s;
}


std::ostream &
TableEntry::toStream (std::ostream & str) const
{
    y2debug ("TableEntry::toStream (%s:%d)", m_entry->toString().c_str(), m_line);
    Bytecode::writeEntry (str, m_entry);
    return Bytecode::writeInt32 (str, m_line);
}


//--------------------------------------------------------------
// SymbolTable

int
SymbolTable::hash (const char *s)
{
    const char *p;
    unsigned int h = 0, g;

    for (p = s; *p != 0; p++)
    {
	h = (h << 4) + *p;
	if ((g = (h & 0xff000000)) != 0)
	{
	    h = h ^ (g >> 24);
	    h = h ^ g;
	}
    }
    return h % m_prime;
}


SymbolTable::SymbolTable (int prime)
{
    if (prime <= 0) 
	prime = 211;

    m_prime = prime;
    m_table = (TableEntry **)calloc (prime, sizeof (TableEntry *));
    y2debug ("New table @ %p", this);
}


SymbolTable::~SymbolTable()
{
    y2debug ("SymbolTable::~SymbolTable %p", this);
    int i;
    int count = 0;
    int used = 0;
    int maxlen = 0;
    int bucketlen;
    const TableEntry *current;

    // for each entry in hashtable

    for (i = 0; i < m_prime; i++)
    {
	current = m_table[i];
	if (current != 0)
	{
	    bucketlen = 0;
	    used++;

	    // for each entry in bucket list
	    while (current)
	    {
		TableEntry *next;

		count++;
		bucketlen++;

		// for each entry in scope stack
		//   delete outermost entries first

		TableEntry *outer = current->m_outer;
		while (outer)
		{
		    TableEntry *next_outer = outer->m_outer;
		    delete outer;
		    outer = next_outer;
		}

		// next in bucket chain
		next = current->m_next;

		// delete innermost entry
		delete current;

		current = next;
	    }
	    if (bucketlen > maxlen)
	    {
		maxlen = bucketlen;
	    }
	}
    }
#if 1
    y2debug ("%d of %d buckets used\n", used, m_prime);
    y2debug ("%d elements, %d bucketuse\n", count, count / m_prime);
    y2debug ("bucket size max %d, average %d\n", maxlen, (maxlen / m_prime) + 1);
#endif
    free (m_table);
}


int
SymbolTable::size() const
{
    return m_prime;
}


TableEntry *
SymbolTable::enter (const char *key, SymbolEntry *entry, int line)
{
    return enter (new TableEntry (key, entry, line, this));
}


TableEntry *
SymbolTable::enter (TableEntry *entry)
{
    TableEntry *bucket;
    const char *key = entry->key();

    entry->m_table = this;

    if (SymbolTableDebug)
    {
	y2debug ("SymbolTable::enter (entry %p(%s) into %p as '%s')\n", entry, entry->sentry()->toString().c_str(), this, key);
	y2debug ("SymbolTable %p before (%s)\n", this, toString().c_str());
    }
    int h = hash (key);			// compute hash
    bucket = m_table[h];

    if (bucket == 0)			// first entry in bucket
    {
	if (SymbolTableDebug) y2debug ("first entry in bucket");
	m_table[h] = entry;
    }
    else
    {
	while (bucket)				// find match in bucket list
	{
	    if (strcmp (key, bucket->m_key) == 0)	// match !
	    {
		if (SymbolTableDebug) y2debug ("match, add as new scope");

		// put entry at start of scope chain

		entry->m_outer = bucket;
		entry->m_inner = 0;
		bucket->m_inner = entry;

		// make entry new bucket list member

		if (bucket->m_prev)
		{
		    if (SymbolTableDebug) y2debug ("bucket->m_prev");
		    entry->m_prev = bucket->m_prev;
		    bucket->m_prev->m_next = entry;
		}
		else		// bucket was first
		{
		    m_table[h] = entry;
		    entry->m_prev = 0;
		}

		if (bucket->m_next)
		{
		    if (SymbolTableDebug) y2debug ("bucket->m_next");
		    entry->m_next = bucket->m_next;
		    bucket->m_next->m_prev = entry;
		}
		else
		    entry->m_next = 0;

		bucket->m_prev = bucket->m_next = 0;

		break;		// done
	    }
	    else if (bucket->m_next == 0)		// end of chain, no equal key found
	    {
		if (SymbolTableDebug) y2debug ("bucket->m_next == 0");
		bucket->m_next = entry;
		entry->m_prev = bucket;

		break;		// done
	    }

	    bucket = bucket->m_next;

	}  // while bucket

    }
    if (SymbolTableDebug) y2debug ("Table after (%s)\n", toString().c_str());

    return entry;
}


TableEntry *
SymbolTable::find (const char *key, SymbolEntry::category_t category)
{
    TableEntry *entry;

    // Not ready during initial __ctor__    y2debug ("SymbolTable::find (%s)\n", key);

    int h = hash (key);			// compute hash
    entry = m_table[h];

    while (entry != 0)			// search in hash chain
    {
	if (strcmp (key, entry->m_key) == 0)
	{
	    if ((category == SymbolEntry::c_unspec)		// wildcard
		|| (entry->sentry()->category() == category))	// or matching
	    {
		return entry;
	    }
	}
	entry = entry->m_next;
    }
    return 0;
}


void
SymbolTable::remove (TableEntry *entry)
{
    y2debug ("SymbolTable(%p)::remove (%p)[%s]\n", this, entry, entry->m_key);
    y2debug ("before remove (%s)", toString().c_str());

    // unlink from bucket stack
    // pop up bucket stack

    // find new candidate for stack and hash chain

    TableEntry *candidate = entry->m_outer;

    if (candidate != 0)			// have an outer entry with equal key
    {
	y2debug ("SymbolTable: Pop scope\n");
	candidate->m_inner = 0;		// the innermost is being deleted

	candidate->m_prev = entry->m_prev;	// link into hash chain
	candidate->m_next = entry->m_next;

	if (entry->m_next != 0)
	    entry->m_next->m_prev = candidate;

	if (entry->m_prev != 0)
	{
	    entry->m_prev->m_next = candidate;
	}
	else
	{
	    int h = hash (entry->m_key);
	    m_table[h] = candidate;		// next is new first
	}

    }
    else if (entry->m_prev != 0)		// not first in bucket list
    {
	entry->m_prev->m_next = entry->m_next;
	if (entry->m_next)
	    entry->m_next->m_prev = entry->m_prev;
    }
    else					// first in bucket list
    {
	y2debug ("SymbolTable: First in bucket\n");
	int h = hash (entry->m_key);
	m_table[h] = entry->m_next;	// next is new first
	if (entry->m_next != 0)
	    entry->m_next->m_prev = 0;
    }

    delete entry;

    y2debug ("after remove (%s)", toString().c_str());
    return;
}

string
SymbolTable::toString() const
{
    string s = "SymbolTable(";
    char buf[32]; sprintf (buf, "%p", this); s += string(buf) + ")";
    int i;
    // for each entry in hashtable

    for (i = 0; i < m_prime; i++)
    {
	TableEntry *current = m_table[i];
	if (current != 0)
	{
	char buf[32]; sprintf (buf, "[%d:%p]", i, current); s += buf;
	}

	if (current != 0)
	{
	    // for each entry in bucket list
	    while (current)
	    {
		SymbolEntry::category_t cat = current->sentry()->category();

		s += "->'";
		if ((cat == SymbolEntry::c_variable)
		    || (cat == SymbolEntry::c_builtin)
		    || (cat == SymbolEntry::c_function))
		{
		    s += current->sentry()->type().toString();
		}
		else
		{
		    s += current->sentry()->catString();
		}
		s += " ";
		s += current->key();
		s += "'";

		TableEntry *next = current->m_next;

		// for each entry in scope stack
		TableEntry *outer = current->m_outer;
		if (outer)
		{
		    s += ":[";
		    while (outer)
		    {
			s += "'";
			s += current->sentry()->type().asString().c_str();
			s += " ";
			s += current->key();
			s += "'";
			outer = outer->m_outer;
			if (outer == 0)
			{
			    break;
			}
			s += ",";
		    }
		    s += "]";
		}
		s += ",";
		current = next;
	    }
	    s += "\n";
	} // table entry used
    }

    return s;    
}
