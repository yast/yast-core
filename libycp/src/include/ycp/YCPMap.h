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

   File:       YCPMap.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPMap_h
#define YCPMap_h


#include "YCPValue.h"
#include "ycpless.h"


typedef map<YCPValue, YCPValue, ycpless> YCPValueYCPValueMap;
class YCPMapIterator;

/**
 * @short A mapping from keys to values.
 * A map is also called assiciative array. It is a mapping from a set
 * of keys to a set of values. Each key in a map is unique. Each key
 * is assigned a value. Keys and values may only be String or Integer
 * constants.
 * Elements inside a map are kept in a sorted order based on the
 * key value.
 */
class YCPMapRep : public YCPValueRep
{
    YCPValueYCPValueMap stl_map;

protected:
    friend class YCPMap;

    /**
     * Creates a new and empty mapping.
     */
    YCPMapRep();

    /**
     * Cleans up
     */
    ~YCPMapRep() {}

public:
    /**
     * Adds a new key/value pair. If the key is
     * existent, the old entry will be overwritten
     * with the new one.
     */
    void add(const YCPValue& key, const YCPValue& value);

    /**
     * Is like @ref #add, but doesn't change this map.
     * It creates a newly created map.
     */
    YCPMap functionalAdd(const YCPValue& key, const YCPValue& value) const;

    /**
     * Creates a copy of this list, i.e. creates a new list with
     * the same elements as this one. The elements themselves
     * are <b>not</b> copied, but only cloned!
     */
    YCPMap shallowCopy() const;

    /**
     * Remove a value from the map.
     */
    void remove(const YCPValue& key);

    /**
     * Returns the number of key/value pairs.
     */
    long size() const;

    /**
     * Looks for a certain key and returns the value assigned
     * to that key. Returns 0, if the key is not found.
     */
    YCPValue value(const YCPValue& key) const;

    /**
     * Returns true of false whether the map contains the key.
     */
    bool haskey (const YCPValue& key) const;

    /**
     * Returns a bidirectional STL iterator for the YCPMap that
     * is positioned at the first value pair in the map.
     * (suitable for iterating over all entries)
     */
    YCPMapIterator begin() const;

    /**
     * Returns a bidirectional STL iterator for the YCPMap that
     * is positioned BEHIND the last value pair in the map.
     * (suitable for iterating over all entries)
     */
    YCPMapIterator end() const;

    /**
     * Compares two YCPMaps for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   shorter <  longer
     *   pairwise comparison for maps of equal length not being empty
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPMap &v) const;

    /**
     * Returns an ASCII representation of the map.
     * Maps are denoted by a comma separated list of pairs
     * of the form key : value enclosed in $[ ... ]
     */
    string toString() const;

    /**
     * Returns YT_MAP. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;


private:
    /**
     * Searches the map for a given key and returns
     * an iterator that is positioned at the entry found.
     * If the key can not be found the iterator is positioned
     * at end().
     */
    YCPMapIterator findKey(const YCPValue& key) const;
};

/**
 * @short Wrapper for YCPMapRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPMapRep
 * with the arrow operator. See @ref YCPMapRep.
 */
class YCPMap : public YCPValue
{
    DEF_COMMON(Map, Value);
public:
    YCPMap() : YCPValue(new YCPMapRep()) {}
};


/**
 * @short Iterator for YCPMap values.
 */
class YCPMapIterator
{
    friend class YCPMapRep;

    YCPValueYCPValueMap::const_iterator position;

protected:
    YCPMapIterator(YCPValueYCPValueMap::const_iterator position)
	: position(position) {}

public:
    /**
     * Return the key of the current position.
     */
    YCPValue key() const { return position->first; }

    /**
     * Return the value of the current position.
     */
    YCPValue value() const { return position->second; }

    /**
     * Check for equality.
     */
    friend bool operator==(const YCPMapIterator &x, const YCPMapIterator &y) {
	return x.position == y.position;
    }

    /**
     * Check for inequality.
     */
    friend bool operator!=(const YCPMapIterator &x, const YCPMapIterator &y) {
	return !(x == y);
    }

    /**
     * Advance to the next position.
     */
    void operator++() { ++position; }
    void operator++(int) { ++position; }
};

#endif   // YCPMap_h
