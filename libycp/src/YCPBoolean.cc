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

   File:       YCPBoolean.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPBoolean data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPBoolean.h"

// YCPBooleanRep

YCPBooleanRep::YCPBooleanRep(bool v)
    : v(v != false)
{
}


YCPBooleanRep::YCPBooleanRep(const char *r)
    : v(!strcmp(r, "true"))
{
}


bool YCPBooleanRep::value() const
{
    return v;
}


string YCPBooleanRep::toString() const
{
    return v ? "true" : "false";
}


YCPValueType YCPBooleanRep::valuetype() const
{
    return YT_BOOLEAN;
}


YCPOrder YCPBooleanRep::compare(const YCPBoolean& b) const
{
    if (v == b->v) return YO_EQUAL;
    else return v < b->v ? YO_LESS : YO_GREATER;
}
