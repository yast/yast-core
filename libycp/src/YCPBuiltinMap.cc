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

   File:       YCPBuiltinMap.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "YCPInterpreter.h"


YCPValue evaluateHasKey (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin haskey (map m, any k) -> boolean
     * Determines whether the map m contains a pair with the key k.
     * Returns true if this is so.
     */

    if (args->size () == 2 && args->value (0)->isMap ())
    {
	return YCPBoolean (args->value(0)->asMap()->haskey(args->value(1)));
    }

    if (args->size () == 2 && args->value (0)->isVoid ())
    {
	return YCPBoolean (false);
    }

    return YCPNull ();
}


YCPValue evaluateLookup(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin lookup(map m, any k, any default) -> any
     * In map m looks up the value matching to given key k. Returns <tt>default</tt>
     * if the key was not found.
     *
     * Example <pre>
     * lookup($[1:"a", 2:"bc"], 1, "") -> "a"
     * lookup($[1:"a", 2:"bc"], 371, "take this") -> "take this"
     * </pre>
     */

    if ((args->size() == 2 || args->size() == 3)
	&& args->value(0)->isMap())
    {
	YCPValue value = args->value(0)->asMap()->value(args->value(1));
	if (!value.isNull()) return value;
	else return args->size() == 3 ? args->value(2) : YCPVoid();
    }

    if (args->size () == 3 && args->value (0)->isVoid ())
    {
	return args->value (2);
    }

    return YCPNull();
}


static YCPValue evaluateForeachMap (YCPInterpreter *interpreter, const YCPList& args)
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
     * foreach(`k, `v, $[1:1,2:4,3:9], ``{ y2debug("v = %1", v); return v; }) -> 9
     * </pre>
     */
    if (args->size() == 4 && args->value(0)->isSymbol() &&
	     args->value(1)->isSymbol() && args->value(2)->isMap())
    {
	YCPSymbol      key         = args->value(0)->asSymbol();
	YCPSymbol      value       = args->value(1)->asSymbol();
	YCPMap         map         = args->value(2)->asMap();
	YCPDeclaration declaration = YCPDeclAny();
	YCPValue       exp         = args->value(3);
	YCPValue       ret         = YCPVoid();

	for (YCPMapIterator pos = map->begin(); pos != map->end(); ++pos)
	{
	    interpreter->openScope();
	    interpreter->declareSymbol (key->symbol(), declaration, pos.key(), false, false, false);
	    interpreter->declareSymbol (value->symbol(), declaration, pos.value(), false, false, false);
	    ret = interpreter->evaluate (exp);
	    interpreter->closeScope ();
	}

	return ret;
    }
    else return YCPNull();
}


static YCPValue evaluateUnionMap (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin union(map m1, map m2) -> map
     * Interprets two maps as sets and returns a new map that has
     * all elements of the first map and all of the second map. If elements
     * have identical keys, values from m2 overwrite elements from m1.
     */
    if (args->size() == 2
	&& args->value(0)->isMap()
	&& args->value(1)->isMap())
    {
	YCPMap newmap;
	for (int l=0; l<args->size(); l++)
	{
	    YCPMap map = args->value(l)->asMap();
	    for (YCPMapIterator pos = map->begin();
		 pos != map->end();
		 pos++)
	    {
		newmap->add (pos.key(), pos.value());
	    }
	}
	return newmap;
    }
    return YCPNull();
}


static YCPValue evaluateAddMap (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin add (map m, any k, any v) -> map
     * Adds the key/value pair <tt>k : v</tt> to the map m and returns the newly
     * Created map. m is not modified. If the key k exists in k, the
     * old key/value pair is replaced with the new one.
     *
     * Example <pre> add($[.a: 17, .b: 11], .b, nil) -> $[.a:17, .b:nil].
     * </pre>
     */

    if (args->size() == 3)
    {
   	return args->value(0)->asMap()->functionalAdd(args->value(1),
   						      args->value(2));
    }
    return YCPNull();
}


static YCPValue evaluateChangeMap (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin change(map m, any k, any v) -> map
     *
     * DO NOT use this yet. Its for a special requst, not for common use!!!
     *
     * Adds the key/value pair <tt>k : v</tt> to the map m and returns
     * the map. m is modified. If the key k exists in k, the
     * old key/value pair is replaced with the new one.
     *
     * Example <pre>
     * change($[.a: 17, .b: 11], .b, nil) -> $[.a:17, .b:nil].
     * </pre>
     */

    if (args->size() == 3)
    {
	args->value(0)->asMap()->add(args->value(1), args->value(2));
        return( args->value(0)->asMap() );
    }

    return YCPNull();
}


YCPValue evaluateMapOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    switch (code)
    {
	case YCPB_SIZE:
	    /**
	     * @builtin size(map m) -> integer
	     * Returns the number of key/value pairs of the map <tt>m</tt>
	     */
	    return YCPInteger (args->value(0)->asMap()->size());
	break;
	case YCPB_FOREACH:
	    return evaluateForeachMap (interpreter, args);
	break;
	case YCPB_UNION:
	case YCPB_PLUS:
	    return evaluateUnionMap (interpreter, args);
	break;
	case YCPB_LOOKUP:
	    return evaluateLookup (interpreter, args);
	break;
	case YCPB_ADD:
	    return evaluateAddMap (interpreter, args);
	break;
	case YCPB_REMOVE:
	    return evaluateRemove (interpreter, args);
	break;
	case YCPB_CHANGE:
	    return evaluateChangeMap (interpreter, args);
	break;
	default:
	    break;
    }
    ycp2warning (interpreter->current_file.c_str(), interpreter->current_line, "evaluateMapOp unknown code %d", code);
    return YCPNull();
}
