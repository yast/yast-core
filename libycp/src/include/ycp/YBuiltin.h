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

   File:       YBuiltin.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef YBuiltin_h
#define YBuiltin_h

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
class TypeCode
{
    /**
     * String representation of a type.
     */
    string m_tc;
    friend inline const char * c_str (TypeCode const &t)
	{
	    return t.m_tc.c_str ();
	}
    
public:
    /**
     * Default ctor, unspecified type.
     */
    TypeCode () {}
    /**
     * Construct from a string literal type code
     * @param s eg. "Ls"
     */
    TypeCode (const char * s);
    /**
     * Construct from a string literal type code
     * @param s eg. string("Ls")
     */
    TypeCode (const string & s) : m_tc (s) {}
    /**
     * Construct from YCPValueType.
     * Used only by YEIs, was vt2type
     * Takes only the basic type.
     */
    static TypeCode vt2type (enum YCPValueType vt);


    /**
     * Are the types exactly equal?
     * @see equaltype
     */
    bool operator == (const TypeCode &b) const { return m_tc == b.m_tc; }
    /**
     * Make a string type code.
     * For debugging
     */
    const char * c_str () const { return m_tc.c_str(); }

    /**
     * Converts a type code to its YCP notation.
     * Eg. "i" -> "integer", "Ls" -> "list(string)"
     */
    string toString () const;
    
    /**
     * Compatibility variant of TypeCode::toString ()
     */
#define type2string(type) (((type).toString ()).c_str ())

    /**
     * Produces a YCP form (LLsi -> "list(list(string))")
     */ 

    //enum YCPValueType type2vt (constTypeCode type);
    /**
     * Used only by YELookup.
     * Takes only the basic type.
     */
    YCPValueType valueType () const;

    // A bit of an encapsulation of the special types
    /**
     * Is the type unspecified? ("")
     */
    bool isUnspec () const { return m_tc.empty (); }
    /**
     * Is it "a"?
     */
    bool isAny () const { return m_tc[0] == 'a'; }
    /**
     * Is it "A"?
     */
    bool isFlex () const { return m_tc[0] == 'A'; }
    /**
     * Is it "w"?
     */
    bool isWild () const { return m_tc[0] == 'w'; }

    /**
     * If this alrealdy is not a block(foo), return block(this).
     */
    TypeCode codify () const;

    // compatibility non-member functions
    /**
     * Concatenates the type codes into a new one.
     * @return newly malloc'd type code
     */
    friend TypeCode newtype (const TypeCode &t1, const TypeCode &t2)
	{ return TypeCode (t1.m_tc + t2.m_tc); }

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
    bool isArgs () const { return m_tc[0] == '|'; }
    /**
     * Is the return type unspecified?
     */
    bool isReturnUnspec () const { return m_tc[0] == '|'; }

    // iterating over the parameters:

    /**
     * Argument part of a function type
     */
    TypeCode args () const {
	const char *bar = strchr (m_tc.c_str (), '|');
	return bar? bar+1 : "";
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
    bool isEnd () const { return m_tc.empty (); }

    /**
     * Given that this is a formal parameter type,
     * check that actual is a matching actual parameter.
     * On success, both of them are modified to advance to the next param.
     * It knows about w, A, L...
     * @return matched?
     */
    bool paramMatch (TypeCode &actual);
};

// compatibility declaration
typedef const TypeCode& constTypeCode;

#if 0
// historically:
typedef char * TypeCode;
typedef char const * constTypeCode;
inline const char * c_str (constTypeCode t)
{
    return t;
}
#endif

// structure for static declarations
enum DeclFlags
{
    DECL_NIL = 1,	// accepts nil
    DECL_WILD = 2,	// wildcard
    DECL_SYMBOL = 4,	// symbol as parameter
    DECL_CODE = 8,	// code as parameter
};

/**
 * A declaration of a (builtin?) function
 */
struct declaration {
  const char *name;		// name of variable/function
  TypeCode type;		// type of variable/function
  int flags;			// parameter acceptance, @ref DeclFlags
  void *ptr;			// pointer to builtin value/function
  struct declaration *next;	// link to next overloaded declaration
};
typedef struct declaration declaration_t;

/**
 * show a declaration
 * @param full if false, just show the name; if true, show name and type
 */
string Decl2String (const declaration_t *declaration, bool full = false);

/**
 * Converts a sequence of type codes to YCP notation.
 * Eg. "si" -> "string, integer"
 * @param types type codes in one C string, eg "iiLs"
 */
string typesToString (constTypeCode types);

/**
 * Extracts the return type-code of a block or function
 * @param type a block (C) or function (|) type code
 */
TypeCode returnType (constTypeCode type);

/**
 * Converts the return type-code of a block or function to YCP notation
 * Eg. "Ls|LLs" -> "list(string)"
 * @param type a block (C) or function (|) type code
 */
string return2string (constTypeCode type);

/**
 * Check for matching types
 * @return 0 for exact matches
 *	   >0 for propagated matches
 *	   <0 for no matches
 */
int matchtype (constTypeCode expected, constTypeCode given);

/**
 * Finds a type that can hold both given types
 * This should be the narrowest such type - TODO
 * Mallocs the result
 */
TypeCode commontype (constTypeCode t1, constTypeCode t2);

/**
 * UNUSED
 * @see YEPropagate
 * @return FIXME LEAK sometimes allocates the result
 */
constTypeCode propagatetype (constTypeCode t1);

#endif   // YBuiltin_h
