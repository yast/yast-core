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

   File:       YCPStatement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPStatement data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPStatement.h"
#include "YCPReturnStatement.h"
#include "YCPIfThenElseStatement.h"
#include "YCPEvaluationStatement.h"
#include "YCPBreakStatement.h"
#include "YCPContinueStatement.h"
#include "YCPNestedStatement.h"
#include "YCPBuiltinStatement.h"

// YCPStatementRep


// statement type checking

bool YCPStatementRep::isEvaluationStatement() const { return statementtype() == YS_EVALUATION; }
bool YCPStatementRep::isBreakStatement()      const { return statementtype() == YS_BREAK; }
bool YCPStatementRep::isContinueStatement()   const { return statementtype() == YS_CONTINUE; }
bool YCPStatementRep::isReturnStatement()     const { return statementtype() == YS_RETURN; }
bool YCPStatementRep::isIfThenElseStatement() const { return statementtype() == YS_IFTHENELSE; }
bool YCPStatementRep::isNestedStatement()     const { return statementtype() == YS_NESTED; }
bool YCPStatementRep::isBuiltinStatement()    const { return statementtype() == YS_BUILTIN; }


// statement type conversions

YCPEvaluationStatement YCPStatementRep::asEvaluationStatement() const
{
    if ( ! isEvaluationStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not Evaluation!",
	      toString().c_str());
	abort();
    }
    return YCPEvaluationStatement(static_cast<const YCPEvaluationStatementRep *>(this)); 
}


YCPBreakStatement YCPStatementRep::asBreakStatement() const
{
    if ( ! isBreakStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not Break!",
	      toString().c_str());
	abort();
    }
    return YCPBreakStatement(static_cast<const YCPBreakStatementRep *>(this)); 
}


YCPContinueStatement YCPStatementRep::asContinueStatement() const
{
    if ( ! isContinueStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not Continue!",
	      toString().c_str());
	abort();
    }
    return YCPContinueStatement(static_cast<const YCPContinueStatementRep *>(this)); 
}


YCPReturnStatement YCPStatementRep::asReturnStatement() const
{
    if ( ! isReturnStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not Return!",
	      toString().c_str());
	abort();
    }
    return YCPReturnStatement(static_cast<const YCPReturnStatementRep *>(this)); 
}


YCPIfThenElseStatement YCPStatementRep::asIfThenElseStatement() const
{
    if ( ! isIfThenElseStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not IfThenElse!",
	      toString().c_str());
	abort();
    }
    return YCPIfThenElseStatement(static_cast<const YCPIfThenElseStatementRep *>(this)); 
}


YCPNestedStatement YCPStatementRep::asNestedStatement() const
{
    if ( ! isNestedStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not Nested!",
	      toString().c_str());
	abort();
    }
    return YCPNestedStatement(static_cast<const YCPNestedStatementRep *>(this)); 
}


YCPBuiltinStatement YCPStatementRep::asBuiltinStatement() const
{
    if ( ! isBuiltinStatement() )
    {
	y2error ("Invalid cast of YCP statement %s! Should be but is not Builtin!",
	      toString().c_str());
	abort();
    }
    return YCPBuiltinStatement(static_cast<const YCPBuiltinStatementRep *>(this)); 
}


// comparison functions

bool YCPStatementRep::equal(const YCPStatement& s) const
{
    return compare( s ) == YO_EQUAL;
}


YCPOrder YCPStatementRep::compare(const YCPStatement& s) const
{
    if (statementtype() == s->statementtype())
    {
	switch ( statementtype() )
	{
	case YS_EVALUATION:
	    return this->asEvaluationStatement()->compare(s->asEvaluationStatement());
	case YS_BREAK:
	    return this->asBreakStatement()->compare(s->asBreakStatement());
	case YS_CONTINUE:
	    return this->asContinueStatement()->compare(s->asContinueStatement());
	case YS_RETURN:
	    return this->asReturnStatement()->compare(s->asReturnStatement());
	case YS_IFTHENELSE:
	    return this->asIfThenElseStatement()->compare(s->asIfThenElseStatement());
	case YS_NESTED:
	    return this->asNestedStatement()->compare(s->asNestedStatement());
	case YS_BUILTIN:
	    return this->asBuiltinStatement()->compare(s->asBuiltinStatement());
	default:
	    y2error ("Sorry, comparison of %s with %s not yet implemented",
		  toString().c_str(), s->toString().c_str());
	    return YO_EQUAL;
	}
    }
    else return statementtype() < s->statementtype() ? YO_LESS : YO_GREATER;
}
