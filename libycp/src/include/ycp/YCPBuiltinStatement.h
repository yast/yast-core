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

   File:       YCPBuiltinStatement.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPBuiltinStatement_h
#define YCPBuiltinStatement_h

#include "YCPValue.h"
#include "YCPBuiltin.h"
#include "YCPStatement.h"


/**
 * @short YCP builtin statement
 * A builtin statement represents a builtin action.
 */
class YCPBuiltinStatementRep : public YCPStatementRep
{
    /**
     * The builtin code
     */
    const builtin_t c;

    /**
     * YCP value representing the builtin's argument
     */
    const YCPValue v;

protected:
    friend class YCPBuiltinStatement;

    /**
     * Creates a new return statement with a given path
     * and an optional value.
     */
    YCPBuiltinStatementRep(int lineno, const builtin_t c, const YCPValue v);

    /**
     * Cleans up
     */
    ~YCPBuiltinStatementRep() { }

public:
    /**
     * Returns YS_BUILTIN. See @ref YCPStatementRep#statementtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Returns the code of this builtin statement.
     */
    builtin_t code() const;

    /**
     * Returns the value of this builtin statement.
     */
    YCPValue value() const;

    /**
     * Compares two YCPBuiltinStatements for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the values of the Builtin Statements.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPBuiltinStatement &s) const;

    /**
     * Returns the syntactical representation of this return
     * statement, which is 'return value', where value is
     * a the ASCII representation of a YCP value.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPBuiltinStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPBuiltinStatementRep
 * with the arrow operator. See @ref YCPBuiltinStatementRep.
 */
class YCPBuiltinStatement : public YCPStatement
{
    DEF_COMMON(BuiltinStatement, Statement);
public:
    YCPBuiltinStatement(int lineno, const builtin_t c, const YCPValue& v)
	: YCPStatement(new YCPBuiltinStatementRep(lineno, c, v)) {}
};

#endif   // YCPBuiltinStatement_h
