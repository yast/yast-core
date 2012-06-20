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

   File:	YCPList.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <aschnell@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "YCP.h"
#include "ycp/y2log.h"
#include "ycp/YCPList.h"
#include <algorithm>
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"
#include "ycp/YCPCodeCompare.h"
#include "ycp/ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

// YCPListRep

YCPListRep::YCPListRep()
{
    // TODO: is this value a good choice
    // elements.reserve(32);
}


int
YCPListRep::size() const
{
    return elements.size();
}


void
YCPListRep::reserve (int size)
{
    elements.reserve (size);
}


bool
YCPListRep::isEmpty() const
{
    return elements.empty();
}


void
YCPListRep::add (const YCPValue& value)
{
    elements.push_back(value);
}


void
YCPListRep::push_back(const YCPValue& value)
{
    elements.push_back(value);
}


void
YCPListRep::set (const int i, const YCPValue& value)
{
    if (i < 0)
	return;
    while (i >= size())
	elements.push_back(YCPVoid());
    elements[i] = value;
}


void
YCPListRep::remove (const int n)
{
    if ((n < 0) || (n >= size()))
    {
        ycp2error("Invalid index %d (max %d) in %s", n, size()-1, __PRETTY_FUNCTION__);
        abort();
    }
    elements.erase (elements.begin () + n);
}


void
YCPListRep::reverse()
{
    std::reverse(elements.begin(), elements.end());
}

void
YCPListRep::swap (int x, int y)
{
    // FIXME: should produce a warning
    if ((x < 0) || (x >= size()) || (y < 0) || (y >= size()))
	return;

    std::swap (elements[x], elements[y]);
}


bool YCPListRep::contains (const YCPValue& value) const
{
    return find_if(begin(), end(), bind2nd(ycp_equal_to(), value)) != end();
}


void
YCPListRep::sortlist()
{
    std::sort(elements.begin(), elements.end(), ycp_less());
}


void
YCPListRep::lsortlist()
{
    std::sort(elements.begin(), elements.end(), ycp_less(true));
}


void
YCPListRep::fsortlist(const YCPCodeCompare& cmp)
{
    std::sort (elements.begin (), elements.end (), cmp);
}

const YCPElementRep*
YCPListRep::shallowCopy() const
{
    y2debug ("YCPListRep::shallowCopy for %s", toString().c_str() );
    YCPListRep* newlist = new YCPListRep ();
    newlist->reserve (size());
    for (int i=0; i<size(); i++)
    {
	newlist->add(value(i));
    }
    y2debug ("YCPListRep::shallowCopy result: %s", newlist->toString ().c_str () );
    return newlist;
}


YCPList
YCPListRep::functionalAdd (const YCPValue& val, bool prepend) const
{
#warning TODO: implement this better. Avoid duplicating the list.
    YCPList newlist;
    newlist->reserve (size() + 1);
    if (prepend)
    {
	newlist->add(val);
    }
    for (int i=0; i<size(); i++)
    {
       newlist->add(value(i));
    }
    if (!prepend)
    {
	newlist->add(val);
    }
    return newlist;
}


YCPValue
YCPListRep::value(int n) const
{
    if ((n < 0) || (n >= size()))
    {
	ycp2error ("invalid index %d (max %d) in %s", n, size()-1, __PRETTY_FUNCTION__);
	//abort();
	return YCPNull();
    }
    return elements[n];
}


YCPListRep::const_iterator
YCPListRep::begin() const
{
    return elements.begin();
}


YCPListRep::const_iterator
YCPListRep::end() const
{
    return elements.end();
}


YCPOrder
YCPListRep::compare(const YCPList& l) const
{
    int size_this  = size();
    int size_l     = l->size();
    int size_short = size_this < size_l ? size_this : size_l;
    int i          = 0;
    int what       = 0;
    YCPOrder order = YO_EQUAL;

    if ( size_this == 0 ) what++;
    if ( size_l    == 0 ) what++;

    switch( what )
    {
    case 0:   // none is empty
	for ( i = 0; i < size_short; i++ )
	{
	    order = value(i)->compare( l->value(i) );
	    if ( order == YO_LESS || order == YO_GREATER ) return order;
	}

	// no difference found in shorter length
	if ( size_this == size_l )     return YO_EQUAL;
	else if ( size_this < size_l ) return YO_LESS;
	else                           return YO_GREATER;
	break;

    case 1:   // one is empty
	if ( size_this == 0 ) return YO_LESS;
	else                  return YO_GREATER;
	break;

    default:   // both are empty
	return YO_EQUAL;
	break;
    }
}

string
YCPListRep::toString() const
{
    return "[" + commaList() + "]";
}


YCPValueType
YCPListRep::valuetype() const
{
    return YT_LIST;
}


string
YCPListRep::commaList() const
{
    string ret;

    for (unsigned index = 0; index < elements.size(); index++)
    {
	if (index != 0) ret += ", ";
	ret += elements[index].isNull() ? "(null)" : elements[index]->toString();
    }
    return ret;
}


/**
 * Output value as bytecode to stream
 */
std::ostream &
YCPListRep::toStream (std::ostream & str) const
{
    Bytecode::writeInt32 (str, elements.size());
    for (unsigned index = 0; index < elements.size(); index++)
    {
	if (!Bytecode::writeValue (str, elements[index]))
	{
	    y2error ("Can't write all values");
	    break;
	}
    }
    return str;
}


std::ostream &
YCPListRep::toXml (std::ostream & str, int indent ) const
{
    str << "<list size=\"" << elements.size() << "\">";
    for (unsigned index = 0; index < elements.size(); index++)
    {
	str << "<element>";
	elements[index]->toXml( str, 0 );
	str << "</element>";
    }
    return str << "</list>";
}


// --------------------------------------------------------

YCPList::YCPList(bytecodeistream & str)
    : YCPValue (YCPList())
{
    u_int32_t len = Bytecode::readInt32 (str);
    if (str.good())
    {
	(const_cast<YCPListRep*>(static_cast<const YCPListRep*>(element)))->reserve (len);
	for (unsigned index = 0; index < len; index++)
	{
	    YCPValue value = Bytecode::readValue (str);
	    (const_cast<YCPListRep*>(static_cast<const YCPListRep*>(element)))->set (index, value);
	}
    }
}

