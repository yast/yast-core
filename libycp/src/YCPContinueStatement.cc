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

   File:       YCPContinueStatement.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPContinueStatement data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPContinueStatement.h"



// YCPContinueStatementRep

YCPContinueStatementRep::YCPContinueStatementRep(int lineno) : YCPStatementRep (lineno) {}

YCPStatementType YCPContinueStatementRep::statementtype() const
{
    return YS_CONTINUE;
}

//compare YCPContinueStatements (trivial)
YCPOrder YCPContinueStatementRep::compare(const YCPContinueStatement& s) const
{
    return YO_EQUAL;
}

// get the statement as string
string YCPContinueStatementRep::toString() const
{
    return "continue;";
}



