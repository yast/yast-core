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

/-*/

#include <algorithm>

#include "y2string.h"

#include "ycp/y2log.h"
#include "ycp/YCPString.h"
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"


// YCPStringRep

YCPStringRep::YCPStringRep(const string& s)
    : v(s), is_ascii(false)
{
    is_ascii = all_of(v.begin(), v.end(), isascii);
}


YCPStringRep::YCPStringRep(const wstring& s)
    : v(), is_ascii(false)
{
    YaST::wchar2utf8(s, &v);
    is_ascii = all_of(v.begin(), v.end(), isascii);
}


bool
YCPStringRep::isEmpty() const
{
    return v.empty();
}


const string&
YCPStringRep::value() const
{
    return v;
}


wstring
YCPStringRep::wvalue() const
{
    std::wstring ret;
    YaST::utf82wchar(v, &ret);
    return ret;
}


YCPOrder YCPStringRep::compare(const YCPString& s, bool rl) const
{
    // This function must not be locale aware per default otherwise
    // extraordinary bad things happen when changing the locale at
    // runtime.

    int tmp = 0;

    if (!rl)
    {
        tmp = v.compare (s->v);
    }
    else
    {
        const char* a = value_cstr ();
        const char* b = s->value_cstr ();

        std::wstring wa, wb;

        if (YaST::utf82wchar (a, &wa) && YaST::utf82wchar (b, &wb))
            tmp = wcscoll (wa.c_str (), wb.c_str ());
        else
            tmp = strcoll (a, b);
    }

    if (tmp == 0)
        return YO_EQUAL;
    else
        return tmp < 0 ? YO_LESS : YO_GREATER;
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
	else if ((unsigned char)*r < 32) {
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


std::ostream &
YCPStringRep::toXml (std::ostream & str, int /*indent*/ ) const
{
    return str << "<const type=\"string\" value=\"" << Xmlcode::xmlify(v) << "\"/>";
}


// --------------------------------------------------------

static string
fromStream (bytecodeistream & str)
{
    string s;
    Bytecode::readString (str, s);
    return s;
}


YCPString::YCPString (bytecodeistream & str)
    : YCPValue (new YCPStringRep (fromStream (str)))
{
}
