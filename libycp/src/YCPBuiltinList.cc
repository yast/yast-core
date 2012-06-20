/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						     (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	YCPBuiltinList.cc
   Summary:     YCP List Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include <set>			// for toset
#include <algorithm>		// sort

using std::set;

#include "ycp/YCPBuiltinList.h"
#include "ycp/YCPList.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPString.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPCodeCompare.h"
#include "ycp/YCPTerm.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
l_find (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin find
     * @id find-list
     * @short Searches for the first occurence of a certain element in a list
     * @param any VAR
     * @param list LIST
     * @param block EXPR
     *
     * @description
     * Searches for a certain item in the list. It applies the expression
     * <tt>EXPR</tt> to each element in the list and returns the first element
     * the makes the expression evaluate to true, if <tt>VAR</tt> is bound to
     * that element.
     *
     * @return any Returns nil, if nothing is found.
     * @usage find (integer n, [3,5,6,4], ``(n >= 5)) -> 5
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPValue ret = YCPNull ();

    SymbolEntryPtr s = symbol->asEntry()->entry();

    for (YCPList::const_iterator it = list->begin(); it != list->end(); ++it)
    {
	s->setValue(*it);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    ycp2error ("Bad find expression %s", expr->toString ().c_str ());
	    break;
	}
	// nil == false
	if (v->isVoid ())
	{
	    ycp2error ("The expression for 'find' returned 'nil'");
	    continue;
	}

	if (v->asBoolean ()->value ())
	{
	    ret = *it;
	    break;
	}
    }

    return ret;
}


static YCPValue
l_prepend (const YCPList &list, const YCPValue &value)
{
    /**
     * @builtin prepend
     * @short Prepends a list with a new element
     * @param list LIST List
     * @param any ELEMENT Element to prepend
     * @return list
     * @description
     * Creates a new list that is identical to the list <tt>LIST</tt> but has
     * the value <tt>ELEMENT</tt> prepended as additional element.
     *
     * @usage prepend ([1, 4], 8) -> [8, 1, 4]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    return list->functionalAdd (value, true);
}


static YCPValue
l_contains (const YCPList &list, const YCPValue &value)
{
    /**
     * @builtin contains
     * @short Checks if a list contains an element
     * @param list LIST List
     * @param any ELEMENT Element
     * @return boolean True if element is in the list.
     * @description
     *
     * Determines, if a certain value <tt>ELEMENT</tt> is contained in
     * a list <tt>LIST</tt>.
     *
     * @usage contains ([1, 2, 5], 2) -> true
     */

    return YCPBoolean(list->contains(value));
}


static YCPValue
l_setcontains (const YCPList &list, const YCPValue &value)
{
    /**
     * @builtin setcontains
     * @short Checks if a sorted list contains an element
     *
     * @param list LIST List
     * @param any ELEMENT Element
     * @return boolean True if element is in the list.
     * @description
     * Determines, if a certain value <tt>ELEMENT</tt> is contained in
     * a list <tt>LIST</tt>, but assumes that <tt>LIST</tt> is sorted. If <tt>LIST</tt> is
     * not sorted, the result is undefined.
     *
     * @usage setcontains ([1, 2, 5], 2) -> true
     */

    return YCPBoolean(binary_search(list->begin(), list->end(), value, ycp_less()));
}


static YCPValue
l_unionlist (const YCPList &list1, const YCPList &list2)
{
    /**
     * @builtin union
     * @id union-list
     * @short Unions of lists
     * @param list LIST1 First List
     * @param list LIST2 Second List
     * @return list
     *
     * @description
     * Interprets two lists as sets and returns a new list that has
     * all elements of the first list and all of the second list. Identical
     * elements are merged. The order of the elements in the new list is
     * preserved. Elements of <tt>l1</tt> are prior to elements from <tt>l2</tt>.
     * <tt>nil</tt> as either argument makes the result <tt>nil</tt> too.
     *
     * WARNING: quadratic complexity so far
     *
     * @see merge
     * @usage union ([1, 2], [3, 4]) -> [1, 2, 3, 4]
     * @usage union ([1, 2, 3], [2, 3, 4]) -> [1, 2, 3, 4]
     * @usage union ([1, 3, 5], [1, 2, 4, 6]) -> [1, 3, 5, 2, 4, 6]
     */

    if (list1.isNull () || list2.isNull ())
    {
	return YCPNull ();
    }

    YCPList newlist;

    for (int l = 0; l < 2; l++)
    {
	YCPList list = (l == 0 ? list1 : list2);

	for (int e = 0; e < list->size (); e++)
	{
	    YCPValue to_insert = list->value (e);

	    // Already contained? I know, this has an _awful_ complexity.
	    // We need to introduce an order on YCPValueRep to solve the problem.

	    if (!newlist->contains(to_insert))
		newlist->add (to_insert);
	}
    }

    return newlist;
}


static YCPValue
l_mergelist (const YCPList &list1, const YCPList &list2)
{
    /**
     * @builtin merge
     * @short Merges two lists into one
     * @param list LIST1 First List
     * @param list LIST2 Second List
     * @return list
     *
     * @description
     * Interprets two lists as sets and returns a new list that has
     * all elements of the first list and all of the second list. Identical
     * elements are preserved. The order of the elements in the new list is
     * preserved. Elements of <tt>l1</tt> are prior to elements from <tt>l2</tt>.
     * <tt>nil</tt> as either argument makes the result <tt>nil</tt> too.
     *
     * @see union
     * @usage merge ([1, 2], [3, 4]) -> [1, 2, 3, 4]
     * @usage merge ([1, 2, 3], [2, 3, 4]) -> [1, 2, 3, 2, 3, 4]
     */

    if (list1.isNull () || list2.isNull ())
    {
	return YCPNull ();
    }

    YCPList newlist;

    for (int l = 0; l < 2; l++)
    {
	YCPList list = (l == 0 ? list1 : list2);

	for (int e = 0; e < list->size (); e++)
	{
	    newlist->add (list->value (e));
	}
    }

    return newlist;
}


static YCPValue
l_sublist1(const YCPList &list, const YCPInteger &offset)
{
   /**
     * @builtin sublist
     * @id sublist-rest
     * @short Extracts a sublist
     *
     * @description
     * Extracts a sublist of the list <tt>LIST</tt> starting at
     * <tt>OFFSET</tt>. The <tt>OFFSET</tt> starts with 0.
     *
     * @param list LIST
     * @param integer OFFSET
     * @return list
     *
     * @usage sublist ([ "a", "b", "c"], 0) -> [ "a", "b", "c" ]
     * @usage sublist ([ "a", "b", "c"], 2) -> [ "c" ]
     */

    if (list.isNull () || offset.isNull())
	return YCPNull ();

    int i1 = offset->value();
    int i2 = list->size();

    if (i1 < 0 || i1 >= list->size ())
    {
	ycp2error ("Offset %s for sublist () out of range", toString (i1).c_str ());
	return YCPNull ();
    }

    YCPList newlist;

    for (int i = i1; i < i2; i++)
    {
	newlist->add (list->value(i));
    }

    return newlist;
}


static YCPValue
l_sublist2(const YCPList &list, const YCPInteger &offset, const YCPInteger &length)
{
    /**
     * @builtin sublist
     * @id sublist-length
     * @short Extracts a sublist
     *
     * @description
     * Extracts a sublist of the list <tt>LIST</tt> starting at
     * <tt>OFFSET</tt> with length <tt>LENGTH</tt>. The <tt>OFFSET</tt>
     * starts with 0.
     *
     * @param list LIST
     * @param integer OFFSET
     * @param integer LENGTH
     * @return list
     *
     * @usage sublist ([ "a", "b", "c"], 0, 2) -> [ "a", "b" ]
     * @usage sublist ([ "a", "b", "c"], 1, 1) -> [ "b" ]
     */

    if (list.isNull () || offset.isNull() || length.isNull ())
	return YCPNull ();

    int i1 = offset->value();
    int i2 = i1 + length->value();

    if (i1 < 0 || i1 >= list->size ())
    {
	ycp2error ("Offset %s for sublist () out of range", toString (i1).c_str ());
	return YCPNull ();
    }

    if (i2 < i1 || i2 > list->size ())
    {
	ycp2error ("Length %s for sublist () out of range", toString (i2).c_str ());
	return YCPNull ();
    }

    YCPList newlist;

    for (int i = i1; i < i2; i++)
    {
	newlist->add (list->value(i));
    }

    return newlist;
}


static YCPValue
l_filter (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin filter
     * @id filter-list
     * @short Filters a List
     * @param any VAR Variable
     * @param list LIST List to be filtered
     * @param block<boolean> EXPR Block
     * @return list
     * @description
     * For each element of the list <tt>LIST</tt> the expression <tt>expr</tt>
     * is executed in a new block, where the variable <tt>VAR</tt>
     * is assigned to that value. If the expression evaluates to true under
     * this circumstances, the value is appended to the result list.
     *
     * @usage filter (integer v, [1, 2, 3, 5], { return (v > 2); }) -> [3, 5]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret;

    SymbolEntryPtr s = symbol->asEntry()->entry();

    for (YCPList::const_iterator it = list->begin(); it != list->end(); ++it)
    {
	s->setValue(*it);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    ycp2error ("Bad filter expression %s", expr->toString ().c_str ());
	    return YCPNull ();
	}
	// nil == false
	if (v->isVoid ())
	{
	    ycp2error ("The expression for 'filter' returned 'nil'");
	    continue;
	}
	if (v->isBreak())
	{
	    break;
	}
	if (v->asBoolean ()->value ())
	{
	    ret->add(*it);
	}
    }

    return ret;
}


static YCPValue
l_maplist (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin maplist
     * @id maplist-list
     * @short Maps an operation onto all elements of a list and thus creates a new list.
     * @param any VAR
     * @param list<any> LIST
     * @param block EXPR
     * @return list<any>
     *
     * @description
     * For each element of the list <tt>LIST</tt> the expression <tt>EXPR</tt>
     * is evaluated in a new block, where the variable <tt>VAR</tt>
     * is assigned to that value. The result is the list of those
     * evaluations.
     *
     * @usage maplist (integer v, [1, 2, 3, 5], { return (v + 1); }) -> [2, 3, 4, 6]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret;
    SymbolEntryPtr s = symbol->asEntry()->entry();

    for (YCPList::const_iterator it = list->begin(); it != list->end(); ++it)
    {
	s->setValue(*it);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    ycp2error ("Bad maplist expression %s", expr->toString ().c_str ());
	    return YCPNull ();
	}
	if (v->isBreak())
	{
	    break;
	}
	ret->add (v);
    }

    return ret;
}


static YCPValue
l_listmap (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin listmap
     * @short Maps an operation onto all elements of a list and thus creates a map.
     * @param any VAR
     * @param list LIST
     * @param block EXPR
     * @return map
     *
     * @description
     * For each element <tt>VAR</tt> of the list <tt>LIST</tt> in the expression
     * <tt>EXPR</tt> is evaluated in a new block. The result is the map of
     * those evaluations.
     *
     * The result of each evaluation <i>must</i> be
     * a map with a single entry which will be added to the result map.
     *
     * @usage listmap (integer k, [1,2,3], { return $[k:"xy"]; })  -> $[1:"xy", 2:"xy"]
     * @usage listmap (integer k, [1,2,3], { integer a = k+10;  any b = sformat ("x%1", k); return $[a:b]; }) -> $[11:"x1", 12:"x2", 13:"x3"]
     */


    if (list.isNull ())
    {
	return YCPNull ();
    }

    SymbolEntryPtr key = symbol->asEntry()->entry();

    YCPMap ret;
    YCPList curr_list;
    YCPMap curr_map;

    for (YCPList::const_iterator it = list->begin(); it != list->end(); ++it)
    {
	key->setValue(*it);

	YCPValue curr_value = expr->evaluate ();

	if (curr_value.isNull ())
	{
	    ycp2error ("Bad listmap expression %s", expr->toString ().c_str ());
	    return YCPNull ();
	}
	else if (curr_value->isBreak())
	{
	    break;
	}
	else if (! curr_value ->isMap () )
	{
            ycp2error("listmap() expression has to deliver a single entry map! You have produced the following value: %s",
                curr_value->toString().c_str());
	    return YCPNull ();
	}
        else
        {
            curr_map = curr_value->asMap();
            if ( curr_map->size() == 1 )
            {
		YCPMap::const_iterator it = curr_map->begin();
		ret->add(it->first, it->second);
            }
            else
            {
                ycp2error("listmap() expression has to deliver a single entry map! You have produced the following value: %s",
                    curr_map->toString().c_str());
	        return YCPNull ();
            }
        }

    }

    return ret;
}


static YCPValue
l_flatten (const YCPList &list)
{
    /**
     * @builtin flatten
     * @short Flattens List
     * @param list<list> LIST
     * @return list
     *
     * @description
     * Gets a list  of lists <tt>LIST</tt> and creates a single list that is
     * the concatenation of those lists in <tt>LIST</tt>.
     *
     * @usage flatten ([ [1, 2], [3, 4] ]) -> [1, 2, 3, 4]
     * @usage flatten ([ [1, 2], [6, 7], [3, 4] ]) -> [1, 2, 6, 7, 3, 4]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret;

    for (int i = 0; i < list->size (); i++)
    {
	if (!list->value (i)->isList ())
	{
	    ycp2error("%s is not a list. Flatten expects a list of lists", list->value (i)->toString ().c_str ());
	    return YCPNull ();
	}

	YCPList sublist = list->value (i)->asList ();
	for (int j = 0; j < sublist->size (); j++)
	{
	    ret->add (sublist->value (j));
	}
    }

    return ret;
}


static YCPValue
l_toset (const YCPList &list)
{
    /**
     * @builtin toset
     * @short Sorts list and removes duplicates
     * @param list LIST
     * @return list Sorted list with unique items
     * @description
     * Scans a list for duplicates, removes them and sorts the list.
     *
     * @usage toset ([1, 5, 3, 2, 3, true, false, true]) -> [false, true, 1, 2, 3, 5]
     */

    set<YCPValue, ycp_less> newset(list->begin(), list->end());

    YCPList setlist;
    for (set <YCPValue, ycp_less>::const_iterator it = newset.begin ();
	 it != newset.end (); ++it)
    {
	setlist->add (*it);
    }
    return setlist;
}


static YCPValue
l_sortlist (const YCPList &list)
{
    /**
     * @builtin sort
     * @id sort-lt
     * @short Sorts a List according to the YCP builtin predicate
     * @param list LIST
     * @return list Sorted list
     *
     * @description
     * Sorts the list LIST according to the YCP builtin predicate.
     * Duplicates are not removed.
     *
     * @usage sort ([2, 1, true, 1]) -> [true, 1, 1, 2]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret = list;
    ret->sortlist ();
    return ret;
}

static YCPValue
l_sort (const YCPValue &sym1, const YCPValue &sym2,
	 const YCPList &list, const YCPCode &order)
{
    /**
     * @builtin sort
     * @id sort-expr
     * @short Sort list using an expression
     * @param any x
     * @param any y
     * @param list LIST
     * @param block EXPR
     * @return list
     * @description
     *
     * Sorts the list <tt>LIST</tt>. You have to specify an order on the
     * list elements by naming formal variables <tt>x</tt> and <tt>y</tt> and
     * specify an expression <tt>EXPR</tt> that evaluates to a boolean
     * value depending on <tt>x</tt> and <tt>y</tt>.
     * Return true if <tt>x</tt>><tt>y</tt> to
     * sort the list ascending.
     *
     * The comparison must be an irreflexive one,
     * that is ">" instead of ">=".
     *
     * It is because we no longer use bubblesort (yuck) but <b>std::sort</b>
     * which requires a
     * <ulink url="href="http://www.sgi.com/tech/stl/StrictWeakOrdering.html">strict
     * weak ordering</ulink>.
     *
     * @usage sort (integer x, integer y, [ 3,6,2,8 ], ``(x < y)) -> [ 2, 3, 6, 8 ]
     * @usage sort (string x, string y, [ "A","C","B" ], ``(x > y)) -> ["C", "B", "A"]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList result = list;
    result->fsortlist (YCPCodeCompare (sym1, sym2, order));

    return result;
}


static YCPValue
l_lsortlist (const YCPList &list)
{
    /**
     * @builtin lsort
     * @short Sort A List respecting locale
     * @param list LIST
     * @return list Sorted list
     * @description
     * Sort the list LIST according to the YCP builtin predicate
     * <b>></b>.
     * Strings are compared using the current locale.
     * Duplicates are not removed.
     *
     * @usage lsort (["česky", "slovensky", "německy", 2, 1]) -> [1,
     * 2, "česky", "německy", "slovensky"]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret = list;
    ret->lsortlist ();
    return ret;
}


static YCPValue
l_splitstring (const YCPString &s, const YCPString &c)
{
    /**
     * @builtin splitstring
     * @short Split a string by delimiter
     * @param string STR
     * @param string DELIM
     * @return list<string>
     *
     * @description
     * Splits <tt>STR</tt> into sub-strings at delimiter chars <tt>DELIM</tt>.
     * the resulting pieces do not contain <tt>DELIM</tt>
     *
     * If <tt>STR</tt> starts with <tt>DELIM</tt>, the first string in the result list is empty
     * If <tt>STR</tt> ends with <tt>DELIM</tt>, the last string in the result list is empty.
     * If <tt>STR</tt> does not contain <tt>DELIM</tt>, the result is a singleton list with <tt>STR</tt>.
     *
     * @see mergestring
     * @usage splitstring ("/abc/dev/ghi", "/") -> ["", "abc", "dev", "ghi" ]
     * @usage splitstring ("abc/dev/ghi/", "/") -> ["abc", "dev", "ghi", "" ]
     * @usage splitstring ("abc/dev/ghi/", ".") -> ["abc/dev/ghi/" ]
     * @usage splitstring ("text/with:different/separators", "/:") -> ["text", "with", "different", "separators"]
     */

    if (s.isNull ())
    {
	return YCPNull ();
    }

    if (c.isNull ())
    {
	ycp2error ("Cannot split string using 'nil'");
	return YCPNull ();
    }

    YCPList ret;

    string ss = s->value ();
    string sc = c->value ();

    if (ss.empty () || sc.empty ())
	return ret;

    string::size_type spos = 0;			// start pos
    string::size_type epos = 0;			// end pos

    while (true)
    {
	epos = ss.find_first_of (sc, spos);

	if (epos == string::npos)	// break if not found
	{
	    ret->add (YCPString (string (ss, spos)));
	    break;
	}

	if (spos == epos)
	    ret->add (YCPString (""));
	else
	    ret->add (YCPString (string (ss, spos, epos - spos)));	// string piece w/o delimiter

	spos = epos + 1;	// skip c in s

	if (spos == ss.size ())	// c was last char
	{
	    ret->add (YCPString (""));	// add "" and break
	    break;
	}
    }

    return ret;
}


static YCPValue
l_changelist (YCPList &list, const YCPValue &value)
{
    /**
     * @builtin change
     * @id change-list
     * @short Changes a list. Deprecated, use LIST[size(LIST)] = value.
     * @param list LIST
     * @param any value
     * @return list
     *
     * @description
     * Before Code 9, this was used to change a list directly
     * without creating a copy. Now it is a synonym for add.
     *
     * @see add
     * @usage change ([1, 4], 8) -> [1, 4, 8]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    ycpinternal ("Change does not work as expected! The argument is not passed by reference.");

    list->add (value);
    return list;
}


static YCPValue
l_add (const YCPValue &list, const YCPValue &value)
{
    /**
     * @builtin add
     * @id add-list
     * @short Create a new list with a new element
     * @param list LIST
     * @param any VAR
     * @return list The new list
     * @description
     * Creates a new list that is identical to the list <tt>LIST</tt> but has
     * the value <tt>VAR</tt> appended as additional element.
     *
     * @usage add ([1, 4], 8) -> [1, 4, 8]
     */

    if (list.isNull () || !list->isList())
    {
	ycp2error ("Adding to a nil list");
	return YCPNull ();
    }

    return list->asList()->functionalAdd (value);
}


static YCPValue
l_isempty(const YCPList& l)
{
    /**
     * @builtin isempty
     * @id isempty-list
     * @short Returns whether the list <tt>l</tt> is empty.
     * @param list l List
     * @return boolean Emptiness of list <tt>l</tt>
     *
     * @description
     * Notice that the list <tt>l</tt> must not be nil.
     *
     * @usage isempty([]) -> true
     * @usage isempty([19, 29]) -> false
     */

    return YCPBoolean(l->isEmpty());
}


// parameter is YCPValue because we accept 'nil'
static YCPValue
l_size (const YCPValue &list)
{
    /**
     * @builtin size
     * @id size-list
     * @short Returns size of list
     * @param list LIST
     * @return integer size of the list
     *
     * @description
     * Returns the number of elements of the list <tt>LIST</tt>
     *
     * @usage size(["A", 1, true, "3", false]) -> 5
     */

    if (list.isNull ()
	|| !list->isList ())
    {
	return YCPInteger (0LL);
    }
    return YCPInteger (list->asList()->size ());
}


static YCPValue
l_remove (const YCPList &list, const YCPInteger &i)
{
    /**
     * @builtin remove
     * @id remove-list
     * @short Removes element from a list
     * @param list LIST
     * @param integer e element index
     * @return list Returns unchanged list if the index is invalid.
     * @description
     * Removes the <tt>i</tt>'th value from a list. The first value has the
     * index 0. The call remove ([1,2,3], 1) thus returns [1,3].
     *
     * The yast2-core version < 2.17.16 returns nil if the key is invalid. This behavior
     * has changed in version 2.17.16 to return unchanged list.
     *
     * @usage remove ([1, 2], 0) -> [2]
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    if (i.isNull ())
    {
	ycp2error ("Cannot remove item at index 'nil'");
	return YCPNull ();
    }

    long idx = i->value ();

    YCPList ret = list;

    if (idx < 0 || idx >= ret->size ())
    {
	ycp2error ("Index %s for remove () out of range", toString (idx).c_str ());
	return ret;
    }

    ret->remove (idx);
    return ret;
}


// parameter is YCPValue because we accept 'nil'
static YCPValue
l_select (const YCPValue &list, const YCPValue &i, const YCPValue &def)
{
    /**
     * @builtin select
     * @id select-list
     * @short Selects a list element (deprecated, use LIST[INDEX]:DEFAULT)
     * @param list LIST
     * @param integer INDEX
     * @param any  DEFAULT
     * @return any
     *
     * @description
     * Gets the <tt>INDEX</tt>'th value of a list. The first value has the
     * index 0. The call select([1,2,3], 1) thus returns 2. Returns <tt>DEFAULT</tt>
     * if the index is invalid or if the found entry has a different type
     * than the default value.
     * Functionality replaced by syntax: <code>list numbers = [1, 2, 3, 4];
     * numbers[2]:nil -> 3
     * numbers[8]:5   -> 5</code>
     *
     * @usage select ([1, 2], 22, 0) -> 0
     * @usage select ([1, "two"], 0, "no") -> "no"
     */

    if (list.isNull()
	|| !list->isList()
	|| i.isNull()
	|| !i->isInteger())
    {
	return def;
    }
    long idx = i->asInteger()->value ();
    if (idx < 0 || idx >= list->asList()->size ())
    {
	return def;
    }

    // FIXME: runtime type check, because of the term variant of select
    // ensure, that it is really a list
    YCPValue tmp = list;
    if ( ! tmp->isList ())
    {
	// for term, call the other builtin
	if ( tmp->isTerm ())
	{
	    extern YCPValue t_select (const YCPValue &list, const YCPValue &i, const YCPValue &def);
	    return t_select (tmp->asTerm (), i->asInteger(), def);
	}
	ycp2error ("Incorrect builtin called, %s is not a list", tmp->toString ().c_str ());
	return def;
    }

    YCPValue v = list->asList()->value (idx);

    return v;
}


static YCPValue
l_foreach (const YCPValue &sym, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin foreach
     * @id foreach-list
     * @short Processes the content of a list
     * @param any VAR
     * @param list LIST
     * @param block EXPR
     *
     * @description
     * For each element of the list <tt>LIST</tt> the expression <tt>EXPR</tt>
     * is executed in a new context, where the variable <tt>VAR</tt> is
     * assigned to that value. The return value of the last execution of
     * <tt>EXPR</tt> is the value of the <tt>foreach</tt> construct.
     *
     * @return any return value of last execution of EXPR
     * @usage foreach (integer v, [1,2,3], { return v + 10; }) -> 13
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    SymbolEntryPtr s = sym->asEntry()->entry();
    YCPValue ret = YCPVoid();

    for (YCPList::const_iterator it = list->begin(); it != list->end(); ++it)
    {
	s->setValue(*it);

	ret = expr->evaluate ();
	if (ret.isNull())
	{
	    ycp2error ("Bad foreach expression %s", expr->toString ().c_str ());
	    continue;
	}
	else if (ret->isBreak())
	{
	    ret = YCPVoid();
	    break;
	}
    }
    return ret;
}


static YCPValue
l_reduce_helper(const YCPSymbol &x, const YCPSymbol &y, const YCPValue &initial,
		const YCPList::const_iterator &first, const YCPList::const_iterator &last,
		const YCPCode &expr)
{
    SymbolEntryPtr xs = x->asEntry()->entry();
    SymbolEntryPtr ys = y->asEntry()->entry();

    YCPValue ret = initial;

    for (YCPList::const_iterator it = first; it != last; ++it)
    {
	xs->setValue(ret);
	ys->setValue(*it);

	YCPValue tmp = expr->evaluate();
	if (tmp.isNull())
	{
	    ycp2error("Bad 'reduce' expression %s", expr->toString().c_str());
	    continue;
	}
	if (tmp->isVoid())
	{
	    ycp2error("The expression for 'reduce' returned 'nil'");
	    continue;
	}
	if (tmp->isBreak())
	{
	    break;
	}

	ret = tmp;
    }

    return ret;
}


static YCPValue
l_reduce1 (const YCPSymbol &x, const YCPSymbol &y, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin list::reduce
     * @id list.reduce-internal
     * @short Reduces a list to a single value.
     * @param flex1 x
     * @param flex1 y
     * @param list<flex1> list
     * @param block<flex1> expression
     * @return flex1
     *
     * @description
     * Apply expression cumulatively to the values of the list, from left to
     * right, to reduce the list to a single value. See
     * http://en.wikipedia.org/wiki/Reduce_(higher-order_function) for a
     * detailed explanation.
     *
     * In this version the initial value is the first value of the list. Thus
     * the list must not be empty.
     *
     * @usage list::reduce (integer x, integer y, [2, 4, 6], { return x < y ? x : y; }) -> 2
     * @usage list::reduce (integer x, integer y, [2, 4, 6], { return x > y ? x : y; }) -> 6
     */

    if (list->isEmpty())
    {
 	ycp2error("Empty list for 'reduce'");
	return YCPNull();
    }

    const YCPList::const_iterator initial = list->begin();
    const YCPList::const_iterator first = initial + 1;
    const YCPList::const_iterator last = list->end();

    return l_reduce_helper(x, y, *initial, first, last, expr);
}


static YCPValue
l_reduce2 (const YCPSymbol &x, const YCPSymbol &y, const YCPValue &initial, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin list::reduce
     * @id list.reduce-external
     * @short Reduces a list to a single value.
     * @param flex1 x
     * @param flex2 y
     * @param flex1 value
     * @param list<flex2> list
     * @param block<flex1> expression
     * @return flex1
     *
     * @description
     * Apply expression cumulatively to the values of the list, from left to
     * right, to reduce the list to a single value. See
     * http://en.wikipedia.org/wiki/Reduce_(higher-order_function) for a
     * detailed explanation.
     *
     * In this version the initial value is explicitly provided. Thus the list
     * can be empty. Also the return type can be different from the type of
     * the list.
     *
     * @usage list::reduce (integer x, integer y, 0, [2, 4, 6], { return x + y; }) -> 12
     * @usage list::reduce (integer x, integer y, 1, [2, 4, 6], { return x * y; }) -> 48
     *
     * @usage list::reduce (term t, float f, `item(`id(`dummy)), [3.14, 2.71], { return add(t, tostring(f)); }) -> `item (`id (`dummy), "3.14", "2.71")
     */

    const YCPList::const_iterator first = list->begin();
    const YCPList::const_iterator last = list->end();

    return l_reduce_helper(x, y, initial, first, last, expr);
}

static YCPValue
l_swaplist (const YCPList &v, YCPInteger &i1, YCPInteger &i2){
    /**
     * @builtin list::swap
     * @id list.swap
     * @short Creates new list with swaped elements at offset i1 and i2.
     * @param list<flex1> v list
     * @param integer i1 index of first element
     * @param integer i2 index of second element
     * @return New list. Changed if offset is correct, otherwise return unchanged list 
     *
     * @description
     * Creates new list with swaped elemetns at offset i1 and i2. Return nil if list or
     * any offset is nil. Return unchanged list if any offset is out of bounds.
     *
     * @usage list::swap ([0,1,2,3],0,2) -> [2,1,0,3]
     * @usage list::swap ([0,1,2,3],1,3) -> [0,3,2,1]
     * @usage list::swap ([0,1,2,3],0,4) -> [0,1,2,3]
     */
    if (v.isNull ())
    {
	ycp2error ("Cannot swap 'nil' list");
	return YCPNull ();
    }

    if (i1.isNull () || i2.isNull())
    {
	ycp2error ("Cannot swap item at index 'nil'");
	return YCPNull ();
    }

    int it1 = i1->value();
    int it2 = i2->value();
    int siz = v->size();
    YCPList ret = v;
    if (it1< 0 || it1>= siz)
    {
 	ycp2error("Index 'i1' is out of bounds for 'swap'");
	return ret;
    }
    if (it2< 0 || it2>= siz)
    {
 	ycp2error("Index 'i2' is out of bounds for 'swap'");
	return ret;
    }
    
    ret->swap(it1,it2);
    return ret;
}

static YCPValue
l_reverselist (const YCPList &v){
    /**
     * @builtin list::reverse
     * @id list.reverse
     * @short Creates new list with reversed order of elements.
     * @param list<flex1> v list
     * @return New reversed list. 
     *
     * @description
     * Creates new list with reversed order of elements. Return nil if list is nil. 
     *
     * @usage list::reverse ([0,1,2,3]) -> [3,2,1,0]
     * @usage list::reverse ([]) -> []
     */
    if (v.isNull ())
    {
	ycp2error ("Cannot reverse 'nil' list");
	return YCPNull ();
    }

    YCPList ret = v;
    ret->reverse();
    return ret;
}

static YCPValue
l_tolist (const YCPValue &v)
{
    /**
     * @builtin tolist
     * @short Converts a value to a list (deprecated, use (list)VAR).
     * @param any VAR
     * @return list
     *
     * @description
     * If the value can't be converted to a list, nillist is returned.
     * Functionality replaced by retyping: <code>any l_1 = [1, 2, 3];
     * list <integer> l_2 = (list<integer>) l_1;</code>
     */

    if (v.isNull())
    {
	return v;
    }
    if (v->valuetype() == YT_LIST)
    {
	return v->asList();
    }
    return YCPNull();
}


YCPBuiltinList::YCPBuiltinList ()
{
#define ETC 0, NULL, constTypePtr(), NULL
#define ETCf   NULL, constTypePtr(), NULL

    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "find",	"flex (variable <flex>, const list <flex>, const block <boolean>)",			(void *)l_find,		DECL_SYMBOL|DECL_FLEX,                  ETCf },
	{ "prepend",	"list <flex> (const list <flex>, const flex)",						(void *)l_prepend,	DECL_FLEX,				ETCf },
	{ "contains",	"boolean (const list <flex>, const flex)",						(void *)l_contains,	DECL_FLEX,				ETCf },
	{ "setcontains","boolean (const list <flex>, const flex)",						(void *)l_setcontains,	DECL_FLEX,				ETCf },
	{ "union",	"list <any> (const list <any>, const list <any>)",					(void *)l_unionlist,						 ETC },
	{ "+",		"list <flex> (const list <flex>, const list <flex>)",					(void *)l_unionlist,	DECL_FLEX,				ETCf },
	{ "merge",	"list <any> (const list <any>, const list <any>)",					(void *)l_mergelist,						 ETC },
	{ "sublist",	"list <flex> (const list <flex>, integer)",						(void *)l_sublist1,     DECL_FLEX,                              ETCf },
	{ "sublist",	"list <flex> (const list <flex>, integer, integer)",					(void *)l_sublist2,     DECL_FLEX,				ETCf },
	{ "filter",	"list <flex> (variable <flex>, const list <flex>, const block <boolean>)",		(void *)l_filter,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX,	ETCf },
	{ "maplist",	"list <flex1> (variable <flex2>, const list <flex2>, const block <flex1>)",		(void *)l_maplist,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX,	ETCf },
	{ "listmap",	"map <flex1,flex2> (variable <flex3>, const list <flex3>, const block <map <flex1,flex2>>)",	(void *)l_listmap,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX,ETCf },
	{ "flatten",	"list <flex> (const list <list <flex>>)",						(void *)l_flatten,	DECL_FLEX,				ETCf },
	{ "toset",	"list <flex> (const list <flex>)",							(void *)l_toset,	DECL_FLEX,				ETCf },
	{ "sort",	"list <flex> (const list <flex>)",							(void *)l_sortlist,	DECL_FLEX,                              ETCf },
	{ "sort",	"list <flex> (variable <flex>, variable <flex>, const list <flex>, const block <boolean>)", (void *)l_sort, 	DECL_SYMBOL|DECL_FLEX,			ETCf },
	{ "lsort",	"list <flex> (const list <flex>)",							(void *)l_lsortlist,	DECL_FLEX,				ETCf },
	{ "splitstring","list <string> (string, string)",							(void *)l_splitstring,						 ETC },
	{ "change", 	"list <flex> (const list <flex>, const flex)",						(void *)l_changelist,	DECL_FLEX|DECL_DEPRECATED,		ETCf },
	{ "add",	"list <flex> (const list <flex>, const flex)",						(void *)l_add,		DECL_FLEX|DECL_NIL,			ETCf },
	{ "+",		"list <flex> (const list <flex>, const flex)",						(void *)l_add,		DECL_FLEX,				ETCf },
	{ "+",		"list <any> (const list <any>, any)",							(void *)l_add,                                                   ETC },
	{ "isempty",	"boolean (const list <any>)",								(void *)l_isempty,						 ETC },
	{ "size",	"integer (const list <any>)",								(void *)l_size,		DECL_NIL,				ETCf },
	{ "remove",	"list <flex> (const list <flex>, const integer)",					(void *)l_remove,	DECL_FLEX,				ETCf },
	{ "select",	"flex (const list <flex>, integer, flex)",						(void *)l_select,	DECL_NIL|DECL_FLEX,			ETCf },
	{ "foreach",    "flex1 (variable <flex2>, const list <flex2>, const block <flex1>)",			(void *)l_foreach,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX,	ETCf },
	{ "tolist",	"list <any> (const any)",								(void *)l_tolist,	DECL_DEPRECATED,			ETCf },
	{ NULL, NULL, NULL, ETC }
    };

    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations_ns[] = {
	{ "list",	"",											NULL,	                DECL_NAMESPACE, ETCf },
	{ "reduce",	"flex1 (variable <flex1>, variable <flex1>, const list <flex1>, const block <flex1>)",  (void *)l_reduce1, DECL_LOOP|DECL_SYMBOL|DECL_FLEX, ETCf },
	{ "reduce",	"flex1 (variable <flex1>, variable <flex2>, const flex1, const list <flex2>, const block <flex1>)", (void *)l_reduce2, DECL_LOOP|DECL_SYMBOL|DECL_FLEX, ETCf },
	{ "swap",	"list <flex> (const list <flex>, const integer, const integer)",			(void *)l_swaplist,	DECL_FLEX, ETCf },
	{ "reverse",	"list <flex> (const list <flex>)",	(void *)l_reverselist,	DECL_FLEX, ETCf },
	{ NULL, NULL, NULL, ETC }
    };

    static_declarations.registerDeclarations ("YCPBuiltinList", declarations);
    static_declarations.registerDeclarations ("YCPBuiltinList", declarations_ns);

#undef ETC
#undef ETCf
}
