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

TableEntry::TableEntry (const char *key, SymbolEntryPtr entry, const Point *point, SymbolTable *table)
    : m_prev (0)
    , m_next (0)
    , m_outer (0)
    , m_key (key)
    , m_entry (entry)
    , m_point (point)
    , m_table (table)
{
}


TableEntry::TableEntry (std::istream & str)
    : m_prev (0)
    , m_next (0)
    , m_outer (0)
    , m_key (0)
    , m_entry (0)
    , m_point (0)
    , m_table (0)
{
    m_entry = Bytecode::readEntry (str);
    m_key = m_entry->name();
    m_point = new Point (str);
    if (m_entry->category() == SymbolEntry::c_function)		// read function prototype
    {
	m_entry->setCode(Bytecode::readCode (str));
	if (m_entry->code()->kind() != YCode::ycFunction)
	{
	    y2error ("TableEntry::fromStream: bad prototype for global function %s", m_entry->toString().c_str());
	}
    }
}


TableEntry::~TableEntry ()
{
    delete m_point;
}


//-------------------------------------------------------------------

const char *
TableEntry::key() const
{
    return m_key;
}


SymbolEntryPtr 
TableEntry::sentry() const
{
    return m_entry;
}


const Point *
TableEntry::point() const
{
    return m_point;
}


TableEntry *
TableEntry::next() const
{
    return m_next;
}


const SymbolTable *
TableEntry::table() const
{
    return m_table;
}

//-------------------------------------------------------------------

// convert declaration to definition (exchanges m_point)
void
TableEntry::makeDefinition (int line)
{
    Point *point = new Point (m_point->sentry(), line, m_point->point());	// duplicate current m_point, replacing line
//    y2debug ("TableEntry::makeDefinition (%s -> %s)", m_point->toString().c_str(), point->toString().c_str());
    delete m_point;
    m_point = point;
}


string
TableEntry::toString() const
{
    static char lbuf[16];
    if (m_point) sprintf (lbuf, "%d", m_point->line());
    else strcpy (lbuf, "<nil>");
    string s = string ("TEntry (") + m_key
	+ "@" + lbuf
	+ ":" + m_entry->toString()
	+ ")";
    return s;
}


std::ostream &
TableEntry::toStream (std::ostream & str) const
{
//    y2debug ("TableEntry::toStream (%s:%s)", m_entry->toString().c_str(), m_point->toString().c_str());
    Bytecode::writeEntry (str, m_entry);
    m_point->toStream (str);
    if ((m_entry->category() == SymbolEntry::c_function)		// write function prototype if it's global
	&& m_entry->isGlobal())
    {
//	y2debug ("TableEntry::toStream: write global function prototype");

	((YFunctionPtr)(m_entry->code()))->toStream (str);
    }
    return str;
}


void
TableEntry::remove ()
{
    if (m_table)
    {
	m_table->remove (this);
	m_table = 0;
    }
}

//-------------------------------------------------------------------

//-------------------------------------------------------------------

// SymbolTable

int
SymbolTable::hash (const char *s)
{
//    y2debug( "hash called for %p", this );
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
    // The 31 used to be 211, but that is too much.
    // because eg. in runlevel there's 1200 blocks
    // and each has its symbol table.
    // Better yet use rehashing - TODO
    : m_prime ((prime <= 0) ? 31 : prime)
    , m_track_usage (true)
    , m_used (0)
{
    m_table = (TableEntry **)calloc (m_prime, sizeof (TableEntry *));

//    y2debug ("New table @ %p", this);
}


SymbolTable::~SymbolTable()
{
//    y2debug ("SymbolTable::~SymbolTable %p", this);

    while (!m_xrefs.empty())
    {
	delete m_xrefs.top();
	m_xrefs.pop();
    }

    endUsage ();

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


//-------------------------------------------------------------------

void
SymbolTable::openXRefs ()
{
    y2debug ("SymbolTable[%p]::openXRefs()", this);
    m_xrefs.push (new (std::vector<TableEntry *>));

    return;
}


void
SymbolTable::closeXRefs ()
{
    y2debug ("SymbolTable[%p]::closeXRefs()", this);
    if (m_xrefs.empty())
    {
	y2error ("SymbolTable[%p]::closeXRefs without openXRefs", this);
	return;
    }
    delete m_xrefs.top();
    m_xrefs.pop();

    return;
}


SymbolEntryPtr 
SymbolTable::getXRef (unsigned int position) const
{
    if (m_xrefs.empty())
    {
	y2error ("SymbolTable[%p]::getXRefs empty !", this);
	return 0;
    }
    std::vector<TableEntry *> *refs = m_xrefs.top();
    if (position >= refs->size())
    {
	y2error ("SymbolTable[%p]::getXRefs position %d >= size %d !", this, position, refs->size());
	return 0;
    }
    return (*refs)[position]->sentry();
}

//-------------------------------------------------------------------

void
SymbolTable::startUsage ()
{
    y2debug ("SymbolTable[%p]::startUsage", this);
    if (m_used == 0)
    {
	m_used = new (std::map<const char *, TableEntry *>);
    }
    return;
}


int
SymbolTable::countUsage ()
{
    if (m_used != 0)
    {
	return m_used->size();
    }
    return 0;
}


void
SymbolTable::endUsage ()
{
    y2debug ("SymbolTable[%p]::endUsage", this);
    if (m_used)
    {
	delete m_used;
	m_used = 0;
    }
    return;
}


void
SymbolTable::enableUsage ()
{
    y2debug ("SymbolTable[%p]::enableUsage", this);
    m_track_usage = true;
}


void
SymbolTable::disableUsage ()
{
    y2debug ("SymbolTable[%p]::disableUsage", this);
    m_track_usage = false;
}


std::ostream &
SymbolTable::writeUsage (std::ostream & str) const
{
    y2debug ("SymbolTable[%p]::writeUsage", this);

    if (m_used == 0)
    {
	Bytecode::writeInt32 (str, 0);
	y2debug ("Unused !");
	return str;
    }

    std::vector<TableEntry *> xrefs;

    std::map<const char *, TableEntry *>::iterator it;

    for (it = m_used->begin(); it != m_used->end(); it++)
    {
	TableEntry *tentry = it->second;
	tentry->sentry()->setPosition (xrefs.size());
	y2debug ("%d -> %s", xrefs.size(), tentry->sentry()->toString().c_str());
	xrefs.push_back (tentry);
    }

    int rsize = xrefs.size();

    bool xref_debug = (getenv (XREFDEBUG) != 0);

    if (xref_debug) y2milestone ("Need %d symbols from table %p\n", rsize, this);
    else y2debug ("Need %d symbols from table %p\n", rsize, this);

    Bytecode::writeInt32 (str, rsize);

    int position = 0;
    while (position < rsize)
    {
	SymbolEntryPtr sentry = xrefs[position]->sentry();

	if (xref_debug) y2milestone("XRef %p::%s @ %d\n", this, sentry->toString().c_str(), position);
	else y2debug("XRef %p::%s @ %d\n", this, sentry->toString().c_str(), position);

	Bytecode::writeCharp (str, sentry->name());
	sentry->type()->toStream (str);
	sentry->setPosition (-position - 1);			// negative position -> Xref
	position++;
    }

    return str;
}

//-------------------------------------------------------------------

int
SymbolTable::size() const
{
    return m_prime;
}


TableEntry *
SymbolTable::enter (const char *key, SymbolEntryPtr entry, const Point *point)
{
    return enter (new TableEntry (key, entry, point, this));
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


// find (innermost) TableEntry by key and add to m_used

TableEntry *
SymbolTable::find (const char *key, SymbolEntry::category_t category)
{
    TableEntry *tentry;

    // Not ready during initial __ctor__    y2debug ("SymbolTable::find (%s)\n", key);

    int h = hash (key);			// compute hash
    tentry = m_table[h];

    while (tentry != 0)			// search in hash chain
    {
	if (strcmp (key, tentry->m_key) == 0)
	{
	    if ((category == SymbolEntry::c_unspec)		// wildcard
		|| (tentry->sentry()->category() == category))	// or matching
	    {
		if (m_track_usage
		    && m_used
		    && m_used->find (tentry->m_key) == m_used->end())
		{
		    tentry->sentry()->setPosition (m_used->size());	// store the position in the sentry
		    (*m_used)[tentry->m_key] = tentry;			// create the position in the usage list
		}
		return tentry;
	    }
	    if (category != SymbolEntry::c_unspec)
	    {
		tentry = tentry->m_outer;			// continue category match in scope chain
	    }
	    else
	    {
		break;
	    }
	}
	else
	{
	    tentry = tentry->m_next;				// continue search in hash chain
	}
    }
    return 0;
}


// find (innermost) TableEntry by key and add to m_xrefs

TableEntry *
SymbolTable::xref (const char *key)
{
    TableEntry *tentry = find (key, SymbolEntry::c_unspec);
    if (tentry != 0)
    {
	m_xrefs.top()->push_back (tentry);
    }
    return tentry;
}


void
SymbolTable::remove (TableEntry *entry)
{
//    y2debug ("SymbolTable(%p)::remove (%p)[%s]\n", this, entry, entry->m_key);
//    y2debug ("before remove (%s)", toString().c_str());

    // unlink from bucket stack
    // pop up bucket stack

    // find new candidate for stack and hash chain

    TableEntry *candidate = entry->m_outer;

    if (candidate != 0)						// have an outer entry with equal key
    {
	//y2debug ("SymbolTable: Pop scope\n");

	candidate->m_prev = entry->m_prev;			// link into hash chain
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
	//y2debug ("SymbolTable: First in bucket\n");
	int h = hash (entry->m_key);
	m_table[h] = entry->m_next;	// next is new first
	if (entry->m_next != 0)
	    entry->m_next->m_prev = 0;
    }

    delete entry;

//    y2debug ("after remove (%s)", toString().c_str());
    return;
}


    void convertUsage2XRef ();

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
		    || (cat == SymbolEntry::c_reference)
		    || (cat == SymbolEntry::c_builtin)
		    || (cat == SymbolEntry::c_function))
		{
		    s += current->sentry()->type()->toString();
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
			s += current->sentry()->type()->toString().c_str();
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

string
SymbolTable::toStringSymbols() const
{
    string s = "SymbolTable";
    int i;
    // for each entry in hashtable

    for (i = 0; i < m_prime; i++)
    {
	TableEntry *current = m_table[i];
	if (current != 0)
	{
	    // for each entry in bucket list
	    while (current)
	    {
		SymbolEntry::category_t cat = current->sentry()->category();

		s += "->'";
		if ((cat == SymbolEntry::c_variable)
		    || (cat == SymbolEntry::c_reference)
		    || (cat == SymbolEntry::c_builtin)
		    || (cat == SymbolEntry::c_function))
		{
		    s += current->sentry()->type()->toString();
		}
		else
		{
		    s += current->sentry()->catString();
		}
		s += " ";
		s += current->key();
		// add also SymbolEntry::position() to catch reordering in source
		char buf[32]; sprintf (buf, "[%d]", current->sentry()->position()); s += buf;
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
			s += current->sentry()->type()->toString().c_str();
			s += " ";
			s += current->key();
			// add also SymbolEntry::position() to catch reordering in source
			char buf[32]; sprintf (buf, "[%d]", current->sentry()->position()); s += buf;
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

// EOF
