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

   File:       YCPVoid.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPVoid data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPVoid.h"




// YCPVoidRep

YCPVoidRep::YCPVoidRep()
{
}


string YCPVoidRep::toString() const
{
    return "nil";
}


YCPValueType YCPVoidRep::valuetype() const
{
    return YT_VOID;
}


YCPOrder YCPVoidRep::compare(const YCPVoid &) const
{
    return YO_EQUAL;
}
