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

class SymbolEntry;

class YBlock;		// forward declaration for YDo, YRepeat

//-------------------------------------------------------------------
/**
 * statement (-> statement, next statement)
 */

class YStatement : public YCode {
    int m_line;					// line number
public:
    YStatement (ycode code, int line = 0);
    YStatement (ycode code, std::istream & str);
    ~YStatement () {};
    virtual string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    int line () const { return m_line; };
    virtual YCPValue evaluate (bool cse = false);
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
    void propagate (TypeCode & from, TypeCode & to);
    YCode *value () const;	// needed in YBlock::justReturn
    void clearValue ();		// needed if justReturn triggers
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};


//-------------------------------------------------------------------
/**
 * typedef (-> type string)
 */

class YSTypedef : public YStatement {
    string m_name;		// name
    TypeCode m_type;		// type
public:
    YSTypedef (const string &name, const TypeCode & type, int line = 0);	// Typedef
    YSTypedef (std::istream & str);
    ~YSTypedef () {};
    string toString() const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};


//-------------------------------------------------------------------
/**
 * variable definition
 *   i.e. "integer i = <initial value>;"
 * sets code parameter of SymbolEntry to default value
 */

class YSVariable : public YStatement {
    SymbolEntry *m_entry;	// the symbol on the left side
public:
    YSVariable (SymbolEntry *entry, YCode *code, int line = 0);
    YSVariable (std::istream & str);
    ~YSVariable ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
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
};


//-------------------------------------------------------------------
/**
 * assignment
 * <m_entry> = <m_code>
 */

class YSAssign : public YStatement {
    SymbolEntry *m_entry;
    YCode *m_code;
public:
    YSAssign (SymbolEntry *entry, YCode *code, int line = 0);
    YSAssign (std::istream & str);
    ~YSAssign ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
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
};


//-------------------------------------------------------------------
/**
 * while-do statement (-> bool expr, loop statement)
 */

class YSWhile : public YStatement {
    YCode *m_condition;		// bool expr
    YCode *m_loop;		// loop statement/block

public:
    YSWhile (YCode *expr, YCode *loop, int line = 0);
    YSWhile (std::istream & str);
    ~YSWhile ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};


//-------------------------------------------------------------------
/**
 * repeat-until statement (-> loop statement, bool expr)
 */

class YSRepeat : public YStatement {
    YBlock *m_loop;		// loop block
    YCode *m_condition;		// bool expr

public:
    YSRepeat (YBlock *loop, YCode *expr, int line = 0);
    YSRepeat (std::istream & str);
    ~YSRepeat ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
};


//-------------------------------------------------------------------
/**
 * do-while statement (-> loop statement, bool expr)
 */

class YSDo : public YStatement {
    YBlock *m_loop;		// loop block
    YCode *m_condition;		// bool expr

public:
    YSDo (YBlock *loop, YCode *expr, int line = 0);
    YSDo (std::istream & str);
    ~YSDo ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
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
};


//-------------------------------------------------------------------
/**
 * include
 */

class YSInclude : public YStatement {
    string m_filename;
public:
    YSInclude (const string &filename, int line = 0);
    YSInclude (std::istream & str);
    ~YSInclude ();
    string toString () const;
    std::ostream & toStream (std::ostream & str) const;
    YCPValue evaluate (bool cse = false);
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
};

#endif   // YStatement_h
