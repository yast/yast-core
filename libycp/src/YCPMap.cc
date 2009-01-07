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

   File:       YCPMap.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <aschnell@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/YCPMap.h"
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"
#include "ycp/ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

// YCPMapRep

YCPMapRep::YCPMapRep()
{
}


YCPMapIterator
YCPMapRep::begin() const
{
    return stl_map.begin();
}


YCPMapIterator
YCPMapRep::end() const
{
    return stl_map.end();
}


void
YCPMapRep::add (const YCPValue& key, const YCPValue& value)
{
    if (!key->isString()
	&& !key->isInteger()
	&& !key->isSymbol())
    {
	ycp2error ("Only integer, string, or symbol constant allowed as key in map");
	return;
    }

    // note: 'stl_map[key] = value' would create a temporary object using the
    // default constructor for YCPValue.

    YCPMap::iterator pos = stl_map.lower_bound(key);
    if (pos == stl_map.end() || !pos->first->equal(key))
    {
	// pos is just a hint but can avoid a second search through the map
	stl_map.insert(pos, YCPMap::value_type(key, value));
    }
    else
    {
	pos->second = value;
    }
}


YCPMap
YCPMapRep::functionalAdd (const YCPValue& key, const YCPValue& value) const
{
    y2debug ("YCPMapRep::functionalAdd ('%s', '%s', '%s')", key->toString().c_str(), value->toString().c_str(), this->toString().c_str());
    if (!key->isString()
	&& !key->isInteger()
	&& !key->isSymbol())
    {
	ycp2error ("Only integer, string, or symbol constant allowed as key in map");
	return YCPNull ();
    }

    YCPMap newmap;

    for (YCPMap::const_iterator pos = begin(); pos != end(); ++pos)
    {
	newmap->add(pos->first, pos->second);
    }

    newmap->add( key, value );

    return newmap;
}


void YCPMapRep::remove(const YCPValue& key)
{
    if (!key->isString() && !key->isInteger() && !key->isSymbol())
    {
        ycp2error ("Only integer, string, or symbol constant allowed as key in map");
        return;
    }

    stl_map.erase (key);
}


const YCPElementRep* YCPMapRep::shallowCopy() const
{
    YCPMapRep* newmap = new YCPMapRep ();

    for (YCPMap::const_iterator pos = begin(); pos != end(); ++pos)
	newmap->add(pos->first, pos->second);

    return newmap;
}


bool
YCPMapRep::isEmpty() const
{
    return stl_map.empty();
}


long
YCPMapRep::size() const
{
    return stl_map.size();
}


bool
YCPMapRep::hasKey(const YCPValue& key) const
{
    return stl_map.find(key) != stl_map.end();
}


YCPValue
YCPMapRep::value(const YCPValue& key) const
{
    YCPMap::const_iterator pos = stl_map.find(key);

    if (pos != end())
	return pos->second;
    else
	return YCPNull();
}


YCPOrder
YCPMapRep::compare(const YCPMap& m) const
{
    int size_this  = size();
    int size_m     = m->size();
    YCPOrder order = YO_EQUAL;

    if ( size_this != 0 || size_m != 0 )
    {
	// any one is not empty ( maybe both ) ==> shorter is less
	if (size_this < size_m)
	{
	    return YO_LESS;
	}
	else if (size_this > size_m)
	{
	    return YO_GREATER;
	}
	else
	{
	    // equal length ==> pairwise comparison
	    for( YCPMap::const_iterator pos_this = begin(), pos_m = m->begin();
		 pos_this != end(), pos_m != m->end();
		 ++pos_this, ++pos_m )
	    {
		// compare keys
		order = pos_this->first->compare(pos_m->first);
		if ( order == YO_LESS || order == YO_GREATER ) return order;

		// equal keys ==> compare values
		order = pos_this->second->compare(pos_m->second);
		if ( order == YO_LESS || order == YO_GREATER ) return order;
	    }

	    // no difference found in equal lengths ==> equal
	    return YO_EQUAL;
	}
    }
    else return YO_EQUAL;   // both are empty
}


string
YCPMapRep::toString() const
{
    string s = "$[";

    for (YCPMap::const_iterator pos = begin(); pos != end(); ++pos)
    {
	if ( pos != begin() ) s += ", ";
	s += pos->first->toString()
	     + ":"
	     + ((pos->second.isNull()) ? "(null)" : pos->second->toString());
    }

    return s + "]";
}


YCPValueType
YCPMapRep::valuetype() const
{
    return YT_MAP;
}


/**
 * Output value as bytecode to stream
 */
std::ostream &
YCPMapRep::toStream (std::ostream & str) const
{
    Bytecode::writeInt32 (str, stl_map.size());
    for (YCPMap::const_iterator pos = begin(); pos != end(); ++pos)
    {
	if (!Bytecode::writeValue (str, pos->first))
	    break;
	if (!Bytecode::writeValue (str, pos->second))
	    break;
    }
    return str;
}


std::ostream &
YCPMapRep::toXml (std::ostream & str, int indent ) const
{
    str << "<map size=\"" << stl_map.size() << "\">";
    for (YCPMap::const_iterator pos = begin(); pos != end(); ++pos)
    {
	str << "<element>";
	str << "<key>"; pos->first->toXml( str, 0 ); str << "</key>";
	str << "<value>"; pos->second->toXml( str, 0 ); str << "</value>";
	str << "</element>";
    }
    return str << "</map>";
}


// --------------------------------------------------------

YCPMap::YCPMap(bytecodeistream & str)
    : YCPValue (YCPMap())
{
    u_int32_t len = Bytecode::readInt32 (str);
    if (str.good())
    {
	for (unsigned index=0; index < len; index++)
	{
	    YCPValue key = Bytecode::readValue (str);
	    YCPValue value = Bytecode::readValue (str);
	    (*this)->add (key, value);
	}
    }
}
