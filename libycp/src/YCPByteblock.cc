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

   File:       YCPByteblock.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/

#include <ycp/y2log.h>
#include "YCPByteblock.h"
#include "Bytecode.h"

using std::min;


// YCPByteblockRep

YCPByteblockRep::YCPByteblockRep (const unsigned char *b, long len)
    : len (len)
{
    bytes = new unsigned char [len];
    memcpy (const_cast<unsigned char *>(bytes), b, len);
}


YCPByteblockRep::YCPByteblockRep (std::istream & str, long len)
    : len (len)
{
    bytes = new unsigned char [len];
    str.read ((char *)bytes, len);
}


YCPByteblockRep::~YCPByteblockRep()
{
    delete bytes;
}


const unsigned char *
YCPByteblockRep::value() const
{
    return bytes;
}


long
YCPByteblockRep::size() const
{
    return len;
}



YCPOrder
YCPByteblockRep::compare (const YCPByteblock& s) const
{
    // first compare sizes, than content. This is no lexical order.

    if      (len < s->len) return YO_LESS;
    else if (len > s->len) return YO_GREATER;
    else {
	for (long i=0; i<len; i++) {
	    if      (bytes[i] < s->bytes[i]) return YO_LESS;
	    else if (bytes[i] > s->bytes[i]) return YO_GREATER;
	}
	return YO_EQUAL;
    }
}


inline char
tohex(int n)
{
    if (n < 10) return n + '0';
    else return n - 10 + 'A';
}


string
YCPByteblockRep::toString() const
{
    const int bytes_per_line = 32;
    char line[bytes_per_line * 2 + 1];

    string ret = "#[";
    for (long i = 0; i < len; i += 32)
    {
	int bytes_this_line = min (32L, len-i);
	for (int j = 0; j < bytes_this_line; j++) {
	    line[j*2]   = tohex (bytes[i+j] >> 4);
	    line[j*2+1] = tohex (bytes[i+j] & 0x0f);
	}
	line[bytes_this_line * 2]=0;
	ret += line;
	if (bytes_this_line == 32) ret += "\n";
    }
    ret += "]";
    return ret;
}


YCPValueType
YCPByteblockRep::valuetype() const
{
    return YT_BYTEBLOCK;
}



/**
 * Output value as bytecode to stream
 */
std::ostream &
YCPByteblockRep::toStream (std::ostream & str) const
{
    return Bytecode::writeBytep (str, bytes, len);
}


// --------------------------------------------------------

static int
fromStream (std::istream & str)
{
    u_int32_t len = Bytecode::readInt32 (str);
    if (str.good())
    {
	return len;
    }
    return 0;
}


YCPByteblock::YCPByteblock (std::istream & str)
    : YCPValue (new YCPByteblockRep (str, fromStream (str)))
{
}
