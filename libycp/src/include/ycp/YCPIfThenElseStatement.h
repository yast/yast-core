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

   File:       YCPIfThenElseStatement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPIfThenElseStatement_h
#define YCPIfThenElseStatement_h


#include "YCPStatement.h"
#include "YCPBlock.h"




/**
 * @short if then else statement
 * This data structure encapsulates an <tt>if</tt> <i>condition  then-block</i> 
 * <tt>else</tt><i> else-block</i>, where the else part is optional.
 */
class YCPIfThenElseStatementRep : public YCPStatementRep
{
    /**
     * The condition, not evaluated of course
     */
    const YCPValue cond;

    /**
     * The then block
     */
    const YCPBlock then_block;

    /**
     * The optional else block. 0 if left out.
     */
    const YCPBlock else_block;

protected:
    friend class YCPIfThenElseStatement;

    /**
     * Creates a new if-then-else statement.
     * @param condition The condition in form of a YCP expression
     * @param then_block The block to be executed in case the condition is fullfilled
     * @param else_block The block to be executed otherwise. The else-block is optional.
     */
    YCPIfThenElseStatementRep(int lineno,
			      const YCPValue& cond, const YCPBlock& then_block,
			      const YCPBlock& else_block);

    /**
     * Cleans up
     */
    ~YCPIfThenElseStatementRep() {}

public:
    /**
     * Returns YS_IFTHENELSE. See @ref YCPStatementRep#statementtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Returns the condition;
     */
    YCPValue condition() const;

    /**
     * Returns the then block
     */
    YCPBlock thenBlock() const;

    /**
     * Returns the else block
     */
    YCPBlock elseBlock() const;

    /**
     * Compares two YCPIfThenElseStatements for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the condition.
     *   If conditons are equal compare then blocks.
     *   If then blocks are equal compare else blocks.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPIfThenElseStatement &v) const;

    /**
     * Returns an ASCII YCP representation of this statement.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPIfThenElseStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPIfThenElseStatementRep
 * with the arrow operator. See @ref YCPIfThenElseStatementRep.
 */
class YCPIfThenElseStatement : public YCPStatement
{
    DEF_COMMON(IfThenElseStatement, Statement);
public:
    YCPIfThenElseStatement(int lineno, const YCPValue& cond, const YCPBlock& then_block,
			   const YCPBlock& else_block = YCPNull())
	: YCPStatement(new YCPIfThenElseStatementRep(lineno, cond, then_block, else_block)) {}
};

#endif   // YCPIfThenElseStatement_h
