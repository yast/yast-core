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

   File:       YCPList.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "YCP.h"
#include "ycp/y2log.h"
#include "ycp/YCPList.h"
#include <algorithm>
#include "ycp/Bytecode.h"

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
    if ((n < 0) || (n >= size())) {
        y2error("Invalid index %d (max %d) in %s", n, size()-1, __PRETTY_FUNCTION__);
        abort();
    }
    elements.erase (elements.begin () + n);
}


void
YCPListRep::swap (int x, int y)
{
    // FIXME: should produce a warning
    if ((x < 0) || (x >= size()) || (y < 0) || (y >= size()))
	return;

    std::swap (elements[x], elements[y]);
}


static bool compareYCP (const YCPValue& y1, const YCPValue& y2)
{
    return (y1->compare(y2)) == YO_LESS;
}


void
YCPListRep::sortlist()
{
    std::sort (elements.begin (), elements.end (), compareYCP);
}


YCPList
YCPListRep::shallowCopy() const
{
    YCPList newlist;
    newlist->reserve (size());
    for (int i=0; i<size(); i++)
	newlist->add(value(i));
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
	y2error ("invalid index %d (max %d) in %s", n, size()-1, __PRETTY_FUNCTION__);
	//abort();
	return YCPNull();
    }
    return elements[n];
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
	ret += elements[index].isNull()?"(null)":elements[index]->toString();
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
	    break;
    }
    return str;
}


// --------------------------------------------------------

YCPList::YCPList(std::istream & str)
    : YCPValue (YCPList())
{
    u_int32_t len = Bytecode::readInt32 (str);
    if (str.good())
    {
	(*this)->reserve (len);
	for (unsigned index = 0; index < len; index++)
	{
	    YCPValue value = Bytecode::readValue (str);
	    (*this)->set (index, value);
	}
    }
}
