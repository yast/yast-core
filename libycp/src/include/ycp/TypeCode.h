/*----------------------------------------------------------*- c++ -*--\
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

   File:       TypeCode.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef TypeCode_h
#define TypeCode_h

#include <iostream>

#include <YaST2/y2util/Ustring.h>
#include "ycp/YCPValue.h"

/**
 * Types are coded as strings.
 * We do not intend to hide it nor try to encapsulate it too hard.
 * Originally they were C strings but that made memory management awkward.

	In the following descriptions, "x" stands for the type code of "X",
	eg. "s" and "string".
	Keep this list sorted! (M-x sort-lines)
	<pre>
	A   any, used specially for lookup and select matching
	C   block: Cx is a block returning X
	E   YT_ERROR (vt2type)
	L   list: Lx is list(X). "list" means "list(any)"
	M   map: Mx is map with keys (values?) of type X	TODO Nxz
	R   reference (builtins only?): Rx is X&
	Y   symbolic variable, Ya is "any <var>", Yx is "<type> <var>"
	_   locale
	a   any
	b   boolean
	f   float
	i   integer
	o   byteblock (for 'o'ctet :-))  scanner errorneously used 'c'
	p   path
	s   string
	t   term
	v   void
	w   ... (wildcard, in type2string)
	y   symbol
	|   function: X|YZ is a func taking Y and Z and returning X. findDeclaration does not expect a return type!
	</pre>
 */

class TypeCode : public Ustring
{
private:
    // hash for unique strings
    static UstringHash _nameHash;

    static string typeToString (const char * &t);
    
public:
    /* unspecified type  */
    static const TypeCode Unspec;

    /* any type  */
    static const TypeCode Any;

    /* void type  */
    static const TypeCode Void;

    /* integer type  */
    static const TypeCode Integer;

    /* float type  */
    static const TypeCode Float;

    /* string type  */
    static const TypeCode String;

    /* symbol type  */
    static const TypeCode Symbol;

    /* term type  */
    static const TypeCode Term;

    /* list(any) type  */
    static const TypeCode List;

    /* map(any) type  */
    static const TypeCode Map;

    /* function type  */
    static const TypeCode Function;

    /* construct block(t), list(t), map(t), reference, function */
    static TypeCode makeBlock (const TypeCode &t);
    static TypeCode makeList (const TypeCode &t);
    static TypeCode makeMap (const TypeCode &t);
    static TypeCode makeReference (const TypeCode &t);
    static TypeCode makeFunction (const TypeCode &t);

    /**
     * default constructor
     */
    TypeCode () : Ustring (_nameHash, "") {}

    /**
     * Construct from a string literal type code
     * @param s eg. "Ls"
     */
    TypeCode (const char * s) : Ustring (_nameHash, s) {}

    /**
     * Construct from a string literal type code
     * @param s eg. string("Ls")
     */
    TypeCode (const string & s) : Ustring (_nameHash, s) {}

    /**
     * Construct from a bytecode stream
     * @param istream
     */
    TypeCode (std::istream & str);

    /**
     * Construct from YCPValueType.
     * Used only by YEIs, was vt2type
     * Takes only the basic type.
     */
    static TypeCode vt2type (enum YCPValueType vt);

    /**
     * Converts a type code to its YCP notation.
     * Eg. "i" -> "integer", "Ls" -> "list(string)"
     */
    string toString () const;

    /**
     * Converts a sequence of type codes to YCP notation.
     * Eg. "si" -> "string, integer"
     * @param types type codes in one C string, eg "iiLs"
     */
    string toStringSequence () const;

    /**
     * write out to stream
     */
    std::ostream & toStream (std::ostream & str) const;
    
    /**
     * Produces a YCP form (LLsi -> "list(list(string))")
     */ 

    //enum YCPValueType type2vt (const TypeCode &type);

    /**
     * Used only by YELookup.
     * Takes only the basic type.
     */
    YCPValueType valueType () const;

    // A bit of an encapsulation of the special types
    /**
     * Is the type unspecified? (== "")
     */
    bool isUnspec () const { return asString().empty (); }

    /**
     * Is the type specified? (!= "")
     */
    bool notUnspec () const { return !asString().empty (); }

    /**
     * Is it "a"?
     */
    bool isAny () const { return asString()[0] == 'a'; }

    /**
     * Is it "v"?
     */
    bool isVoid () const { return asString()[0] == 'v'; }

    /**
     * Is it "b"?
     */
    bool isBoolean () const { return asString()[0] == 'b'; }

    /**
     * Is it "i"?
     */
    bool isInteger () const { return asString()[0] == 'i'; }

    /**
     * Is it "f"?
     */
    bool isFloat () const { return asString()[0] == 'f'; }

    /**
     * Is it "s"?
     */
    bool isString () const { return asString()[0] == 's'; }

    /**
     * Is it "A"?
     */
    bool isFlex () const { return asString()[0] == 'A'; }

    /**
     * Is it "C"?
     */
    bool isCode () const { return asString()[0] == 'C'; }

    /**
     * Is it "w"?
     */
    bool isWild () const { return asString()[0] == 'w'; }

    /**
     * If this is not a block(foo) already, return block(this).
     */
    TypeCode codify () const;

    // compatibility non-member functions
    /**
     * Concatenates the type codes into a new one.
     * @return newly malloc'd type code
     */
    friend TypeCode newtype (const TypeCode &t1, const TypeCode &t2)
	{ return TypeCode (t1.asString() + t2.asString()); }

    /**
     * Are the types exactly equal?
     */
    friend bool equaltype (const TypeCode &t1, const TypeCode &t2)
	{ return t1 == t2; }

    // ------------------------------------------------------------
    // These operate on a function signature type codes
    // maybe we should make it a separate class?

    /**
     * Cannot return | or w or ""
     */
    bool isValidReturn () const;

    /**
     * Want to match function arguments
     */
    bool isArgs () const { return asString()[0] == '|'; }

    /**
     * Is the return type unspecified?
     */
    bool isReturnUnspec () const { return asString()[0] == '|'; }

    // iterating over the parameters:

    /**
     * Argument part of a function type
     */
    TypeCode args () const {
	const char *bar = strchr (asString().c_str (), '|');
	return bar ? TypeCode (bar+1) : TypeCode ("");
    }

    /**
     * first type, eg LLA
     */
    TypeCode firstT () const;

    /**
     * Next argument
     * eg LLsfi -> fi
     */
    TypeCode next () const;

    /**
     * Is this an end of an argument list/compound type?
     */
    bool isEnd () const { return asString().empty (); }

    /**
     * Given that this is a formal parameter type,
     * check that actual is a matching actual parameter.
     * On success, both of them are modified to advance to the next param.
     * It knows about w, A, L...
     * @return matched?
     */
    bool paramMatch (TypeCode &actual);

    /**
     * Extracts the return type-code of a block or function
     * @param type a block (C) or function (|) type code
     */
    TypeCode returnType () const;

    /**
     * Converts the return type-code of a block or function to YCP notation
     * Eg. "Ls|LLs" -> "list(string)"
     * @param type a block (C) or function (|) type code
     */
    string return2string () const;

    /**
     * Check for matching types
     * @return 0 for exact matches
     *	   >0 for propagated matches
     *	   <0 for no matches
     */
    int matchtype (const TypeCode &expectedType) const;

    /**
     * Check for matching (function) parameters
     */
    static bool matchParameters (const char * &declared_type, const char * &actual_type);

    /**
     * Finds a type that can hold both given types
     * This should be the narrowest such type - TODO
     * Mallocs the result
     */
    TypeCode commontype (const TypeCode &type) const;

    /**
     * UNUSED
     * @see YEPropagate
     * @return FIXME LEAK sometimes allocates the result
     */
    const TypeCode propagatetype () const;
};

#endif   // TypeCode_h
