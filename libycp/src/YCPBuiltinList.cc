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

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include <set>			// for toset

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
#include "ycp/YCPTerm.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
l_find (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin find (symbol s, list l, expression e) -> any
     * Searches for a certain item in the list. It applies the expression
     * e to each element in the list and returns the first element
     * the makes the expression evaluate to true, if s is bound to
     * that element. Returns nil, if none is found.
     *
     * Example: <pre>
     * find (`n, [3,5,6,4], ``(n >= 5)) -> 5
     * </pre>
     */
     
    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPValue ret = YCPNull ();

    SymbolEntry *s = symbol->asEntry()->entry();

    for (int i = 0; i < list->size (); i++)
    {
	YCPValue element = list->value (i);
	s->setValue (element);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    ycp2error ("Bad find expression %s", expr->toString ().c_str ());
	    break;
	}

	if (v->asBoolean ()->value ())
	{
	    ret = element;
	    break;
	}
    }

    return ret;
}


static YCPValue
l_prepend (const YCPList &list, const YCPValue &value)
{
    /**
     * @builtin prepend (list l, value v) -> list
     * Creates a new list that is identical to the list <tt>l</tt> but has
     * the value <tt>v</tt> prepended as additional element.
     *
     * Example: <pre>
     * prepend ([1, 4], 8) -> [8, 1, 4]
     * </pre>
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
     * @builtin contains (list l, any v) -> boolean
     * Determines, if a certain value <tt>v</tt> is contained in
     * a list <tt>l</tt>. Returns true, if this is so.
     *
     * Example: <pre>
     * contains ([1, 2, 5], 2) -> true
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    for (int i = 0; i < list->size (); i++)
    {
	if (list->value (i)->equal (value))
	{
	    return YCPBoolean (true);
	}
    }

    return YCPBoolean (false);
}


static YCPValue
l_setcontains (const YCPList &list, const YCPValue &value)
{
    /**
     * @builtin setcontains (list l, any v) -> boolean
     * Determines, if a certain value <tt>v</tt> is contained in
     * a list <tt>l</tt>, but assumes that <tt>l</tt> is sorted. If <tt>l</tt> is
     * not sorted, the result is undefined.
     *
     * Example: <pre>
     * setcontains ([1, 2, 5], 2) -> true
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    int hi = list->size () - 1;
    int lo = 0;

    while (lo <= hi)
    {
	int mid = (hi + lo) / 2;
	YCPValue midvalue = list->value (mid);
	YCPOrder comp = value->compare (midvalue);

	switch (comp)
	{
	    case YO_EQUAL:
		return YCPBoolean (true);
		break;
	    case YO_LESS:
		hi = mid - 1;
		break;
	    case YO_GREATER:
		lo = mid + 1;
		break;
	}
    }

    return YCPBoolean (false);
}


static YCPValue
l_unionlist (const YCPList &list1, const YCPList &list2)
{
    /**
     * @builtin union (list l1, list l2) -> list
     * Interprets two lists as sets and returns a new list that has
     * all elements of the first list and all of the second list. Identical
     * elements are dropped. The order of the elements in the new list is
     * preserved. Elements of <tt>l1</tt> are prior to elements from <tt>l2</tt>.
     * See also "<tt>mergelist</tt>".
     *
     * Examples: <pre>
     * union ([1, 2], [3, 4]) -> [1, 2, 3, 4]
     * union ([1, 2, 3], [2, 3, 4]) -> [1, 2, 3, 4]
     * </pre>
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
	    bool contained = false;

	    for (int a = 0; a < newlist->size (); a++)
	    {
		if (newlist->value (a)->equal (to_insert))
		{
		    contained = true;
		    break;
		}
	    }

	    if (!contained)
		newlist->add (to_insert);
	}
    }

    return newlist;
}


static YCPValue
l_mergelist (const YCPList &list1, const YCPList &list2)
{
    /**
     * @builtin mergelist (list l1, list l2) -> list
     * Interprets two lists as sets and returns a new list that has
     * all elements of the first list and all of the second list. Identical
     * elements are preserved. The order of the elements in the new list is
     * preserved. Elements of <tt>l1</tt> are prior to elements from <tt>l2</tt>.
     * See also "<tt>union</tt>".
     *
     * Examples: <pre>
     * merge ([1, 2], [3, 4]) -> [1, 2, 3, 4]
     * merge ([1, 2, 3], [2, 3, 4]) -> [1, 2, 3, 2, 3, 4]
     * </pre>
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
l_filter (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin filter (symbol s, list l, block (boolean) c) -> list
     * For each element of the list <tt>l</tt> the expression <tt>v</tt>
     * is executed in a new context, where the variable <tt>s</tt>
     * is assigned to that value. If the expression evaluates to true under
     * this circumstances, the value is appended to the result list.
     *
     * Example: <pre>
     * filter (`v, [1, 2, 3, 5], { return (v > 2); }) -> [3, 5]
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret;

    SymbolEntry *s = symbol->asEntry()->entry();

    for (int i = 0; i < list->size (); i++)
    {
	YCPValue element = list->value (i);
	s->setValue (element);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    ycp2error ("Bad filter expression %s", expr->toString ().c_str ());
	    return YCPNull ();
	}
	if (v->isBreak())
	{
	    break;
	}
	if (v->asBoolean ()->value ())
	{
	    ret->add (element);
	}
    }

    return ret;
}


static YCPValue
l_maplist (const YCPSymbol &symbol, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin maplist (symbol s, list l, block c) -> list
     * Maps an operation onto all elements of a list and thus creates
     * a new list.
     * For each element of the list <tt>l</tt> the expression <tt>v</tt>
     * is evaluated in a new context, where the variable <tt>s</tt>
     * is assigned to that value. The result is the list of those
     * evaluations.
     *
     * Example: <pre>
     * maplist (`v, [1, 2, 3, 5], { return (v + 1); }) -> [2, 3, 4, 6]
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    YCPList ret;
    SymbolEntry *s = symbol->asEntry()->entry();

    for (int i = 0; i < list->size (); i++)
    {
	s->setValue (list->value (i));

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
     * @builtin listmap (symbol k, list l, block c) -> map
     * Maps an operation onto all elements of a list and thus creates a map.
     * For each element <tt>k</tt> of the list <tt>l</tt> in the expression
     * <tt>exp</tt> is evaluated in a new context. The result is the map of
     * those evaluations.
     *
     * The result of each evaluation <i>must</i> be a list with two items.
     * The first item is the key of the new mapentry, the second is the
     * value of the new entry.
     *
     * Examples: <pre>
     * listmap (`k, [1,2,3], { return [k, "xy"]; }) -> $[ 1:"xy", 2:"xy" ]
     * listmap (`k, [1,2,3], { any a = k+10; any b = sformat("x%1",k); list ret = [a,b]; return (ret); }) -> $[ 11:"x1", 12:"x2", 13:"x3" ]
     * </pre>
     */


    if (list.isNull ())
    {
	return YCPNull ();
    }

    SymbolEntry *key = symbol->asEntry()->entry();

    YCPMap ret;
    YCPList curr_list;
    YCPMap curr_map;

    for (int i = 0; i < list->size (); i++)
    {
	key->setValue (list->value (i));

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
                YCPMapIterator it = curr_map->begin();
                ret->add (it.key(), it.value());
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
     * @builtin flatten (list (list, list) l) -> list
     * Gets a list l of lists and creates a single list that is
     * the concatenation of those lists in l.
     *
     * Example: <pre>
     * flatten ([ [1, 2], [3, 4] ]) -> [1, 2, 3, 4]
     * </pre>
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
     * @builtin toset (list l) -> list
     * Scans a list for duplicates, removes them and sorts the list.
     *
     * Example: <pre>
     * toset ([1, 5, 3, 2, 3, true, false, true]) -> [false, true, 1, 2, 3, 5]
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    set <YCPValue, ycpless> newset;

    for (int i = 0; i < list->size (); i++)
    {
	newset.insert (list->value (i));
    }

    YCPList setlist;
    for (set <YCPValue, ycpless>::const_iterator it = newset.begin ();
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
     * @builtin sort (list l) -> list
     * Sort the list l according to the YCP builtin predicate <=.
     * Duplicates are not removed.
     *
     * Example: <pre>
     * sort ([2, 1, true, 1]) -> [true, 1, 1, 2]
     * </pre>
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
     * @builtin sort (symbol x, symbol y, list l, bool order) -> list
     * Sorts the list l. You have to specify an order on the
     * list elements by naming to formal variables x und y and
     * specify an expression order, that evaluates to a boolean
     * value depending on x and y. Return true, if x <= y to
     * sort the list ascending.
     *
     * Examples: <pre>
     * sort (`x, `y, [ 3,6,2,8 ], ``(x<=y)) -> [ 2, 3, 6, 8 ]
     * sort (`x, `y, [1, 2], false) -> endless loop!
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    if (list->size () < 2)
	return list;

    // First make a copy of the list, than make a
    // destructive sort. Sorry, we implement a bubble sort
    // here that has an awful complexity. Feel free so
    // send a patch with a better implementation ;-)

    YCPList result = list;

    SymbolEntry *s1 = sym1->asEntry()->entry();
    SymbolEntry *s2 = sym2->asEntry()->entry();

    bool sorted;
    do
    {
	sorted = true;
	for (int i = 0; i < result->size () - 1; i++)
	{
	    // Compare two items

	    s1->setValue (result->value (i));
	    s2->setValue (result->value (i+1));

	    YCPValue ret = order->evaluate ();
	    if (ret.isNull ())
	    {
		ycp2error ("Bad sort order %s", order->toString ().c_str ());
		return YCPNull ();
	    }

	    if (!ret->isBoolean ())
	    {
		ycp2error ("sort(): order %s evaluates to %s, which is not a boolean", order->toString ().c_str () 
			, ret->toString ().c_str ());
		return YCPNull ();
	    }
	    else if (!ret->asBoolean ()->value ())
	    {
		result->swap (i, i + 1);
		sorted = false;
	    }

	}
    }
    while (!sorted);

    return result;
}


static YCPValue
l_lsortlist (const YCPList &list)
{
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
     * @builtin splitstring (string s, string c) -> list (string)
     * Splits s into sub-strings at delimter chars c.
     * the resulting pieces do not contain c
     *
     * see also: mergestring
     *
     * If s starts with c, the first string in the result list is empty
     * If s ends with c, the last string in the result list is empty.
     * If s does not contain c, the result is a list with s.
     *
     * Examples: <pre>
     * splitstring ("/abc/dev/ghi", "/") -> ["", "abc", "dev", "ghi" ]
     * splitstring ("abc/dev/ghi/", "/") -> ["abc", "dev", "ghi", "" ]
     * splitstring ("abc/dev/ghi/", ".") -> ["abc/dev/ghi/" ]
     * splitstring ("text/with:different/separators", "/:") -> ["text", "with", "different", "separators"]
     * </pre>
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
     * @builtin change (list l, value v) -> list
     *
     * DO NOT use this yet. Its for a special requst, not for common use!!!
     *
     * changes the list l adds a new element
     *
     * Example: <pre>
     * change ([1, 4], 8) -> [1, 4, 8]
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    y2internal ("Change does not work as expected! The argument is not passed by reference.");

    list->add (value);
    return list;
}


static YCPValue
l_add (const YCPList &list, const YCPValue &value)
{
    /**
     * @builtin add (list l, value v) -> list
     * Creates a new list that is identical to the list <tt>l</tt> but has
     * the value <tt>v</tt> appended as additional element.
     *
     * Example: <pre>
     * add ([1, 4], 8) -> [1, 4, 8]
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    return list->functionalAdd (value);
}


static YCPValue
l_size (const YCPList &list)
{
    /**
     * @builtin size (list l) -> integer
     * Returns the number of elements of the list <tt>l</tt>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }

    return YCPInteger (list->size ());
}


static YCPValue
l_remove (const YCPList &list, const YCPInteger &i)
{
    /**
     * @builtin remove (list l, integer i) -> list
     * Remove the <tt>i</tt>'th value from a list. The first value has the
     * index 0. The call remove ([1,2,3], 1) thus returns [1,3]. Returns
     * nil if the index is invalid.
     *
     * Example: <pre>
     * remove ([1, 2], 0) -> [2]
     * </pre>
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
	return YCPNull ();
    }

    ret->remove (idx);
    return ret;
}


static YCPValue
l_select (const YCPList &list, const YCPInteger &i, const YCPValue &def)
{
    /**
     * @builtin select (list l, integer i, any default) -> any
     * Gets the i'th value of a list. The first value has the
     * index 0. The call select([1,2,3], 1) thus returns 2. Returns default
     * if the index is invalid or if the found entry has a different type
     * than the default value.
     *
     * Examples: <pre>
     * select ([1, 2], 22, 0) -> 0
     * select ([1, "two"], 0, "no") -> "no"
     * </pre>
     */

    if (list.isNull() || i.isNull())
    {
	return def;
    }
    long idx = i->value ();
    if (idx < 0 || idx >= list->size ())
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
	    extern YCPValue t_select (const YCPTerm &list, const YCPInteger &i, const YCPValue &def);
	    return t_select (tmp->asTerm (), i, def);
	}
	ycp2error ("Incorrect builtin called, %s is not a list", tmp->toString ().c_str ());
	return def;
    }
    
    YCPValue v = list->value (idx);
    
    return v;
}


static YCPValue
l_foreach (const YCPValue &sym, const YCPList &list, const YCPCode &expr)
{
    /**
     * @builtin foreach(symbol s, list l, any exp) -> any
     * For each element of the list <tt>l</tt> the expression <tt>exp</tt>
     * is executed in a new context, where the variable <tt>s</tt> is
     * assigned to that value. The return value of the last execution of
     * exp is the value of the <tt>foreach</tt> construct.
     *
     * Example <pre>
     * foreach (integer v, [1,2,3], { return v; }) -> 3
     * </pre>
     */

    if (list.isNull ())
    {
	return YCPNull ();
    }
    
    SymbolEntry *s = sym->asEntry()->entry();
    YCPValue ret = YCPVoid();

    for (int i=0; i < list->size(); i++)
    {
	s->setValue (list->value (i));

	ret = expr->evaluate ();
	if (ret.isNull())
	{
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
l_tolist (const YCPValue &v)
{
    /**
     * @builtin tolist (any value) -> list
     * Converts a value to a list.
     * If the value can't be converted to a list, nillist is returned.
     *
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
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "find",	"flex (variable <any>, const list <flex>, const block <boolean>)",			(void *)l_find,		DECL_SYMBOL|DECL_FLEX },
	{ "prepend",	"list <flex> (const list <any>, const flex)",						(void *)l_prepend,	DECL_FLEX },
	{ "contains",	"boolean (const list <flex>, const flex)",						(void *)l_contains,	DECL_FLEX },
	{ "setcontains","boolean (list <flex>, const flex)",							(void *)l_setcontains,	DECL_FLEX },
	{ "union",	"list <any> (const list <any>, const list <any>)",					(void *)l_unionlist	},
	{ "+",		"list <flex> (const list <flex>, const list <flex>)",					(void *)l_unionlist,	DECL_FLEX },
	{ "merge",	"list <any> (const list <any>, const list <any>)",					(void *)l_mergelist	},
	{ "filter",	"list <flex> (variable <flex>, const list <flex>, const block <boolean>)",		(void *)l_filter,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "maplist",	"list <flex> (variable <any>, const list <any>, const block <flex>)",			(void *)l_maplist,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "listmap",	"map <any,any> (variable <any>, const list <any>, const block <map <any,any>>)",	(void *)l_listmap,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "flatten",	"list <flex> (const list <list <flex>>)",						(void *)l_flatten,	DECL_FLEX },
	{ "toset",	"list <flex> (const list <flex>)",							(void *)l_toset,	DECL_FLEX },
	{ "sort",	"list <flex> (const list <flex>)",							(void *)l_sortlist,	DECL_FLEX },
	{ "sort",	"list <flex> (variable <flex>, variable <flex>, const list <flex>, const block <boolean>)", (void *)l_sort, 	DECL_SYMBOL|DECL_FLEX },
	{ "lsort",	"list <flex> (const list <flex>)",							(void *)l_lsortlist,	DECL_FLEX },
	{ "splitstring","list <string> (string, string)",							(void *)l_splitstring	},
	{ "change", 	"list <flex> (const list <flex>, const flex)",						(void *)l_changelist,	DECL_FLEX },
	{ "add",	"list <flex> (const list <flex>, const flex)",						(void *)l_add,		DECL_FLEX },
	{ "+",		"list <flex> (const list <flex>, const flex)",						(void *)l_add,		DECL_FLEX },
	{ "+",		"list <any> (const list <any>, any)",							(void *)l_add		},
	{ "size",	"integer (const list <any>)",								(void *)l_size		},
	{ "remove",	"list <flex> (const list <flex>, const integer)",					(void *)l_remove,	DECL_FLEX },
	{ "select",	"flex (const list <flex>, integer, flex)",						(void *)l_select,	DECL_NIL|DECL_FLEX },
	{ "foreach",    "flex (variable <any>, const list <any>, const block <flex>)",				(void *)l_foreach,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "tolist",	"list <any> (const any)",								(void *)l_tolist	},
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinList", declarations);
}
