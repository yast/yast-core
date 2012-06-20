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

   File:	YCPList.h

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <aschnell@suse.de>
   Maintainer:	Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPList_h
#define YCPList_h


#include "YCPValue.h"


class YCPCodeCompare;


/**
 * @short List of YCPValues that is a value itself
 * In YCP there is no distinction between lists, tuples and structs.
 * All these kind of complex data is represented by YCPListRep. The type
 * of a list is constructed by the valuetype list, which
 * has a list of types as arguments. The list's type is implicitely
 * given through the fact that its valuetype is list and
 * trough the types of its elements. There is no restriction about
 * the types of a list's elements. If you want to declare a variable
 * or parameter to be a list of a certain signature, you can use
 * the RangeRestrictor YCP_RRList or YCP_RRTyple. object.
 */
class YCPListRep : public YCPValueRep
{
private:

    typedef vector<YCPValue> YCPValueList;

    YCPValueList elements;

protected:

    typedef YCPValueList::iterator iterator;
    typedef YCPValueList::const_iterator const_iterator;
    typedef YCPValueList::value_type value_type;
    typedef YCPValueList::const_reference const_reference;

    friend class YCPList;

    /**
     * Creates a new and empty list of type [ value ]
     */
    YCPListRep();

    /**
     * Cleans up.
     */
    ~YCPListRep() {}

public:

    /**
     * Returns the number of elements in the list.
     */
    int size() const;

    /**
     * Reserves a number of elements in the list.
     */
    void reserve (int size);

    /**
     * Returns true, if this list is empty.
     */
    bool isEmpty() const;

    /**
     * Appends a value to the list. Takes over the memory management
     * of that value. Use @ref YCPElementRep, if you need it
     * yourself.
     */
    void add(const YCPValue& value);

    /**
     * Appends a value to the list. Takes over the memory management
     * of that value. Use @ref YCPElementRep, if you need it
     * yourself.
     */
    void push_back(const YCPValue& value);

    /**
     * Sets a value in the list. Takes over the memory management
     * of that value. Use @ref YCPElementRep, if you need it
     * yourself.
     */
    void set(const int n, const YCPValue& value);

    /**
     * Remove an element from the list.
     */
    void remove(const int n);

    /**
     * Reverses the elements in the list. This function
     * changes the list.
     */
    void reverse();

    /**
     * Exchanges the elements at the indices x and y. This function
     * changes the list.
     */
    void swap(int x, int y);

    /**
     * Returns true if the list contains the value, otherwise false.
     */
    bool contains (const YCPValue& value) const;

    /**
     * Sorts the list. This function changes the list.
     */
    void sortlist();

    /**
     * Sorts the list according to the locale. This function changes the list.
     */
    void lsortlist();

    /**
     * Sorts the list according to a comparison function.
     * This function changes the list.
     */
    void fsortlist(const YCPCodeCompare& cmp);

    /**
     * Creates a copy of this list, i.e. creates a new list with
     * the same elements as this one. The elements themselves
     * are <b>not</b> copied, but only cloned!
     */
    virtual const YCPElementRep* shallowCopy() const;

    /**
     * Creates a new list, that is identical to this one with but
     * one new value appended. Doesn't change this list.
     * @param value the value to add
     * @param append determinates whether append to the end of the list
     * or prepend.
     */
    YCPList functionalAdd(const YCPValue& value, bool prepend = false) const;

    /**
     * Returns the n'th value of the list whereas 0 <= n < size().
     */
    YCPValue value(int n) const;

    /**
     * Returns a random access iterator for the YCPList that
     * is positioned at the first value in the list.
     */
    const_iterator begin() const;

    /**
     * Returns a random access iterator for the YCPList that
     * is positioned behind the last value in the list.
     */
    const_iterator end() const;

    /**
     * Compares two YCPLists for equality, greaterness or smallerness.
     * The relation is lexicographically with respect to the list elements,
     * i.e. elementwise comparison up to the shorter length.
     *
     * (( [ ] == [ ] ) == true )
     * (( [ 1, 2, 3 ] > [ 1, 2 ] ) == true )
     * (( [ 1, 2 ] > [ 1, 1, 1 ] ) == true )
     * (( [ 1, "string" ] > [ 1, 1, 1 ] ) == true )
     * (( [ 1, "string_long" ] > [ 1, "string", 1 ] ) == true )
     * (( [ 1 ] < [ { integer number = 2; return number; } ] ) == true )   #int/int
     * (( [ 1 ] > [ { integer number = 2; } ] ) == true )                  #int/nil
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     *
     */
    YCPOrder compare(const YCPList &v) const;

    /**
     * Returns an ASCII representation of the list.
     * Lists are denoted by comma separated values enclosed
     * by square brackets.
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;

    /**
     * Returns YT_LIST. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;

    /**
     * Helper function used by this class and by @ref YCPTermRep
     * that creates a comma separated string representation of
     * the members string representations.
     */
    string commaList() const;
};


#define CONST_ELEMENT (static_cast<const YCPListRep*>(element))
#define ELEMENT (const_cast<YCPListRep*>(static_cast<const YCPListRep*>(this->writeCopy())))

/**
 * @short Wrapper for YCPListRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPListRep
 * with the arrow operator. See @ref YCPListRep.
 */
class YCPList : public YCPValue
{
    DEF_COW_COMMON(List, Value);

public:

    typedef YCPListRep::iterator iterator;
    typedef YCPListRep::const_iterator const_iterator;
    typedef YCPListRep::value_type value_type;
    typedef YCPListRep::const_reference const_reference;

    YCPList() : YCPValue(new YCPListRep()) {}
    YCPList(bytecodeistream & str);

    int size() const { return CONST_ELEMENT->size (); }
    void reserve (int size) { ELEMENT->reserve (size); }
    bool isEmpty() const { return CONST_ELEMENT->isEmpty (); }
    void add(const YCPValue& value) { ELEMENT->add (value);  }
    void push_back(const YCPValue& value) { ELEMENT->push_back(value); }
    void set(const int n, const YCPValue& value) { ELEMENT->set (n, value); }
    void remove(const int n) { ELEMENT->remove (n); }
    void reverse() { ELEMENT->reverse(); }
    void swap(int x, int y) { ELEMENT->swap (x, y); }
    bool contains (const YCPValue& value) const { return CONST_ELEMENT->contains (value); }
    void sortlist() { ELEMENT->sortlist (); }
    void lsortlist() { ELEMENT->lsortlist (); }
    void fsortlist(const YCPCodeCompare& cmp) { ELEMENT->fsortlist (cmp); }

    YCPList functionalAdd(const YCPValue& value, bool prepend = false) const
	{ return CONST_ELEMENT->functionalAdd (value, prepend); }
    YCPValue value(int n) const { return CONST_ELEMENT->value (n); }
    const_iterator begin() const { return CONST_ELEMENT->begin(); }
    const_iterator end() const { return CONST_ELEMENT->end(); }
    string commaList() const { return CONST_ELEMENT->commaList (); }
};

#undef CONST_ELEMENT
#undef ELEMENT

#endif   // YCPList_h
