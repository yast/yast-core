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

   File:	YCPBuiltinList.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
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
#include "ycp/YCPError.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPCode.h"
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

    YCPValue ret = YCPVoid ();

    SymbolEntry *s = symbol->asEntry()->entry();

    for (int i = 0; i < list->size (); i++)
    {
	YCPValue element = list->value (i);
	s->setValue (element);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    ret = YCPError ("Bad find expression " + expr->toString ());
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

    YCPList ret;

    SymbolEntry *s = symbol->asEntry()->entry();

    for (int i = 0; i < list->size (); i++)
    {
	YCPValue element = list->value (i);
	s->setValue (element);

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    return YCPError ("Bad filter expression " + expr->toString ());
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

    YCPList ret;
    SymbolEntry *s = symbol->asEntry()->entry();

    for (int i = 0; i < list->size (); i++)
    {
	s->setValue (list->value (i));

	YCPValue v = expr->evaluate ();

	if (v.isNull ())
	{
	    return YCPError ("Bad maplist expression " + expr->toString ());
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
	    return YCPError ("Bad listmap expression " + expr->toString ());
	}
	if (curr_value->isBreak())
	{
	    break;
	}
	curr_list = curr_value->asList ();
	if (curr_list->size () >= 2)
	{
	    ret->add (curr_list->value (0), curr_list->value (1));
	}
	else
	{
	    return YCPError ("listmap () expression has to deliver a list "
			     "with two entries! You have produced this list "
			     + curr_list->toString ());
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

    YCPList ret;

    for (int i = 0; i < list->size (); i++)
    {
	if (!list->value (i)->isList ())
	{
	    return YCPError (list->value (i)->toString () + " is not a list. "
			     "Flatten expects a list of lists");
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

    YCPList ret = list->shallowCopy ();
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

    if (list->size () < 2)
	return list;

    // First make a copy of the list, than make a
    // destructive sort. Sorry, we implement a bubble sort
    // here that has an awful complexity. Feel free so
    // send a patch with a better implementation ;-)

    YCPList result = list->shallowCopy ();

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
		return YCPError ("Bad sort order " + order->toString ());

	    if (!ret->isBoolean ())
	    {
		return YCPError ("sort(): order " + order->toString () +
				 " evaluates " "to " + ret->toString () +
				 "which is not a boolean");
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

    return list->functionalAdd (value);
}


static YCPValue
l_size (const YCPList &list)
{
    /**
     * @builtin size (list l) -> integer
     * Returns the number of elements of the list <tt>l</tt>
     */

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

    long idx = i->value ();

    YCPList ret = list->shallowCopy ();

    if (idx < 0 || idx >= ret->size ())
	return YCPError ("Index " + toString (idx) + " for remove () out of range");

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

    return list->value (idx);
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
	{ "find",	"A|YaLACb", DECL_SYMBOL,		(void *)l_find, 0 },
	{ "prepend",	"LA|LaA",   0,				(void *)l_prepend, 0 },
	{ "contains",	"b|LAA",    0,				(void *)l_contains, 0 },
	{ "setcontains","b|LAA",    0,				(void *)l_setcontains, 0 },
	{ "union",	"La|LaLa",  0,				(void *)l_unionlist, 0 },
	{ "merge",	"La|LaLa",  0,				(void *)l_mergelist, 0 },
	{ "filter",	"LA|YALACb", DECL_LOOP|DECL_SYMBOL,	(void *)l_filter, 0 },
	{ "maplist",	"LA|YaLaCA", DECL_LOOP|DECL_SYMBOL,	(void *)l_maplist, 0 },
	{ "listmap",	"MA|YaLaCLA",DECL_LOOP|DECL_SYMBOL,	(void *)l_listmap, 0 },
	{ "flatten",	"LA|LLA",   0,				(void *)l_flatten, 0 },
	{ "toset",	"LA|LA",    0,				(void *)l_toset, 0 },
	{ "sort",	"LA|LA",    0,				(void *)l_sortlist, 0 },
	{ "sort",	"LA|YAYALACb",DECL_SYMBOL,		(void *)l_sort, 0 },
	{ "splitstring","Ls|ss",    0,				(void *)l_splitstring, 0 },
	{ "changelist", "La|Laa",   0,				(void *)l_changelist, 0 },
	{ "add",	"La|Laa",   0,				(void *)l_add, 0 },
	{ "size",	"i|La",     0,				(void *)l_size, 0 },
	{ "remove",	"LA|LAi",   0,				(void *)l_remove, 0 },
	{ "select",	"A|LaiA",   DECL_NIL,			(void *)l_select, 0 },
	{ "foreach",    "A|YaLaCA", DECL_LOOP|DECL_SYMBOL,	(void *)l_foreach, 0},
	{ "tolist",	"La|a",	    0,				(void *)l_tolist, 0 },
	{ 0, 0, 0, 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinList", declarations);
}
