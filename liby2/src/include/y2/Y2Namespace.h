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

class SymbolEntry;
class SymbolTable;
class Point;
class Y2Function;

/**
 * Y2Namespace implements a hash(?) table of nested(?) SymbolEntries and
 * allows to look them up
 */
class Y2Namespace {

protected:
    SymbolTable* m_table;
    unsigned int m_symbolcount;
    map<unsigned int, SymbolEntry *> m_symbols;

    void enterSymbol (string name, SymbolEntry* entry, Point *point = 0);
public:
    
    Y2Namespace ();

    virtual ~Y2Namespace();

    //! what namespace do we implement
    virtual const string name () const = 0;
    //! used for error reporting
    virtual const string filename () const = 0;
    
    //! somehow needed for function declarations ?!
    virtual unsigned int symbolCount () const;

    //! function parameters ??
    // bytecode uses unsigneds
    virtual SymbolEntry* symbolEntry (unsigned int position) const;

    //! unparse. useful  only for YCP namespaces??
    virtual string toString () const;
    //! called when evaluating the import statement
    // constructor is handled separately
    virtual YCPValue evaluate (bool cse = false) = 0;

    //! get our whole symbol table?
    virtual SymbolTable* table () const;
    
    /**
     * Creates a function call instance, which can be used to call a 
     * function from this namespace.
     *
     * @param name	name of the required function
     * @return 		an object, that can be used to call the function, or NULL on error
     */
    virtual Y2Function* createFunctionCall (const string name) = 0;
};


#endif // Y2Namespace_h
