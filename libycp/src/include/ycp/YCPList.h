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

   Author:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPList_h
#define YCPList_h


#include "YCPValue.h"




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
    vector<YCPValue> elements;

protected:
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
     * Sets a value in the list. Takes over the memory management
     * of that value. Use @ref YCPElementRep, if you need it
     * yourself.
     */
    void set(const int n, const YCPValue& value);

    /**
     * Remove a value from the list.
     */
    void remove(const int n);

    /**
     * Exchanges the elements at the indices x and y. This function
     * changes the list.
     */
    void swap(int x, int y);

    /**
     * Sorts the list. This function changes the list.
     */
    void sortlist();

    /**
     * Sorts the list (locale aware). This function changes the list.
     */
    void lsortlist();

    /**
     * Creates a copy of this list, i.e. creates a new list with
     * the same elements as this one. The elements themselves
     * are <b>not</b> copied, but only cloned!
     */
    YCPList shallowCopy() const;

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

/**
 * @short Wrapper for YCPListRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPListRep
 * with the arrow operator. See @ref YCPListRep.
 */
class YCPList : public YCPValue
{
    DEF_COMMON(List, Value);
public:
    YCPList() : YCPValue(new YCPListRep()) {}
};

#endif   // YCPList_h
