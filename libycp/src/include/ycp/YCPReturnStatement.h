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

   File:       YCPReturnStatement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPReturnStatement_h
#define YCPReturnStatement_h


#include "YCPValue.h"
#include "YCPStatement.h"




/**
 * @short YCP return statement
 * A return statement finishes the execution of the block
 * and sets the return value of the block. It is much like
 * the C statement 'return <value>'.
 */
class YCPReturnStatementRep : public YCPStatementRep
{
    const YCPValue v;

protected:
    friend class YCPReturnStatement;

    /**
     * Creates a new return statement with a given path
     * and an optional value.
     */
    YCPReturnStatementRep(int lineno, const YCPValue v);

    /**
     * Cleans up
     */
    ~YCPReturnStatementRep() {}

public:
    /**
     * Returns YS_RETURN. See @ref YCPStatementRep#statementtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Returns the value of this return statement.
     */
    YCPValue value() const;

    /**
     * Compares two YCPReturnStatements for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the values of the Return Statements.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPReturnStatement &s) const;

    /**
     * Returns the syntactical representation of this return
     * statement, which is 'return value', where value is
     * a the ASCII representation of a YCP value.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPReturnStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPReturnStatementRep
 * with the arrow operator. See @ref YCPReturnStatementRep.
 */
class YCPReturnStatement : public YCPStatement
{
    DEF_COMMON(ReturnStatement, Statement);
public:
    YCPReturnStatement(int lineno, const YCPValue& v = YCPNull())
	: YCPStatement(new YCPReturnStatementRep(lineno, v)) {}
};

#endif   // YCPReturnStatement_h
