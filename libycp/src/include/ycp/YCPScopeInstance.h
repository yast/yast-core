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

   File:	YCPScopeInstance.h
		scope instance handling for variables and definitions

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPScopeInstance_h
#define YCPScopeInstance_h

#include <map>
#include "YCP.h"

/**
 * a scope element is usually a variable or function
 * definition with a defined scope.
 * the scope element is unambigously identified by
 * its name and its scope. There are no two YCPScopeElements
 * with the same name and same scope.
 *
 * For each element, its name, its type (YCPDeclaration)
 * and its value is stored.
 */

class YCPScopeElement
{
private:
    /**
     * the declaration (type) of the element
     */
    YCPDeclaration m_type;

    /**
     * the (current) value of the element
     */
    YCPValue m_value;

    /**
     * declared as "const" ?
     */
    bool m_const;

    /**
     * line of declaration, used to determine double declaration warning
     */
    int m_line;

public:
    YCPScopeElement ()
	: m_type (YCPNull())
	, m_value (YCPNull())
	, m_const (false)
	, m_line (0) {}
    YCPScopeElement (const YCPDeclaration& d, const YCPValue& v, bool as_const = false, int line = 0)
	: m_type (d)
	, m_value (v)
	, m_const (as_const)
	, m_line (line)
    {}

    YCPDeclaration get_type () const { return m_type; }
    bool is_const () const { return m_const; }
    int get_line () const { return m_line; }
    void set_line ( int line ) { m_line = line; }
    YCPValue get_value () const { return m_value; }
    YCPValue set_value (const YCPValue& v) {
	if (is_const()) {
	    return YCPNull();
	}
	YCPValue old = m_value;
	m_value = v;
	return old;
    }

};

//-------------------------------------------------------------------


/**
 * this implements the (stacked) scope.
 *
 * Since the scope access always goes through the initial scope
 * (base class of YCPBasicInterpreter), the lookup functions use
 * the topmost scope (via scopeTop) to start with the innermost
 * ScopeInstance.
 *
 * Scopes are nested and only created on demand (delayed creation)
 * so we count the global nesting level only and track the scope
 * level with the ScopeInstance. open()/close() increment/decrement the
 * global level and destroy only levels matching the global
 * level.
 */

class YCPScopeInstance
{
public:
    YCPScopeInstance ();
    ~YCPScopeInstance ();

    /**
     * the active level of this scope.
     */
    int level;

    typedef map<string, YCPScopeElement *> YCPScopeContainer;

    YCPScopeContainer *container;

    /**
     * scopes are stacked. the most recent scope is the 'inner' scope,
     * all others are 'outer' scopes. this link leads to the nearest
     * outer scope.
     */
    YCPScopeInstance *outer;

    /**
     * pointer to local scope for global module instances
     */
    YCPScopeInstance *inner;


    /**
     * Check if the scope is empty
     */
    bool empty () const;

    void dumpScope () const;

    int declareSymbol (const string& s, const YCPDeclaration& d, const YCPValue& v, bool as_const = false, int line = 0);

    bool removeSymbol (const string& s);

    YCPValue lookupValue (const string& symbolname) const;

    YCPDeclaration lookupDeclaration (const string& symbolname) const;

    YCPValue assignSymbol (const string &symbolname, const YCPValue &newvalue);

//    int symbolLine (const string& symbolname) const;

    /**
     * Give the debugger access to everything.
     */
    friend class YCPDebugger;
};

#endif // YCPScopeInstance_h
