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

   File:       YCPTerm.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPTerm_h
#define YCPTerm_h


#include <ycp/YCPSymbol.h>
#include <ycp/YCPList.h>

/**
 * @short YCPValueRep representing a term.
 * A YCPTermRep is a YCPValue containing a list plus a symbol representing
 * the term's name.
 */
class YCPTermRep : public YCPValueRep
{
#define GLOBALNAME "_"
    /**
     * The terms symbol
     */
    const YCPSymbol s;

    /**
     * The terms namespace
     * if empty, the term has local namespace (i.e "foo (bar)")
     * if "_", the term has global namespace (i.e. "::foo (bar)")
     * otherwise, the term has defined namespace (i.e. "name::foo (bar)")
     */
    const string n;

    /**
     * YCP list representing the term's arguments
     */
    YCPList l;

protected:
    friend class YCPTerm;

    /**
     * Creates a new and empty term with the symbol s.
     * and local namespace
     */
    YCPTermRep(const YCPSymbol& s);

    /**
     * Creates a new and empty term with the symbol s
     * and given namespace
     */
    YCPTermRep(const YCPSymbol& s, const string& n);

    /**
     * Creates a new term with the symbol s and argument list l.
     * and local namespace
     */
    YCPTermRep(const YCPSymbol& s, const YCPList& l);

    /**
     * Creates a new term with the symbol s and argument list l.
     * and given namespace
     */
    YCPTermRep(const YCPSymbol& s, const string& n, const YCPList& l);

    /**
     * Creates a new and empty term with the symbol s.
     * This time s is of type string.
     * @param s the symbol
     * @param quoted true, if the symbol should be quoted.
     */
    YCPTermRep(string s, bool quoted);

    /**
     * Cleans up
     */
    ~YCPTermRep() {}

public:
    /**
     * Returns the term's symbol
     */
    YCPSymbol symbol() const;

    /**
     * Returns the term's namespace
     */
    string name_space() const;

    /**
     * Returns the term's arguments list
     */

    YCPList args() const;

    /**
     * Returns whether the terms symbol is quoted.
     * A quoted term evaluates to itself wheras an unquoted
     * term is lookup up as macro or builtin function.
     */

    bool isQuoted() const;

    /**
     * Compares two YCPTerms for equality, greaterness or smallerness.
     * The relation is lexicographically with respect to
     *   1. the symbol of the term
     *   2. the list of the term if the symbols are the same.
     *
     * (( `alpha() == `alpha() ) == true )
     * (( `alpha() < `alpha( 1 ) ) == true )
     * (( `alpha( 1 ) == `alpha( 1 ) ) == true )
     * (( `alpha( 1 ) < `beta( 1 ) ) == true )
     * (( `alpha( 1 ) < `alpha( 2 ) ) == true )
     * (( `alpha( 1 ) < { term b = `beta( 1 ); return b; } ) == true )   #term/term
     * (( `alpha( 1 ) > { term b = `beta( 1 ); } ) == true )             #term/nil
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     *
     */
    YCPOrder compare(const YCPTerm &v) const;

    /**
     * Creates a copy of this term, i.e. creates a new term with
     * the same elements and the same symbol as this one.
     * The elements themselves
     * are <b>not</b> copied, but only cloned!
     */
    YCPTerm shallowCopy() const;

    /**
     * Creates a new term, that is identical to this one with but
     * one new value appended. Doesn't change this term.
     */
    YCPTerm functionalAdd(const YCPValue& value) const;

    /**
     * Returns an ASCII representation of the term.
     * Term are denoted by comma separated values enclosed
     * by brackets precedeed by a symbol, for example a(1,2)
     * or b() or Hugo_17("hirn", c(true)).
     */
    string toString() const;

    /**
     * Mapping for the term's list isEmpty() function
     */
    bool isEmpty() const;

    /**
     * Mapping for the term's list size() function
     */
    int size() const;

    /**
     * Mapping for the term's list reserve (int) function
     */
    void reserve (int size);

    /**
     * Mapping for the term's list value() function
     */
    YCPValue value(int n) const;

    /**
     * Mapping for the term's list add() function
     */
    void add(const YCPValue& value);

    /**
     * Returns YT_TERM. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPTermRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPTermRep
 * with the arrow operator. See @ref YCPTermRep.
 */
class YCPTerm : public YCPValue
{
    DEF_COMMON(Term, Value);
public:
    YCPTerm(const YCPSymbol& s) : YCPValue(new YCPTermRep(s)) {}
    YCPTerm(const YCPSymbol& s, const string& n) : YCPValue(new YCPTermRep(s, n)) {}
    YCPTerm(const YCPSymbol& s, const YCPList& l) : YCPValue(new YCPTermRep(s, l)) {}
    YCPTerm(const YCPSymbol& s, const string& n, const YCPList& l) : YCPValue(new YCPTermRep(s, n, l)) {}
    YCPTerm(string s, bool quoted) : YCPValue(new YCPTermRep(s, quoted)) {}
};

#endif   // YCPTerm_h
