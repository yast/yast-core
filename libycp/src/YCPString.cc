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

   File:       YCPString.cc

   Author:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "ycp/y2log.h"
#include "ycp/YCPString.h"
#include "ycp/Bytecode.h"


// YCPStringRep

YCPStringRep::YCPStringRep(string s)
    : v(s)
{
}

string
YCPStringRep::value() const
{
    return v;
}


YCPOrder
YCPStringRep::compare(const YCPString& s) const
{
    if (v == s->v) return YO_EQUAL;
    else return v < s->v ? YO_LESS : YO_GREATER;
}


const char *
YCPStringRep::value_cstr() const
{
    return v.c_str();
}


string
YCPStringRep::toString() const
{
    string ret = "\"";
    const char *r = v.c_str();
    while (*r) {
	if (*r == '\n') ret += "\\n";
	else if (*r == '\t') ret += "\\t";
	else if (*r == '\r') ret += "\\r";
	else if (*r == '\f') ret += "\\f";
	else if (*r == '\b') ret += "\\b";
	else if (*r < 32) {
	    char s[8];
	    snprintf (s, 8, "\\%03o", (unsigned char)*r);
	    ret += s;
	}
	else {
	    if (*r == '"' || *r == '\\') ret += "\\";
	    ret += *r;
	}
	r++;
    }
    return ret + "\"";
}


YCPValueType
YCPStringRep::valuetype() const
{
    return YT_STRING;
}


/**
 * Output value as bytecode to stream
 */

std::ostream &
YCPStringRep::toStream (std::ostream & str) const
{
    return Bytecode::writeString (str, v);
}


// --------------------------------------------------------

static string
fromStream (std::istream & str)
{
    string s;
    Bytecode::readString (str, s);
    return s;
}


YCPString::YCPString (std::istream & str)
    : YCPValue (new YCPStringRep (fromStream (str)))
{
}

