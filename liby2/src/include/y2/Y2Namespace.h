/*------------------------------------------------------------*- c++ -*-\
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

   File:	Y2Namespace.h
		a generic interface for accessing a namespace from YCP interpreter

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#ifndef Y2Namespace_h
#define Y2Namespace_h

#include <string>
using std::string;

#include "ycp/YCPValue.h"
#include "ycp/SymbolEntryPtr.h"

class SymbolTable;
class Point;
class Y2Function;
class StaticDeclaration;

/**
 * Y2Namespace implements a hash(?) table of nested(?) SymbolEntries and
 * allows to look them up
 */
class Y2Namespace {
protected:
    SymbolTable* m_table;
    unsigned int m_symbolcount;
    map<unsigned int, SymbolEntryPtr> m_symbols;

    // add symbol to namespace, it now belongs here
    // returns the index into m_symbols
    //
    // this is used for blocks with a local environment but no table
    unsigned int addSymbol (SymbolEntryPtr sentry);

    // add symbol _and_ enter into table for lookup
    //
    // this is used for namespaces with a global environment and a table
    void enterSymbol (SymbolEntryPtr sentry, Point *point = 0);

    // lookup symbol by name in m_symbols
    SymbolEntryPtr lookupSymbol (const char *name) const;

    // find symbol by pointer
    // return index if found, -1 if not found
    int findSymbol (const SymbolEntryPtr sentry) const;

    // release symbol from m_symbols
    //   it's no longer owned by this block but by a ysFunction()
    void releaseSymbol (unsigned int position);
    void releaseSymbol (SymbolEntryPtr sentry);
    
    bool m_initialized;

public:
    
    Y2Namespace ();

    virtual ~Y2Namespace();

    // end of symbols, finish and clean up m_symbols
    void finish ();

    //! what namespace do we implement
    virtual const string name () const;
    //! used for error reporting
    virtual const string filename () const = 0;
    
    //! gives the number of symbol declarations
    //  e.g. needed for function declarations which keep their symbolic
    //   parameters in a Y2Namespace
    virtual unsigned int symbolCount () const;

    //! access to definitions of this namespace
    // bytecode uses unsigneds
    virtual SymbolEntryPtr symbolEntry (unsigned int position) const;

    //! unparse. useful for debugging
    virtual string toString () const;

    // just m_symbols, for debugging and YBlock::toString
    string symbolsToString () const;

    //! called when evaluating the import statement
    // constructor is handled separately
    virtual YCPValue evaluate (bool cse = false) = 0;

    //! get our whole symbol table?
    virtual SymbolTable* table () const;

    // this will ensure existence of the table.
    // after calling this function @ref table will always return a valid pointer
    void createTable ();

    /**
     * Creates a function call instance, which can be used to call a 
     * function from this namespace.
     *
     * @param name	name of the required function
     * @return 		an object, that can be used to call the function, or NULL on error
     */
    virtual Y2Function* createFunctionCall (const string name) = 0;

    // push all local variables to stack, uses SymbolEntry::push()
    void pushToStack ();

    // pop all local variables from stack, uses SymbolEntry::pop()
    void popFromStack ();
    
    // ensure that the namespace is initialized
    virtual void initialize ();

};


#endif // Y2Namespace_h
