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

   File:       YCPDeclAny.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPDeclAny data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPDeclAny.h"



// YCPDeclAnyRep

YCPDeclarationType YCPDeclAnyRep::declarationtype() const
{
    return YD_ANY;
}


bool YCPDeclAnyRep::allows(const YCPValue&) const
{
    return true;
}


YCPOrder YCPDeclAnyRep::compare(const YCPDeclAny& d) const
{
    return YO_EQUAL;
}


string YCPDeclAnyRep::toString() const
{
    return "any";
}
