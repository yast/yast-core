/*---------------------------------------------------------*- c++ -*---\
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

   File:	YStatement.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YStatement_h
#define YStatement_h

#include <string>
using std::string;

#include "ycp/YCode.h"
#include "ycp/SymbolTable.h"
#include "ycp/YSymbolEntry.h"
#include "ycp/Import.h"

class YBlock;		// forward declaration for YDo, YRepeat

//-------------------------------------------------------------------

DEFINE_DERIVED_POINTER(YStatement, YCode);
DEFINE_DERIVED_POINTER(YSExpression, YCode);
DEFINE_DERIVED_POINTER(YSBlock, YCode);
DEFINE_DERIVED_POINTER(YSReturn, YCode);
DEFINE_DERIVED_POINTER(YSTypedef, YCode);
DEFINE_DERIVED_POINTER(YSFunction, YCode);
DEFINE_DERIVED_POINTER(YSAssign, YCode);
DEFINE_DERIVED_POINTER(YSBracket, YCode);
DEFINE_DERIVED_POINTER(YSIf, YCode);
DEFINE_DERIVED_POINTER(YSWhile, YCode);
DEFINE_DERIVED_POINTER(YSRepeat, YCode);
DEFINE_DERIVED_POINTER(YSDo, YCode);
DEFINE_DERIVED_POINTER(YSTextdomain, YCode);
DEFINE_DERIVED_POINTER(YSInclude, YCode);
DEFINE_DERIVED_POINTER(YSImport, YCode);
DEFINE_DERIVED_POINTER(YSFilename, YCode);

//-------------------------------------------------------------------
/**
 * statement (-> statement, next statement)
 */

class YStatement : public YCode
{
    REP_BODY(YStatement);
    int m_line;					// line number
public:
    YStatement (ykind kind, int line = 0);
    YStatement (ykind kind, bytecodeistream & str);
    ~YStatement () {};
    virtual string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    int line () const { return m_line; };
    virtual YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * an expression, i.e. a function call
 */

class YSExpression : public YStatement
{
    REP_BODY(YSExpression);
    YCodePtr m_expr;
public:
    YSExpression (YCodePtr expr, int line = 0);		// statement
    YSExpression (bytecodeistream & str);
    ~YSExpression ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * a block
 */

class YSBlock : public YStatement
{
    REP_BODY(YSBlock);
    YBlockPtr m_block;
public:
    YSBlock (YBlockPtr block, int line = 0);
    YSBlock (bytecodeistream & str);
    ~YSBlock ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * return (-> value)
 */

class YSReturn : public YStatement
{
    REP_BODY(YSReturn);
    YCodePtr m_value;
public:
    YSReturn (YCodePtr value, int line = 0);
    YSReturn (bytecodeistream & str);
    ~YSReturn ();
    void propagate (constTypePtr from, constTypePtr to);
    YCodePtr value () const;	// needed in YBlock::justReturn
    void clearValue ();		// needed if justReturn triggers
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * typedef (-> type string)
 */

class YSTypedef : public YStatement
{
    REP_BODY(YSTypedef);
    Ustring m_name;		// name
    constTypePtr m_type;	// type
public:
    YSTypedef (const string &name, constTypePtr type, int line = 0);	// Typedef
    YSTypedef (bytecodeistream & str);
    ~YSTypedef () {};
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * function declaration (m_body == 0) or definition (m_body != 0)
 */

class YSFunction : public YStatement
{
    REP_BODY(YSFunction);
    // the functions' symbol, it's code is this YSFunction !
    YSymbolEntryPtr m_entry;

public:
    YSFunction (YSymbolEntryPtr entry, int line = 0);
    YSFunction (bytecodeistream & str);
    ~YSFunction ();

    // symbol entry of function itself
    SymbolEntryPtr entry () const;

    // access to function definition
    YFunctionPtr function () const;

    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * assignment or definition
 * [<type>] <m_entry> = <m_code>
 */

class YSAssign : public YStatement
{
    REP_BODY(YSAssign);
    SymbolEntryPtr m_entry;
    YCodePtr m_code;
public:
    YSAssign (bool definition, SymbolEntryPtr entry, YCodePtr code, int line = 0);
    YSAssign (bool definition, bytecodeistream & str);
    ~YSAssign ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * bracket assignment
 * <m_entry>[<m_arg>] = <m_code>
 */

class YSBracket : public YStatement
{
    REP_BODY(YSBracket);
    SymbolEntryPtr m_entry;
    YCodePtr m_arg;
    YCodePtr m_code;
public:
    YSBracket (SymbolEntryPtr entry, YCodePtr arg, YCodePtr code, int line = 0);
    YSBracket (bytecodeistream & str);
    ~YSBracket ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    // recursively extract list arg at idx, get value from current at idx
    // and replace with value. re-generating the list/map/term during unwind
    YCPValue commit (YCPValue current, int idx, YCPList arg, YCPValue value);
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * If-then-else statement (-> bool expr, true statement, false statement)
 */

class YSIf : public YStatement
{
    REP_BODY(YSIf);
    YCodePtr m_condition;		// bool expr
    YCodePtr m_true;		// true statement/block
    YCodePtr m_false;		// false statement/block
public:
    YSIf (YCodePtr a_expr, YCodePtr a_true, YCodePtr a_false, int line = 0);
    YSIf (bytecodeistream & str);
    ~YSIf ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * while-do statement (-> bool expr, loop statement)
 */

class YSWhile : public YStatement
{
    REP_BODY(YSWhile);
    YCodePtr m_condition;		// bool expr
    YCodePtr m_loop;		// loop statement

public:
    YSWhile (YCodePtr expr, YCodePtr loop, int line = 0);
    YSWhile (bytecodeistream & str);
    ~YSWhile ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * repeat-until statement (-> loop statement, bool expr)
 */

class YSRepeat : public YStatement
{
    REP_BODY(YSRepeat);
    YCodePtr m_loop;		// loop statement
    YCodePtr m_condition;		// bool expr

public:
    YSRepeat (YCodePtr loop, YCodePtr expr, int line = 0);
    YSRepeat (bytecodeistream & str);
    ~YSRepeat ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * do-while statement (-> loop statement, bool expr)
 */

class YSDo : public YStatement
{
    REP_BODY(YSDo);
    YCodePtr m_loop;		// loop statement
    YCodePtr m_condition;		// bool expr

public:
    YSDo (YCodePtr loop, YCodePtr expr, int line = 0);
    YSDo (bytecodeistream & str);
    ~YSDo ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * textdomain
 */

class YSTextdomain : public YStatement
{
    REP_BODY(YSTextdomain);
    Ustring m_domain;
public:
    YSTextdomain (const string &textdomain, int line = 0);
    YSTextdomain (bytecodeistream & str);
    ~YSTextdomain ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
    const char *domain () const { return m_domain->c_str(); };
private:
    void bind ();
};


//-------------------------------------------------------------------
/**
 * include
 */

class YSInclude : public YStatement
{
    REP_BODY(YSInclude);
    Ustring m_filename;
    bool m_skipped;
public:
    YSInclude (const string &filename, int line = 0, bool skipped = false);
    YSInclude (bytecodeistream & str);
    ~YSInclude ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
    string filename () const { return m_filename; };
};


//-------------------------------------------------------------------
/**
 * import
 */

class YSImport : public YStatement, public Import
{
    REP_BODY(YSImport);
public:
    YSImport (const string &name, int line = 0);
    YSImport (const string &name, Y2Namespace *name_space);
    YSImport (bytecodeistream & str);
    ~YSImport ();
    string name () const;
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * internal: Filename
 */

class YSFilename : public YStatement
{
    REP_BODY(YSFilename);
    Ustring m_filename;
public:
    YSFilename (const string &filename, int line = 0);
    YSFilename (bytecodeistream & str);
    ~YSFilename ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};

#endif   // YStatement_h
