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
   Summary:     Map Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

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
     * @builtin haskey
     * @short Check if map has a certain key
     *
     * @description
     * Determines whether the map <tt>MAP</tt> contains a pair with the
     * key <tt>KEY</tt>. Returns true if this is true.
     *
     * @param map MAP
     * @param any KEY
     * @return boolean
     *
     * @usage haskey($["a":1, "b":2], "a") -> true
     * @usage haskey($["a":1, "b":2], "c") -> false
     */
     
    if (map.isNull ())
    {
	return YCPNull ();
    }	
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
     * @builtin filter
     * @id filter-map
     * @short Filter a Map
     * @description
     * For each key/value pair of the map <tt>MAP</tt> the expression <tt>EXPR</tt>
     * is evaluated in a new block, where the variable <tt>KEY</tt> is assigned
     * to the key and <tt>VALUE</tt> to the value of the pair. If the expression
     * evaluates to true, the key/value pair is appended to the returned map.
     *
     * @param any KEY
     * @param any VALUE
     * @param map MAP
     * @param blocl EXPR
     * @return map
     * @usage filter (`k, `v, $[1:"a", 2:"b", 3:3, 5:5], { return (k == v); }) -> $[3:3, 5:5]
     */

    if (map.isNull ())
	return YCPNull ();

    YCPMap ret;
    SymbolEntryPtr k = key->asEntry()->entry();
    SymbolEntryPtr v = value->asEntry()->entry();

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
     * @builtin mapmap
     * @short Maps an operation onto all key/value pairs of a map
     *
     * @description
     * Maps an operation onto all key/value pairs of the map <tt>MAP</tt> and
     * thus creates a new map. For each key/value pair of the map <tt>MAP</tt>
     * the expression <tt>EXPR</tt> is evaluated in a new block, where the
     * variable <tt>KEY</tt> is assigned to the key and <tt>VALUE</tt> to the value
     * of the pair. The result is the map of those evaluations.
     *
     * The result of each evaluation <i>must</i> be a map with a single entry
     * which will be added to the result map.
     *
     * @param any KEY
     * @param any VALUE
     * @param map MAP
     * @param block EXPR
     * @return map
     *
     * @usage mapmap (integer k, string v, $[1:"a", 2:"b"], { return ($[k+10 : v+"x"]); }) -> $[ 11:"ax", 12:"bx" ]
     * @usage mapmap (integer k, string v, $[1:"a", 2:"b"], { integer a = k + 10; string b = v + "x"; return $[a:b]; }) -> $[ 11:"ax", 12:"bx" ]
     */

    if (map.isNull ())
	return YCPNull ();

    YCPMap expr_map;
    YCPMap curr_map;
    YCPMap ret;
    SymbolEntryPtr k = key->asEntry()->entry();
    SymbolEntryPtr v = value->asEntry()->entry();

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
     * @builtin maplist 
     * @id maplist-map
     * @short Maps an operation onto all elements key/value and create a list
     * @description
     * Maps an operation onto all elements key/value pairs of a map and thus creates
     * a list.
     *
     * For each key/value pair of the map <tt>MAP</tt> the expression <tt>e</tt>
     * is evaluated in a new block, where the variable <tt>KEY</tt>
     * is assigned to the key and <tt>VALUE</tt> to the value of the pair.
     * The result is the list of those evaluations.
     * @param any KEY
     * @param any VALUE
     * @param map MAP
     * @param block EXPR
     * @return list
     *
     * @usage maplist (`k, `v, $[1:"a", 2:"b"], { return [k+10, v+"x"]; }) -> [ [11, "ax"], [ 12, "bx" ] ]
     */

    if (map.isNull ())
	return YCPNull ();

    YCPList ret;

    SymbolEntryPtr k = key->asEntry()->entry();
    SymbolEntryPtr v = value->asEntry()->entry();

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
     * @builtin union
     * @id union-map
     * @short Union of 2 maps
     * @description
     * Interprets two maps as sets and returns a new map that has all
     * elements of the first map <tt>MAP1</tt>and all of the second map
     * <tt>MAP2</tt>. If elements have identical keys, values from
     * <tt>MAP2</tt> overwrite elements from <tt>MAP1</tt>.
     *
     * @param map MAP1
     * @param map MAP2
     * @return map
     *
     * @usage union($["a":1, "b":2], $[1:"a", 2:"b"]) -> $[1:"a", 2:"b", "a":1, "b":2]
     * @usage union($["a":1, "b":2], $["b":10, "c":20]) -> $["a":1, "b":10, "c":20]
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
     * @builtin add 
     * @id add-map
     * @short Add a key/value pair to a map
     *
     * @description
     * Adds the key/value pair <tt>k : v</tt> to the map <tt>MAP</tt> and
     * returns the newly Created map. If the key <tt>KEY</tt> exists in
     * <tt>KEY</tt>, the old key/value pair is replaced with the new one.
     * Functionality partly replaced with syntax: <code>map map m = $["a":1];
     * m["b"] = 2; -> $["a":1, "b":2]</code>
     *
     * @param map MAP
     * @param any KEY
     * @param any VALUE
     * @return map
     *
     * @usage add ($["a": 17, "b": 11], "c", 2) -> $["a":17, "b":11, "c":2]
     * @usage add ($["a": 17, "b": 11], "b", 2) -> $["a":17, "b":2]
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
     * @builtin change
     * @id change-map
     * @short Change element pair in a map. Deprecated, use MAP[KEY] = VALUE.
     * @description
     * Before Code 9, this was used to change a map directly
     * without creating a copy. Now it is a synonym for add.
     *
     * @param map MAP
     * @param any KEY
     * @param any VALUE
     *
     * @usage change ($[.a: 17, .b: 11], .b, nil) -> $[.a:17, .b:nil].
     */

    if (map.isNull ())
	return YCPNull ();
	
    if (key.isNull ())
    {
	ycp2error ("Cannot use 'nil' as key in change ()");
	return YCPNull ();
    }
    
    ycpinternal ("Change does not work as expected! The argument is not passed by reference.");

    map->add (key, value);
    return map;
}


// parameter is YCPValue because we accept 'nil'
static YCPValue
m_size (const YCPValue &map)
{
    /**
     * @builtin size
     * @id size-map
     * @short Size of a map
     *
     * @description
     * Returns the number of key/value pairs in the map <tt>MAP</tt>.
     *
     * @param map MAP
     * @return integer
     *
     * @usage size($["a":1, "aa":2, "b":3]) -> 3
     */

    if (map.isNull ()
	|| !map->isMap())
    {
	return YCPInteger (0LL);
    }
    return YCPInteger (map->asMap()->size ());
}


static YCPValue
m_foreach (const YCPValue &key, const YCPValue &val, const YCPMap &map, const YCPCode &expr)
{
    /**
     * @builtin foreach
     * @id foreach-map
     * @short Process the content of a map
     * @description
     * For each key:value pair of the map <tt>MAP</tt> the expression
     * <tt>EXPR</tt> is executed in a new block, where the variables
     * <tt>KEY</tt> is bound to the key and <tt>VALUE</tt> is bound to the
     * value. The return value of the last execution of exp is the value
     * of the <tt>foreach</tt> construct.
     * 
     * @param any KEY
     * @param any VALUE
     * @param map MAP
     * @param any EXPR
     * @return map
     *
     * @usage foreach (integer k, integer v, $[1:1,2:4,3:9], { y2debug("v = %1", v); return v; }) -> 9 
     */
     
    if (map.isNull ())
	return YCPNull ();

    SymbolEntryPtr k = key->asEntry()->entry();
    SymbolEntryPtr v = val->asEntry()->entry();
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
	    ret = YCPVoid();
	    break;
	}
    }
    return ret;
}


static YCPValue
m_tomap (const YCPValue &v)
{
    /**
     * @builtin tomap
     * @short Converts a value to a map.
     *
     * @description
     * If the value can't be converted to a map, nilmap is returned.
     * Functionality partly replaced with retyping: <code>any a = $[1:1, 2:2];
     * map m = (map) a;</code>
     *
     * @param any VALUE
     * @return map
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
     * @builtin remove
     * @id remove-map
     * @short Remove key/value pair from a map
     *
     * @description
     * Remove the value with the key <tt>KEY</tt> from a map. Returns
     * unchanged map if the key is invalid.
     *
     * The yast2-core version < 2.17.16 returns nil if the key is invalid. This behavior
     * has changed in version 2.17.16 to return unchanged map.
     *
     * @param map MAP
     * @param any KEY
     * @return map
     *
     * @usage remove($[1:2], 0) -> $[1:2]
     * @usage remove($[1:2], 1) -> $[]
     * @usage remove ($[1:2, 3:4], 1) -> $[3:4]
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
	return ret;
    }
    
    ret->remove (key);
    return ret;
}

// just put here the lookup builtin docs
    /**
     * @builtin lookup
     * @short Select a map element (deprecated, use MAP[KEY]:DEFAULT)   
     * @param map MAP
     * @param any KEY
     * @param any DEFAULT
     * @return any
     *
     * @description
     * Gets the <tt>KEY</tt>'s value of a map. 
     * Returns <tt>DEFAULT</tt>
     * if the key does not exist. Returns nil if the found 
     * entry has a different type than the default value.
     * Functionality replaced with syntax: <code>map m = $["a":1, "b":2];
     * m["a"]:100 -> 1;
     * m["c"]:100 -> 100;</code>
     *
     * @usage lookup ($["a":42], "b", 0) -> 0
     */


YCPBuiltinMap::YCPBuiltinMap ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "haskey", "boolean (const map <any,any>, const any)",								    (void *)m_haskey },
	{ "mapmap", "map <flex3,flex4> (variable <flex1>, variable <flex2>, const map <flex1,flex2>, const block <map <flex3, flex4>>)",  (void *)m_mapmap,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "maplist","list <flex3> (variable <flex1>, variable <flex2>, const map <flex1,flex2>, const block <flex3>)",	    (void *)m_maplist,  DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "filter", "map <flex1,flex2> (variable <flex1>, variable <flex2>, const map <flex1,flex2>, const block <boolean>)",(void *)m_filter,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "union",  "map <any,any> (const map <any,any>, const map <any,any>)",						    (void *)m_unionmap	},
	{ "+",	    "map <any,any> (const map <any,any>, const map <any,any>)",						    (void *)m_unionmap	},
	{ "add",    "map <flex1,flex2> (const map <flex1,flex2>, const flex1, const flex2)",				    (void *)m_addmap,	DECL_FLEX },
	{ "change", "map <flex1,flex2> (const map <flex1,flex2>, const flex1, const flex2)",				    (void *)m_changemap,DECL_FLEX|DECL_DEPRECATED },
	{ "size",   "integer (const map <any,any>)",									    (void *)m_size,	DECL_NIL },
	{ "foreach","flex1 (variable <flex2>, variable <flex3>, const map <flex2,flex3>, const block <flex1>)",		    (void *)m_foreach,	DECL_LOOP|DECL_SYMBOL|DECL_FLEX },
	{ "tomap",  "map <any,any> (const any)",									    (void *)m_tomap,	DECL_FLEX },
        { "remove", "map <flex1,flex2> (const map <flex1,flex2>, const flex1)", 					    (void *)m_remove,	DECL_FLEX },
	{ 0 }
	// "lookup" is in parser.yy
    };

    static_declarations.registerDeclarations ("YCPBuiltinMap", declarations);
}
