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

   File:	YCPScope.h
		scope handling for variables and definitions

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPScope_h
#define YCPScope_h

#include "YCP.h"
#include "ycp/y2log.h"

class YCPScopeInstance;

class YCPScope
{
private:
    int scopeDebugEnabled;
    void scopeDebug(const char *message, ...) const;

    vector <YCPScopeInstance *> global_stack;
    vector <YCPScopeInstance *> top_stack;
    vector <int> level_stack;

protected:
    /**
     * the current global nesting level
     */
    int scopeLevel;

    /**
     * points to the lowest (outermost) created ScopeInstance.
     *  this is the global scope, hence the name.
     */
    YCPScopeInstance *scopeGlobal;

    /**
     * points to the highest (innermost) created ScopeInstance.
     */
    YCPScopeInstance *scopeTop;

    /**
     * map of named scope instances.
     */
    typedef map<string, YCPScopeInstance *> YCPScopeInstanceContainer;
    YCPScopeInstanceContainer *instances;

    /**
     * find scope instance by name
     */
    YCPScopeInstance *lookupInstance (const string& scopename) const;

public:
    /**
     * line number of the currently evaluated statement
     */
    int current_line;

    /**
     * file name of the currently evaluated ycp code
     */
    string current_file;

    /**
     * Constructor.
     */
    YCPScope ();

    /**
     * Destructor.
     */
    ~YCPScope ();	// no virtual destructor needed

    /**
     * Creates a named scope instance. Scope instances are not stacked
     * but build up namespaces instead. You can only access such instances
     * by name.
     */
    bool createInstance (const string& name);

    /**
     * Opens a named scope instance. All further scope accesses only
     * see this instance !
     */
    bool openInstance (const string& name);

    /**
     * Close a named scope instance. Reverts previous openInstance()
     */
    void closeInstance (const string& name);

    /**
     * Deteting an instance destroys all symbols defined within that instance.
     */
    void deleteInstance (const string& name);

    /**
     * Opens a new scope. This is much like a stackframe in C. New symbols
     * can be declared and shadow symbols with the same name in outer scopes.
     * Variable lookup and manipulation goes from inner to outer scopes.
     * Opening a new scope just increments the scopeLevel. A new scope will
     * be allocated on the first declare().
     */
    void openScope ();

    /**
     * Closes to innermost (current) symbol scope. All current local
     * symbols are dropped.
     * You must keep track that the open... and close... calls are balanced.
     * If the current scope has the same level as the scopeLevel, it will
     * be destroyed. Else the open() was delayed and just a decrement of
     * scopeLevel takes place.
     */
    void closeScope ();

    /**
     * Declares a new symbol in the current scope.
     * @param s the name of the symbol
     * @param d the declaration of the symbol (function)
     * @param v the initial value of the symbol
     * @param globally defines a symbol in the global scope
     * @param as_const defines a constant symbol
     * @param do_warn issues a warning if the symbol shadows another one
     */
    void declareSymbol (const string& s, const YCPDeclaration& d, const YCPValue& v, bool globally, bool as_const = false, bool do_warn = true);

    /**
     * Removes a symbol from the current scope.
     * @param s the name of the symbol
     */
    void removeSymbol (const string& s);

    /**
     * Implements the builtin _dump_scope()
     */
    const YCPValue dumpScope (const YCPList& args) const;

    /**
     * Looks up the value of a symbol that may be declared either
     * in the current or in any outer scope. Inner scopes are looked up
     * first.
     *
     * @returns the value of the symbol or YCPNull(), if
     * no such symbol is declared.
     */
    const YCPValue lookupValue (const string& symbolname, const string& scopename = string("")) const;

    /**
     * Looks up the declaration (type) of a symbol that may be declared either
     * in the current or in any outer scope. Inner scopes are looked up
     * first.
     *
     * @returns the declaration of the symbol or YCPNull(), if
     * no such symbol is declared.
     */
    const YCPDeclaration lookupDeclaration (const string& symbolname, const string& scopename = string("")) const;

    /**
     * Checks if a symbol is locally declared, i.e. if it
     * is contained in the current symbol scope @ref #symbolmap.
     * @param symbolname Name of the symbol
     */
    bool symbolDeclaredLocally (const string& symbolname) const;

    /**
     * Checks if a symbol is globally declared.
     * @param symbolname Name of the symbol
     */
    bool symbolDeclaredGlobally (const string& symbolname) const;

    /**
     * Checks, if a symbol is declared in any scope.
     */
    bool symbolDeclared (const string& symbolname, const string& scopename = string("")) const;

    /**
     * assign new value to already declared symbol
     * return YCPVoid if assignment done (no error)
     * return YCPDeclaration if newvalue doesnt matched declared type
     * return YCPNull if symbolname undeclared
     */
    const YCPValue assignSymbol (const string &symbolname, const YCPValue &newvalue, const string& scopename = string(""));

    /**
     * Give the debugger access to everything.
     */
    friend class YCPDebugger;
};


#endif // YCPScope_h
