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

   File:	YCPBuiltinMap.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include "ycp/YCPBuiltinMap.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPVoid.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
m_haskey (const YCPMap &map, const YCPValue &value)
{
    /**
     * @builtin haskey (map m, any k) -> boolean
     * Determines whether the map <tt>m</tt> contains a pair with the
     * key <tt>k</tt>. Returns true if this is so.
     */
     
    if (map.isNull ())
	return YCPNull ();
	
    if (value.isNull ())
    {
	ycp2error ("Cannot use 'nil' as key in haskey ()");
	return YCPNull ();
    }

    YCPValue tmp = map->value (value);
    return YCPBoolean (!tmp.isNull ());
}


static YCPValue
m_filter (const YCPSymbol &key, const YCPSymbol &value,
			  const YCPMap &map, const YCPCode &expr)
{
    /**
     * @builtin filter (key k, value v, map m, block (boolean) c) -> map
     * For each key/value pair of the map <tt>m</tt> the expression <tt>exp</tt>
     * is evaluated in a new context, where the variable <tt>k</tt> is assigned
     * to the key and <tt>v</tt> to the value of the pair. If the expression
     * evaluates to true, the key/value pair is appended to the returned map.
     *
     * Example: <pre>
     * filter (`k, `v, $[1:"a", 2:"b", 3:3, 5:5], { return (k == v); }) -> $[3:3, 5:5]
     * </pre>
     */

    if (map.isNull ())
	return YCPNull ();

    YCPMap ret;
    SymbolEntry *k = key->asEntry()->entry();
    SymbolEntry *v = value->asEntry()->entry();

    for (YCPMapIterator pos = map->begin (); pos != map->end (); ++pos)
    {
	k->setValue (pos.key());
	v->setValue (pos.value());

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
	    ret->add (pos.key (), pos.value ());
	}
    }

    return ret;
}


static YCPValue
m_mapmap (const YCPSymbol &key, const YCPSymbol &value,
			  const YCPMap &map, const YCPCode &expr)
{
    /**
     * @builtin mapmap (symbol k, symbol v, map m, expression exp) -> map
     * Maps an operation onto all key/value pairs of the map <tt>m</tt> and
     * thus creates a new map. For each key/value pair of the map <tt>m</tt>
     * the expression <tt>exp</tt> is evaluated in a new context, where the
     * variable <tt>k</tt> is assigned to the key and <tt>v</tt> to the value
     * of the pair. The result is the map of those evaluations.
     *
     * The result of each evaluation <i>must</i> be a map with a single entry
     * which will be added to the result map.
     *
     * Examples: <pre>
     * mapmap (`k, `v, $[1:"a", 2:"b"], { return ($[k+10 : v+"x"]); }) -> $[ 11:"ax", 12:"bx" ]
     * mapmap (`k, `v, $[1:"a", 2:"b"], { any a = k+10; any b = v+"x"; map ret = $[a:b]; return (ret); }) -> $[ 11:"ax", 12:"bx" ]
     * </pre>
     */

    if (map.isNull ())
	return YCPNull ();

    YCPMap expr_map;
    YCPMap curr_map;
    YCPMap ret;
    SymbolEntry *k = key->asEntry()->entry();
    SymbolEntry *v = value->asEntry()->entry();

    for (YCPMapIterator pos = map->begin (); pos != map->end (); ++pos)
    {
	k->setValue (pos.key());
	v->setValue (pos.value());

	YCPValue curr_value = expr->evaluate ();

	if (!curr_value.isNull())
	{
	    if (curr_value->isBreak())
	    {
		break;
	    }

	    expr_map = curr_value->asMap();
	    YCPMapIterator it = expr_map->begin();
	    ret->add (it.key(), it.value());
	}
	else
	{
	    ycp2error ("Bad mapmap expression %s", expr->toString ().c_str ());
	    return YCPNull ();
	}
    }

    return ret;
}


static YCPValue
m_maplist (const YCPSymbol &key, const YCPSymbol &value,
			    const YCPMap &map, const YCPCode &expr)
{
    /**
     * @builtin maplist (symbol k, symbol v, map m, block c) -> list
     * Maps an operation onto all elements key/value pairs of a map and thus creates
     * a list.
     * For each key/value pair of the map <tt>m</tt> the expression <tt>e</tt>
     * is evaluated in a new context, where the variable <tt>k</tt>
     * is assigned to the key and <tt>v</tt> to the value of the pair.
     * The result is the list of those evaluations.
     *
     * Example: <pre>
     * maplist (`k, `v, $[1:"a", 2:"b"], { return [k+10, v+"x"]; }) -> [ [11, "ax"], [ 12, "bx" ] ]
     * </pre>
     */

    if (map.isNull ())
	return YCPNull ();

    YCPList ret;

    SymbolEntry *k = key->asEntry()->entry();
    SymbolEntry *v = value->asEntry()->entry();

    for (YCPMapIterator pos = map->begin (); pos != map->end (); pos++)
    {
	k->setValue (pos.key());
	v->setValue (pos.value());

	YCPValue v = expr->evaluate();

	if (v.isNull())
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
m_unionmap (const YCPMap &map1, const YCPMap &map2)
{
    /**
     * @builtin union (map m1, map m2) -> map
     * Interprets two maps as sets and returns a new map that has all
     * elements of the first map <tt>m1>/tt>and all of the second map
     * <tt>m2</tt>. If elements have identical keys, values from
     * <tt>m2</tt> overwrite elements from <tt>m1</tt>.
     */

    if (map1.isNull () || map2.isNull ())
	return YCPNull ();

    YCPMap newmap;

    for (int m = 0; m < 2; m++)
    {
	YCPMap map = (m == 0 ? map1 : map2);

	for (YCPMapIterator pos = map->begin (); pos != map->end (); pos++)
	{
	    newmap->add (pos.key (), pos.value ());
	}
    }

    return newmap;
}


static YCPValue
m_addmap (const YCPMap &map, const YCPValue &key, const YCPValue &value)
{
    /**
     * @builtin add (map m, any k, any v) -> map
     * Adds the key/value pair <tt>k : v</tt> to the map <tt>m</tt> and
     * returns the newly Created map. If the key <tt>k</tt> exists in
     * <tt>k</tt>, the old key/value pair is replaced with the new one.
     *
     * Example: <pre>
     * add ($[a: 17, b: 11], `b, nil) -> $[a:17, b:nil].
     * </pre>
     */

    if (map.isNull ())
	return YCPNull ();
	
    if (key.isNull ())
    {
	ycp2error ("Cannot use 'nil' as key in add ()");
	return YCPNull ();
    }

    return map->functionalAdd (key, value);
}


static YCPValue
m_changemap (YCPMap &map, const YCPValue &key, const YCPValue &value)
{
    /**
     * @builtin change (map m, any k, any v) -> map
     *
     * DO NOT use this yet. It's for a special requst, not for common use!!!
     *
     * Adds the key/value pair <tt>k : v</tt> to the map <tt>m</tt> and
     * returns the map. <tt>m</tt> <i>is</i> modified. If the key <tt>k</tt>
     * exists in <tt>k</tt>, the old key/value pair is replaced with the new
     * one.
     *
     * Example: <pre>
     * change ($[.a: 17, .b: 11], .b, nil) -> $[.a:17, .b:nil].
     * </pre>
     */

    if (map.isNull ())
	return YCPNull ();
	
    if (key.isNull ())
    {
	ycp2error ("Cannot use 'nil' as key in change ()");
	return YCPNull ();
    }

    map->add (key, value);
    return map;
}


static YCPValue
m_size (const YCPMap &map)
{
    /**
     * @builtin size (map m) -> integer
     * Returns the number of key/value pairs in the map <tt>m</tt>
     */

    return YCPInteger (map->size ());
}


static YCPValue
m_foreach (const YCPValue &key, const YCPValue &val, const YCPMap &map, const YCPCode &expr)
{
    /**
     * @builtin foreach(symbol key, symbol value, map m, any exp) -> any
     * For each key:value pair of the map <tt>m</tt> the expression
     * <tt>exp</tt> is executed in a new context, where the variables
     * <tt>key</tt> is bound to the key and <tt>value</tt> is bound to the
     * value. The return value of the last execution of exp is the value
     * of the <tt>foreach</tt> construct.
     * 
     * Example <pre>
     * foreach (integer k, integer v, $[1:1,2:4,3:9], { y2debug("v = %1", v); return v; }) -> 9 
     * </pre>
     */
     
    if (map.isNull ())
	return YCPNull ();

    SymbolEntry *k = key->asEntry()->entry();
    SymbolEntry *v = val->asEntry()->entry();
    YCPValue ret = YCPVoid();

    for (YCPMapIterator pos = map->begin(); pos != map->end(); ++pos)
    {
	k->setValue (pos.key());
	v->setValue (pos.value());

	ret = expr->evaluate ();
	if (ret.isNull())
	{
	    ycp2error ("Bad foreach expression %s", expr->toString ().c_str ());
	    return YCPNull ();
	}
	if (ret->isBreak())
	{
	    break;
	}
    }
    return ret;
}


static YCPValue
m_tomap (const YCPValue &v)
{
    /**
     * @builtin tomap (any value) -> map
     * Converts a value to a map.
     * If the value can't be converted to a map, nilmap is returned.
     *
     */

    if (v.isNull())
    {
	return v;
    }
    if (v->valuetype() == YT_MAP)
    {
	return v->asMap();
    }
    return YCPNull();
}


static YCPValue
m_remove (const YCPMap &map, const YCPValue &key)
{
    /**
     * @builtin remove (map l, any key) -> map
     * Remove the value with the key <tt>key</tt> from a map. Returns
     * nil if the key is invalid.
     *
     * Example: <pre>
     * remove($[1:2], 0) -> nil
     * remove ($[1:2, 3:4], 1) -> $[3:4]
     * </pre>
     */

    if (map.isNull ())
	return YCPNull ();
	
    if (key.isNull ())
    {
	ycp2error ("Cannot use 'nil' as key in remove ()");
	return YCPNull ();
    }

    YCPMap ret = map;

    if(map->value (key).isNull ())
    {
        ycp2error ( "Key %s for remove () does not exist", key->toString ().c_str ());
	return YCPNull ();
    }
    
    ret->remove (key);
    return ret;
}


YCPBuiltinMap::YCPBuiltinMap ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "haskey", "boolean (const map <any,any>, const any)",								    (void *)m_haskey	},
	{ "mapmap", "map <any,flex> (variable <any>, variable <flex>, const map <any,flex>, const block <map <any, flex>>)",	    (void *)m_mapmap,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "maplist","list <any> (variable <any>, variable <any>, const map <any,any>, const block <any>)",		    (void *)m_maplist,  DECL_LOOP|DECL_SYMBOL },
	{ "filter", "map <any,flex> (variable <any>, variable <flex>, const map <any,flex>, const block <boolean>)",	    (void *)m_filter,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "union",  "map <any,any> (const map <any,any>, const map <any,any>)",						    (void *)m_unionmap	},
	{ "+",	    "map <any,any> (const map <any,any>, const map <any,any>)",						    (void *)m_unionmap	},
	{ "add",    "map <any,any> (const map <any,any>, const any, const any)",					    (void *)m_addmap	},
	{ "change", "map <any,any> (const map <any,any>, const any, const any)",					    (void *)m_changemap	},
	{ "size",   "integer (const map <any,any>)",									    (void *)m_size	},
	{ "foreach","flex (variable <any>, variable <any>, const map <any,any>, const block <flex>)",			    (void *)m_foreach,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX},
	{ "tomap",  "map <any,any> (const any)",									    (void *)m_tomap,	DECL_FLEX },
        { "remove", "map <flex,any> (const map <flex,any>, const flex)", 						    (void *)m_remove,	DECL_FLEX },
	{ 0 }
	// "lookup" is in parser.yy
    };

    static_declarations.registerDeclarations ("YCPBuiltinMap", declarations);
}
