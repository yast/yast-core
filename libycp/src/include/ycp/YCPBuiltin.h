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

   File:       YCPBuiltin.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPBuiltin_h
#define YCPBuiltin_h

// ***** WARNING *****
// changes here _must_ be reflected in YCPBuiltin::toString()
//
enum builtin_t {
	YCPB_UNKNOWN = 0,
	YCPB_DEEPQUOTE,		/*  1 */
	YCPB_LOCALDOMAIN,	/*  2 */
	YCPB_IS,		/*  3 */
	YCPB_PLUS,		/*  4 */
	YCPB_MINUS,		/*  5 */
	YCPB_MULT,		/*  6 */
	YCPB_DIV,		/*  7 */
	YCPB_MOD,		/*  8 */
	YCPB_NEG,		/*  9 */
	YCPB_AND,		/* 10 */
	YCPB_OR,		/* 11 */
	YCPB_BNOT,		/* 12 */
	YCPB_LOGAND,		/* 13 */
	YCPB_LOGOR,		/* 14 */
	YCPB_NOT,		/* 15 */
	YCPB_EQ,		/* 16 */
	YCPB_ST,		/* 17 */
	YCPB_GT,		/* 18 */
	YCPB_SE,		/* 19 */
	YCPB_GE,		/* 20 */
	YCPB_NEQ,		/* 21 */
	YCPB_LOCALDEFINE,	/* 22 */
	YCPB_LOCALDECLARE,	/* 23 */
	YCPB_GLOBALDEFINE,	/* 24 */
	YCPB_GLOBALDECLARE,	/* 25 */
	YCPB_SIZE,		/* 26 */
	YCPB_LOOKUP,		/* 27 */
	YCPB_SELECT,		/* 28 */
	YCPB_REMOVE,		/* 29 */
	YCPB_FOREACH,		/* 30 */
	YCPB_EVAL,		/* 31 */
	YCPB_SYMBOLOF,		/* 32 */
	YCPB_CALLBACK,		/* 33 */
	YCPB_DUMPSCOPE,		/* 34 */
	YCPB_MEMINFO,		/* 35 */
	YCPB_INCLUDE,		/* 36 */
	YCPB_MODULE,		/* 37 */
	YCPB_IMPORT,		/* 38 */
	YCPB_EXPORT,		/* 39 */
	YCPB_TEXTDOMAIN,	/* 40 */
	YCPB_UNDEFINE,		/* 41 */
	YCPB_FULLNAME,		/* 42 */
	YCPB_ISNIL,		/* 43 */
	YCPB_TRIPLE,		/* 44 */
	YCPB_UNION,		/* 45 */
	YCPB_ADD,		/* 46 */
	YCPB_CHANGE,		/* 47 */
	YCPB_MERGE,		/* 48 */
	YCPB_NISNIL,		/* 49 */
	YCPB_WFM,		/* 50 */
	YCPB_SCR,		/* 51 */
	YCPB_UI,		/* 52 */
	YCPB_LEFT,		/* 53 */
	YCPB_RIGHT,		/* 54 */
	YCPB_NLOCALE,		/* 55 */
	YCPB_BRACKET,		/* 56 */
	YCPB_BASSIGN,		/* 57 */
	YCPB_GETTEXTDOMAIN,	/* 58 */
	YCPB_SORT,		/* 59 */
	/* assign must be last here, see YCPBuiltin.cc  */
	YCPB_ASSIGN		/* 60 */
};


#include <ycp/YCPList.h>

/**
 * @short YCPValueRep representing a builtin term.
 * A YCPBuiltinRep is a YCPValue containing a list plus a builtin_t
 * representing the builtin.
 */
class YCPBuiltinRep : public YCPValueRep
{
    /**
     * The builtin
     */
    const builtin_t code;

    /**
     * YCP list representing the builtin's arguments
     */
    YCPList l;

protected:
    friend class YCPBuiltin;

    /**
     * Creates a new builtin with void argument list
     */
    YCPBuiltinRep(const builtin_t b);

    /**
     * Creates a new builtin with a single argument
     */
    YCPBuiltinRep(const builtin_t b, const YCPValue& v);

    /**
     *  Creates a new builtin with argument list l.
     */
    YCPBuiltinRep(const builtin_t b, const YCPList& l);

    /**
     * Cleans up
     */
    ~YCPBuiltinRep() {}

public:
    /**
     * Returns the term's symbol
     */
    builtin_t builtin_code() const;

    /**
     * Returns the term's arguments list
     */

    YCPList args() const;

    /**
     * Compares two YCPBuiltins for equality, greaterness or smallerness.
     * The order is defined by builtin_t.
     */
    YCPOrder compare(const YCPBuiltin &v) const;

    /**
     * Returns an ASCII representation of the builtin.
     * Builtin are denoted by comma separated values enclosed
     * by brackets precedeed by a symbol, for example a(1,2)
     * or b() or Hugo_17("hirn", c(true)).
     */
    string toString() const;

    /**
     * Mapping for the builtin's list isEmpty() function
     */
    bool isEmpty() const;

    /**
     * Mapping for the builtin's list size() function
     */
    int size() const;

    /**
     * Mapping for the builtin's list reserve (int) function
     */
    void reserve (int size);

    /**
     * Mapping for the builtin's list value() function
     */
    YCPValue value(int n) const;

    /**
     * Mapping for the builtin's list add() function
     */
    void add(const YCPValue& value);

    /**
     * Mapping for the builtin's list functionalAdd() function
     */
    YCPBuiltin functionalAdd (const YCPValue& value, bool prepend) const;

    /**
     * Returns YT_BUILTIN. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPBuiltinRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPBuiltinRep
 * with the arrow operator. See @ref YCPBuiltinRep.
 */
class YCPBuiltin : public YCPValue
{
    DEF_COMMON(Builtin, Value);
public:
    YCPBuiltin(const builtin_t b) : YCPValue(new YCPBuiltinRep(b)) {}
    YCPBuiltin(const builtin_t b, const YCPValue& v) : YCPValue(new YCPBuiltinRep(b, v)) {}
    YCPBuiltin(const builtin_t b, const YCPList& l) : YCPValue(new YCPBuiltinRep(b, l)) {}
};

#endif   // YCPBuiltin_h
