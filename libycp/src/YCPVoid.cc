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

   File:       YCPVoid.cc

   Author:	Klaus Kaempf <kkaempf@suse.com>
   Maintainer:	

/-*/

#include "ycp/y2log.h"
#include "ycp/YCPVoid.h"
#include "ycp/Bytecode.h"


// YCPVoidRep

YCPVoidRep::YCPVoidRep()
{
}


string
YCPVoidRep::toString() const
{
    return "nil";
}


YCPValueType
YCPVoidRep::valuetype() const
{
    return YT_VOID;
}


YCPOrder
YCPVoidRep::compare(const YCPVoid &) const
{
    return YO_EQUAL;
}

/**
 * Output value as bytecode to stream
 */
std::ostream &
YCPVoidRep::toStream (std::ostream & str) const
{
    return str;
}

std::ostream &
YCPVoidRep::toXml (std::ostream & str, int indent ) const
{
    return str << "<const type=\"void\"/>";
}


// --------------------------------------------------------

YCPVoid* YCPVoid::nil = NULL;

YCPVoid::YCPVoid ()
    : YCPValue( *(nil ? nil : (nil = new YCPVoid (new YCPVoidRep()) )))
{
}

YCPVoid::YCPVoid (bytecodeistream &)
    : YCPValue ( *(nil ? nil : (nil = new YCPVoid(new YCPVoidRep()) )))
{
}
