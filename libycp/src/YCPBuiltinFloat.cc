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

   File:       YCPBuiltinFloat.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/


#include "YCPInterpreter.h"


YCPValue evaluateFloatOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    double value1 = args->value(0)->asFloat()->value();

    if (code == YCPB_NEG)
	return YCPFloat (-(value1));

    if (args->size() != 2)
	return YCPError ("evaluateFloatOp: wrong number of arguments");

    if (!args->value(1)->isFloat())
	return YCPError ("evaluateFloatOp: second arg not float");

    double value2 = args->value(1)->asFloat()->value();

    switch (code) {
	case YCPB_PLUS:	 return YCPFloat(value1 + value2);
	case YCPB_MINUS: return YCPFloat(value1 - value2);
	case YCPB_MULT:	 return YCPFloat(value1 * value2);
	case YCPB_DIV:	 return YCPFloat(value1 / value2);
	default:	 break;
    }
    return YCPNull();
}


YCPValue evaluateToFloat(YCPInterpreter *interpreter, const YCPList& args)
{
    if (args->size() == 1) {
	YCPValue value = args->value(0);
	switch (value->valuetype()) {
	    /**
	     * @builtin tofloat(float i) -> float
	     * Does convert a floating point number to itself. This is for
	     * consistency with the other tofloat functions.
	     * 
	     * Example <pre>
	     * tofloat(4711.0) -> 4711.0 
	     * </pre>
	     */
	case YT_FLOAT: return value;


	    /**
	     * @builtin tofloat(integer i) -> float
	     * Converts an integer to a floating point number.
	     * 
	     * Example <pre>
	     * tofloat(4) -> 4.0 
	     * </pre>
	     */
	case YT_INTEGER:  return YCPFloat(double(value->asInteger()->value()));

	    /**
	     * @builtin tofloat(string s) -> float
	     * Converts a string to an integer. Currently no checking
	     * of the syntax is performed and no warnings are printed.
	     * 
	     * Example <pre>
	     * tofloat("42") -> 42.0 
	     * </pre>
	     */

	case YT_STRING:  return YCPFloat(value->asString()->value_cstr());
	default: return YCPNull();
	}
    }
    else return YCPNull();
}

