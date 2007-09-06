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
#include <list>
#include <stack>

// MemUsage.h defines/undefines D_MEMUSAGE
#include <y2util/MemUsage.h>

#include <y2/SymbolEntry.h>

class SymbolTable;
class Point;

// TableEntry

class TableEntry
#ifdef D_MEMUSAGE
   : public MemUsage
#endif
{
    // hash bucket pointers (all TableEntries of a bucket have the same hash value)
    TableEntry *m_prev;
    TableEntry *m_next;
    
    // pointers to the next/prev entry with the same key and type -> overloaded
    // entries. Only the first one has a valid m_outer pointer!!!
    TableEntry *m_overloaded_prev;
    TableEntry *m_overloaded_next;

    // nesting pointers, implements stacked environments (of nested blocks)
    //   when adding a new entry (via SymbolTable::enter())
    //   and another entry with the same key (variable name) already exists,
    //   this adding must be the result of a new block (scope)
    //   since duplicate entries into the same block are checked by
    //   the parser.
    // when looking up a key, we start with the innermost entry which
    //   represents the 'most recent' definition
    // when removing a key, only the innermost entry is removed.

    TableEntry *m_outer;

    const char *m_key;			// search key, usually the symbol name
    SymbolEntryPtr m_entry;		// complete symbol data, cannot be const since category might change
    const Point *m_point;		// definition point (file, line)

    SymbolTable *m_table;		// backpointer to table

protected:
    friend class SymbolTable;

public:
    size_t mem_size () const { return sizeof (TableEntry); }
    TableEntry (const char *key, SymbolEntryPtr entry, const Point *point, SymbolTable *table = 0);
    TableEntry (bytecodeistream & str);
    ~TableEntry ();
    const char *key () const;
    TableEntry *next () const;
    TableEntry *next_overloaded () const;
    bool isOverloaded () const;
    const SymbolTable *table () const;
    SymbolEntryPtr sentry () const;
    const Point *point () const;
    string toString () const;
    string toStringSymbols () const;
    void makeDefinition (int line);	// convert declaration to definition (exchanges m_point)
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;

    // remove yourself from the SymbolTable.
    void remove ();
};


class SymbolTable
#ifdef D_MEMUSAGE
   : public MemUsage
#endif
{
private:
    // number of buckets in hash table
    int m_prime;

    // the hash function
    int hash (const char *s);

    // the hash table [0 ... prime-1]
    // a bucket is a (doubly) linked list of TableEntries
    // (via m_prev/m_next) each having the same hash value
    // (standard hash table implementation)
    // Additionally, scopes are represented by linking
    // TableEntries with equal key values (and naturally the
    // same hash value) via m_outer. The start of
    // this chain always represents the innermost (most
    // recent) definition of a symbol.

    TableEntry **m_table;

    // these are the actually used entries of this table
    // they are only stored if needed
    bool m_track_usage;
    std::map<const char *, TableEntry *> *m_used;

    // stack of external references, needed during bytecode I/O by YSImport
    //  (triggered by openReferences())
    typedef std::stack <std::vector<TableEntry *> *> xrefs_t;
    xrefs_t* m_xrefs;

public:
    size_t mem_size () const { return sizeof (SymbolTable); }
    //---------------------------------------------------------------
    // Constructor/Destructor

    // create SymbolTable with hashsize prime
    SymbolTable (int prime);
    ~SymbolTable();

    //---------------------------------------------------------------
    // Table access

    // full copy of the current table into a namespace
    void tableCopy(Y2Namespace* tofill) const;

    // return size of hash table
    int size() const;

    // consumer returns true to continue iterating
    typedef bool (* EntryConsumer) (const SymbolEntry&);
    /**
     * call the consumer for each entry (or until it requests a break)
     *
     * Example:
     *   static bool MyClass::DoIt (const SymbolEntry & se) {
     *     cout << se.name() << endl;
     *     return true;
     *   }
     *
     *   void MyClass::Foo () {
     *     my_y2namespace->table()->forEach (&DoIt);
     *   }
     */
    void forEach (EntryConsumer consumer) const;

    //---------------------------------------------------------------
    // enter/find/remove

    // enter SymbolEntry to table as innermost definition
    TableEntry *enter (const char *key, SymbolEntryPtr entry, const Point *point);

    // enter TableEntry to table as innermost definition
    TableEntry *enter (TableEntry *entry);

    // find (innermost) TableEntry by key and add to m_used
    TableEntry *find (const char *key, SymbolEntry::category_t category = SymbolEntry::c_unspec);

    // find (innermost) TableEntry by key and add to m_xrefs
    TableEntry *xref (const char *key);

    // remove the entry from table
    void remove (TableEntry *entry);

    //---------------------------------------------------------------
    // xref tracking

    // push empty list of references on top of m_references
    // start tracking references, keep a list of actually referenced (-> find()) entries
    void openXRefs ();

    // pop current list of references from top of m_references
    void closeXRefs ();

    // return the vector of references from top of m_references
    SymbolEntryPtr getXRef (unsigned int position) const;

    //---------------------------------------------------------------
    // usage tracking

    void startUsage ();

    int countUsage ();

    void endUsage ();

    void enableUsage ();
    void disableUsage ();

    //---------------------------------------------------------------
    // write usage to stream, see YSImport

    std::ostream &writeUsage (std::ostream & str) const;
    std::ostream &writeXmlUsage( std::ostream & str, int indent ) const;

    //---------------------------------------------------------------
    // string

    string toString() const;
    string toStringSymbols() const;
};

#endif // SymbolTable_h
