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

   File:	YCode.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCode_h
#define YCode_h

#include <string>
using std::string;

#include "ycp/YCPValue.h"
#include "ycp/YCPString.h"
#include "ycp/StaticDeclaration.h" //redundant?
#include "ycp/SymbolEntry.h"

/**
 * linked list of actual parameters
 *
 */

typedef struct yparmlist {
    struct yparmlist *next;
    int kind;		// 0 = code, 1 = symbol, 2 = as Code, 3 = table
    union {
	YCode *c;
	SymbolEntry *s;
	TableEntry *t;
    } parm;
} yparmlist_t;


/**
 * linked list of ycode pointers
 *
 * used in forming a block consisting of an ordered
 * set of statements.
 */

struct ycodelist {
    struct ycodelist *next;
    YCode *code;
};
typedef struct ycodelist ycodelist_t;

/**
 * @short YCode for precompiled ycp code
 */
class YCode
{
public:
    enum ycode {
	yxError = 0,
	// [1] Constants	(-> YCPValue, except(!) locale and term -> yeLocale, yeTerm)
	ycVoid, ycBoolean, ycInteger, ycFloat,	// constants
	ycString, ycByteblock, ycPath, ycSymbol,
	ycList,					// list
	ycMap,					// map

	ycEntry,

	ycConstant,		// -- placeholder --
	ycLocale,				// locale constant (gettext)
	ycFunction,				// a function definition (parameters and body)

	// [15] Expressions	(-> declaration_t + values)
	yePropagate,		// type propagation (value, type)
	yeUnary,		// unary (prefix) operator
	yeBinary,		// binary (infix) operator
	yeTriple,		// <exp> ? <exp> : <exp>
	yeCompare,		// compare
	yePrefix,		// normal function

	// [21] Value expressions (-> values + internal)
	yeLocale,		// locale expression (ngettext)
	yeList,			// list expression
	yeMap,			// map expression
	yeTerm,			// <name> ( ...)
	yeLookup,		// lookup ()
	yeIs,			// is()
	yeBracket,		// <name> [ <expr>, ... ] : <expr>

	// [28] Block (-> linked list of statements)
	yeBlock,		// block expression
	yeReturn,		// quoted expression

	// [30] Symbolref (-> SymbolEntry)
	yeVariable,		// variable ref
	yeBuiltin,		// builtin ref + args
	yeSymFunc,		// builtin ref with symbols + args
	yeFunction,		// function ref + args

	yeExpression,		// -- placeholder --

	// [35] Statements	(-> YCode + next)
	ysTypedef,		// typedef
	ysVariable,		// variable definition
	ysFunction,		// function definition
	ysAssign,		// variable assignment
	ysBracket,		// <name> [ <expr>, ... ] = <expr>
	ysIf,			// if, then, else
	ysWhile,		// while () do ...
	ysDo,			// do ... while ()
	ysRepeat,		// repeat ... until ()
	ysExpression,		// any expression (function call)
	ysReturn,
	ysBreak,
	ysContinue,
	ysTextdomain,
	ysInclude,
	ysFilename,
	ysStatement		// -- placeholder --
    };

protected:
    ycode m_code;

public:

    /**
     * Creates a new YCode element
     */
    YCode (ycode code);

    /**
     * Cleans up
     */
    virtual ~YCode();

    /**
     * Returns the YCode code
     */
    ycode code() const;

    /**
     * Returns an ASCII representation of the YCode.
     */
    virtual string toString() const;
    static string toString(ycode code);

    /**
     * writes YCode to a stream
     * see Bytecode for read
     */
    virtual std::ostream & toStream (std::ostream & str) const = 0;

    /**
     * returns true if the YCode represents a constant
     */
    bool isConstant () const;

    /**
     * returns true if the YCode represents an error
     */
    bool isError () const;

    /**
     * returns true if the YCode represents a statement
     */
    bool isStatement () const;

    /**
     * returns true if the YCode represents a block
     */
    bool isBlock () const;

    /**
     * evaluate YCode to YCPValue
     * if debugger == 0
     *	 called for parse time evaluation (i.e. constant subexpression elimination)
     * else
     *	 called for runtime evaluation		
     */
    virtual YCPValue evaluate (bool cse = false);
};


/**
 * YError
 * represents an error state as a YCode object
 *
 */

class YError : public YCode {
    int m_line;
    const char *m_msg;
public:
    YError (int line=0, const char *msg=0);
    ~YError () {}
    YCPValue evaluate (bool cse = false);
    string toString();
    std::ostream & toStream (std::ostream & str) const;
};

/**
 * constant (-> YCPValue)
 */

class YConst : public YCode {
    YCPValue m_value;		// constant (not allowed in union :-( )
public:
    YConst (ycode code, YCPValue value);		// Constant
    YConst (ycode code, std::istream & str);
    ~YConst () {};
    YCPValue value() const;
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);

//    bool saveTo (FILE *out) const;
//    YCode *loadFrom (FILE *in);
};

/**
 * locale
 * string and textdomain
 * @see: YELocale
 */

class YLocale : public YCode {
    const char *m_locale;
    const char *m_domain;
public:
    YLocale (const char *locale, const char *textdomain);
    YLocale (std::istream & str);
    ~YLocale ();
    const char *value () const;
    const char *domain () const;
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};

/**
 * declaration (-> declaration_t)
 */

class YDeclaration : public YCode {
    declaration_t *m_value;	// builtin declaration
public:
    YDeclaration (ycode code, declaration_t *value);	// Builtin decl
    YDeclaration (std::istream & str);
    ~YDeclaration () {};
    declaration_t *value() const;
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};

/**
 * function declaration (m_body == 0) or definition (m_body != 0)
 *   it's anonymouse here ! The code() member of the corresponding
 *   SymbolEntry points to YFunction.
 */

class YFunction : public YCode {
    // array of formal arguments of a function
    // the formal parameters must be available in the local scope during parse
    // of the function body (startDefinition()) and removed afterwards (endDefintion()).
    // Keep track of the table entries in this block.
    //
    // When calling a function during execution, the actual
    // arguments (values) are bound to the formal arguments
    // (this array) so the function body can be evaluated.
    // @see YEFunction::attachActualParameter()
    //
    // if NULL, it's a () function
    YBlock *m_parameterblock;

    // the function YBlock 'body' is the block defining this function
    YBlock *m_body;

public:
    YFunction (YBlock *parameterblock, YBlock *body);
    YFunction (std::istream & str);
    ~YFunction ();

    // access to formal parameters
    unsigned int parameterCount () const;
    YBlock *parameterBlock () const;
    SymbolEntry *parameter (unsigned int position) const;

    // access to definition block (= 0 if declaration only)
    YBlock *body () const;
    void setBody (YBlock *body);

    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};


#endif   // YCode_h
