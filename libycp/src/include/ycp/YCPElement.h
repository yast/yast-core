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

   File:       YCPElement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPElement_h
#define YCPElement_h


#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::pair;


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
class YCPDeclaration;
class YCPDeclAny;
class YCPDeclType;
class YCPDeclList;
class YCPDeclStruct;
class YCPDeclTerm;
class YCPLocale;
class YCPList;
class YCPTerm;
class YCPMap;
class YCPBlock;
class YCPWhileBlock;
class YCPDoWhileBlock;
class YCPStatement;
class YCPEvaluationStatement;
class YCPBreakStatement;
class YCPContinueStatement;
class YCPReturnStatement;
class YCPIfThenElseStatement;
class YCPNestedStatement;
class YCPBuiltinStatement;
class YCPBasicInterpreter;
class YCPBuiltin;
class YCPIdentifier;
class YCPError;


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


class YCPNull {};

/***
 * <h2>The YCP Datastructures</h2>
 *
 * <p>YCP is both a scripting language and a communication protocol.
 * A YCP value is a data structure. It has currently two possible
 * representations. One ist an ASCII representation, the other is
 * a representation a network of C++ objects. The class framework
 * for this object representation is laid in this library. Furthermore
 * It contains the @ref YCPParser, that transforms an ASCII representation
 * of YCP values into the object-representation. It also contains a
 * generic embeddable interpreter @ref YCPInterpreter, that executes
 * YCP scripts (based on the object representation).
 *
 * <p>An example for use of both the @ref YCPParser and the @ref YCPInterpreter
 * can be found in the source code of <tt>y2ycp</tt>, the generic YCP
 * interpreter. It is found in the source code in the subdirectory ycp.
 *
 * <h2>YCP Data management</h2>
 *
 * YCP data are managed via "smart pointers". This means that an application
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
 * <p>Important: Applications _NEVER_ use "Rep" classes directly!
 *
 * <p>Example:
 * <pre> ...
 * {
 *    YCPInteger a(18);       // here a YCPIntegerRep is created automatically
 *    YCPInteger b = a;       // b points to the same YCPIntegerRep as a
 *
 *    cout << b->toString();  // use pointer notation to access members
 *    cout << b->value() - 18;
 * }                        // a and b out of scope, last reference lost, YCPIntegerRep is destroyed automatically
 * ... </pre>
 */

/**
 * @short Abstract base class of all YCP elements.
 * There are some basic rules of memory managesment common
 * to all YCPElementRep classes. If you call a constructor of any
 * YCPElementRep subclass or if you call
 * an add method, that adds elements to a @ref YCPListRep, @ref YCPTermRep
 * @ref YCPDeclTermRep or @ref YCPBlockRep, and if this constructor or add method
 * has arguments of type const YCPElementRep * (or of a subclass),
 * then the responsibility for the values you gave for those arguments
 * goes over the the object whose constructor or add function has been
 * called. You may refer that object afterwards in any way. Therefore
 * create the object either with new or with the @ref YCPElementRep#clone
 * method.
 *
 * Example1:
 * <pre>  YCPTermRep *term = new YCPTermRep(new YCPSymbolRep("foo")) </pre>
 *
 * Example2:
 * <pre>  YCPSymbolRep sym("foo");
 *  YCPTermRep term(sym.clone()); </pre>
 *
 * The second rule is that you never should delete any YCPElementRep
 * object directly. Rather use the method @ref YCPElementRep#destroy
 * for that purpose. @ref YCPElementRep#clone and YCPElementRep#destroy
 * keep keep reference counter in order to decide, if any valid reference
 * to the object exists. destroy calls delete when the last reference
 * is dropped.
 *
 * The third rule is about return values of the methods YCPElementRep and
 * its subclasses. Member methods that return pointers to YCPElementRep subclasses
 * simply return a pointer to
 * the answer without increasing its reference counter. You must not
 * destroy such an answer. Example:
 *
 * <pre> YCPTermRep *term = new YCPTermRep(new YCPSymbolRep("foo"));
 *  cout << term->symbol()->toString(); </pre>
 *
 * If you want to keep the result beyond the scope of the object whose
 * method you called, use @ref YCPElementRep#clone that create a valid
 * reference to the returned object.
 *
 * The fourth rule is: Pointers to YCPElementRep are always const with
 * the only exception of @ref YCPListRep, @ref YCPDeclTermRep, @ref YCPTermRep
 * and @ref YCPBlockRep who have an add member function that allows the
 * parser to construct those objects element by element.
 */
class YCPElementRep
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
     * Counts the references the this value. Each time the @ref
     * #clone method is called, the reference counter is increased.
     * The @ref #destroy method decreases the reference counter and
     * deletes the object, if the counter drops to 0.
     */
    mutable int reference_counter;

protected:
    friend class YCPElement;

    /**
     * Initializes this object. Sets the @ref #reference_counter to 1.
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
     * Casts this element into a pointer of type YCPStatementRep
     */
    YCPStatement asStatement() const;

    /**
     * Returns the ASCII representation of this element.
     * This representation can be parsed with ycp_parse
     * to create an object whose value is identical to
     * the value of this object.
     */
    virtual string toString() const = 0;

private:
    /**
     * Call this method instead of delete, when you have a pointer to
     * a YCPElementRep that you don't need any longer. The YCPElementRep the
     * pointer points to may be shared data. This method handles reference
     * counting and delete this object when the reference counter drops
     * to 0.
     * Destroy is declared const, but of course changes the objects data.
     * But it doesn't change the actual YCP value / meaning of the YCPElementRep.
     */
    void destroy() const;

    /**
     * Gives you a new reference to this object. Increases the @ref #reference_counter.
     * This method is declared const though it changes a member value - the
     * reference_counter. But it _is_ constant in that it doesn't change
     * the YCP meaning of this YCPElementRep.
     */
    const YCPElementRep *clone() const;
};

/**
 * @short Wrapper for YCPElementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPElementRep
 * with the arrow operator. See @ref YCPElementRep.
 */
class YCPElement
{
    DEF_OPS(Element);
protected:
    const YCPElementRep *element;
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
