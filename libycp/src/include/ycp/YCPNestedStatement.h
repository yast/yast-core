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

   File:	YCPNestedStatement.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPNestedStatement_h
#define YCPNestedStatement_h


#include "YCPBlock.h"
#include "YCPStatement.h"


/**
 * @short YCP term nested statement.
 * A YCPNestedStatementRep constist of a block
 * evalutated by the @ref YCPInterpreter. @see YCPInterpreter#evaluateBlock.
 */
class YCPNestedStatementRep : public YCPStatementRep
{
    const YCPBlock b;

protected:
    friend class YCPNestedStatement;

    /**
     * Creates a new term evaluation statement.
     * @param value Value to be evaluated.
     */
    YCPNestedStatementRep(int lineno, const YCPBlock value);

    /**
     * Cleans up
     */ 
    ~YCPNestedStatementRep() {}

public:
    /**
     * Returns YS_NESTED. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Returns the value (=block) that should be evaluated.
     */
    YCPBlock value() const;

    /**
     * Compares two YCPNestedStatements for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the values of the NestedStatements.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPNestedStatement &v) const;

    /**
     * Returns the ASCII representation of this statement.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPNestedStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPNestedStatementRep
 * with the arrow operator. See @ref YCPNestedStatementRep.
 */
class YCPNestedStatement : public YCPStatement
{
    DEF_COMMON(NestedStatement, Statement);
public:
    YCPNestedStatement(int lineno, const YCPBlock& value)
	: YCPStatement(new YCPNestedStatementRep(lineno, value)) {}
};

#endif   // YCPNestedStatement_h
