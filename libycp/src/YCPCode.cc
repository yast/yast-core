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

   File:       YCPCode.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "YCP.h"
#include "ycp/y2log.h"
#include "ycp/YCPCode.h"

// YCPCodeRep

YCPCodeRep::YCPCodeRep()
{
    m_code = 0;
}


YCPCodeRep::YCPCodeRep(YCode *code)
{
    m_code = code;
}


YCode *
YCPCodeRep::code() const
{
    return m_code;
}


YCPOrder
YCPCodeRep::compare(const YCPCode& l) const
{
    return YO_LESS;
}

string
YCPCodeRep::toString() const
{
    if (m_code == 0)
	return "<NULL>";
    return m_code->toString();
}


YCPValueType
YCPCodeRep::valuetype() const
{
    return YT_CODE;
}

YCPValue
YCPCodeRep::evaluate (bool cse) const
{
    YCPValue retval = YCPVoid();

    y2debug ("YCPCodeRep::evaluate (%s)", this->toString().c_str());

    retval = m_code->evaluate (cse);
    return retval;
}


std::ostream &
YCPCodeRep::toStream (std::ostream & str) const
{
    return m_code->toStream (str);
}

//-------------------------------------------------------------------
// YCPEntryRep

YCPEntryRep::YCPEntryRep (SymbolEntry *entry)
{
    m_entry = entry;
}


SymbolEntry *
YCPEntryRep::entry() const
{
    return m_entry;
}


YCPOrder
YCPEntryRep::compare (const YCPEntry& l) const
{
    return YO_LESS;
}

string
YCPEntryRep::toString() const
{
    if (m_entry == 0)
	return "<NULL>";
    return m_entry->toString();
}


YCPValueType
YCPEntryRep::valuetype() const
{
    return YT_ENTRY;
}


YCPValue
YCPEntryRep::evaluate (bool cse) const
{
    y2debug ("YCPEntryRep::evaluate (%s)", this->toString().c_str());

    return m_entry->value();
}


std::ostream &
YCPEntryRep::toStream (std::ostream & str) const
{
    return m_entry->toStream (str);
}



// EOF
