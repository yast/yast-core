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

   File:       YCPInteger.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPInteger data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */


#include <stdio.h>

#include "y2log.h"
#include "YCPInteger.h"


// YCPIntegerRep

YCPIntegerRep::YCPIntegerRep(long long v)
    : v(v)
{
}


YCPIntegerRep::YCPIntegerRep(const char *r)
{
    
    int converted;
    if (r && r[0] == '0')
	if (r[1] == 'x')
	    converted = sscanf(r, "%Lx", &v);
	else
	    converted = sscanf(r, "%Lo", &v);
    else
	converted = sscanf(r, "%Ld", &v);

    if (converted != 1) {
        y2warning("Cannot convert %s to an integer", r);
        v = 0;
    }
}


long long YCPIntegerRep::value() const
{
    return v;
}


YCPOrder YCPIntegerRep::compare(const YCPInteger& i) const
{
    if (v == i->v) return YO_EQUAL;
    else return v < i->v ? YO_LESS : YO_GREATER;
}


string YCPIntegerRep::toString() const
{
    char s[64];
    snprintf (s, 64, "%Ld", v);
    return string(s);
}


YCPValueType YCPIntegerRep::valuetype() const
{
    return YT_INTEGER;
}
