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

   File:       YCPBuildinPath.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/


#include "YCPInterpreter.h"

YCPValue evaluatePathOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    const YCPPath arg1 = args->value(0)->asPath();

    switch (code) {
    case  YCPB_SIZE:
	/**
	 * @builtin size(path p) -> integer
	 * Returns the number of path elements of the path p, i.e. the length of <tt>p</tt>. 
	 * The root path <tt>.</tt> has the length 0.
	 */
	return YCPInteger(arg1->length());
    case YCPB_ADD: 
        /**
         * @builtin add(path p, string s) -> path
         * Returns path with added path element created from string <tt>s</tt>,
         * Example <pre>
         * add (.aaa, "anypath...\n\"") -> .aaa."anypath...\n\""
         * </pre>
         */
        if (args->value(0)->isPath() && args->value(1)->isString()) {
            string s = args->value(1)->asString()->value_cstr();
            YCPPath result;
            result->append(arg1);
            result->append(s);
            return result;
        }
        y2error ("Invalid operation add (path, xxx). Only string is valid here.\n");
        return YCPNull();
    case YCPB_PLUS:
        if (args->value(0)->isPath() && args->value(1)->isPath()) {
            YCPPath arg2 = args->value(1)->asPath();
            if (arg1->length() == 0) return arg2;
            YCPPath result;
            result->append(arg1);
            result->append(arg2);
            return result;
        }
        y2error ("Invalid operation path + xxx. Only path may be added to path.\n");
        return YCPNull();
    default: // to satisfy compiler
        ;
    }
    return YCPNull();
}


YCPValue evaluateToPath(YCPInterpreter *interpreter, const YCPList& args)
{
    if (args->size() == 1) {
	YCPValue value = args->value(0);
	switch (value->valuetype()) {
	    /**
	     * @builtin topath(path p) -> path
	     * Does convert a path to itself. This is for consistency with
	     * the other topath functions.
	     *
	     * Example <pre>
	     * topath(.some.path) -> .some.path 
	     * </pre>
	     */
	case YT_PATH: return value;

	    /**
	     * @builtin topath(string s) -> path
	     * Converts a string notation of a path to a path. The string may 
	     * represent more symbols than one.
	     *
	     * Example <pre>
	     * topath(".some.path") -> .some.path 
	     * </pre>
	     */
	case YT_STRING:
#warning TODO: make a syntax check for the path. The constructor of YCPPath does none!
	    return YCPPath(value->asString()->value_cstr());

	default: return YCPNull();
	}
    }
    else return YCPNull();
}

