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

   File:	YCPNestedStatement.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * YCPNestedStatement data type
 */

#include "y2log.h"
#include "YCPNestedStatement.h"

// YCPNestedStatementRep

YCPNestedStatementRep::YCPNestedStatementRep(int lineno, const YCPBlock b)
    : YCPStatementRep (lineno)
    , b(b)
{
}


YCPStatementType YCPNestedStatementRep::statementtype() const
{
    return YS_NESTED;
}


YCPBlock YCPNestedStatementRep::value() const
{
    return b;
}


//compare NestedStatements
YCPOrder YCPNestedStatementRep::compare(const YCPNestedStatement& s) const
{
    return b->compare( s->b );
}


// get the statement as string
string YCPNestedStatementRep::toString() const
{
    return b->toString();
}
