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

   File:	YBlock.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YBlock_h
#define YBlock_h

#include <string>
using std::string;

#include "ycp/YStatement.h"

/**
 * block (-> list of statements, list of symbols)
 */

class YBlock : public YCode {
public:
    // block kinds
    typedef enum {
	b_unknown = 0,		// unspecified
	b_module,		// toplevel module (-> m_table != 0)
	b_file,			// toplevel file block
	b_statement,		// used as statement (local block)
	b_body,			// used as function body
	b_value,		// used as value (intermediate block)
	b_namespace,		// block defines a namespace
	b_using			// block is evaluated in different namespace
    } blockkind_t;

private:
    // Block kind,
    // b_statement == implict return YCPNull() (block is statement, default)
    // else YCPVoid () (treat block as expression)
    blockkind_t m_kind;

    // Pointer to name of block.
    //   normaly empty, non-empty for b_module and b_namespace
    string m_name;

    // Environment

    // symbol table for global module environment (m_kind == b_module)
    SymbolTable *m_table;

    struct yTElist {
	struct yTElist *next;
	TableEntry *tentry;
	unsigned int position;
    };
    typedef struct yTElist yTElist_t;

    // block environment (linked list of local declarations)
    //  as TableEntry used during parse time
    yTElist_t *m_tenvironment;

    // pointer to last declaration for easier append
    // points to 0 after detachEnvironment()
    yTElist_t *m_last_tparm;

    // block environment as SymbolEntry array used during
    //  execution time
    unsigned int m_count;
    SymbolEntry **m_senvironment;

    // add symbol to m_senvironment
    unsigned int addSymbol (SymbolEntry *entry);

    // Block body

    struct stmtlist {
	YStatement *stmt;
	struct stmtlist *next;
    };
    typedef struct stmtlist stmtlist_t;

    // linked list of statements
    stmtlist_t *m_statements;

    // pointer to last statement for easier append
    stmtlist_t *m_last_statement;

    // used internally in stream constructor
    YBlock *importModule (const string & name);

public:
    YBlock (blockkind_t kind = b_unknown);
    YBlock (std::istream & str);
    ~YBlock ();

    // add new value code to this block
    //   (used for functions which accept either symbolic variables or values, e.g. foreach())
    // returns position
    unsigned int newValue (const TypeCode &type, YCode *code);

    // SymbolTable for global module environment (m_kind == b_module)
    SymbolTable *table () const;
    void setTable (SymbolTable *table);

    // add a new table entry to this block
    //  and attach it to m_tenvironment
    //   return NULL if symbol of same name already declared in this block
    TableEntry *newEntry (const char *name, SymbolEntry::category_t cat, const TypeCode &type, unsigned int line);

    // add a new module entry to this block
    //  and attach it to m_tenvironment
    //   return NULL if symbol of same name already declared in this block
    TableEntry *importModule (const string & name, int line);

    // find symbol in m_senvironment, return -1 if not found
    int findSymbol (const SymbolEntry *entry);

    // release symbol entry from m_senvironment
    //   it's no longer owned by this block but by a ysFunction()
    void releaseSymbol (unsigned int position);
    void releaseSymbol (SymbolEntry *entry);

    // number of local variables (environment entries)
    unsigned int symbolCount () const;

    // get entry by position
    SymbolEntry *symbolEntry (unsigned int position) const;

    // Attach entry (variable, typedef, ...) to local environment
    void attachEntry (TableEntry *entry);

    // Attach statement to block
    void attachStatement (YStatement *statement);

    // Detach local environment from symbol table
    //  convert m_tenvironment to m_senvironment
    void detachEnvironment (SymbolTable *table);

    // set block kind
    void setKind (blockkind_t kind);

    // get block kind
    blockkind_t kind () const;

    // block is toplevel block of a module
    bool isModule () const;

    // returns the return statement if the block just consists of a single return
    YSReturn *justReturn () const;

    // returns the name of the block
    const string & name () const;
    void setName (const string & name);

    string toString () const;
    string environmentToString () const;

    std::ostream & toStream (std::ostream & str) const;

    // normal evaluation with initialization of local environment
    YCPValue evaluate (bool cse = false);

    // loop block evaluation without initialization of local environment
    YCPValue evaluateNoInit ();
};


#endif // YBlock_h
