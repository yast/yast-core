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

   File:	YBlock.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/
// -*- c++ -*-

#include "ycp/YBlock.h"
#include "ycp/YCPVoid.h"

// needed for YBlock

#include "ycp/SymbolEntry.h"
#include "ycp/SymbolTable.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"

extern ExecutionEnvironment ee;

// ------------------------------------------------------------------
// block (-> list of statements, list of symbols, kind)

YBlock::YBlock (YBlock::blockkind_t kind)
    : YCode (yeBlock)
    , m_kind (kind)
    , m_name ("")
    , m_table (0)
    , m_tenvironment (0)
    , m_last_tparm (0)
    , m_count (0)
    , m_senvironment (0)
    , m_statements (0)
    , m_last_statement (0)
{
}


YBlock::~YBlock ()
{
    stmtlist_t *stmt = m_statements;
    while (stmt)
    {
	stmtlist_t *nexts = stmt->next;
	delete stmt->stmt;
	delete stmt;
	stmt = nexts;
    }
    yTElist_t *tp = m_tenvironment;
    while (tp)
    {
	yTElist_t *next = tp->next;
	delete tp;
	tp = next;
    }
    if (m_table)
    {
	delete m_table;
    }
    if (m_senvironment)
    {
	for (unsigned int p = 0; p < m_count; p++)
	{
	    if (m_senvironment[p] != 0)
	    {
		delete m_senvironment[p];
	    }
	}
	free (m_senvironment);
    }
}


// private:
// add symbol to m_senvironment
unsigned int
YBlock::addSymbol (SymbolEntry *entry)
{
    m_count++;
    m_senvironment = (SymbolEntry **)realloc (m_senvironment, sizeof (SymbolEntry *) * m_count);

    if (m_senvironment == 0)
    {
	y2error ("OOM !");
	abort();
    }
    m_senvironment[m_count-1] = entry;
    return m_count;
}


// add new value code to this block
//   (used for functions which accept either symbolic variables or values, e.g. foreach())
// returns position
unsigned int
YBlock::newValue (const TypeCode &type, YCode *code)
{
    static char name[8];
    sprintf (name, "_%d", m_count);		// create 'fake' name

    y2debug ("YBlock::newValue (%s %s)", type.toString().c_str(), name);

// FIXME: check duplicates
    return addSymbol (new SymbolEntry (this, m_count, name, SymbolEntry::c_const, type, code));
}


// SymbolTable for global module environment (m_kind == b_module)
SymbolTable *
YBlock::table () const
{
    return m_table;
}


void
YBlock::setTable (SymbolTable *table)
{
    m_table = table;
    return;
}


// add new table entry to this block
//  and attach it to m_tenvironment
//   return NULL if symbol of same name already declared in this block
TableEntry *
YBlock::newEntry (const char *name, SymbolEntry::category_t cat, const TypeCode &type, unsigned int line)
{
    y2debug ("YBlock(%p)::newEntry (%s, cat %d, type %s, line %d)", this, name, (int)cat, type.toString().c_str(), line);
    // check duplicates
    for (unsigned int p = 0; p < m_count; p++)
    {
	if ((m_senvironment[p] != 0)
	    && (strcmp (m_senvironment[p]->name(), name) == 0))
	{
	    return 0;
	}
    }

    SymbolEntry *entry = new SymbolEntry (this, m_count, name, cat, type);
    addSymbol (entry);

    TableEntry *tentry = new TableEntry (name, entry, line);
    attachEntry (tentry);

    return tentry;
}


// find symbol in m_senvironment, return -1 if not found
int
YBlock::findSymbol (const SymbolEntry *entry)
{
    for (unsigned int p = 0; p < m_count; p++)
    {
	if (m_senvironment[p] == entry)
	{
	    return p;
	}
    }
    return -1;
}


// release symbol entry from m_senvironment
//   it's no longer owned by this block but by a ysFunction()
void
YBlock::releaseSymbol (unsigned int position)
{
    y2debug ("YBlock::releaseSymbol (%d)", position);
    if (position < m_count)
    {
	if (m_senvironment[position] != 0)
	{
	    m_senvironment[position]->setBlock (0);
	    m_senvironment[position] = 0;
	}
    }
    return;
}


// release symbol entry from m_senvironment
//   it's no longer owned by this block but by a ysFunction()
void
YBlock::releaseSymbol (SymbolEntry *entry)
{
    y2debug ("YBlock::releaseSymbol (%s)", entry->toString().c_str());
    int p = findSymbol (entry);
    if (p >= 0)
    {
	releaseSymbol (p);
    }
    return;
}


// number of local variables (environment entries)
unsigned int
YBlock::symbolCount (void) const
{
    return m_count;
}


// get entry by position
SymbolEntry *
YBlock::symbolEntry (unsigned int position) const
{
    if (position >= m_count)
    {
	return 0;
    }
    return m_senvironment[position];
}


// Attach entry (variable, typedef, ...) to local environment
void
YBlock::attachEntry (TableEntry *tentry)
{
    y2debug ("YBlock::attachEntry to %p (%s)", this, tentry->toString().c_str());

    yTElist_t *newt = new (yTElist_t);
    newt->tentry = tentry;
    newt->next = 0;

    if (m_tenvironment == 0)
    {
	m_tenvironment = newt;
	m_last_tparm = m_tenvironment;
    }
    else if (m_last_tparm == 0)
    {
	y2error ("YBlock::attachEntry after detach_environemt !");
    }
    else
    {
	m_last_tparm->next = newt;
	m_last_tparm = newt;
    }
    return;
}


void
YBlock::attachStatement (YStatement *statement)
{
    y2debug ("YBlock::attachStatement (%s)", statement->toString().c_str());

    stmtlist_t *newstmt = new (stmtlist_t);
    newstmt->stmt = statement;
    newstmt->next = 0;
    if (m_statements == 0)
    {
	m_statements = newstmt;
	m_last_statement = m_statements;
    }
    else
    {
	m_last_statement->next = newstmt;
        m_last_statement = newstmt;
    }
    return;
}


// used internally in stream constructor
YBlock *
YBlock::importModule (const string & name)
{
    y2debug ("YBlock::importModule (%s) to %p", name.c_str(), this);
    YCode *code = Bytecode::readModule (name);
    if ((code == 0)
	|| !code->isBlock()
	|| !((YBlock *)code)->isModule())
    {
	y2error ("Bad import %s", name.c_str());
	return 0;
    }

    return (YBlock *)code;
}


TableEntry *
YBlock::importModule (const string & name, int line)
{
    y2debug ("YBlock::importModule (%s, %d) to %p", name.c_str(), line, this);
    YBlock *block = importModule (name);
    if (block == 0)
    {
	return 0;
    }

    TableEntry *tentry = newEntry (strdup (name.c_str()), SymbolEntry::c_module, TypeCode::Unspec, line);
    tentry->sentry()->setCode (block);

    y2debug ("environment of %p after import (%s)", this, environmentToString().c_str());
    return tentry;
}


// detach environment from symbol table
//   remove all in m_tenvironment listed table entries
//   compact m_senvironment (delete all released slots)

void
YBlock::detachEnvironment (SymbolTable *table)
{
    y2debug ("YBlock::detachEnvironment of %p (%s) from %s", this, environmentToString().c_str(), table->toString().c_str());

    // unlink table entries belonging to table (usually, these are the local symbols)

    yTElist_t *tp = m_tenvironment;
    yTElist_t *prev = 0;
    while (tp)
    {
	yTElist_t *next = tp->next;

	if (tp->tentry
	    && tp->tentry->table() == table)
	{
	    y2debug ("Remove %s", tp->tentry->toString().c_str());
	    table->remove (tp->tentry);			// remove the TableEntry
	    delete tp;
	    if (prev != 0)
	    {
		prev->next = next;
	    }
	    else
	    {
		m_tenvironment = next;
	    }
	}
	else
	{
	    prev = tp;
	}
	tp = next;
    }

    if (prev == 0)			// all removed
    {
	m_tenvironment = 0;
    }
    m_last_tparm = 0;

    int free_slots = 0;

    for (unsigned int i = 0; i < m_count; i++)
    {
	if (m_senvironment[i] == 0)		// slot is free, move all one down
	{
	    y2debug ("Slot %d of %d is free", i, m_count);
	    free_slots++;
	    for (unsigned j = i+1; j < m_count; j++)
	    {
		y2debug ("Moving %d to %d", j, j-1);
		if (m_senvironment[j] != 0)
		{
		    m_senvironment[j]->setPosition (j-1);
		}
		m_senvironment[j-1] = m_senvironment[j];
	    }
	}
    }
    m_count -= free_slots;

    return;
}


void
YBlock::setKind (YBlock::blockkind_t kind)
{
    y2debug ("YBlock::setKind %p: %d", this, (int)kind);
    m_kind = kind;
    return;
}


YBlock::blockkind_t
YBlock::kind () const
{
    return m_kind;
}


bool
YBlock::isModule () const
{
    return (m_kind == b_module);
}


YSReturn *
YBlock::justReturn () const
{
    if (m_statements != 0)
    {
	YStatement *stmt = m_statements->stmt;
	if (stmt->code() == ysReturn)
	{
	    return (YSReturn *)stmt;
	}
    }
    return 0;
}


const string &
YBlock::name () const
{
    return m_name;
}


void
YBlock::setName (const string &name)
{
    m_name = name;
    return;
}


string
YBlock::environmentToString () const
{
    string s;

    yTElist_t *tp = m_tenvironment;
    while (tp)
    {
	s += "\n    //T: ";
	s += tp->tentry->toString();
	tp = tp->next;
    }

    for (unsigned int p = 0; p < m_count; p++)
    {
	s += "\n    // ";
	if (m_senvironment[p])
	{
	    s += m_senvironment[p]->toString();
	    if (m_senvironment[p]->category() == SymbolEntry::c_module)
	    {
		s += (string ("\n    import \"") + m_senvironment[p]->name() + "\";");
	    }
	    else if (m_senvironment[p]->category() == SymbolEntry::c_function)
	    {
		s += "()";
	    }
	}
	else
	{
	    s += "<released>";
	}
    }

    return s;
}


string
YBlock::toString () const
{
    string s;

    if (m_kind == b_using)
    {
	s = (m_name + "::");
    }
    s += "{";

    if (isModule())
    {
	s += "\n    module \"" + m_name + "\";\n";
    }

    s += environmentToString ();

    stmtlist_t *stmt = m_statements;
    while (stmt)
    {
	s += "\n    ";
	s += stmt->stmt->toString();
	stmt = stmt->next;
    }

    s += "\n}\n";
    return s;
}


YCPValue
YBlock::evaluate (bool cse)
{
    y2debug ("YBlock::evaluate([%d]%s)\n", (int)m_kind, toString().c_str());

    if (cse)
    {
	return YCPNull();
    }

    // initialize environment
    // set all values (which still belong to this block)
    //   to NULL so yeVariable triggers to re-evaluate the code

    for (unsigned int p = 0; p < m_count; p++)
    {
	if (m_senvironment[p] != 0)
	{
	    m_senvironment[p]->setValue (YCPNull());
	}
    }

    return evaluateNoInit ();
}


YCPValue
YBlock::evaluateNoInit ()
{
    y2debug ("YBlock::evaluateNoInit(%s)\n", toString().c_str());
    // no environment initialization takes place here
    // so loop blocks only initialize their local variables once.

    stmtlist_t *stmt = m_statements;
    YCPValue value = YCPVoid ();
    while (stmt)
    {
	YStatement *statement = stmt->stmt;

	ee.setLinenumber (statement->line());
	value = statement->evaluate ();

	if (!value.isNull())
	{
	    y2debug ("Block break (%s)", value->toString().c_str());
	    break;
	}
	stmt = stmt->next;
    }
    y2debug ("YBlock::evaluateNoInit done (stmt %p, kind %d, value '%s')\n", stmt, m_kind, value.isNull() ? "NULL" : value->toString().c_str());
    return (stmt==0) ? ((m_kind == b_statement) ? YCPNull() : YCPVoid()) : value;
}


// --------------------------
// bytecode I/O

YBlock::YBlock (std::istream & str)
    : YCode (yeBlock)
    , m_kind (b_unknown)
    , m_name ("")
    , m_tenvironment (0)
    , m_last_tparm (0)
    , m_senvironment (0)
    , m_statements (0)
    , m_last_statement (0)
{
    Bytecode::readString (str, m_name);		// read name

    m_kind = (blockkind_t)Bytecode::readInt32 (str);

    Bytecode::pushBlock (this);

    m_count = Bytecode::readInt32 (str);	// read m_senvironment

    y2debug("YBlock::fromStream (%p:\"%s\", %d entries, kind %d)", this, m_name.c_str(), m_count, m_kind);

    // read all symbol entries belonging to this block
    //   push module block for imported modules !
    if (m_count > 0)
    {
	m_senvironment = (SymbolEntry **)realloc (m_senvironment, sizeof (SymbolEntry *) * m_count);

	for (unsigned int i = 0; i < m_count; i++)
	{
	    SymbolEntry *entry = new SymbolEntry (str, this);
	    m_senvironment[i] = entry;
	    if (entry->category() == SymbolEntry::c_module)
	    {
		YBlock *block = importModule (entry->name());
		entry->setCode (block);
		Bytecode::pushBlock (block);
	    }
	}

	if (m_kind == b_module)			// if its a module, re-construct the table
	{
	    m_table = new SymbolTable (211);
	    int tcount = Bytecode::readInt32 (str);
	    y2debug ("Module with %d table entries", tcount);
	    while (tcount-- > 0)
	    {
		TableEntry *tentry = new TableEntry (str);
		attachEntry (tentry);
		m_table->enter (tentry);
	    }
	}
    }

    u_int32_t count = Bytecode::readInt32 (str);
    y2debug ("YBlock::fromStream (%p:\"%s\", %d statements)", this, m_name.c_str(), count);

    stmtlist_t *stmt;
    while (count-- > 0)
    {
	stmt = new stmtlist_t;
	stmt->stmt = Bytecode::readStatement (str);
	stmt->next = 0;
	if (stmt->stmt == 0)
	{
	    break;
	}
	if (m_statements == 0)
	{
	    m_statements = stmt;
	}
	else
	{
	    m_last_statement->next = stmt;
	}
	m_last_statement = stmt;
    }

    // pop imported modules blocks

    if (m_count > 0)
    {
	for (int i = m_count-1; i >= 0; i--)
	{
	    if (m_senvironment[i]->category() == SymbolEntry::c_module)
	    {
		Bytecode::popBlock ((YBlock *)(m_senvironment[i]->code()));
	    }
	}
    }

    Bytecode::popBlock (this);
    y2debug ("done");
}


std::ostream &
YBlock::toStream (std::ostream & str) const
{
    y2debug ("YBlock::toStream (%p:\"%s\", kind %d)", this, m_name.c_str(), m_kind);
    YCode::toStream (str);

    Bytecode::writeString (str, m_name);		// write name
    Bytecode::writeInt32 (str, m_kind);			// write kind

    Bytecode::pushBlock (this);

    y2debug ("YBlock %p: %d symbol entries", this, m_count);
    Bytecode::writeInt32 (str, m_count);		// write m_senvironment

    if (m_count > 0)
    {
    for (unsigned int i = 0; i < m_count; i++)
    {
	SymbolEntry *entry = m_senvironment[i];
	entry->toStream (str);				// write SymbolEntry
	if (entry->category() == SymbolEntry::c_module)
	{
	    Bytecode::pushBlock ((YBlock *)(entry->code()));	// push imported blocks
	}
    }

    if (isModule())					// if its a module, write the table
    {
	yTElist_t *tptr = m_tenvironment;
	int tcount = 0;					// count the table entries
	while (tptr)
	{
	    tcount++;
	    tptr = tptr->next;
	}
	y2debug ("Module with %d table entries", tcount);

	Bytecode::writeInt32 (str, tcount);

	tptr = m_tenvironment;
	while (tptr)
	{
	    tptr->tentry->toStream (str);		// write the table entries
	    tptr = tptr->next;
	}
    }
    }

    u_int32_t count = 0;				// count statements
    stmtlist_t *stmt = m_statements;
    while (stmt)
    {
	count++;
	stmt = stmt->next;
    }

    y2debug ("YBlock %p: %d statements", this, count);
    Bytecode::writeInt32 (str, count);

    stmt = m_statements;				// write statements
    while (stmt)
    {
	stmt->stmt->toStream (str);
	stmt = stmt->next;
    }

    for (int i = m_count-1; i >= 0; i--)		// pop imported blocks
    {
	if (m_senvironment[i]->category() == SymbolEntry::c_module)
	{
	    Bytecode::popBlock ((YBlock *)(m_senvironment[i]->code()));
	}
    }

    Bytecode::popBlock (this);

    y2debug ("done");
    return str;
}


// EOF
