/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:       YCPCode.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

   Various wrappers for YCode elements as YCPValue

$Id$
/-*/

#include "YCP.h"
#include "ycp/y2log.h"
#include "ycp/YCPCode.h"
#include "ycp/Bytecode.h"

//---------------------------------------------------------------------------
// YCPCodeRep

YCPCodeRep::YCPCodeRep()
{
    m_code = 0;
    m_owner = false;
}


YCPCodeRep::YCPCodeRep(YCode *code, bool owner)
{
    m_code = code;
    m_owner = owner;
}


YCPCodeRep::~YCPCodeRep ()
{
    if (m_owner)
    {
	delete m_code;
    }
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

// --------------------------------------------------------

YCPCode::YCPCode (std::istream & str)
    : YCPValue (YCPCode())
{
    (const_cast<YCPCodeRep*>(static_cast<const YCPCodeRep*>(element)))->m_code = Bytecode::readCode (str);
}


//---------------------------------------------------------------------------
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
    y2debug ("YCPEntryRep::toStream()");
    return Bytecode::writeEntry (str, m_entry);
}



//---------------------------------------------------------------------------
// YCPReferenceRep


YCPReferenceRep::YCPReferenceRep (SymbolEntry *entry)
{
    m_entry = entry;
}


SymbolEntry *
YCPReferenceRep::entry() const
{
    return m_entry;
}


YCPOrder
YCPReferenceRep::compare (const YCPReference& l) const
{
    return YO_LESS;
}

string
YCPReferenceRep::toString() const
{
    if (m_entry == 0)
	return "<NULL>";
    return string ("<YCPRef:") + m_entry->toString() + ">";
}


YCPValueType
YCPReferenceRep::valuetype() const
{
    return YT_REFERENCE;
}


YCPValue
YCPReferenceRep::evaluate (bool cse) const
{
    y2debug ("YCPReferenceRep::evaluate (%s)", this->toString().c_str());

    return m_entry->value();
}


std::ostream &
YCPReferenceRep::toStream (std::ostream & str) const
{
    return m_entry->toStream (str);
}



// EOF
