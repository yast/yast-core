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

#include <stack>

// needed for YBlock

#include "ycp/SymbolEntry.h"
#include "ycp/SymbolTable.h"
#include "ycp/YExpression.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"
#include "ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

UstringHash YBlock::m_filenameHash;

// ------------------------------------------------------------------
// block (-> list of statements, list of symbols, kind)

YBlock::YBlock (const std::string & filename, YBlock::blockkind_t kind)
    : YCode (yeBlock)
    , m_kind (kind)
    , m_name ("")
    , m_filename (m_filenameHash.add (filename))
    , m_timestamp ("")
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


const std::string 
YBlock::filename () const
{
    return m_filename;
}


const std::string 
YBlock::timestamp ()
{
    if ( m_timestamp == "" )
    {
	// we don't have timestamp computed yet, do it now.
	m_timestamp = Y2Namespace::timestamp ();
    }
    return m_timestamp;
}


// private:
// add symbol to m_senvironment
unsigned int
YBlock::addSymbol (SymbolEntry *entry)
{
    y2debug ("YBlock::addSymbol (%s @%d)", entry->toString().c_str(), m_count);

    m_count++;
    // FIXME: make a more room at once
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
YBlock::newValue (constTypePtr type, YCode *code)
{
    static char name[8];
    sprintf (name, "_%d", m_count);		// create 'fake' name

    y2debug ("YBlock::newValue (%s %s)", type->toString().c_str(), name);

// FIXME: check duplicates
    return addSymbol (new SymbolEntry (this, m_count, name, SymbolEntry::c_const, type, code));
}


// SymbolTable for global module environment (m_kind == b_module)
SymbolTable *
YBlock::table ()
{
    y2debug ("returning table");
    y2debug (" %p", m_table );
    return m_table;
}


// add new table entry to this block
//  and attach it to m_tenvironment
//   return NULL if symbol of same name already declared in this block
TableEntry *
YBlock::newEntry (const char *name, SymbolEntry::category_t cat, constTypePtr type, unsigned int line)
{
    y2debug ("YBlock(%p)::newEntry (%s, cat %d, type %s, line %d)", this, name, (int)cat, type->toString().c_str(), line);

    // check duplicates
    for (unsigned int p = 0; p < m_count; p++)
    {
	if ((m_senvironment[p] != 0)
	    && (strcmp (m_senvironment[p]->name(), name) == 0)
	    && !m_senvironment[p]->likeNamespace())		// allow symbol if namespace of same name already declared
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
YBlock::symbolCount (void)
{
    return m_count;
}


// get entry by position
SymbolEntry *
YBlock::symbolEntry (unsigned int position)
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
    y2debug ("YBlock[%p]::attachEntry (%p)", this, tentry/*->toString().c_str()*/);

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
    y2debug ("YBlock[%p]::attachStatement (%p:%s)", this, statement, statement->toString().c_str());

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


// bind namespace entry
TableEntry *
YBlock::newNamespace (const string & name, YSImport *imp, int line)
{
    Y2Namespace *block = imp->block ();

    y2debug ("YBlock::newNamespace (%s<%p>):%d to %p", name.c_str(), block, line, this);
    
    TableEntry *tentry = newEntry (strdup (name.c_str()), SymbolEntry::c_module, Type::Unspec, line);
    tentry->sentry()->setNamespace (block);

    y2debug ("environment of %p after addNamespace (%s)", this, environmentToString().c_str());
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
YBlock::finishBlock ()
{
    // ATM, we only reorder sentries, so global ones are on top of the table
    // it is important to allow a bytecode ignore changes in local symbols - the indexes for globals
    // are not changed
    if ( m_count == 0 ) return;
    
    y2debug ("Going to reorder");
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

    y2debug ("Reorder done");
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


YSReturn *
YBlock::justReturn () const
{
    if (m_statements != 0)
    {
	YStatement *stmt = m_statements->stmt;
	if (stmt->kind() == YCode::ysReturn)
	{
	    return (YSReturn *)stmt;
	}
    }
    return 0;
}


const string 
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
    if (isModule()
	|| isFile())
    {
	s += "\n    // filename: \"" + (std::string)m_filename + "\"";
    }

    stmtlist_t *stmt = m_statements;
    while (stmt)
    {
//	y2debug ("Statements so far: %s", s.c_str ());
	s += "\n    ";
	s += stmt->stmt->toString();
	stmt = stmt->next;
    }

    s += "\n}\n";
    return s;
}


void
YBlock::push_to_stack ()
{
    // push the stack for recursion    
    if (m_senvironment)
    {
        for (unsigned int p = 0; p < m_count; p++)
        {
            if (m_senvironment[p] != 0 && m_senvironment[p]->isVariable ())
            {
                y2debug ("Push %s", m_senvironment[p]->name ());
		m_senvironment[p]->push ();
            }
        }
    }
}

void
YBlock::pop_from_stack ()
{
    // pop the stack for recursion    
    if (m_senvironment)
    {
        for (unsigned int p = 0; p < m_count; p++)
        {
            if (m_senvironment[p] != 0 && m_senvironment[p]->isVariable ())
            {
                y2debug ("Pop %s", m_senvironment[p]->name ());
		m_senvironment[p]->pop ();
            }
        }
    }
}

YCPValue
YBlock::evaluate (bool cse)
{
    if (cse)
    {
	return YCPNull();
    }

    y2debug ("YBlock::evaluate([%d]%s)\n", (int)m_kind, toString().c_str());
    
    ee.pushBlock (this);

    stmtlist_t *stmt = m_statements;
    YCPValue value = YCPVoid ();
    while (stmt)
    {
	YStatement *statement = stmt->stmt;
	
#if 0
	y2warning ("%d: %s", statement->line (), statement->toString ().c_str ());
#endif

	ee.setStatement (statement);
	value = statement->evaluate ();

	if (!value.isNull())
	{
	    y2debug ("Block exit (%s)", value->toString().c_str());
	    break;
	}

	stmt = stmt->next;
    }
    
    ee.popBlock ();

    y2debug ("YBlock::evaluate done (stmt %p, kind %d, value '%s')\n", stmt, m_kind, value.isNull() ? "NULL" : value->toString().c_str());

    // if stmt==0 we're at the end of the block. If the block is evaluated as a statement,
    //   it returns NULL, else it returns Void
    if (stmt == 0)
    {
	if (m_kind == b_statement)
	{
	    return YCPNull();
	}
	return YCPVoid();
    }

    // if stmt!=0 we just evaluated a break or return statement. A 'return;' evaluates to YCPReturn
    return value;
}


YCPValue
YBlock::evaluate (int statement_index)
{
    y2debug("YBlock::evaluate(#%d)\n", statement_index);
    
    stmtlist_t *stmt = m_statements;
    YCPValue value = YCPVoid ();
    while (stmt && statement_index > 0)
    {
	stmt = stmt->next;
	statement_index--;
    }
    
    if (!stmt)
    {
	// we are at the end
	return YCPNull ();
    }
    
    y2milestone("YBlock::evaluating:\n%s", stmt->stmt->toString ().c_str());
    
    value = stmt->stmt->evaluate ();
    
    // y2debug("YBlock::evaluate statement done (value '%s')\n", value.isNull() ? "NULL" : value->toString().c_str());

    return value;
}


int
YBlock::statementCount () const
{
    int res = 0;
    stmtlist_t *stmt = m_statements;

    while (stmt)
    {
	stmt = stmt->next;
	res++;
    }
    
    return res;
}

// --------------------------
// bytecode I/O

YBlock::YBlock (std::istream & str)
    : YCode (yeBlock)
    , m_kind (b_unknown)
    , m_name ("")
    , m_filename ("")
    , m_tenvironment (0)
    , m_last_tparm (0)
    , m_senvironment (0)
    , m_statements (0)
    , m_last_statement (0)
{
    Bytecode::readString (str, m_name);		// read name
    Bytecode::readString (str, m_filename);	// FIXME
    Bytecode::readString (str, m_timestamp);	// FIXME

    m_kind = (blockkind_t)Bytecode::readInt32 (str);

    Bytecode::pushBlock (this);

    m_count = Bytecode::readInt32 (str);	// read m_senvironment

    y2debug("YBlock::fromStream (%p:\"%s\", %d entries, kind %d)", this, m_name.c_str(), m_count, m_kind);

    // read all symbol entries belonging to this block

    if (m_count > 0)
    {
	m_senvironment = (SymbolEntry **)realloc (m_senvironment, sizeof (SymbolEntry *) * m_count);

	for (unsigned int i = 0; i < m_count; i++)
	{
	    SymbolEntry *entry = new SymbolEntry (str, this);
	    m_senvironment[i] = entry;
	}

	if (m_kind == b_module)			// if its a module, re-construct the table
	{
	    int tcount = Bytecode::readInt32 (str);
	    y2debug ("Module with %d table entries", tcount);
	    
	    // HACK ahead: Y2ALLGLOBAL should make all
	    // symbols visible. It works now, but as a trade-off,
	    // line numbers are lost also for globals.
	    while (tcount-- > 0)
	    {
		if (getenv("Y2ALLGLOBAL") != NULL)
		{
		    TableEntry t(str);
		}
		else
		{
		    TableEntry *tentry = new TableEntry (str);
		    attachEntry (tentry);
		    m_table->enter (tentry);
		}
	    }
	    
	    if (getenv("Y2ALLGLOBAL") != NULL)
	    {
		delete m_table; // FIXME: memory leak
		m_table = new SymbolTable(211);
		for (uint i = 0 ; i < m_count ; i++)
		{
		    TableEntry* te = m_table->enter(m_senvironment[i]->name (), 
			m_senvironment[i], 0);
		    attachEntry (te);
		}
	    }
	}
    }

    u_int32_t count = Bytecode::readInt32 (str);
    y2debug ("YBlock::fromStream (%p:\"%s\", %d statements)", this, m_name.c_str(), count);

    stmtlist_t *stmt;
    for (u_int32_t i = 0; i < count; i++)
    {
	stmt = new stmtlist_t;

	YCode *code = Bytecode::readCode (str);
	if (code == 0)
	{
	    m_kind = b_unknown;
	    break;
	}
	if (!code->isStatement())			// code must be statement
	{
	    y2error ("Code is no statement: %d", code->kind());
	    break;
	}

	stmt->stmt = (YStatement *)code;
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

	//   push module block for imported modules !
	if (stmt->stmt->kind() == YCode::ysImport)
	{
	    y2debug ("pushBlock import");
	    Bytecode::pushBlock (((YSImport *)(stmt->stmt))->block());
        }
    }

    // pop imported modules blocks
    stack<Y2Namespace*> to_pop;

    stmt = m_statements;
    for (u_int32_t i = 0; i < count; i++)
    {
	if (!stmt)
	{
	    y2error ("Statement chain broken after %d of %d statements", i, count);
	    m_kind = b_unknown;
	    break;
	}

	if (stmt->stmt->kind() == YCode::ysImport)
	{
	    to_pop.push (((YSImport *)(stmt->stmt))->block ());
	}
	stmt = stmt->next;
    }
    
    while( ! to_pop.empty () ) {
	y2debug ("popBlock import");
	Bytecode::popBlock (to_pop.top ());
	to_pop.pop();
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
    Bytecode::writeString (str, m_filename);		// FIXME
    Bytecode::writeString (str, m_timestamp);		// FIXME
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
	}

	// if its a module, write the table

	if (isModule())	
	{
	    yTElist_t *tptr = m_tenvironment;
	    int tcount = 0;				// count the table entries
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
//	y2debug ("%d: %s", count, stmt->stmt->toString().c_str());
	stmt = stmt->next;
    }

    y2debug ("YBlock %p: %d statements", this, count);
    Bytecode::writeInt32 (str, count);

    stmt = m_statements;				// write statements
    while (stmt)
    {
//	y2debug ("toStream: %s", stmt->stmt->toString().c_str());
	stmt->stmt->toStream (str);

	// push imported modules
        if (stmt->stmt->kind() == YCode::ysImport)
        {
            y2debug ("pushBlock import");
            Bytecode::pushBlock (((YSImport *)(stmt->stmt))->block());
        }

	stmt = stmt->next;
    }

    for (int i = m_count-1; i >= 0; i--)		// pop imported blocks
    {
	if (m_senvironment[i]->category() == SymbolEntry::c_module)
	{
	    Bytecode::popBlock (m_senvironment[i]->name_space());
	}
    }

    Bytecode::popBlock (this);

    y2debug ("done");
    return str;
}

bool YBlock::isIncluded (string includefile) const
{
    return find (m_includes.begin (), m_includes.end (), includefile) != m_includes.end ();
}

void YBlock::addIncluded (string includefile)
{
    if (find (m_includes.begin (), m_includes.end (), includefile) == m_includes.end ())
    {
	m_includes.push_back (includefile);
    }
}

Y2Function* YBlock::createFunctionCall (const string name)
{
    TableEntry *func_te = table ()->find (name.c_str (), SymbolEntry::c_function);
    
    // can't find the function definition
    if (func_te == NULL || !func_te->sentry ()->isFunction ()) 
	return NULL;
    
    y2debug ("allocating new YEFunction %s", name.c_str ());
    return new YEFunction (func_te->sentry ());
}

// EOF
