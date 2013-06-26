/*-----------------------------------------------------------*- c++ -*-\
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

/**
 * \page ycplanguage YCP Scripting Language
 * \todo Enhance
 *
 * YCP is a very simple imperative scripting language with builtin support for
 * \ref ycpvalues, easy access to \ref components, common control structures and more.
 * It is designed to be well suited for system configuration manipulation.
 *
 * YCP interpreter consists of the following parts:
 * - YCP \ref Parser which parses ASCII representation into in-memory representation (C++ classes
 *   inheriting from the \ref YCode class.
 * - YCP interpreter - the \ref YCode based classes
 * - \ref YCP bytecode implementation
 */

#include <string>
using std::string;

// MemUsage.h defines/undefines D_MEMUSAGE
#include <y2util/MemUsage.h>
#include "ycp/YCodePtr.h"

#include "ycp/YCPValue.h"
#include "ycp/YCPString.h"
#include "ycp/Type.h"
#include "ycp/YSymbolEntry.h"

/**
 * linked list of ycode pointers
 *
 * used in forming a block consisting of an ordered
 * set of statements.
 */

struct ycodelist {
    struct ycodelist *next;
    YCodePtr code;
    constTypePtr type;
};
typedef struct ycodelist ycodelist_t;

/**
 * \brief YCode for precompiled ycp code
 *
 * A class representing parsed YCP code. This is an abstract base class for implementing 
 * any YCP language feature. \ref kind uniquely identifies the type of the class. 
 * The class provides infrastructure for dumping a bytecode representation (\ref toStream), 
 * XML representation (\ref toXML) and ASCII representation (\ref toString).
 * 
 * The represented YCP code is executed via invoking \ref evaluate.
 */
class YCode : public Rep
#ifdef D_MEMUSAGE
  , public MemUsage
#endif
{
protected:
    mutable const char * comment_before;
    mutable const char * comment_after;

private:
    REP_BODY(YCode);

public:
    enum ykind {
	yxError = 0,
	// [1] Constants	(-> YCPValue, except(!) term -> yeLocale)
	ycVoid, ycBoolean, ycInteger, ycFloat,	// constants
	ycString, ycByteblock, ycPath, ycSymbol,
	ycList,					// list
	ycMap,					// map
	ycTerm,					// term

	ycEntry,

	ycConstant,		// -- placeholder --
	ycLocale,				// locale constant (gettext)
	ycFunction,				// a function definition (parameters and body)

	// [16] Expressions	(-> declaration_t + values)
	yePropagate,		// type propagation (value, type)
	yeUnary,		// unary (prefix) operator
	yeBinary,		// binary (infix) operator
	yeTriple,		// <exp> ? <exp> : <exp>
	yeCompare,		// compare

	// [21] Value expressions (-> values + internal)
	yeLocale,		// locale expression (ngettext)
	yeList,			// list expression
	yeMap,			// map expression
	yeTerm,			// <name> ( ...)
	yeIs,			// is()
	yeBracket,		// <name> [ <expr>, ... ] : <expr>

	// [27] Block (-> linked list of statements)
	yeBlock,		// block expression
	yeReturn,		// quoted expression, e.g. "``(<exp>)" which really is "{ return <exp>; }"

	// [29] Symbolref (-> SymbolEntry)
	yeVariable,		// variable ref
	yeBuiltin,		// builtin ref + args
	yeFunction,		// function ref + args
	yeReference,		// reference to a variable (identical to yeVariable but with different semantics)
	// SuSE Linux v9.2
	yeFunctionPointer,	// function pointer

	yeExpression,		// -- placeholder --

	// [35] Statements	(-> YCode + next)
	ysTypedef,		// typedef
	ysVariable,		// variable defintion (-> YSAssign)
	ysFunction,		// function definition
	ysAssign,		// variable assignment (-> YSAssign)
	ysBracket,		// <name> [ <expr>, ... ] = <expr>
	ysIf,			// if, then, else
	ysWhile,		// while () do ...
	ysDo,			// do ... while ()
	ysRepeat,		// repeat ... until ()
	ysExpression,		//  any expression (function call)
	ysReturn,		// return
	ysBreak,		// break
	ysContinue,		// continue
	ysTextdomain,		// textdomain
	ysInclude,		// include
	ysFilename,		//  restore filename after include
	ysImport,		// import
	ysBlock,		// a block as statement
	ysSwitch,		// switch (since 10.0)
	ysStatement,		// [54] -- placeholder --
	
	// internal
	yiBreakpoint		// [55] -- debugger breakpoint
    };

public:

    /**
     * Creates a new YCode element
     */
    YCode ();

    /**
     * Destructor
     */
    virtual ~YCode();

    /**
     * Kind of this \ref YCode. This method must be reimplemented in the inherited classes.
     *
     * \return the YCode kind
     */
    virtual ykind kind() const = 0;
   
    /**
     * Return ASCII represtation of this YCP code.
     * 
     * \return ASCII string representation
     */
    virtual string toString() const;

    /**
     * String representation of the YCode kind value. For debugging purposes.
     *
     * \param kind    which kind representation should be returned
     * \return string representation
     */
    static string toString(ykind kind);

    /**
     * Write YCP code to a byte stream (bytecode implementation). Every
     * class inheriting from \ref YCode must reimplement this method.
     * \param str	byte stream to store into
     * \return 		byte stream for chaining writing bytecode (str)
     */
    virtual std::ostream & toStream (std::ostream & str) const = 0;

    /**
     * Write YCP code as XML representation. Every class inheriting from 
     * \ref YCode must reimplement this method.
     * \param str	string stream to store into
     * \param indend	indentation level for pretty print
     * \return 		string stream for chaining writing XML (str)
     */
    virtual std::ostream & toXml (std::ostream & str, int indent ) const = 0;


    /**
     * \return comments as xml fragment:
     *   ' comment_before="..." comment_after="..."'
     */
    std::string commentToXml () const;
    /**
     * Writes comment attributes to a xml stream
     */
    std::ostream & commentToXml (std::ostream & str ) const;

    /**
     * Is this code constant?
     *
     * \return true if the \ref YCode represents a constant
     */
    virtual bool isConstant () const;

    /**
     * Is this code a representation of an error?
     *
     * \returns true if the \ref YCode represents an error
     */
    bool isError () const;

    /**
     * Is this a YCP statement (e.g. if, while, ...)
     *
     * \return true if the \ref YCode represents a statement
     */
    virtual bool isStatement () const;

    /**
     * Is this a YCP block?
     *
     * \return true if the \ref YCode represents a block
     */
    virtual bool isBlock () const;

    /**
     * Can this code be stored in a variable of a type reference?
     *
     * \return true if the \ref YCode represents something we can reference to
     */
    virtual bool isReferenceable () const;

    /**
     * Execute YCP code to get the resulting \ref YCPValue. Every inherited class of YCode should reimplement
     * this method.
     * 
     * \return \ref YCPValue after executing the code
     * \param cse  should the evaluation be done for parse time evaluation (i.e. constant subexpression elimination)
     */
    virtual YCPValue evaluate (bool cse = false);

   /**
    * Return type of this YCP code (interesting mostly for function calls).
    *
    * \return type of the value to be returned after calling \ref evaluate
    */
  virtual constTypePtr type() const;

   /** Setter for comment before code
    *  Take care about deallocation of pointer.
    * \param comment pointer to comment
    */
    void setCommentBefore( const char * comment);

   /** Setter for comment after code
    *  Take care about deallocation of pointer.
    * \param comment pointer to comment
    */
    void setCommentAfter( const char * comment);

};


/**
 * \brief YCP Constant
 *
 * The base class for YCP constants (\ref YCPValue).
 */
class YConst : public YCode
{
    REP_BODY(YConst);
    ykind m_kind;
    YCPValue m_value;		// constant (not allowed in union :-( )
public:
    YConst (ykind kind, YCPValue value);		// Constant
    YConst (ykind kind, bytecodeistream & str);
    ~YConst () {};
    YCPValue value() const;
    virtual ykind kind() const;
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;
    /** yes */
    virtual bool isConstant () const { return true; }
    YCPValue evaluate (bool cse = false);
    constTypePtr type() const;
};

// bother, 4.3 requires -std=c++0x
// so without a condition in configure.in you can't have code
// that works with 4.2 and 4.3 without warnings
#ifdef HAVE_CXX0X
#include <unordered_map>
#else
#include <ext/hash_map>
#endif

#include <string>
#include <cstddef>

/**
 * locale
 * string and textdomain
 * @see: YELocale
 */

class YELocale;

class YLocale : public YCode
{
    REP_BODY(YLocale);
    const char *m_locale;		// the string to be translated

    struct eqstr
    {
	bool operator()(const char* s1, const char* s2) const
	{
	    return strcmp(s1, s2) == 0;
	}
    };

public:
#ifdef HAVE_CXX0X
    typedef unordered_map<const char*, bool, hash<const char*>, eqstr> t_uniquedomains;
#else
    typedef __gnu_cxx::hash_map<const char*, bool, __gnu_cxx::hash<const char*>, eqstr> t_uniquedomains;
#endif

    static t_uniquedomains domains;	// keep every textdomain only once
    static t_uniquedomains::const_iterator setDomainStatus (const string& domain, bool status);
    static void ensureBindDomain (const string& domain);
    static void bindDomainDir (const string& domain, const string& domain_path);
    static bool findDomain(const string& domain);
    YLocale (const char *locale, const char *textdomain);
    YLocale (bytecodeistream & str);
    ~YLocale ();
    virtual ykind kind () const { return ycLocale; }
    const char *value () const;
    const char *domain () const;
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type() const { return Type::Locale; }
    
private:

    t_uniquedomains::const_iterator m_domain;

};

/**
 * function declaration (m_body == 0) or definition (m_body != 0)
 *   it's anonymouse here ! The code() member of the corresponding
 *   SymbolEntry points to YFunction.
 */

class YFunction : public YCode
{
    REP_BODY(YFunction);
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
    // if NULL, it's a (void) function
    YBlockPtr m_declaration;

    // the function definition ('body') is the block defining this function
    YCodePtr m_definition;

    bool m_is_global;

public:
    YFunction (YBlockPtr parameterblock, const SymbolEntryPtr entry = 0);
    YFunction (bytecodeistream & str);
    ~YFunction ();
    virtual ykind kind () const { return ycFunction; }

    // access to formal parameters
    unsigned int parameterCount () const;
    YBlockPtr declaration () const;
    SymbolEntryPtr parameter (unsigned int position) const;

    // access to definition block (= 0 if declaration only)
    YCodePtr definition () const;
    void setDefinition (YBlockPtr body);
    void setDefinition (YBreakpointPtr body);
    // read definition from stream
    void setDefinition (bytecodeistream & str);

    string toStringDeclaration () const;
    string toString () const;
    std::ostream & toStreamDefinition (std::ostream & str ) const;
    std::ostream & toXmlDefinition (std::ostream & str, int indent ) const;
    std::ostream & toStream (std::ostream & str ) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;
    virtual YCPValue evaluate (bool cse = false);
    constTypePtr type() const;
};


#endif   // YCode_h
