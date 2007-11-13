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

#include <ctype.h>

#include "ycp/y2log.h"
#include "ycp/YCPFloat.h"
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

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

    if ((*endptr == '.' && isdigit(*(endptr+1))                      )||
	(*endptr == 'e' && isdigit(*(endptr+1))                      )||
	(*endptr == 'e' && *(endptr+1) == '-' && isdigit(*(endptr+2)))||
	(*endptr == 'e' && *(endptr+1) == '+' && isdigit(*(endptr+2)))||
	(*endptr == 'E' && isdigit(*(endptr+1))                      )||
	(*endptr == 'E' && *(endptr+1) == '-' && isdigit(*(endptr+2)))||
	(*endptr == 'E' && *(endptr+1) == '+' && isdigit(*(endptr+2)))   )   // real float  
    {
	v = atof(r);   // use default
    }
}


double
YCPFloatRep::value() const
{
    return v;
}


YCPOrder
YCPFloatRep::compare(const YCPFloat& f) const
{
    if (v == f->v) return YO_EQUAL;
    else return v < f->v ? YO_LESS : YO_GREATER;
}


string
YCPFloatRep::toString() const
{
    char s[64];
    if ((v == (long)v)				// force decimal point for integer value range
	&& (v < 100000))
    {
	snprintf (s, 64, "%g.", v);
    }
    else
    {
	snprintf (s, 64, "%g", v);
    }
    return string(s);
}


YCPValueType
YCPFloatRep::valuetype() const
{
    return YT_FLOAT;
}

/**
 * Output value as bytecode to stream
 */
std::ostream &
YCPFloatRep::toStream (std::ostream & str) const
{
    return Bytecode::writeString (str, toString());
}

std::ostream &
YCPFloatRep::toXml (std::ostream & str, int indent ) const
{
    return str << "<const type=\"float\" value=\"" << toString() << "\"/>";
}


// --------------------------------------------------------

static const string
fromStream (bytecodeistream & str)
{
    string v;
    Bytecode::readString (str, v);
    return v;
}

YCPFloat::YCPFloat (bytecodeistream & str)
    : YCPValue (new YCPFloatRep (fromStream (str).c_str()))
{
}
