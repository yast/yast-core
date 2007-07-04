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

   File:       YCPExternal.cc

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#include "ycp/y2log.h"
#include "ycp/YCPExternal.h"
#include "ycp/Bytecode.h"
#include "ycp/ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

// YCPExternalRep

YCPExternalRep::YCPExternalRep(void * payload, string magic, void (*destructor)(void*,string))
    : m_payload (payload)
    , m_magic (magic)
    , m_destructor (destructor)
{
}


YCPExternalRep::~YCPExternalRep()
{
    if (m_destructor) 
	(*m_destructor)(m_payload, m_magic);
}


void *
YCPExternalRep::payload() const
{
    return m_payload;
}


string
YCPExternalRep::magic() const
{
    return m_magic;
}


YCPValueType
YCPExternalRep::valuetype() const
{
    return YT_EXTERNAL;
}


string
YCPExternalRep::toString () const
{
    return string ("<YCPExternal payload: magic '"+m_magic+"'>");
}


// bytecode
std::ostream &
YCPExternalRep::toStream (std::ostream & str) const
{
    y2error ("Trying to store an external payload in stream");
    return str;
}

std::ostream &
YCPExternalRep::toXml (std::ostream & str, int indent ) const
{
    y2error ("Trying to store an external payload in xml");
    return str;
}


// ----------------------------------------------

YCPExternal::YCPExternal (bytecodeistream &)
    : YCPValue (new YCPExternalRep (NULL, ""))
{
    y2error ("Trying to load an external payload in stream");
}
