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

   File:       YCPEvaluationStatement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPEvaluationStatement data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPBuiltin.h"
#include "YCPEvaluationStatement.h"




// YCPEvaluationStatementRep

YCPEvaluationStatementRep::YCPEvaluationStatementRep(int lineno, const YCPValue& value)
    : YCPStatementRep (lineno)
    , v(value)
{
}

YCPStatementType YCPEvaluationStatementRep::statementtype() const
{
    return YS_EVALUATION;
}

YCPValue YCPEvaluationStatementRep::value() const
{
    return v;
}

bool YCPEvaluationStatementRep::isSubBlock() const
{
    return v->isBlock();
}


bool YCPEvaluationStatementRep::isVariableDeclaration() const
{
    return v->isBuiltin() 
	&& ((v->asBuiltin()->builtin_code() == YCPB_LOCALDECLARE)
	    || (v->asBuiltin()->builtin_code() == YCPB_GLOBALDECLARE));
}


//compare EvaluationStatements
YCPOrder YCPEvaluationStatementRep::compare(const YCPEvaluationStatement& s) const
{
    return v->compare( s->v );
}


// get the statement as string
string YCPEvaluationStatementRep::toString() const
{
    return v->toString() + (isSubBlock() ? "" : ";");
}

