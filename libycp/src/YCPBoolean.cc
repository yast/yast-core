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

#include <ycp/y2log.h>
#include "YCPBoolean.h"
#include "Bytecode.h"
#include "Xmlcode.h"

// YCPBooleanRep

YCPBooleanRep::YCPBooleanRep(bool v)
    : v(v != false)
{
}


YCPBooleanRep::YCPBooleanRep(const char *r)
    : v(!strcmp(r, "true"))
{
}

bool
YCPBooleanRep::value() const
{
    return v;
}


string
YCPBooleanRep::toString() const
{
    return v ? "true" : "false";
}


/**
 * Output value as bytecode to stream
 */
std::ostream &
YCPBooleanRep::toStream (std::ostream & str) const
{
    return Bytecode::writeBool (str, v);
}


std::ostream &
YCPBooleanRep::toXml (std::ostream & str, int /*indent*/ ) const
{
    return str << "<const type=\"bool\" value=\"" << ( v ? "true" : "false" ) << "\"/>";
}


YCPValueType
YCPBooleanRep::valuetype() const
{
    return YT_BOOLEAN;
}


YCPOrder
YCPBooleanRep::compare(const YCPBoolean& b) const
{
    if (v == b->v) return YO_EQUAL;
    else return v < b->v ? YO_LESS : YO_GREATER;
}


// --------------------------------------------------------

YCPBoolean* YCPBoolean::trueboolean = NULL;
YCPBoolean* YCPBoolean::falseboolean = NULL;

YCPBoolean::YCPBoolean (bool v)
    : YCPValue ( *(
	v ?
	trueboolean ? trueboolean : (trueboolean = new YCPBoolean (new YCPBooleanRep (true)) )
	:
	falseboolean ? falseboolean : (falseboolean = new YCPBoolean (new YCPBooleanRep (false)) )
    ))
{
}

YCPBoolean::YCPBoolean (bytecodeistream & str)
    : YCPValue (*(
	Bytecode::readBool (str) ? 
	trueboolean ? trueboolean : (trueboolean = new YCPBoolean (new YCPBooleanRep (true)) )
	:
	falseboolean ? falseboolean : (falseboolean = new YCPBoolean (new YCPBooleanRep (false)) )
    ))
{
}
