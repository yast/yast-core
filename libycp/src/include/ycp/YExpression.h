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

   File:	YExpression.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

   This file defines the various 'expressions' in YCode

/-*/
// -*- c++ -*-

#ifndef YExpression_h
#define YExpression_h

#include <iostream>
#include <string>
using std::string;

#include "ycp/YCode.h"

//---------------------------------------------------------
// variable ref (-> Block + position)

class YEVariable : public YCode {
    SymbolEntry *m_entry;
public:
    YEVariable (SymbolEntry *entry);
    YEVariable (std::istream & str);
    ~YEVariable () {};
    const char *name () const;
    SymbolEntry *entry () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};

//---------------------------------------------------------
// Term (-> name, args)

class YETerm : public YCode {
    const char *m_name;
    ycodelist_t *m_parameters;
    ycodelist_t *m_last;
public:
    YETerm (const char *name);
    YETerm (std::istream & str);
    ~YETerm ();
    void attachTermParameter (YCode *code);
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// Compare (-> arg1, arg2, type)

class YECompare : public YCode {
public:
    enum cmp_op { C_NOT = 1, C_EQ = 2, C_LT = 4,	// base operations
		  C_NEQ = C_NOT|C_EQ,
		  C_LE =  C_EQ|C_LT,
		  C_GE =  C_NOT|C_LT,
		  C_GT =  C_NOT|C_EQ|C_LT
    };
    typedef cmp_op c_op;	    
private:
    YCode *m_left;
    c_op m_op;
    YCode *m_right;
public:
    YECompare (YCode *left, c_op op, YCode *right);
    YECompare (std::istream & str);
    ~YECompare ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// Builtin ref (-> declaration_t, type, Args)

class YEBuiltin : public YCode {
    declaration_t *m_decl;
    TypeCode m_type;
    ycodelist_t *m_parameters;
    ycodelist_t *m_last;
public:
    YEBuiltin (declaration_t *decl, const TypeCode &type = "|");
    YEBuiltin (std::istream & str);
    ~YEBuiltin ();
    declaration_t *decl () const;
    // set final (type matching) declaration
    // return 0 if decl does not match parameters,
    // return decl if decl does match parameters
    declaration_t *setDecl (declaration_t *decl);
    TypeCode type () const;
    TypeCode returnType () const;
    declaration_t *attachBuiltinParameter (YCode *code, const TypeCode &type);
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// Function ref (-> SymbolEntry ( param, param, ...) )

class YEFunction : public YCode {
    SymbolEntry *m_entry;
    ycodelist_t *m_parameters;
public:
    YEFunction (SymbolEntry *entry);
    YEFunction (std::istream & str);
    ~YEFunction ();
    SymbolEntry *entry () const;
    /**
     * Attach parameter to external function call
     * @param code if 0, 'close' function, perform final parameter check
     * against declaration
     * @param type parameter type
     * @return  "" if success, otherwise type of expected param
     * ("*" if excess params)
     */
    TypeCode attachFunctionParameter (YCode *code, const TypeCode &type);
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// locale expression (-> singular, plural, count)

class YELocale : public YCode {
    const char *m_singular;
    const char *m_plural;
    YCode *m_count;
    const char *m_domain;
public:
    YELocale (const char *singular, const char *plural, YCode *count, const char *textdomain);
    YELocale (std::istream & str);
    ~YELocale ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// list expression (-> value, next list value)

class YEList : public YCode {
    ycodelist_t *m_first;
    ycodelist_t *m_last;
public:
    YEList (YCode *code);
    YEList (std::istream & str);
    ~YEList ();
    void attach (YCode *element);
//    YCode *code () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// map expression (-> key, value, next key/value pair)

class YEMap : public YCode {
    typedef struct mapval { YCode *key; YCode *value; struct mapval *next; } mapval_t;
    mapval_t *m_first;
    mapval_t *m_last;
public:
    YEMap (YCode *key, YCode *value);
    YEMap (std::istream & str);
    ~YEMap ();
    void attach (YCode *key, YCode *value);
//    YCode *key () const;
//    YCode *value () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// lookup expression (-> map, key, default value, default type)

class YELookup : public YCode {
    YCode *m_map;
    YCode *m_key;
    YCode *m_default;
    enum YCPValueType m_type;
public:
    YELookup (YCode *map, YCode *key, YCode *dftl, const TypeCode &type);
    YELookup (std::istream & str);
    ~YELookup ();
//    YCode *map () const;
//    YCode *key () const;
//    YCode *dflt () const;
//    YCode *value () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// propagation expression (-> value, from type, to type)

class YEPropagate : public YCode {
    TypeCode m_from;
    TypeCode m_to;
    YCode *m_value;
public:
    YEPropagate (YCode *value, const TypeCode & from, const TypeCode & to);
    YEPropagate (std::istream & str);
    ~YEPropagate ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// unary expression (-> declaration_t, arg)

class YEUnary : public YCode {
    declaration_t *m_decl;
    YCode *m_arg;		// argument
public:
    YEUnary (declaration_t *decl, YCode *arg);		// expression
    YEUnary (std::istream & str);
    ~YEUnary ();
    declaration_t *decl () const;
//    YCode *arg () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// binary expression (-> declaration_t, arg1, arg2)

class YEBinary : public YCode {
    declaration_t *m_decl;
    YCode *m_arg1;		// argument1
    YCode *m_arg2;		// argument2
public:
    YEBinary (declaration_t *decl, YCode *arg1, YCode *arg2);
    YEBinary (std::istream & str);
    ~YEBinary () {};
    declaration_t *decl ();
//    YCode *arg1 () const;
//    YCode *arg2 () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// Triple (? :) expression (-> bool expr, true value, false value)

class YETriple : public YCode {
    YCode *m_expr;		// bool expr
    YCode *m_true;		// true value
    YCode *m_false;		// false value
public:
    YETriple (YCode *a_expr, YCode *a_true, YCode *a_false);
    YETriple (std::istream & str);
    ~YETriple () {};
//    YCode *expr () const;
//    YCode *iftrue () const;
//    YCode *iffalse () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// is (-> expression, type)

class YEIs : public YCode {
    YCode *m_expr;
    TypeCode m_type;
public:
    YEIs (YCode *expr, const TypeCode &type);
    YEIs (std::istream & str);
    ~YEIs ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// Function with symbols ref (-> decl (param, param, ...) )
// this is just a temporary parser object while scanning
// a function call and collecting parameters.
// It is converted to a YEBuiltin via toBuiltin() later.

class YESymFunc : public YCode {
    declaration_t *m_decl;
    TypeCode m_type;

    // symbol parameters
    YBlock *m_parameterblock;

public:
    YESymFunc (declaration_t *decl, YBlock *parameterblock);
    ~YESymFunc ();
    declaration_t *decl () const;
    // set final (type matching) declaration
    // return 0 if decl does not match parameters,
    // return decl if decl does match parameters
    declaration_t *setDecl (declaration_t *decl);
    TypeCode type () const;
    TypeCode returnType () const;
    // after all parameters are complete, return the matching builtin
    YEBuiltin *toBuiltin (SymbolTable *table);
    // attach value parameter to function
    declaration_t *attachSymValue (const TypeCode &type, unsigned int line, YCode *code);
    // attach symbolic variable parameter to function, return created TableEntry
    declaration_t *attachSymVariable (const char *name, const TypeCode &type, unsigned int line, TableEntry *&tentry);
    YBlock *parameterBlock () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};


//---------------------------------------------------------
// return (-> expression)

class YEReturn : public YCode {
    YCode *m_expr;
public:
    YEReturn (YCode *expr);
    YEReturn (std::istream & str);
    ~YEReturn ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};

// bracket (-> expression)

class YEBracket : public YCode {
    YCode *m_var;		// (list, map) variable
    YCode *m_arg;		// bracket arguments
    YCode *m_def;		// default expression
public:
    YEBracket (YCode *var, YCode *arg, YCode *def);
    YEBracket (std::istream & str);
    ~YEBracket ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
};

#endif   // YExpression_h
