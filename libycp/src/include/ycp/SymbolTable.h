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

   File:	SymbolTable.h
		hash table class

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

   SymbolTable implements a hash table of nested SymbolEntries

/-*/
// -*- c++ -*-

#ifndef SymbolTable_h
#define SymbolTable_h

#include <string>
using std::string;

#include "ycp/SymbolEntry.h"

class SymbolTable;

// TableEntry

class TableEntry {
    // hash bucket pointers (all TableEntries of a bucket have the same hash value)
    TableEntry *m_prev;
    TableEntry *m_next;

    // nesting pointers, implements stacked environments (of nested blocks)
    //   when adding a new entry (via SymbolTable::enter())
    //   and another entry with the same key (variable name) already exists,
    //   this adding must be the result of a new block (scope)
    //   since duplicate entries into the same block are checked by
    //   the parser.
    // when looking up a key, we start with the innermost entry which
    //   represents the 'most recent' definition
    // when removing a key, only the innermost entry is removed.

    TableEntry *m_inner;
    TableEntry *m_outer;

    const char *m_key;			// search key, usually the symbol name
    SymbolEntry *m_entry;		// complete symbol data, cannot be const since category might change
    int m_line;				// line number in defining file

    SymbolTable *m_table;		// backpointer to table

protected:
    friend class SymbolTable;

public:
    TableEntry (const char *key, SymbolEntry *entry, int line, SymbolTable *table = 0);
    TableEntry (std::istream & str);
    ~TableEntry ();
    const char *key() const;
    TableEntry *next() const;
    const SymbolTable *table() const;
    SymbolEntry *sentry() const;
    int line() const;
    void setLine (int line);
    string toString() const;
    string toStringSymbols() const;
    std::ostream & toStream (std::ostream & str) const;

    // remove yourself from the SymbolTable.
    void remove ();
};


class SymbolTable {
private:
    // number of buckets in hash table
    int m_prime;

    // the hash function
    int hash (const char *s);

    // the hash table [0 ... prime-1]
    // a bucket is a (doubly) linked list of TableEntries
    // (via m_prev/m_next) each having the same hash value
    // (standard hash table implementation)
    // Additionally, scopes are represented by doubly linking
    // TableEntries with equal key values (and naturally the
    // same hash value) via m_inner/m_outer. The start of
    // this chain always represents the innermost (most
    // recent) definition of a symbol.

    TableEntry **m_table;


public:
    // create SymbolTable with hashsize prime
    SymbolTable(int prime);
    ~SymbolTable();

    // access table (for traversal)
    const TableEntry **table() const;

    // return size of hash table
    int size() const;

    // enter SymbolEntry to table as innermost definition
    TableEntry *enter (const char *key, SymbolEntry *entry, int line);

    // enter TableEntry to table as innermost definition
    TableEntry *enter (TableEntry *entry);

    // find (innermost) TableEntry by key
    TableEntry *find (const char *key, SymbolEntry::category_t category = SymbolEntry::c_unspec);

    // remove the entry from table
    void remove (TableEntry *entry);

    string toString() const;
    string toStringSymbols() const;
};

#endif // SymbolTable_h
