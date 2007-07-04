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

   File:       YCPTerm.cc
		YCPTerm data type

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/YCPTerm.h"
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

// YCPTermRep
YCPTermRep::YCPTermRep(const string& s)
    : s(Ustring (*SymbolEntry::_nameHash, s))
{
}


YCPTermRep::YCPTermRep(const string& s, const YCPList& l)
    : s(Ustring (*SymbolEntry::_nameHash, s))
    , l(l) 
{
}


// get name of term
string
YCPTermRep::name() const
{
    return s.asString();
}


// set name of term
void
YCPTermRep::setName(string name)
{
    s = Ustring (*SymbolEntry::_nameHash, name);
}


// get list of term
YCPList
YCPTermRep::args() const
{
    return l;
}


//compare terms
YCPOrder
YCPTermRep::compare (const YCPTerm& t) const
{
    if (name() == t->name())
	return l->compare (t->args ());

    return (name() < t->name())?YO_LESS:YO_GREATER;
}


// clone term
const YCPElementRep*
YCPTermRep::shallowCopy() const
{
   return new YCPTermRep (s.asString(), l);
}


// add value to term
YCPTerm
YCPTermRep::functionalAdd (const YCPValue& val) const
{
   return YCPTerm (s.asString(), l->functionalAdd(val));
}


// get the term as string
string
YCPTermRep::toString() const
{
    return "`" + s.asString() + " (" + l->commaList() + ")";   // comma separated list
}


// test if the term's list is empty
bool
YCPTermRep::isEmpty() const
{
  return l->isEmpty();
}


// get the size of the term's list
int
YCPTermRep::size() const
{
    return l->size();
}


// Reserves a number of elements in the term's list.
void
YCPTermRep::reserve (int size)
{
    l->reserve (size);
}


// get the n-th element of the term's list
YCPValue
YCPTermRep::value (int n) const
{
    return l->value(n);
}


// set the n-th element of the term's list
void
YCPTermRep::set (const int n, const YCPValue& value)
{
    return l->set (n, value);
}


// add an element to the term's list
void
YCPTermRep::add (const YCPValue& value)
{
    l->add(value);
}


YCPValueType
YCPTermRep::valuetype() const
{
    return YT_TERM;
}


/**
 * Output value as bytecode to stream
 */

std::ostream &
YCPTermRep::toStream (std::ostream & str) const
{
    y2debug ("Writing a term");
    Bytecode::writeUstring (str, s);
    return l->toStream (str);
}

std::ostream &
YCPTermRep::toXml (std::ostream & str, int indent ) const
{
    str << "<term name=\"" << s << "\">";
    l->toXml( str, indent );
    return str << "</term>";
}


// --------------------------------------------------------

YCPTerm::YCPTerm (bytecodeistream & str)
    : YCPValue (YCPTerm("no-name-so-far"))
{
    string s;
    if (Bytecode::readString (str, s))
    {
	YCPList list (str);
	if (!list.isNull())
	{
	    (const_cast<YCPTermRep*>(static_cast<const YCPTermRep*>(element)))->setName(s);
	    for (int i = 0 ; i < list->size () ; i++)
	    {
		(const_cast<YCPTermRep*>(static_cast<const YCPTermRep*>(element)))->add (list->value (i));
	    }
	}
    }
}
