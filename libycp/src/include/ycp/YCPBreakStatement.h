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

   File:       YCPBreakStatement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPBreakStatement_h
#define YCPBreakStatement_h


#include "YCPStatement.h"




/**
 * @short YCPBreak statement without parameters
 */
class YCPBreakStatementRep : public YCPStatementRep
{
protected:
    friend class YCPBreakStatement;

    YCPBreakStatementRep(int lineno);

public:
    /**
     * Returns YS_BREAK. See @ref YCPStatementRep#statementtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Compares two YCPBreakStatements for equality, greaterness or smallerness.
     * (trivial - just for symmetry - always equal)
     */
    YCPOrder compare(const YCPBreakStatement &s) const;

    /**
     * Returns an ASCII YCP representation of this statement.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPBreakStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPBreakStatementRep
 * with the arrow operator. See @ref YCPBreakStatementRep.
 */
class YCPBreakStatement : public YCPStatement
{
    DEF_COMMON(BreakStatement, Statement);
public:
    YCPBreakStatement(int lineno) : YCPStatement(new YCPBreakStatementRep(lineno)) {}
};

#endif   // YCPBreakStatement_h
