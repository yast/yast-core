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

   File:       YCPEvaluationStatement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPEvaluationStatement_h
#define YCPEvaluationStatement_h


#include "YCPValue.h"
#include "YCPStatement.h"




/**
 * @short YCP term evaluation statement.
 * A YCPEvaluationStatementRep constist of a term is to be
 * evalutated by the @ref YCPInterpreter. @see YCPInterpreter#evaluateTerm.
 */
class YCPEvaluationStatementRep : public YCPStatementRep
{
    const YCPValue v;

protected:
    friend class YCPEvaluationStatement;

    /**
     * Creates a new term evaluation statement.
     * @param value Value to be evaluated.
     */
    YCPEvaluationStatementRep(int lineno, const YCPValue& value);

    /**
     * Cleans up
     */ 
    ~YCPEvaluationStatementRep() {}

public:
    /**
     * Returns YS_EVALUATION. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Returns the value (=expression) that should be evaluted.
     */
    YCPValue value() const;

    /**
     * Determines, if the expression to evaluted is a block.
     */
    bool isSubBlock() const;

    /**
     * Determines, if this statement is a variable declaration.
     */
    bool isVariableDeclaration() const;

    /**
     * Compares two YCPEvaluationStatements for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the values of the EvaluationStatements.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPEvaluationStatement &v) const;

    /**
     * Returns the ASCII representation of this statement.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPEvaluationStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPEvaluationStatementRep
 * with the arrow operator. See @ref YCPEvaluationStatementRep.
 */
class YCPEvaluationStatement : public YCPStatement
{
    DEF_COMMON(EvaluationStatement, Statement);
public:
    YCPEvaluationStatement(int lineno, const YCPValue& value)
	: YCPStatement(new YCPEvaluationStatementRep(lineno, value)) {}
};

#endif   // YCPEvaluationStatement_h
