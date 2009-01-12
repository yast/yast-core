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

   File:	YCPMap.h

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <aschnell@suse.de>
   Maintainer:	Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPMap_h
#define YCPMap_h


#include "YCPValue.h"
#include "ycpless.h"


// Only for backwards compatibility. See mail from aschnell on yast-devel on
// 2009-01-07. http://lists.opensuse.org/yast-devel/2009-01/msg00016.html
typedef map<YCPValue, YCPValue, ycp_less> YCPValueYCPValueMap;
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
private:

    YCPValueYCPValueMap stl_map;

protected:

    typedef YCPValueYCPValueMap::iterator iterator;
    typedef YCPValueYCPValueMap::const_iterator const_iterator;
    typedef YCPValueYCPValueMap::value_type value_type;
    typedef YCPValueYCPValueMap::const_reference const_reference;
    typedef YCPValueYCPValueMap::key_compare key_compare;

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
    virtual const YCPElementRep* shallowCopy() const;

    /**
     * Remove a value from the map.
     */
    void remove(const YCPValue& key);

    /**
     * Returns true, iff this map is empty.
     */
    bool isEmpty() const;

    /**
     * Returns the number of key/value pairs.
     */
    long size() const;

    /**
     * Returns true iff the map contains the key.
     */
    bool hasKey(const YCPValue& key) const;

    /**
     * Looks for a certain key and returns the value assigned
     * to that key. Returns 0, if the key is not found.
     */
    YCPValue value(const YCPValue& key) const;

    /**
     * Returns a bidirectional iterator for the YCPMap that
     * is positioned at the first value pair in the map.
     */
    YCPMapIterator begin() const;

    /**
     * Returns a bidirectional iterator for the YCPMap that
     * is positioned behind the last value pair in the map.
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
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;

    /**
     * Returns YT_MAP. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};


// Only for backwards compatibility. See mail from aschnell on yast-devel on
// 2009-01-07. http://lists.opensuse.org/yast-devel/2009-01/msg00016.html
struct YCPMapIterator : public YCPValueYCPValueMap::const_iterator
{
    YCPMapIterator(YCPValueYCPValueMap::const_iterator it)
        : YCPValueYCPValueMap::const_iterator(it) {}

    YCPValue key() const __attribute__ ((deprecated)) { return (*this)->first; }
    YCPValue value() const __attribute__ ((deprecated)) { return (*this)->second; }
};


#define CONST_ELEMENT (static_cast<const YCPMapRep*>(element))
#define ELEMENT (const_cast<YCPMapRep*>(static_cast<const YCPMapRep*>(this->writeCopy())))

/**
 * @short Wrapper for YCPMapRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPMapRep
 * with the arrow operator. See @ref YCPMapRep.
 */
class YCPMap : public YCPValue
{
    DEF_COW_COMMON(Map, Value);

public:

    typedef YCPMapRep::iterator iterator;
    typedef YCPMapRep::const_iterator const_iterator;
    typedef YCPMapRep::value_type value_type;
    typedef YCPMapRep::const_reference const_reference;
    typedef YCPMapRep::key_compare key_compare;

    YCPMap() : YCPValue(new YCPMapRep()) {}
    YCPMap(bytecodeistream & str);

    void add(const YCPValue& key, const YCPValue& value) { ELEMENT->add (key,value); }
    YCPMap functionalAdd(const YCPValue& key, const YCPValue& value) const { return CONST_ELEMENT-> functionalAdd (key,value); }
    void remove(const YCPValue& key) { ELEMENT-> remove (key); }
    bool isEmpty() const { return CONST_ELEMENT->isEmpty(); }
    long size() const { return CONST_ELEMENT-> size (); }
    bool hasKey(const YCPValue& key) const { return CONST_ELEMENT->hasKey(key); }
    YCPValue value(const YCPValue& key) const { return CONST_ELEMENT-> value (key); }
    YCPMapIterator begin() const { return CONST_ELEMENT-> begin (); }
    YCPMapIterator end() const { return CONST_ELEMENT-> end (); }
};

#undef ELEMENT
#undef CONST_ELEMENT

#endif   // YCPMap_h
