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

   File:       YCPBreakStatement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPBreakStatement data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPBreakStatement.h"



// YCPBreakStatementRep

YCPBreakStatementRep::YCPBreakStatementRep(int lineno) : YCPStatementRep (lineno) {}

YCPStatementType YCPBreakStatementRep::statementtype() const
{
    return YS_BREAK;
}

//compare YCPBreakStatements (trivial)
YCPOrder YCPBreakStatementRep::compare(const YCPBreakStatement& s) const
{
    return YO_EQUAL;
}

// get the statement as string
string YCPBreakStatementRep::toString() const
{
    return "break;";
}


