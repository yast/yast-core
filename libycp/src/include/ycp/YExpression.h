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

#include <iosfwd>
#include <string>
using std::string;

#include "ycp/YCode.h"
#include "y2/Y2Function.h"

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
    constTypePtr type() const { return m_entry->type(); }
};

//---------------------------------------------------------
// reference (-> Block + position)

class YEReference : public YCode {
    SymbolEntry *m_entry;
public:
    YEReference (SymbolEntry *entry);
    YEReference (std::istream & str);
    ~YEReference () {};
    const char *name () const;
    SymbolEntry *entry () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const;
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
    // dummy is here just to make it similar to YEBuiltin and YEFunction
    constTypePtr attachParameter (YCode *code, constTypePtr dummy = Type::Unspec);
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const { return Type::Term; }
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
    constTypePtr type() const { return Type::Boolean; }
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
    constTypePtr type() const { return Type::Locale; }
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
    constTypePtr type() const { return Type::List(); }
    int count () const;
    YCode* value (int index) const;
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
    constTypePtr type() const { return Type::Map(); }
};


//---------------------------------------------------------
// lookup expression (-> map, key, default value, default type)

class YELookup : public YCode {
    YCode *m_map;
    YCode *m_key;
    YCode *m_default;
    enum YCPValueType m_type;
public:
    YELookup (YCode *map, YCode *key, YCode *dftl, constTypePtr type);
    YELookup (std::istream & str);
    ~YELookup ();
//    YCode *map () const;
//    YCode *key () const;
//    YCode *dflt () const;
//    YCode *value () const;
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const { return m_default->type(); }
};


//---------------------------------------------------------
// propagation expression (-> value, from type, to type)

class YEPropagate : public YCode {
    constTypePtr m_from;
    constTypePtr m_to;
    YCode *m_value;
public:
    YEPropagate (YCode *value, constTypePtr from, constTypePtr to);
    YEPropagate (std::istream & str);
    ~YEPropagate ();
    string toString () const;
    bool canPropagate(const YCPValue& value, constTypePtr to_type) const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const { return m_to; }
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
    constTypePtr type() const { return m_decl->type; }
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
    constTypePtr type() const { return m_decl->type; }
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
    constTypePtr type() const { return m_true->type ()->commontype (m_false->type ()); }
};


//---------------------------------------------------------
// is (-> expression, type)

class YEIs : public YCode {
    YCode *m_expr;
    constTypePtr m_type;
public:
    YEIs (YCode *expr, constTypePtr type);
    YEIs (std::istream & str);
    ~YEIs ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const { return Type::Boolean; }
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
    constTypePtr type() const { return m_expr->type(); }
};


//---------------------------------------------------------
// bracket (-> expression)

class YEBracket : public YCode {
    YCode *m_var;		// (list, map) variable
    YCode *m_arg;		// bracket arguments
    YCode *m_def;		// default expression
    constTypePtr m_resultType;	// result type according to the parser
public:
    YEBracket (YCode *var, YCode *arg, YCode *def, constTypePtr resultType);
    YEBracket (std::istream & str);
    ~YEBracket ();
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const { return m_resultType; }
};


//---------------------------------------------------------
// Builtin ref (-> declaration_t, type, Args)

class YEBuiltin : public YCode {
    declaration_t *m_decl;
    FunctionTypePtr m_type;

    // symbol parameters (NULL, if no symbolic parameters)
    YBlock *m_parameterblock;

    ycodelist_t *m_parameters;
    ycodelist_t *m_last;
public:
    YEBuiltin (declaration_t *decl, YBlock *parameterblock = 0, constTypePtr type = 0);
    YEBuiltin (std::istream & str);
    ~YEBuiltin ();
    declaration_t *decl () const;
    // see YEFunction::finalize
    constTypePtr finalize ();
    constTypePtr returnType () const;
    // see YEFunction::attachParameter
    constTypePtr attachParameter (YCode *code, constTypePtr type = Type::Unspec);
    // attach symbolic variable parameter to function, return created TableEntry
    constTypePtr attachSymVariable (const char *name, constTypePtr type, unsigned int line, TableEntry *&tentry);
    string toString () const;
    YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type () const;
    YBlock *parameterBlock () const;
};

//---------------------------------------------------------
// Function ref (-> SymbolEntry ( param, param, ...) )

class YEFunction : public YCode, public Y2Function {
    SymbolEntry *m_entry;
    ycodelist_t *m_parameters;
    // no need for m_last since we're counting the parameters anyway in attachParameter()
    string qualifiedName () const;
public:
    YEFunction (SymbolEntry *entry);
    YEFunction (std::istream & str);
    ~YEFunction ();
    SymbolEntry *entry () const;
    /**
     * Attach parameter to external function call
     * @param code: parameter code
     * @param type: parameter type
     * @return NULL if success,
     *    != NULL (expected type) if wrong parameter type
     *    Type::Unspec if bad code (NULL or isError)
     *    Type::Error if excessive parameter
     */
    constTypePtr attachParameter (YCode *code, constTypePtr type);
    /**
     * 'close' function, perform final parameter check
     * if ok, return 0
     * if missing parameter, return its type
     * if undefined, return Type::Error (wrong type was already
     *   reported in attachParameter()
     */
    constTypePtr finalize ();
    string toString () const;
    virtual YCPValue evaluate (bool cse = false);
    std::ostream & toStream (std::ostream & str) const;
    constTypePtr type() const;

    // implementation of the Y2Function interface:

    virtual bool attachParameter (const YCPValue& arg, const int pos);
    virtual constTypePtr wantedParameterType () const;
    virtual bool appendParameter (const YCPValue& arg);
    virtual bool finishParameters ();
    virtual YCPValue evaluateCall () { return evaluate (); }
};

#endif   // YExpression_h
