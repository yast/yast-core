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

   File:       YCPStatement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPStatement_h
#define YCPStatement_h


#include <ycp/YCPValue.h>

/**
 * @short Statement Type
 * Defines constants for the Statement types. The Statement type specifies the
 * class the YCPStatementRep object belongs to.
 */
enum YCPStatementType
{
    YS_BREAK         = 0,
    YS_CONTINUE      = 1,
    YS_EVALUATION    = 2,
    YS_RETURN        = 3,
    YS_IFTHENELSE    = 4,
    YS_NESTED	     = 5,
    YS_BUILTIN	     = 6
};


/**
 * @short Abstract base class of all YCP program statements.
 * A YCP protocol block is a sequence of statements.
 * A statement is _not_ an YCPValueRep. A statement consists of
 * an optional label, an optional condition and the actual
 * statement, that may be a return statement, a term evaluation
 * or a goto statement.
 */
class YCPStatementRep : public YCPElementRep
{
private:
    /**
     * line number where statement is defined
     */
    int line_number;
public:

    YCPStatementRep (int lineno) : line_number (lineno) {};

    /**
     * Returns the type of the statement. If you just want to check, whether
     * it is legal to cast an object of the YCPStatementRep to a certain more
     * specific object, you should use one of the is... methods.
     */
    virtual YCPStatementType statementtype() const = 0;
    
    /**
     * Returns true, if this object is of the type YCPBuiltinStatementRep.
     */
    bool isBuiltinStatement() const;

    /**
     * Returns true, if this object is of the type YCPNestedStatementRep.
     */
    bool isNestedStatement() const;

    /**
     * Returns true, if this object is of the type YCPEvaluationStatementRep.
     */
    bool isEvaluationStatement() const;

    /**
     * Returns true, if this object is of the type YCPBreakStatementRep.
     */
    bool isBreakStatement() const;

    /**
     * Returns true, if this object is of the type YCPContinueStatementRep.
     */
    bool isContinueStatement() const;
    
    /**
     * Returns true, if this object is of the type YCPReturnStatementRep.
     */
    bool isReturnStatement() const;
    
    /**
     * Returns true, if this object is of the type YCPIfThenElseStatementRep.
     */
    bool isIfThenElseStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPBuiltinStatementRep *.
     */
    YCPBuiltinStatement asBuiltinStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPNestedStatementRep *.
     */
    YCPNestedStatement asNestedStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPEvaluationStatementRep *.
     */
    YCPEvaluationStatement asEvaluationStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPBreakStatementRep *.
     */
    YCPBreakStatement asBreakStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPContinueStatementRep *.
     */
    YCPContinueStatement asContinueStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPReturnStatementRep *.
     */
    YCPReturnStatement asReturnStatement() const;

    /**
     * Casts this statement into a pointer of type const YCPIfThenElseStatementRep *.
     */
    YCPIfThenElseStatement asIfThenElseStatement() const;

    /**
     * Compares two YCPStatements for equality. Two statements are equal if
     * they have the same subtype and the same contents.
     */
    bool equal(const YCPStatement&) const;

    /**
     * Returns the line number of the statement
     */
    int linenumber() const { return line_number; };

    /**
     * Compares two statements for equality, greaterness or smallerness.
     * @param s statement to compare against
     * @return YO_LESS,    if this is smaller than s,
     *         YO_EQUAL,   if this is equal to s,
     *         YO_GREATER, if this is greater than s
     */ 
    YCPOrder compare(const YCPStatement &s) const;
};

/**
 * @short Wrapper for YCPStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPStatementRep
 * with the arrow operator. See @ref YCPStatementRep.
 */
class YCPStatement : public YCPElement
{
    DEF_COMMON(Statement, Element);
public:
};

#endif   // YCPStatement_h
