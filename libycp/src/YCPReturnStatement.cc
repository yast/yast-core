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

   File:       YCPReturnStatement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPReturnStatement data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPReturnStatement.h"




// YCPReturnStatementRep

YCPReturnStatementRep::YCPReturnStatementRep(int lineno, const YCPValue v)
    : YCPStatementRep (lineno)
    , v(v)
{
}


YCPStatementType YCPReturnStatementRep::statementtype() const
{
    return YS_RETURN;
}


YCPValue YCPReturnStatementRep::value() const
{
    return v;
}


//compare ReturnStatements
YCPOrder YCPReturnStatementRep::compare(const YCPReturnStatement& s) const
{
    return v->compare( s->v );
}


// get the statement as string
string YCPReturnStatementRep::toString() const
{
    string ret = "return";
    if (!v.isNull()) ret += " " + v->toString();
    return ret + ";";
}



