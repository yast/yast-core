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

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPMap data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPMap.h"
#include "YCPError.h"


// YCPMapRep

YCPMapRep::YCPMapRep()
{
}


YCPMapIterator YCPMapRep::begin() const
{
    return stl_map.begin();
}


YCPMapIterator YCPMapRep::end() const
{
    return stl_map.end();
}


void YCPMapRep::add(const YCPValue& key, const YCPValue& value)
{
    if (!key->isString() && !key->isInteger() && !key->isSymbol()) {
	y2error ("Only integer, string, or symbol constant allowed as key in map");
	return;
    }
    YCPValueYCPValueMap::iterator pos = stl_map.find( key );

    if ( pos == stl_map.end() )
	stl_map.insert( YCPValueYCPValueMap::value_type( key, value ) );
    else
	pos->second = value;
}


void YCPMapRep::remove(const YCPValue& key)
{
    if (!key->isString() && !key->isInteger() && !key->isSymbol())
    {
	y2error ("Only integer, string, or symbol constant allowed as key in map");
	return;
    }

    stl_map.erase (key);
}


YCPMap YCPMapRep::shallowCopy() const
{
    YCPMap newmap;

    for (YCPMapIterator pos = begin(); pos != end(); ++pos )
	newmap->add (pos.key(), pos.value());

    return newmap;
}


YCPMap YCPMapRep::functionalAdd(const YCPValue& key, const YCPValue& value) const
{
    if (!key->isString() && !key->isInteger() && !key->isSymbol()) {
	y2error ("Only integer, string, or symbol constant allowed as key in map");
	return YCPNull ();
    }

    YCPMap newmap;

    for (YCPMapIterator pos = begin(); pos != end(); ++pos )
	newmap->add( pos.key(), pos.value() );

    newmap->add( key, value );

    return newmap;
}


long YCPMapRep::size() const
{
    return stl_map.size();
}


YCPValue YCPMapRep::value(const YCPValue& key) const
{
    YCPMapIterator pos = stl_map.find( key );

    if ( pos != end() ) return pos.value();
    else                return YCPNull();
}


bool
YCPMapRep::haskey (const YCPValue& key) const
{
    return stl_map.find (key) != end ();
}


YCPOrder YCPMapRep::compare(const YCPMap& m) const
{
    int size_this  = size();
    int size_m     = m->size();
    YCPOrder order = YO_EQUAL;

    if ( size_this != 0 || size_m != 0 )
    {
	// any one is not empty ( maybe both ) ==> shorter is less
	if ( size_this < size_m )      return YO_LESS;
	else if ( size_this > size_m ) return YO_GREATER;
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


string YCPMapRep::toString() const
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


YCPMapIterator YCPMapRep::findKey(const YCPValue& key) const
{
    return stl_map.find( key );
}


YCPValueType YCPMapRep::valuetype() const
{
    return YT_MAP;
}


