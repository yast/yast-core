/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	YCPString.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
/*
 * YCPString data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include <stdio.h>

#include "y2log.h"
#include "YCPString.h"


// YCPStringRep

YCPStringRep::YCPStringRep (string s)
    : v (s)
{
}


YCPStringRep::YCPStringRep (const char* s)
    : v (s ? s : "")
{
}


string YCPStringRep::value() const
{
    return v;
}


const char* YCPStringRep::value_cstr() const
{
    return v.c_str();
}


YCPOrder YCPStringRep::compare(const YCPString& s) const
{
    const int tmp = strcoll (v.c_str (), s->v.c_str ());

    if (tmp == 0)
	return YO_EQUAL;
    else
	return tmp < 0 ? YO_LESS : YO_GREATER;
}


string YCPStringRep::toString() const
{
    string ret = "\"";
    const char *r = v.c_str();
    while (*r) {
	if (*r == '\n') ret += "\\n";
	else if (*r == '\t') ret += "\\t";
	else if (*r == '\r') ret += "\\r";
	else if (*r == '\f') ret += "\\f";
	else if (*r == '\b') ret += "\\b";
	// char from 0..31 and 128..159 as octal
	else if ((((unsigned char)(*r)) & 0x60) == 0) {
	    char s[8];
	    snprintf (s, 8, "\\%03o", (unsigned char) *r);
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


YCPValueType YCPStringRep::valuetype() const
{
    return YT_STRING;
}
