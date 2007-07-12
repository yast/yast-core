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

   Author:	Mathias Kettner <kettner@suse.de>
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
    YCPValueYCPValueMap::iterator pos = stl_map.find( key );

    if ( pos == stl_map.end() )
	stl_map.insert( YCPValueYCPValueMap::value_type( key, value ) );
    else
	pos->second = value;
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

    for (YCPMapIterator pos = begin(); pos != end(); ++pos )
    {
	newmap->add( pos.key(), pos.value() );
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

    for (YCPMapIterator pos = begin(); pos != end(); ++pos )
        newmap->add (pos.key(), pos.value());

    return newmap;
}


long
YCPMapRep::size() const
{
    return stl_map.size();
}


YCPValue
YCPMapRep::value(const YCPValue& key) const
{
    YCPMapIterator pos = stl_map.find( key );

    if ( pos != end() ) return pos.value();
    else                return YCPNull();
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
	    for( YCPMapIterator pos_this = begin(), pos_m = m->begin();
		 pos_this != end(), pos_m != m->end();
		 ++pos_this, ++pos_m )
	    {
		// compare keys
		order = pos_this.key()->compare( pos_m.key() );
		if ( order == YO_LESS || order == YO_GREATER ) return order;

		// equal keys ==> compare values
		order = pos_this.value()->compare( pos_m.value() );
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

    for(YCPMapIterator pos = begin(); pos != end(); ++pos )
    {
	if ( pos != begin() ) s += ", ";
	s += pos.key()->toString()
	     + ":"
	     + ((pos.value().isNull()) ? "(null)" : pos.value()->toString());
    }

    return s + "]";
}


YCPMapIterator
YCPMapRep::findKey(const YCPValue& key) const
{
    return stl_map.find( key );
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
    for(YCPMapIterator pos = begin(); pos != end(); ++pos )
    {
	if (!Bytecode::writeValue (str, pos.key()))
	    break;
	if (!Bytecode::writeValue (str, pos.value()))
	    break;
    }
    return str;
}


std::ostream &
YCPMapRep::toXml (std::ostream & str, int indent ) const
{
    str << "<map size=\"" << stl_map.size() << "\">";
    for(YCPMapIterator pos = begin(); pos != end(); ++pos )
    {
	str << "<element>";
	str << "<key>"; pos.key()->toXml( str, 0 ); str << "</key>";
	str << "<value>"; pos.value()->toXml( str, 0 ); str << "</value>";
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
