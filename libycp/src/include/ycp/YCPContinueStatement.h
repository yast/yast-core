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

   File:       YCPContinueStatement.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPContinueStatement_h
#define YCPContinueStatement_h


#include "YCPStatement.h"




/**
 * @short YCPContinue statement without parameters
 */
class YCPContinueStatementRep : public YCPStatementRep
{
protected:
    friend class YCPContinueStatement;

    YCPContinueStatementRep(int lineno);

public:
    /**
     * Returns YS_CONTINUE. See @ref YCPStatementRep#statementtype.
     */
    YCPStatementType statementtype() const;

    /**
     * Compares two YCPContinueStatements for equality, greaterness or smallerness.
     * (trivial - just for symmetry - always equal)
     */
    YCPOrder compare(const YCPContinueStatement &s) const;

    /**
     * Returns an ASCII YCP representation of this statement.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPContinueStatementRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPContinueStatementRep
 * with the arrow operator. See @ref YCPContinueStatementRep.
 */
class YCPContinueStatement : public YCPStatement
{
    DEF_COMMON(ContinueStatement, Statement);
public:
    YCPContinueStatement(int lineno) : YCPStatement(new YCPContinueStatementRep(lineno)) {}
};

#endif   // YCPContinueStatement_h
