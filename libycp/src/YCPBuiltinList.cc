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

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

 $Id$

/-*/


#include <set>                  // for toset

using std::set;

#include "YCPInterpreter.h"
#include "y2log.h"


static YCPValue evaluateForeachList (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin foreach(symbol s, list l, any exp) -> any
     * For each element of the list <tt>l</tt> the expression <tt>exp</tt>
     * is executed in a new context, where the variable <tt>s</tt> is
     * assigned to that value. The return value of the last execution of
     * exp is the value of the <tt>foreach</tt> construct.
     *
     * Example <pre>
     * foreach(`v, [1,2,3], ``{ return v; }) -> 3
     * </pre>
     */
    if (args->size() == 3 && args->value(0)->isSymbol() && args->value(1)->isList())
    {
	YCPSymbol      symbol      = args->value(0)->asSymbol();
	YCPList        list        = args->value(1)->asList();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       exp         = args->value(2);
	YCPValue       ret         = YCPVoid();

	for (int i=0; i<list->size(); i++) {
	    interpreter->openScope();
	    interpreter->declareSymbol (symbol->symbol(), declaration, list->value(i), false, false, false);
	    ret = interpreter->evaluate (exp);
	    interpreter->closeScope ();
	}
	return ret;
    }
    else return YCPError("Wrong arguments to foreach");
}


YCPValue evaluateFind (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin find(symbol s, list l, expression e) -> any
     * Searches for a certain item in the list. It applies the expression
     * e to each element in the list and returns the first element
     * the makes the expression evaluate to true, if s is bound to
     * that element. Returns nil, if none is found.
     *
     * Example <pre>
     * find(`n, [3,5,6,4], ``(n >= 5)) -> 5
     * </pre>
     */
    if (args->size() == 3
	&& args->value(0)->isSymbol() && args->value(1)->isList())
    {
	YCPSymbol      symbol      = args->value(0)->asSymbol();
	YCPList        list        = args->value(1)->asList();
	YCPValue       expression  = args->value(2);
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       ret         = YCPVoid();

	for (int i=0; i<list->size(); i++)
	{
	    YCPValue element = list->value(i);
	    interpreter->openScope();
	    interpreter->declareSymbol (symbol->symbol(), declaration, element, false, false, false);
	    YCPValue v = interpreter->evaluate (expression);
	    if (v.isNull())
	    {
		ret = YCPError ("Bad find expression " + expression->toString());
		interpreter->closeScope();
		break;
	    }
	    if ( v->isBoolean())
	    {
		if (v->asBoolean()->value())
		{
		    ret = element;
		    interpreter->closeScope();
		    break;
		}
	    }
	    else
		interpreter->reportError(LOG_WARNING, "Expression %s does not evaluate to a boolean, but to %s",
		      expression->toString().c_str(), v->toString().c_str());
	    interpreter->closeScope();
	}
	return ret;
    }

    /**
     * @builtin find(string s1, string s2) -> integer
     * Returns the first position in <tt>s1</tt> where the
     * string <tt>s2</tt> is contained in <tt>s1</tt>.
     * Returns -1 if the string is not found.
     *
     * Example <pre>
     * find( "abcdefghi", "efg" ) -> 4
     * find("aaaaa", "z") -> -1
     * </pre>
     */

    else if ( args->size() == 2 && args->value(0)->isString() && args->value(1)->isString() )
    {
       string s = args->value(0)->asString()->value();
       string::size_type pos = s.find( args->value(1)->asString()->value() );

       if ( pos == s.npos ) return YCPInteger (-1);	// not found
       else return YCPInteger( pos );			// found
    }

    else return YCPError("Wrong arguments to find()");
}


YCPValue evaluatePrepend (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin prepend (list l, value v) -> list
     * Creates a new list that is identical to the list <tt>l</tt> but has
     * the value <tt>v</tt> prepended as additional element.
     *
     * Example <pre>
     * prepend([1,4], 8) -> [8,1,4]
     * </pre>
     */

    if (args->size() == 2 && args->value(0)->isList())
	return args->value(0)->asList()->functionalAdd(args->value(1), true);

    return YCPError("Wrong arguments to prepend()");
}


YCPValue evaluateContains (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin contains (list l, any v) -> boolean
     * Determines, if a certain value <tt>v</tt> is contained in
     * a list <tt>l</tt>. Returns true, if this is so.
     *
     * Example <pre>
     * contains([1,2,5], 2) -> true
     * </pre>
     */

    if (args->size() == 2 && args->value(0)->isList())
    {
	YCPList l = args->value(0)->asList();
	YCPValue v = args->value(1);
	for (int i=0; i<l->size(); i++)
	    if (l->value(i)->equal(v)) return YCPBoolean(true);
	return YCPBoolean(false);
    }
    else return YCPError("Wrong arguments to contains()");
}


static YCPValue evaluateUnionList (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin union(list l1, list l2) -> list
     * Interprets two lists as sets and returns a new list that has
     * all elements of the first list and all of the second list. Identical
     * elements are dropped. The order of the elements in the new list is
     * preserved. Elements of <tt>l1</tt> are prior to elements from <tt>l2</tt>.
     * see also "<tt>merge</tt>"
     */
    if (args->size() == 2
	&& args->value(0)->isList()
	&& args->value(1)->isList())
    {
	YCPList newlist;
	for (int l=0; l<args->size(); l++)
	{
	    YCPList list = args->value(l)->asList();
	    for (int e=0; e<list->size(); e++)
	    {
		YCPValue to_insert = list->value(e);
		// Already contained? I know, this has an _awful_ complexity.
		// We need to introduce an order on YCPValueRep to solve the problem.
		bool contained = false;
		for (int a=0; a<newlist->size(); a++)
		{
		    if (newlist->value(a)->equal(to_insert))
		    {
			contained = true;
			break;
		    }
		}
		if (!contained)
		{
		    newlist->add(to_insert);
		}
	    }
	}
	return newlist;
    }
    return YCPError("Wrong arguments to union()");
}


static YCPValue evaluateMergeList (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin merge(list l1, list l2) -> list
     * Interprets two lists as sets and returns a new list that has
     * all elements of the first list and all of the second list. Identical
     * elements are preserved. The order of the elements in the new list is
     * preserved. Elements of <tt>l1</tt> are prior to elements from <tt>l2</tt>.
     * see also "<tt>union</tt>"
     */
    if (args->size() == 2
	&& args->value(0)->isList()
	&& args->value(1)->isList())
    {
	YCPList newlist;
	for (int l=0; l<args->size(); l++)
	{
	    YCPList list = args->value(l)->asList();
	    for (int e=0; e<list->size(); e++)
	    {
		newlist->add (list->value (e));
	    }
	}
	return newlist;
    }
    return YCPError("Wrong arguments to merge()");
}


YCPValue evaluateFilter(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin filter(symbol s, list l, expression e) -> list
     * For each element of the list <tt>l</tt> the expression <tt>v</tt>
     * is executed in a new context, where the variable <tt>s</tt>
     * is assigned to that value. If the expression evaluates to true under
     * this circumstances, the value is appended to the result list.
     *
     * Example <pre>
     * filter(`v, [1,2,3,5], ``(v > 2)) -> [3,5]
     * </pre>
     */
    if (args->size() == 3 && args->value(0)->isSymbol() && args->value(1)->isList())
    {
	YCPSymbol      symbol      = args->value(0)->asSymbol();
	YCPList        list        = args->value(1)->asList();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       expression  = args->value(2);
	YCPList        ret;

	for (int i=0; i<list->size(); i++)
	{
	    YCPValue element = list->value(i);
	    interpreter->openScope();
	    interpreter->declareSymbol (symbol->symbol(), declaration, element, false, false, false);
	    YCPValue v = interpreter->evaluate(expression);
	    if (v.isNull())
	    {
		interpreter->closeScope();
		return YCPError ("Bad fiter expression " + expression->toString());
	    }
	    if ( v->isBoolean())
	    {
		if (v->asBoolean()->value()) ret->add(element);
	    }
	    else
	    {
		y2warning("Expression %s does not evaluate to a boolean, but to %s",
		      expression->toString().c_str(), v->toString().c_str());
	    }
	    interpreter->closeScope();
	}
	return ret;
    }
    /**
     * @builtin filter(key k, value v, map m, expression e) -> map
     * For each key/value pair of the map <tt>m</tt> the expression <tt>e</tt>
     * is evaluated in a new context, where the variable <tt>k</tt>
     * is assigned to the key and <tt>v</tt> to the value of the pair.
     * If the expression evaluates to true,
     * the key/value pair is appended to the result map.
     *
     * Example <pre>
     * filter(`k, `v, $[1:"a",2:"b",3:3,5:5], ``(k == v)) -> $[3:3,5:5]
     * </pre>
     */
    else  if (args->size() == 4 && args->value(0)->isSymbol() && args->value(1)->isSymbol() && args->value(2)->isMap())
    {
	YCPSymbol      key         = args->value(0)->asSymbol();
	YCPSymbol      value       = args->value(1)->asSymbol();
	YCPMap         map         = args->value(2)->asMap();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       expression  = args->value(3);
	YCPMap         ret;

	for (YCPMapIterator pos = map->begin(); pos != map->end(); ++pos)
	{
	    interpreter->openScope();
	    interpreter->declareSymbol (key->symbol(), declaration, pos.key(), false, false, false);
	    interpreter->declareSymbol (value->symbol(), declaration, pos.value(), false, false, false);
	    YCPValue v = interpreter->evaluate(expression);
	    if (v.isNull())
	    {
		interpreter->closeScope();
		return YCPError ("Bad filter expression " + expression->toString());
	    }
	    if (!v.isNull() && v->isBoolean())
	    {
		if (v->asBoolean()->value()) ret->add( pos.key(), pos.value() );
	    }
	    else
	    {
		y2warning("Expression %s does not evaluate to a boolean, but to %s",
		      expression->toString().c_str(), v->toString().c_str());
	    }
	    interpreter->closeScope();
	}
	return ret;
    }
    else return YCPError("Wrong arguments to filter()");
}



YCPValue evaluateMaplist(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin maplist(symbol s, list l, expression e) -> list
     * Maps an operation onto all elements of a list and thus creates
     * a new list.
     * For each element of the list <tt>l</tt> the expression <tt>v</tt>
     * is evaluated in a new context, where the variable <tt>s</tt>
     * is assigned to that value. The result is the list of those
     * evaluations.
     *
     * Example <pre>
     * maplist(`v, [1,2,3,5], ``(v + 1)) -> [2,3,4,6]
     * </pre>
     */
    if (args->size() == 3 && args->value(0)->isSymbol() && args->value(1)->isList())
    {
	YCPSymbol      symbol      = args->value(0)->asSymbol();
	YCPList        list        = args->value(1)->asList();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       expression  = args->value(2);
	YCPList        ret;

	for (int i=0; i<list->size(); i++)
	{
	    YCPValue element = list->value(i);
	    interpreter->openScope();
	    interpreter->declareSymbol (symbol->symbol(), declaration, element, false, false, false);
	    YCPValue v = interpreter->evaluate(expression);
	    if (v.isNull())
	    {
		interpreter->closeScope();
		return YCPError ("Bad maplist expression " + expression->toString());
	    }

	    ret->add(v);
	    interpreter->closeScope();
	}
	return ret;
    }

    /**
     * @builtin maplist(symbol k, symbol v, map m, expression e) -> list
     * Maps an operation onto all elements key/value pairs of a map and thus creates
     * a list.
     * For each key/value pair of the map <tt>m</tt> the expression <tt>e</tt>
     * is evaluated in a new context, where the variable <tt>k</tt>
     * is assigned to the key and <tt>v</tt> to the value of the pair.
     * The result is the list of those
     * evaluations.
     *
     * Example <pre>
     * maplist(`k, `v, $[1:"a", 2:"b"], ``[k+10, v+"x"]) -> [ [11, "ax"], [ 12, "bx" ] ]
     * </pre>
     */
    else if (args->size() == 4 && args->value(0)->isSymbol() && args->value(1)->isSymbol() && args->value(2)->isMap())
    {
	YCPSymbol      key         = args->value(0)->asSymbol();
	YCPSymbol      value       = args->value(1)->asSymbol();
	YCPMap         map         = args->value(2)->asMap();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       expression  = args->value(3);
	YCPList        ret;

	for (YCPMapIterator pos = map->begin(); pos != map->end(); ++pos)
	{
	    interpreter->openScope();
	    interpreter->declareSymbol (key->symbol(), declaration, pos.key(), false, false, false);
	    interpreter->declareSymbol (value->symbol(), declaration, pos.value(), false, false, false);
	    YCPValue v = interpreter->evaluate(expression);
	    if (v.isNull())
	    {
		interpreter->closeScope();
		return YCPError ("Bad maplist expression " + expression->toString());
	    }

	    ret->add(v);
	    interpreter->closeScope ();
	}
	return ret;
    }
    else return YCPError("Wrong arguments to maplist()");
}



YCPValue evaluateMapmap(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin mapmap(symbol k, symbol v, map m, expression e) -> map
     * Maps an operation onto all elements key/value pairs of a map and thus creates
     * a map.
     * For each key/value pair of the map <tt>m</tt> the expression <tt>e</tt>
     * is evaluated in a new context, where the variable <tt>k</tt>
     * is assigned to the key and <tt>v</tt> to the value of the pair.
     * The result is the map of those evaluations.
     *
     * The result of each evaluation MUST be a list with two items.
     * The first item is the key of the new mapentry, the second
     * is the value of the new entry.
     *
     *
     * Example <pre>
     * mapmap(`k, `v, $[1:"a", 2:"b"], ``([k+10, v+"x"])) -> $[ 11:"ax",  12:"bx" ]
     * mapmap(`k, `v, $[1:"a", 2:"b"], ``{ any a = k+10; any b = v+"x"; list ret = [a,b]; return(ret); }) -> $[ 11:"ax",  12:"bx" ]
     * </pre>
     */
    if (args->size() == 4 && args->value(0)->isSymbol() && args->value(1)->isSymbol() && args->value(2)->isMap())
    {
	YCPSymbol      key         = args->value(0)->asSymbol();
	YCPSymbol      value       = args->value(1)->asSymbol();
	YCPMap         map         = args->value(2)->asMap();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       expression  = args->value(3);
	YCPList        curr_list;
	YCPMap         curr_map;
	YCPMap         ret;

	for (YCPMapIterator pos = map->begin(); pos != map->end(); ++pos)
	{
	    interpreter->openScope();
	    interpreter->declareSymbol (key->symbol(), declaration, pos.key(), false, false, false);
	    interpreter->declareSymbol (value->symbol(), declaration, pos.value(), false, false, false);
	    YCPValue curr_value = interpreter->evaluate(expression);
	    if (curr_value.isNull())
	    {
		interpreter->closeScope ();
		return YCPError ("Bad mapmap expression " + expression->toString());
	    }
	    if (curr_value->isList() )
	    {
	       curr_list = curr_value->asList();
	       if ( curr_list->size() >= 2 )
	       {
		  ret->add( curr_list->value(0), curr_list->value(1) );
	       }
	       else
	       {
		  y2error("mapmap() expression has to deliver a list with two entrys! You have produced this list %s",
			curr_list->toString().c_str());
	       }
	    }
	    else
	    {
	       y2error("mapmap() expression has to deliver a list! You have only %s",
		  curr_value->toString().c_str());
	    }
	    interpreter->closeScope ();
	}
	return ret;
    }
    else return YCPError("Wrong arguments to mapmap()");
}


YCPValue evaluateListmap(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin listmap(symbol k,  list l, expression e) -> map
     * Maps an operation onto all elements of a list and thus creates
     * a map.
     * For each element k of the list <tt>l</tt> in the expression <tt>e</tt>
     * is evaluated in a new context,
     * The result is the map of those evaluations.
     *
     * The result of each evaluation MUST be a list with two items.
     * The first item is the key of the new mapentry, the second
     * is the value of the new entry.
     *
     *
     * Example <pre>
     * listmap(`k, [1,2,3], ``( [k, "xy"])) -> $[ 1:"xy",  2:"xy" ]
     * listmap(`k, [1,2,3], ``{ any a = k+10; any b = sformat("x%1",k); list ret = [a,b]; return(ret); }) -> $[ 11:"x1",  12:"x2", 13:"x3" ]
     * </pre>
     */
    if (args->size() == 3 && args->value(0)->isSymbol() && args->value(1)->isList())
    {
	YCPSymbol      symbol      = args->value(0)->asSymbol();
	YCPList        list        = args->value(1)->asList();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       expression  = args->value(2);
	YCPMap         ret;
	YCPList        curr_list;
	YCPMap         curr_map;

	for (int i=0; i<list->size(); i++)
	{
	    YCPValue element = list->value(i);
	    interpreter->openScope();
	    interpreter->declareSymbol (symbol->symbol(), declaration, element, false, false, false);

	    YCPValue curr_value = interpreter->evaluate(expression);

	    if (curr_value.isNull())
	    {
		interpreter->closeScope ();
		return YCPError ("Bad listmap expression " + expression->toString());
	    }

	    if ( curr_value->isList() )
	    {
	       curr_list = curr_value->asList();
	       if ( curr_list->size() >= 2 )
	       {
		  ret->add( curr_list->value(0), curr_list->value(1) );
	       }
	       else
	       {
		  y2error("listmap() expression has to deliver a list with two entrys! You have produced this list %s",
			curr_list->toString().c_str());
	       }
	    }
	    else
	    {
	       y2error("listmap() expression has to deliver a list! You have only %s",
		  curr_value->toString().c_str());
	    }

	    interpreter->closeScope();
	}
	return ret;
    }
    else return YCPError("Wrong arguments to listmap");
}


YCPValue evaluateFlatten(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin flatten(list(list(type)) l) -> list(type)
     * Gets a list l of lists and creates a single list that is
     * the concatenation of those lists in l.
     *
     * Example <pre>
     * flatten([ [1,2], [3,4] ]) -> [1, 2, 3, 4]
     * </pre>
     */
    if (args->size() == 1 && args->value(0)->isList())
    {
	YCPList toplist = args->value(0)->asList();
	YCPList flatlist;
	for (int i=0; i<toplist->size(); i++)
	{
	    if (!toplist->value(i)->isList())
	    {
		y2error("%s is not a list. Flatten expects a list of lists",
		      toplist->value(i)->toString().c_str());
	    }
	    else
	    {
		YCPList sublist = toplist->value(i)->asList();
		for (int j=0; j<sublist->size(); j++)
		    flatlist->add(sublist->value(j));
	    }
	}
	return flatlist;
    }
    else return YCPError("Wrong arguments to flatten()");
}


YCPValue evaluateToSet(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin toset(list l) -> list
     * Scans a list for duplicates, removes them and sorts the list.
     *
     * Example <pre>
     * toset([1,5,3,2,3,true,false,true]) -> [false,true,1,2,3,5]
     * </pre>
     */
    if (args->size()==1 && args->value(0)->isList())
    {
	YCPList l = args->value(0)->asList();

	set<YCPValue, ycpless> newset;
	for (int i=0; i<l->size(); i++) newset.insert(l->value(i));

	YCPList setlist;
	set<YCPValue, ycpless>::iterator it;
	for (it = newset.begin(); it != newset.end(); ++it) setlist->add(*it);
	return setlist;
    }
    else return YCPError("Wrong arguments to toset()");
}


YCPValue evaluateSort(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin sort(list l) -> list
     * Sort the list l according to the YCP builtin predicate <=.
     * Duplicates are not removed.
     *
     * Example <pre>
     * sort([2,1,true,1]) -> [true,1,1,2]
     * </pre>
     */
    if (args->size() == 1 && args->value(0)->isList())
    {
	YCPList result = args->value(0)->asList()->shallowCopy();

	result->sortlist();

	return result;
    }

    /**
     * @builtin sort(symbol x, symbol y, list l, any order) -> list
     * Sorts the list l. You have to specify an order on the
     * list elements by naming to formal variables x und y and
     * specify an expression order, that evaluates to a boolean
     * value depending on x and y. Return true, if x <= y to
     * sort the list ascending.
     *
     * Example <pre>
     * sort(`x, `y, [ 3,6,2,8 ], ``(x<=y)) -> [ 2, 3, 6, 8 ]
     * sort(`x, `y, [1, 2], false) -> endless loop!
     * </pre>
     */
    else if (args->size() == 4
	&& args->value(0)->isSymbol()
	&& args->value(1)->isSymbol()
	&& args->value(2)->isList())
    {
	YCPSymbol      x           = args->value(0)->asSymbol();
	YCPSymbol      y           = args->value(1)->asSymbol();
	YCPList        l           = args->value(2)->asList();
	YCPValue       order       = args->value(3);
	YCPDeclaration declaration = YCPDeclAny();

	if (x->equal(y)) {
	    y2error("sort() requires two different variable names. You use only %s",
		  y->toString().c_str());
	    return l;
	}

	if (l->size() < 2) return l;

	// First make a copy of the list, than make a
	// destructive sort. Sorry, we implement a bubble sort
	// here that has an awful complexity. Feel free so
	// send a patch with a better implementation ;-)

	YCPList result = l->shallowCopy();

	interpreter->openScope();

	const string xname = x->symbol();
	const string yname = y->symbol();
	interpreter->declareSymbol (xname, declaration, YCPVoid(), false, false, false);
	interpreter->declareSymbol (yname, declaration, YCPVoid(), false, false, false);

	bool sorted;
	do {
	    sorted = true;
	    for (int i=0; i < result->size()-1; i++)
	    {
		// Compare two items
		interpreter->assignSymbol (xname, result->value(i), "");
		interpreter->assignSymbol (yname, result->value(i+1), "");
		YCPValue ret = interpreter->evaluate(order);
		if (ret.isNull())
		{
		    ret = YCPError ("Bad sort order " + order->toString());
		    sorted = true;
		    break;
		}
		if (!ret->isBoolean())
		{
		    y2error("sort(): order %s evaluates to %s, "
			  "which is not a boolean", order->toString().c_str(), ret->toString().c_str());
		}
		else if (!ret->asBoolean()->value())
		{ // swap
		    result->swap(i, i+1);
		    sorted = false;
		}
	    }
	} while (!sorted);
	interpreter->closeScope ();
	return result;
    }
    return YCPError("Wrong arguments to sort()");
}


YCPValue evaluateLSort(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin lsort(list l) -> list
     * Sort the list l according to the current locale.
     * Duplicates are not removed.
     *
     * Example <pre>
     * lsort (["a", "b", "ä"]) -> ["a", "ä", "b"]
     * </pre>
     */
    if (args->size() == 1 && args->value(0)->isList())
    {
	YCPList result = args->value(0)->asList()->shallowCopy();

	result->lsortlist();

	return result;
    }

    return YCPError("Wrong arguments to lsort()");
}


YCPValue evaluateSplitString(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin splitstring(string s, string c) -> list (string)
    * Splits s into sub-strings at delimter chars c.
    * the resulting pieces do not contain c
    *
    * see also: mergestring
    *
    * If s starts with c, the first string in the result list is empty
    * If s ends with c, the last string in the result list is empty.
    * If s does not contain c, the result is a list with s.
    *
    *
    * Example <pre>
    * splitstring("/abc/dev/ghi", "/") -> ["", "abc", "dev", "ghi" ]
    * splitstring("abc/dev/ghi/", "/") -> ["abc", "dev", "ghi", "" ]
    * splitstring("abc/dev/ghi/", ".") -> ["abc/dev/ghi/" ]
    * splitstring("text/with:different/separators", "/:") -> ["text", "with", "different", "separators"]
    * </pre>
    */

   if (args->size() == 2 && args->value(0)->isString() &&
       args->value(1)->isString())
   {
      YCPList l;
      string s = args->value(0)->asString()->value();
      if (s.empty())
	return l;

      string c = args->value(1)->asString()->value();
      if (c.empty())
	return l;

      string::size_type spos = 0;	// start pos
      string::size_type epos = 0;	// end pos

      for (;;) {
	epos = s.find_first_of(c, spos);

	if (epos == string::npos) {	// break if not found
	  l->add (YCPString (string (s, spos)));
	  break;
	}
	if (spos == epos)
	  l->add (YCPString (""));
	else
	  l->add (YCPString (string (s, spos, epos-spos)));	// string piece w/o delimiter

	spos = epos+1;			// skip c in s

	if (spos == s.size()) {		// c was last char
	  l->add (YCPString (""));	// add "" and break
	  break;
	}
      }

      return l;
   }
   else return YCPError("Wrong arguments to splitstring()");
}


static YCPValue evaluateChangeList(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin change(list l, value v) -> list
     *
     * DO NOT use this yet. Its for a special requst, not for common use!!!
     *
     * changes the list l adds a new element
     *
     * Example <pre>
     * change([1,4], 8) -> [1,4,8]
     * </pre>
     */
    args->value(0)->asList()->add(args->value(1));
    return( args->value(0)->asList() );
}


YCPValue evaluateListOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    switch (code)
    {
	case YCPB_SELECT:
	    return evaluateSelect (interpreter, args);
	break;
	case YCPB_UNION:
	case YCPB_PLUS:
	    return evaluateUnionList (interpreter, args);
	break;
	case YCPB_REMOVE:
	    return evaluateRemove (interpreter, args);
	break;
	case YCPB_FOREACH:
	    return evaluateForeachList (interpreter, args);
	break;
	case YCPB_MERGE:
	    return evaluateMergeList (interpreter, args);
	break;
	case YCPB_ADD:
	{
	    /**
	     * @builtin add (list l, value v) -> list
	     * Creates a new list that is identical to the list <tt>l</tt> but has
	     * the value <tt>v</tt> appended as additional element.
	     *
	     * Example <pre>
	     * add([1,4], 8) -> [1,4,8]
	     * </pre>
	     */
	    if (args->size() == 2)
	        return args->value(0)->asList()->functionalAdd(args->value(1));
	}
	break;
	case YCPB_CHANGE:
	    return evaluateChangeList (interpreter, args);
	break;
	case YCPB_SIZE:
	    /**
	     * @builtin size(list l) -> integer
	     * Returns the number of elements of the list <tt>l</tt>
	     */
	    return YCPInteger (args->value(0)->asList()->size());
	break;
	default:
	    break;
    }
    return YCPError("evaluateListOp unknown builtin op");
}
