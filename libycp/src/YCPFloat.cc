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

   File:       YCPFloat.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPFloat data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include <stdio.h>
#include <ctype.h>

#include "y2log.h"
#include "YCPFloat.h"



// YCPFloatRep

YCPFloatRep::YCPFloatRep(double v)
    : v(v)
{
}


YCPFloatRep::YCPFloatRep(const char *r)
{
    char *endptr = NULL;

    // first try HEX and OCT --> "-0x000b", "-000010", ...
    v = (double) strtol(r, &endptr, 0);

    if ( *endptr == '.' && isdigit(*(endptr+1))                       ||
	 *endptr == 'e' && isdigit(*(endptr+1))                       ||
	 *endptr == 'e' && *(endptr+1) == '-' && isdigit(*(endptr+2)) ||
	 *endptr == 'e' && *(endptr+1) == '+' && isdigit(*(endptr+2)) ||
	 *endptr == 'E' && isdigit(*(endptr+1))                       ||
	 *endptr == 'E' && *(endptr+1) == '-' && isdigit(*(endptr+2)) ||
	 *endptr == 'E' && *(endptr+1) == '+' && isdigit(*(endptr+2))    )   // real float  
    {
	v = atof(r);   // use default
    }
}


double YCPFloatRep::value() const
{
    return v;
}


YCPOrder YCPFloatRep::compare(const YCPFloat& f) const
{
    if (v == f->v) return YO_EQUAL;
    else return v < f->v ? YO_LESS : YO_GREATER;
}


string YCPFloatRep::toString() const
{
    char s[64];
    snprintf (s, 64, "%#.15g", v);
    return string(s);
}


YCPValueType YCPFloatRep::valuetype() const
{
    return YT_FLOAT;
}

