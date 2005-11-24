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

   Author:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/YCPInteger.h"
#include "ycp/Bytecode.h"
#include "ycp/ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

// YCPIntegerRep

YCPIntegerRep::YCPIntegerRep(long long v)
    : v(v)
{
}


YCPIntegerRep::YCPIntegerRep(const char *r, bool *valid)
{
    
    int converted;
    if (r && r[0] == '0')
    {
	if (r[1] == 'x')
	    converted = sscanf(r, "%Lx", &v);
	else
	    converted = sscanf(r, "%Lo", &v);
    }
    else
    {
	converted = sscanf(r, "%Ld", &v);
    }

    if (valid != NULL)			// return validity of passed value, if wanted
    {
	*valid = (converted == 1);
    }

    if (converted != 1)
    {
        ycp2warning (ee.filename().c_str(), ee.linenumber(), "Cannot convert '%s' to an integer", r);
        v = 0;
    }
}


long long
YCPIntegerRep::value() const
{
    return v;
}


YCPOrder
YCPIntegerRep::compare(const YCPInteger& i) const
{
    if (v == i->v) return YO_EQUAL;
    else return v < i->v ? YO_LESS : YO_GREATER;
}


string
YCPIntegerRep::toString() const
{
    char s[64];
    snprintf (s, 64, "%Ld", v);
    return string(s);
}


YCPValueType
YCPIntegerRep::valuetype() const
{
    return YT_INTEGER;
}


// bytecode
std::ostream &
YCPIntegerRep::toStream (std::ostream & str) const
{
    unsigned char c = sizeof (long long);
    if (!str.put (c))
	return str;
    long long ll = v;
    while (c-- > 0)				// write LSB first
    {
	if (!str.put ((unsigned char)(ll & 0xff)))
	    return str;
	ll >>= 8;
    }
    return str;
}


// ----------------------------------------------

static long long fromStream (bytecodeistream & str)
{
    char c;
    long long ll = 0LL;
    if (!str.get (c))
	return ll;

    while (c-- > 0)
    {
	char v;
	if (!str.get (v))
	    break;

	long long lc = (unsigned char)v;
	ll |= (lc << (sizeof (long long) - c - 1) * 8);
    }
    return ll;
}

YCPInteger::YCPInteger (bytecodeistream & str)
    : YCPValue (new YCPIntegerRep (fromStream (str)))
{
}
