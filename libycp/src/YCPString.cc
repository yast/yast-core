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

#include "y2string.h"

#include "ycp/y2log.h"
#include "ycp/YCPString.h"
#include "ycp/Bytecode.h"


// YCPStringRep

YCPStringRep::YCPStringRep(string s)
    : v(Ustring (*SymbolEntry::_nameHash, s))
{
}

string
YCPStringRep::value() const
{
    return v.asString();
}


YCPOrder YCPStringRep::compare(const YCPString& s, bool rl) const
{
    // This function must not be locale aware per default otherwise
    // extraordinary bad things happen when changing the locale at
    // runtime.

    int tmp = 0;

    if (!rl)
    {
        tmp = v.asString().compare (s->v.asString());
    }
    else
    {
        const char* a = value_cstr ();
        const char* b = s->value_cstr ();

        std::wstring wa, wb;

        if (utf82wchar (a, &wa) && utf82wchar (b, &wb))
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
    return v.asString().c_str();
}


string
YCPStringRep::toString() const
{
    string ret = "\"";
    const char *r = v.asString().c_str();
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
    return Bytecode::writeUstring (str, v);
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

