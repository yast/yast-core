/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       YCPElement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPElement_h
#define YCPElement_h

#include <y2util/MemUsage.h>

// include only forward declarations of iostream
#include <iosfwd>
#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::pair;

#include "toString.h"

// forward declarations

class YCPElement;
class YCPValue;
class YCPVoid;
class YCPBoolean;
class YCPInteger;
class YCPFloat;
class YCPString;
class YCPByteblock;
class YCPPath;
class YCPSymbol;
class YCPLocale;
class YCPList;
class YCPTerm;
class YCPMap;
class YCPBuiltin;
class YCPCode;
class YCPEntry;
class YCPReference;


#define DEF_OPS(name)						\
public:								\
    const YCP##name##Rep *operator ->() const {			\
	return static_cast<const YCP##name##Rep *>(element); }	\
    YCP##name##Rep *operator ->() {				\
	return const_cast<YCP##name##Rep *>(			\
	       static_cast<const YCP##name##Rep *>(element)); }	\
private:							\
    int operator !() const;					\
    int operator ==(const YCPElement &) const;

#define DEF_COMMON(name, base)					\
    DEF_OPS(name)						\
    friend class YCP##base##Rep;				\
public:								\
    YCP##name(const YCPNull &n) : YCP##base(n) {}		\
protected:							\
    YCP##name (const YCP##name##Rep *x) : YCP##base(x) {}


#define DEF_COW_OPS(name)					\
public:								\
    const YCP##name *operator ->() const {			\
	return static_cast<const YCP##name *>(this); }		\
    YCP##name *operator ->() {					\
	return const_cast<YCP##name *>(				\
	       static_cast<const YCP##name *>(this)); }		\
private:							\
    int operator !() const;					\
    int operator ==(const YCPElement &) const;

#define DEF_COW_COMMON(name, base)				\
    friend class YCP##base##Rep;				\
    DEF_COW_OPS(name)						\
public:								\
    YCP##name(const YCPNull &n) : YCP##base(n) {}		\
protected:							\
    YCP##name (const YCP##name##Rep *x) : YCP##base(x) {}	\
public:								\
    YCPOrder compare(const YCP##name x) const {			\
	return (static_cast<const YCP##name##Rep*>(element))->compare(x);				\
    }								\
    string toString () const { return element->toString (); }	\
    std::ostream & toStream (std::ostream & str ) const {	\
	return element->toStream (str);				\
    }								\
    YCPValueType valuetype () const { return (static_cast<const YCP##name##Rep*>(element))->valuetype (); }


class YCPNull {};

/***
 * <h2>The YCP Datastructures</h2>
 *
 * <p>YCP is both a scripting language and a communication protocol.
 * A YCP value is a data structure. It has currently two possible
 * representations. One ist an ASCII representation, the other is
 * a representation a network of C++ objects. The class framework
 * for this object representation is laid in this library. Furthermore
 * It contains the @ref Parser, that transforms an ASCII representation
 * of YCP values into the object-representation.
 *
 * <h2>YCP Data management</h2>
 *
 * <p>YCP data are managed via "smart pointers". This means that an application
 * instantiates an object from a class in a conventional way but does not get
 * the object itself. The result is a wrapper object that "points" to the real
 * object which is automatically created on instantiation. This real object
 * (the representation of the actual data) holds a reference counter and
 * therefore "knows" who is using it. After all references to this object have
 * diminished (e.g. auto variables get out of scope) the real object is
 * automatically destroyed. This way it is neither necessary nor allowed
 * to "new" or "delete" YCP data objects. Furthermore all members of the
 * object must be accessed using pointer notation.
 *
 * <p>So all YCP data objects do exist in two flavours:
 * <ul>
 * <li>Class name without "Rep" is the usable object, e.g. @ref YCPInteger.</li>
 * <li>Class name with "Rep" is the real Object, e.g. @ref YCPIntegerRep.</li>
 * </ul>
 *
 * <p>Important: Applications <i>never</i> use "Rep" classes directly!
 *
 * <p>Example:
 * <pre>
 * {
 *    YCPInteger a (18);         // here a YCPIntegerRep is created automatically
 *    YCPInteger b = a;          // b points to the same YCPIntegerRep as a
 *
 *    cout << b->toString ();    // use pointer notation to access members
 *    cout << b->value () - 18;  // a and b out of scope, last reference lost, YCPIntegerRep is destroyed automatically
 * }
 * </pre>
 */

/**
 * @short Abstract base class of all YCP elements.
 * <p>There are some basic rules of memory managesment common to all
 * YCPElementRep classes. If you call a constructor of any YCPElementRep
 * subclass or if you call an add method, that adds elements to a @ref
 * YCPListRep, @ref YCPTermRep @ref YCPDeclTermRep or @ref YCPBlockRep, and
 * if this constructor or add method has arguments of type const
 * YCPElementRep * (or of a subclass), then the responsibility for the values
 * you gave for those arguments goes over to the object whose constructor or
 * add function has been called. You may refer that object afterwards in any
 * way. Therefore create the object either with new or with the @ref
 * YCPElementRep#clone method.
 *
 * <p>Example1:
 * <pre>
 * YCPTermRep *term = new YCPTermRep (new YCPSymbolRep ("foo"));
 * </pre>
 *
 * <p>Example2:
 * <pre>
 * YCPSymbolRep sym ("foo");
 * YCPTermRep term (sym.clone ());
 * </pre>
 *
 * <p>The second rule is that you never should delete any YCPElementRep
 * object directly. Rather use the method @ref YCPElementRep#destroy
 * for that purpose. @ref YCPElementRep#clone and YCPElementRep#destroy
 * keep keep reference counter in order to decide, if any valid reference
 * to the object exists. destroy calls delete when the last reference
 * is dropped.
 *
 * <p>The third rule is about return values of the methods YCPElementRep and
 * its subclasses. Member methods that return pointers to YCPElementRep
 * subclasses simply return a pointer to the answer without increasing its
 * reference counter. You must not destroy such an answer.
 *
 * <p>Example:
 * <pre>
 * YCPTermRep *term = new YCPTermRep (new YCPSymbolRep ("foo"));
 * cout << term->symbol ()->toString ();
 * </pre>
 *
 * <p>If you want to keep the result beyond the scope of the object whose
 * method you called, use @ref YCPElementRep@clone that create a valid
 * reference to the returned object.
 *
 * <p>The fourth rule is: Pointers to YCPElementRep are always const with
 * the only exception of @ref YCPListRep, @ref YCPDeclTermRep, @ref YCPTermRep
 * and @ref YCPBlockRep who have an add member function that allows the
 * parser to construct those objects element by element.
 */
class YCPElementRep : public MemUsage
{
    /**
     * Prevent implicite definition of assignment. Assignment of
     * YCPElements is <b>not</b> possible nor allowed.
     */
    YCPElementRep& operator=(const YCPElementRep&);

    /**
     * Prevent implicite definition of copy constructor. Assignment of
     * YCPElements is <b>not</b> possible nor allowed.
     */
    YCPElementRep(const YCPElementRep&);

    /**
     * Counts the references for this value. Each time the @ref
     * #clone method is called, the reference counter is increased.
     * The @ref #destroy method decreases the reference counter and
     * deletes the object, if the counter drops to 0.
     */
    mutable int reference_counter;

protected:

    friend class YCPElement;

    /**
     * Initializes this object. Sets the @ref @reference_counter to 1.
     */
    YCPElementRep();

    /**
     * Frees all resources used by this object. Never call this method
     * directly (don't use delete). Rather use the member function @ref
     * #destroy. It manages the reference counting.
     */
    virtual ~YCPElementRep();

public:
    /**
     * Casts this element into a pointer of type YCPValueRep
     */
    YCPValue asValue() const;

    /**
     * Returns the ASCII representation of this element.
     * This representation can be parsed with ycp_parse
     * to create an object whose value is identical to
     * the value of this object.
     */
    virtual string toString() const = 0;

    /**
     * Writes the value to a stream in bytecode format.
     */
    virtual std::ostream & toStream (std::ostream & str) const = 0;
    
    /**
     * Returns a shallow copy of this elementRep. Redefine this method 
     * to implement copy on write.
     */
    virtual const YCPElementRep* shallowCopy() const { return this; }

private:
    /**
     * Call this method instead of delete, when you have a pointer to
     * a YCPElementRep that you don't need any longer. The data the
     * YCPElementRep pointer points to may be shared data. This method
     * handles reference counting and deletes the object when the reference
     * counter drops to 0.
     * Destroy is declared const, but of course changes the objects data.
     * But it doesn't change the actual YCP value / meaning of the
     * YCPElementRep.
     */
    void destroy() const;

    /**
     * Gives you a new reference to this object. Increases the @ref
     * #reference_counter. This method is delared const though it changes
     * a member value - the reference_counter. But it <i>is</i> constant in
     * that it doesn't change the YCP meaning of this YCPElementRep.
     */
    const YCPElementRep *clone() const;
};

/**
 * @short Wrapper for YCPElementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPElementRep
 * with the arrow operator. See @ref YCPElementRep.
 */
class YCPElement : public MemUsage
{
    DEF_OPS(Element);
protected:
    const YCPElementRep *element;
    
    /**
     * Use this method to get an element which is ready to change. This
     * will ensure copy-on-write semantics
     */
    const YCPElementRep *writeCopy() { 
	if (element->reference_counter == 1 ) return element;
	
	const YCPElementRep* old = element;
	element = element->shallowCopy ();
	element->clone ();
	old->destroy ();
	return element;
    }

public:
    YCPElement();
    YCPElement(const YCPNull&);
    YCPElement(const YCPElementRep *e);
    YCPElement(const YCPElement &e);
    virtual ~YCPElement();
    const YCPElement& operator=(const YCPElement& e);
    bool isNull() const { return element == 0; }
    bool refersToSameElementAs(const YCPElement& e) const { return element == e.element; }    
};

#endif   // YCPElement_h
