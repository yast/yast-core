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
#include <y2util/Ustring.h>

/**
 * @short YCPValueRep representing a term.
 * A YCPTermRep is a YCPValue containing a list plus a string representing
 * the term's name.
 */
class YCPTermRep : public YCPValueRep
{
#define GLOBALNAME "_"
    /**
     * The terms name
     */
    Ustring s;

    /**
     * YCP list representing the term's arguments
     */
    YCPList l;

    /**
     * Set the new term name
     */
    void setName (string name);

protected:
    friend class YCPTerm;

    /**
     * Creates a new and empty term with the string s.
     */
    YCPTermRep(const string& s);

    /**
     * Creates a new term with the string s and argument list l.
     */
    YCPTermRep(const string& s, const YCPList& l);
    
    /**
     * Cleans up
     */
    ~YCPTermRep() {}

public:
    /**
     * Returns the term's name
     */
    string name() const;
    
    /**
     * Returns the term's arguments list
     */

    YCPList args() const;
    
    /**
     * Compares two YCPTerms for equality, greaterness or smallerness.
     * The relation is lexicographically with respect to
     *   1. the name of the term
     *   2. the list of the term if the names are the same.
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
     * the same elements and the same name as this one. 
     * The elements themselves 
     * are <b>not</b> copied, but only cloned!
     */
    virtual const YCPElementRep* shallowCopy() const;

    /**
     * Creates a new term, that is identical to this one with but
     * one new value appended. Doesn't change this term.
     */ 
    YCPTerm functionalAdd(const YCPValue& value) const;

    /**
     * Returns an ASCII representation of the term.
     * Term are denoted by comma separated values enclosed
     * by brackets precedeed by a name, for example `a(1,2) 
     * or `b() or `Hugo_17("hirn", c(true)).
     */
    string toString() const;
    
    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;

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
     * Mapping for the term's list set() function
     */
    void set(const int n, const YCPValue& value);

    /**
     * Mapping for the term's list add() function
     */
    void add(const YCPValue& value);
    
    /**
     * Returns YT_TERM. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

#define CONST_ELEMENT (static_cast<const YCPTermRep*>(element))
#define ELEMENT (const_cast<YCPTermRep*>(static_cast<const YCPTermRep*>(this->writeCopy ())))

/**
 * @short Wrapper for YCPTermRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPTermRep
 * with the arrow operator. See @ref YCPTermRep.
 */
class YCPTerm : public YCPValue
{
    DEF_COW_COMMON(Term, Value);
public:
    YCPTerm(const string& s) : YCPValue(new YCPTermRep(s)) {}
    YCPTerm(const string& s, const YCPList& l) : YCPValue(new YCPTermRep(s, l)) {}
    YCPTerm(bytecodeistream & str);

    string name() const { return CONST_ELEMENT->name (); }
    YCPList args() const { return CONST_ELEMENT->args (); }
    YCPTerm functionalAdd(const YCPValue& value) const { return CONST_ELEMENT->functionalAdd (value); }
    bool isEmpty() const { return CONST_ELEMENT->isEmpty (); }
    int size() const { return CONST_ELEMENT->size (); }
    void reserve (int size) { ELEMENT->reserve (size); }
    YCPValue value(int n) const { return CONST_ELEMENT->value (n); }
    void set(const int n, const YCPValue& value) { ELEMENT->set (n, value); }
    void add(const YCPValue& value) { ELEMENT->add (value); }
};

#undef CONST_ELEMENT
#undef ELEMENT

#endif   // YCPTerm_h
