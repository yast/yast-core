/*----------------------------------------------------------*- c++ -*--\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:       Type.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef Type_h
#define Type_h

#include <iosfwd>
#include <vector>

// MemUsage.h defines/undefines D_MEMUSAGE
#include <y2util/MemUsage.h>
#include "ycp/YCPValue.h"
#include "ycp/TypePtr.h"

class FunctionType;
class bytecodeistream;

class Type : public Rep
#ifdef D_MEMUSAGE
  , public MemUsage
#endif
{
    REP_BODY(Type);

public:
    // type codes for basic types
    typedef enum type_kind {
	UnspecT = 0,		//  0 unspecified
	ErrorT,			//  1 error
	AnyT,			//  2 any
	BooleanT,		//  3 boolean
	ByteblockT,		//  4 byteblock
	FloatT,			//  5 float
	IntegerT,		//  6 integer
	LocaleT,		//  7 locale
	PathT,			//  8 path
	StringT,		//  9 string
	SymbolT,		// 10 symbol
	TermT,			// 11 term
	VoidT,			// 12 void
	WildcardT,		// 13 wildcard

	FlexT,			// 14 flex
	VariableT,		// 15 variable <kind>
	ListT,			// 16 list <kind>
	MapT,			// 17 map <key_kind, value_kind>
	BlockT,			// 18 block <kind>
	TupleT,			// 19 tuple <kind, kind, kind, ...>
	FunctionT,		// 20 function <ret_kind, kind, kind, ...>

	NilT,			// 21 only for "return;" (void) vs. "return nil;" (nil)
	NFlexT			// 22 multiple Flex
    } tkind;

protected:
    tkind m_kind;
    bool m_const;
    bool m_reference;

    Type (tkind kind, bool as_const = false, bool as_reference = false) : m_kind (kind), m_const (as_const), m_reference(as_reference) { };

public:
    //-------------------------------------------------
    // static member functions

    /**
     * enable/disable type checking
     */
    static void setNocheck (bool nocheck);

    /**
     * convert YCPValueType to Type
     */
    static constTypePtr vt2type (enum YCPValueType vt);

    /**
     * signature parser, get next token
     */
    static int nextToken (const char **signature);

    /**
     * Construct from a string literal type code
     */
    static constTypePtr fromSignature (const char **signature);

    /**
     * Construct from a string literal type code
     * @param s eg. string("list <string>")
     */
    static constTypePtr fromSignature (const string & signature) { const char *s = signature.c_str(); return Type::fromSignature (&s); }

    /**
     * determine actual type if declared type contains 'flex' or 'flexN'
     * Returns actual - unchanged or fixed
     */
    static constTypePtr determineFlexType (constFunctionTypePtr actual, constFunctionTypePtr declared);

public:

    static const constTypePtr Unspec;	/* unspecified type  */
    static const constTypePtr Error;	/* error type  */
    static const constTypePtr Any;	/* any type  */

    static const constTypePtr Void;	/* void type  */
    static const constTypePtr Boolean;	/* boolean type  */
    static const constTypePtr Byteblock;/* byteblock type  */
    static const constTypePtr Float;	/* float type  */
    static const constTypePtr Integer;	/* integer type  */
    static const constTypePtr Locale;	/* locale type  */
    static const constTypePtr Path;	/* path type  */
    static const constTypePtr String;	/* string type  */
    static const constTypePtr Symbol;	/* symbol type  */
    static const constTypePtr Term;	/* term type  */
    static const constTypePtr Wildcard;	/* wildcard (...) type  */

    static const constTypePtr ConstAny;		/* any type  */
    static const constTypePtr ConstVoid;	/* void type  */
    static const constTypePtr ConstBoolean;	/* boolean type  */
    static const constTypePtr ConstByteblock;	/* byteblock type  */
    static const constTypePtr ConstFloat;	/* float type  */
    static const constTypePtr ConstInteger;	/* integer type  */
    static const constTypePtr ConstLocale;	/* locale type  */
    static const constTypePtr ConstPath;	/* path type  */
    static const constTypePtr ConstString;	/* string type  */
    static const constTypePtr ConstSymbol;	/* symbol type  */
    static const constTypePtr ConstTerm;	/* term type  */

    static const constTypePtr ConstList;	/* list type  */
    static const constTypePtr ConstMap;		/* map type  */

    static const constTypePtr Flex;
    static const constTypePtr ConstFlex;
    static const constTypePtr NFlex1;
    static const constTypePtr ConstNFlex1;
    static const constTypePtr NFlex2;
    static const constTypePtr ConstNFlex2;
    static const constTypePtr NFlex3;
    static const constTypePtr ConstNFlex3;
    static const constTypePtr NFlex4;
    static const constTypePtr ConstNFlex4;

    static const constTypePtr ListUnspec;
    static const constTypePtr List;
    static const constTypePtr MapUnspec;
    static const constTypePtr Map;
    static const constTypePtr Variable;
    static const constTypePtr Block;

    static FunctionTypePtr Function(constTypePtr return_type);

    static const constTypePtr Nil;	/* "return nil;" type */

private:
    /*
     * get kind
     */
    tkind kind () const { return m_kind; }

public:
    Type ();
    Type (tkind kind, bytecodeistream & str);
    virtual ~Type ();

    /**
     * Converts a type code to its YCP notation.
     */
    virtual string toString () const;

    /**
     * write out to stream
     */
    virtual std::ostream & toStream (std::ostream & str) const;

    /*
     * is base or constructed type
     */
    virtual bool isBasetype () const { return true; }

    /*
     * match <flex<number>> to type, return type if <flex<number>> matches
     */
    virtual constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const { return 0; }

    /**
     * check match with expected type
     * <0: no match, ==0: full match, >0: propagated match
     */
    virtual int match (constTypePtr expected) const;

    /**
     * check match with value
     * <0: no match, ==0: full match, >0: propagated match
     */
    virtual int matchvalue (YCPValue value) const;

    /**
     * check, if the type can be casted (at runtime considered
     * to be - similar to dynamic_cast) to another type
     */
    virtual bool canCast (constTypePtr to) const;

    /**
     * clone this type
     */
    virtual TypePtr clone () const;

    /**
     * replace any 'FlexT' (number == 0) or 'NFlexT' (number != 0) with 'type'
     */
    virtual constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;

    /**
     * prefix qualifier
     */
    string preToString () const { return (m_const ? "const " : ""); }

    /**
     * postfix qualifier
     */
    string postToString () const { return (m_reference ? " &" : ""); }

    /**
     * return const qualifier
     */
    bool isConst () const { return m_const; }

    /**
     * set const qualifier
     */
    void asConst () { m_const = true; }

    /**
     * return reference qualifier
     */
    bool isReference () const { return m_reference; }

    /**
     * set reference qualifier
     */
    void asReference () { m_reference = true; }

    /**
     * check if base matches with expected type
     * <0: no match, ==0: full match, >0: propagated match
     */
    int basematch (constTypePtr expected) const;

    /**
     * check equality of the types, without any assumptions like any == unspec
     */
    virtual bool equals (constTypePtr expected) const;

    // ------------------------------------------------------------
    // checking types

    // kind
    bool isUnspec () const	{ return m_kind == UnspecT; }
    bool isError () const	{ return m_kind == ErrorT; }
    bool isAny () const		{ return m_kind == AnyT; }
    bool isBoolean () const	{ return m_kind == BooleanT; }
    bool isByteblock () const	{ return m_kind == ByteblockT; }
    bool isFloat () const	{ return m_kind == FloatT; }
    bool isInteger () const	{ return m_kind == IntegerT; }
    bool isLocale () const	{ return m_kind == LocaleT; }
    bool isPath () const	{ return m_kind == PathT; }
    bool isString () const	{ return m_kind == StringT; }
    bool isSymbol () const	{ return m_kind == SymbolT; }
    bool isTerm () const	{ return m_kind == TermT; }
    bool isVoid () const	{ return m_kind == VoidT; }
    bool isWildcard () const	{ return m_kind == WildcardT; }
    bool isFlex () const	{ return ((m_kind == FlexT) || (m_kind == NFlexT)); }
    bool isNFlex () const	{ return m_kind == NFlexT; }

    bool isVariable () const	{ return m_kind == VariableT; }
    bool isList () const	{ return m_kind == ListT; }
    bool isMap () const		{ return m_kind == MapT; }
    bool isBlock () const	{ return m_kind == BlockT; }
    bool isTuple () const	{ return m_kind == TupleT; }
    bool isFunction () const	{ return m_kind == FunctionT; }

    bool isNil () const		{ return m_kind == NilT; }
    // ------------------------------------------------------------
    // misc methods

    YCPValueType valueType () const;

    // determine the common type of two types, used to determine the type of lists
    // and maps with various elements.
    // -> returns the largets (containing least amount of information) matching
    //    type (Any if types do not match)
    //    the return type is 'least common denominator'
    virtual constTypePtr commontype (constTypePtr type) const;

    // determine the more detailed type of two types, used to determine the type of bracket
    // element vs. bracket default
    // -> returns the smallest (containing most amount of information) matching
    //    type (Error if types do not match)
    virtual constTypePtr detailedtype (constTypePtr type) const;
};

// <flex>

class FlexType : public Type
{
    REP_BODY(FlexType);
public:
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    FlexType (bool as_const = false);
    FlexType (bytecodeistream & str);
    ~FlexType ();
};


// <flexN>

class NFlexType : public Type
{
    REP_BODY(NFlexType);
    unsigned int m_number;		// there can be more than one flex
public:
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    unsigned int number () const;
    NFlexType (unsigned int number, bool as_const = false);
    NFlexType (bytecodeistream & str);
    ~NFlexType ();
};


// Variable <type>

class VariableType : public Type
{
    REP_BODY(VariableType);
private:
    const constTypePtr m_type;
public:
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    bool equals (constTypePtr expected) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    constTypePtr type () const { return m_type; }
    VariableType (constTypePtr type = Type::Unspec, bool as_const = false);
    VariableType (bytecodeistream & str);
    ~VariableType ();
};


// List <type>

class ListType : public Type
{
    REP_BODY(ListType);
private:
    const constTypePtr m_type;
public:
    string toString () const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    bool equals (constTypePtr expected) const;
    constTypePtr commontype (constTypePtr type) const;
    constTypePtr detailedtype (constTypePtr type) const;
    bool canCast (constTypePtr to) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    constTypePtr type () const { return m_type; }
    std::ostream & toStream (std::ostream & str) const;
    ListType (constTypePtr type = Type::Unspec, bool as_const = false);
    ListType (bytecodeistream & str);
    ~ListType ();
};


// Map <keytype, valuetype>

class MapType : public Type
{
    REP_BODY(MapType);
private:
    const constTypePtr m_keytype;
    const constTypePtr m_valuetype;
public:
    string toString () const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    bool equals (constTypePtr expected) const;
    constTypePtr commontype (constTypePtr type) const;
    constTypePtr detailedtype (constTypePtr type) const;
    bool canCast (constTypePtr to) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    constTypePtr keytype () const { return m_keytype; }
    constTypePtr valuetype () const { return m_valuetype; }
    std::ostream & toStream (std::ostream & str) const;
    MapType (constTypePtr key = Type::Unspec, constTypePtr value = Type::Unspec, bool as_const = false);
    MapType (bytecodeistream & str);
    ~MapType ();
};


// Block <type>

class BlockType : public Type
{
    REP_BODY(BlockType);
private:
    const constTypePtr m_type;
public:
    string toString () const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    bool equals (constTypePtr expected) const;
    bool canCast (constTypePtr to) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    constTypePtr returnType () const { return m_type; }
    std::ostream & toStream (std::ostream & str) const;
    BlockType (constTypePtr type, bool as_const = false);
    BlockType (bytecodeistream & str);
    ~BlockType ();
};


// Tuple <type, type, ...>

class TupleType : public Type
{
    REP_BODY(TupleType);
protected:
    std::vector <constTypePtr> m_types;
public:
    string toString () const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    bool equals (constTypePtr expected) const;
    bool canCast (constTypePtr to) const;
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    std::ostream & toStream (std::ostream & str) const;
    TupleType (constTypePtr type, bool as_const = false);
    TupleType (bytecodeistream & str);
    void concat (constTypePtr t);
    unsigned int parameterCount () const { return m_types.size(); }
    constTypePtr parameterType (unsigned int parameter_number) const;
    ~TupleType ();
};


// Function <returntype, arg1type, arg2type, ...>

class FunctionType : public Type
{
    REP_BODY(FunctionType);
private:
    const constTypePtr m_returntype;
    TupleTypePtr m_arguments;
public:
    FunctionType (constTypePtr return_type, constFunctionTypePtr arguments);
    string toString () const;
    bool isBasetype () const { return false; }
    constTypePtr matchFlex (constTypePtr type, unsigned int number = 0) const;
    int match (constTypePtr expected) const;
    bool equals (constTypePtr expected) const;
    bool canCast (constTypePtr to) const { return false; }
    TypePtr clone () const;
    constTypePtr unflex (constTypePtr type, unsigned int number = 0) const;
    std::ostream & toStream (std::ostream & str) const;
    FunctionType (constTypePtr returntype = Type::Unspec, bool as_const = false);
    FunctionType (bytecodeistream & str);
    ~FunctionType ();
    constTypePtr returnType () const { return m_returntype; }
    void concat (constTypePtr t);
    int parameterCount () const;
    constTypePtr parameterType (unsigned int parameter_number) const;
    constTupleTypePtr parameters () const;
};


#endif   // Type_h
