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
#include "ycp/Import.h"

class SymbolEntry;

class YBlock;		// forward declaration for YDo, YRepeat

//-------------------------------------------------------------------
/**
 * statement (-> statement, next statement)
 */

class YStatement : public YCode {
    int m_line;					// line number
public:
    YStatement (ykind kind, int line = 0);
    YStatement (ykind kind, std::istream & str);
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

class YSExpression : public YStatement {
    YCode *m_expr;
public:
    YSExpression (YCode *expr, int line = 0);		// statement
    YSExpression (std::istream & str);
    ~YSExpression ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};


//-------------------------------------------------------------------
/**
 * return (-> value)
 */

class YSReturn : public YStatement {
    YCode *m_value;
public:
    YSReturn (YCode *value, int line = 0);
    YSReturn (std::istream & str);
    ~YSReturn ();
    void propagate (constTypePtr from, constTypePtr to);
    YCode *value () const;	// needed in YBlock::justReturn
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

class YSTypedef : public YStatement {
    string m_name;		// name
    constTypePtr m_type;	// type
public:
    YSTypedef (const string &name, constTypePtr type, int line = 0);	// Typedef
    YSTypedef (std::istream & str);
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

class YSFunction : public YStatement {
    // the functions' symbol, it's code is this YSFunction !
    SymbolEntry *m_entry;

public:
    YSFunction (SymbolEntry *entry, int line = 0);
    YSFunction (std::istream & str);
    ~YSFunction ();

    // symbol entry of function itself
    SymbolEntry *entry () const;

    // access to function definition
    YFunction *function () const;

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

class YSAssign : public YStatement {
    SymbolEntry *m_entry;
    YCode *m_code;
public:
    YSAssign (bool definition, SymbolEntry *entry, YCode *code, int line = 0);
    YSAssign (bool definition, std::istream & str);
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

class YSBracket : public YStatement {
    SymbolEntry *m_entry;
    YCode *m_arg;
    YCode *m_code;
public:
    YSBracket (SymbolEntry *entry, YCode *arg, YCode *code, int line = 0);
    YSBracket (std::istream & str);
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

class YSIf : public YStatement {
    YCode *m_condition;		// bool expr
    YCode *m_true;		// true statement/block
    YCode *m_false;		// false statement/block
public:
    YSIf (YCode *a_expr, YCode *a_true, YCode *a_false, int line = 0);
    YSIf (std::istream & str);
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

class YSWhile : public YStatement {
    YCode *m_condition;		// bool expr
    YCode *m_loop;		// loop statement

public:
    YSWhile (YCode *expr, YCode *loop, int line = 0);
    YSWhile (std::istream & str);
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

class YSRepeat : public YStatement {
    YCode *m_loop;		// loop statement
    YCode *m_condition;		// bool expr

public:
    YSRepeat (YCode *loop, YCode *expr, int line = 0);
    YSRepeat (std::istream & str);
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

class YSDo : public YStatement {
    YCode *m_loop;		// loop statement
    YCode *m_condition;		// bool expr

public:
    YSDo (YCode *loop, YCode *expr, int line = 0);
    YSDo (std::istream & str);
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

class YSTextdomain : public YStatement {
    string m_domain;
public:
    YSTextdomain (const string &textdomain, int line = 0);
    YSTextdomain (std::istream & str);
    ~YSTextdomain ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
private:
    void bind ();
};


//-------------------------------------------------------------------
/**
 * include
 */

class YSInclude : public YStatement {
    string m_filename;
    bool m_skipped;
public:
    YSInclude (const string &filename, int line = 0, bool skipped = false);
    YSInclude (std::istream & str);
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

class YSImport : public YStatement, public Import {
public:
    YSImport (const string &name, int line = 0);
    YSImport (const string &name, Y2Namespace *name_space);
    YSImport (std::istream & str);
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

class YSFilename : public YStatement {
    string m_filename;
public:
    YSFilename (const string &filename, int line = 0);
    YSFilename (std::istream & str);
    ~YSFilename ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
    constTypePtr type () const { return Type::Void; };
};

#endif   // YStatement_h
