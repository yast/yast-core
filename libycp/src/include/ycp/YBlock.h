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
#include <list>
using std::string;
#include <y2util/Ustring.h>

#include <y2/Y2Namespace.h>
#include "ycp/YStatement.h"


/**
 * filename hash
 * can't use Ustring directly since we need to traverse the
 * hash table during bytecode I/O
 *
 */


class YSImport;

/**
 * block (-> list of statements, list of symbols)
 */

class YBlock : public YCode, public Y2Namespace  {
private:
    // hash for unique strings, used for filenames
    static UstringHash m_filenameHash;

public:
    // block kinds
    typedef enum {
	b_unknown = 0,		// 0: unspecified
	b_module,		// 1: toplevel module (-> m_table != 0)
	b_file,			// 2: toplevel file block
	b_statement,		// 3: used as statement (local block)
	b_definition,		// 4: used as function definition
	b_value,		// 5: used as value (intermediate block)
	b_namespace,		// 6: block defines a namespace
	b_using			// 7: block is evaluated in different namespace
    } blockkind_t;

private:
    // Block kind,
    // b_statement == implict return YCPNull() (block is statement, default)
    // else YCPVoid () (treat block as expression)
    blockkind_t m_kind;

    // Pointer to name of block.
    //   normaly empty, non-empty for b_module and b_namespace
    string m_name;

    // Filename of source file
    string m_filename;
    // Timestamp of this block
    string m_timestamp;

    // Environment

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

    // Block content

    struct stmtlist {
	YStatement *stmt;
	struct stmtlist *next;
    };
    typedef struct stmtlist stmtlist_t;

    // linked list of statements
    stmtlist_t *m_statements;

    // pointer to last statement for easier append
    stmtlist_t *m_last_statement;
    
    /**
     * List of all included files so far.
     */
    list<string> m_includes;

public:
    YBlock (const std::string & filename, blockkind_t kind = b_unknown);
    YBlock (std::istream & str);
    ~YBlock ();

    // return name of source file
    virtual const std::string filename () const;
    virtual const std::string timestamp ();
    
    virtual Y2Function* createFunctionCall (const string name);

    // add new value code to this block
    //   (used for functions which accept either symbolic variables or values, e.g. foreach())
    // returns position
    unsigned int newValue (constTypePtr type, YCode *code);

    // SymbolTable for global module environment (m_kind == b_module)
    virtual SymbolTable *table ();

    // add a new table entry to this block
    //  and attach it to m_tenvironment
    //   return NULL if symbol of same name already declared in this block
    TableEntry *newEntry (const char *name, SymbolEntry::category_t cat, constTypePtr type, unsigned int line);

    // add a new namespace entry to this block
    //  and attach it to m_tenvironment
    //   return NULL if symbol of same name already declared in this block
    TableEntry *newNamespace (const string & name, YSImport *block, int line);

    // find symbol in m_senvironment, return -1 if not found
    virtual int findSymbol (const SymbolEntry *entry);

    // release symbol entry from m_senvironment
    //   it's no longer owned by this block but by a ysFunction()
    void releaseSymbol (unsigned int position);
    void releaseSymbol (SymbolEntry *entry);

    // number of local variables (environment entries)
    virtual unsigned int symbolCount ();

    // get entry by position
    virtual SymbolEntry *symbolEntry (unsigned int position);

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
    bool isModule () const	{ return (m_kind == b_module); }		// toplevel module block
    bool isFile () const	{ return (m_kind == b_file); }			// toplevel file block
    bool isStatement () const	{ return (m_kind == b_statement); }		// used as statement (local block)
    bool isDefinition () const	{ return (m_kind == b_definition); }		// used as function definition
    bool isValue () const	{ return (m_kind == b_value); }			// used as value (intermediate block)
    bool isNamespace () const	{ return (m_kind == b_namespace); }		// block defines a namespace

    // returns the return statement if the block just consists of a single return
    YSReturn *justReturn () const;

    // returns the name of the block
    const string name () const;
    void setName (const string & name);

    string toString () const;
    string environmentToString () const;

    // write block to stream
    std::ostream & toStream (std::ostream & str) const;

    virtual YCPValue evaluate (bool cse = false);
    
    // evaluate a single statement
    // this is a special purpose interface for macro player
    // does not handle break, return
    YCPValue evaluate (int statement_index);
    
    int statementCount () const;
    
    // push all local variables to stack, uses SymbolEntry::push()
    void push_to_stack ();
    // pop all local variables from stack, uses SymbolEntry::pop()
    void pop_from_stack ();
    
    // the whole block is parsed, do final changes
    void finishBlock ();

    constTypePtr type () const { return Type::Any; }
    
    /**
     *  Checks, if the given include name is already included in the current block.
     */
    bool isIncluded (string includename) const;
    void addIncluded (string includename);
};


#endif // YBlock_h
