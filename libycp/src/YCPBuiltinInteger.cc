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

   File:       YCPBuiltinInteger.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "YCPInterpreter.h"


YCPValue evaluateIntegerOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    // YCPInterpreter guarantees args->value(0)->isInteger()

    long long value1 = args->value(0)->asInteger()->value();

    switch (code) {
        case YCPB_NEG:   return YCPInteger (-(value1));
        case YCPB_BNOT:  return YCPInteger (~(value1));
        default:	 break;
    }

    if (args->size() != 2)
	return YCPError ("evaluateIntegerOp: wrong number of arguments");

    if (!args->value(1)->isInteger())
	return YCPError ("evaluateIntegerOp: second arg not integer");

    long long value2 = args->value(1)->asInteger()->value();

    switch (code)
    {
	case YCPB_PLUS:	 return YCPInteger (value1 +  value2);
	case YCPB_MINUS: return YCPInteger (value1 -  value2);
	case YCPB_MULT:	 return YCPInteger (value1 *  value2);
	case YCPB_DIV:
	    {
		if (value2 == 0)
		{
		    ycp2error (interpreter->current_file.c_str(),
			       interpreter->current_line, "division by zero");
		    return YCPNull ();
		}
		return YCPInteger (value1 / value2);
	    }
	case YCPB_MOD:	 return YCPInteger (value1 %  value2);
	case YCPB_AND:	 return YCPInteger (value1 &  value2);
	case YCPB_OR:	 return YCPInteger (value1 |  value2);
	case YCPB_LEFT:  return YCPInteger (value1 << value2);
	case YCPB_RIGHT: return YCPInteger (value1 >> value2);
	default:	 break;
    }
    return YCPNull ();
}


YCPValue evaluateToInteger(YCPInterpreter *interpreter, const YCPList& args)
{
    if (args->size() == 1) {
	YCPValue value = args->value(0);
	switch (value->valuetype()) {
	    /**
	     * @builtin tointeger(integer i) -> integer
	     * Does convert an integer to itself. This is for consistency with
	     * the other tointeger functions.
	     *
	     * Example <pre>
	     * tointeger(7411) -> 7411
	     * </pre>
	     */
	case YT_INTEGER: return value;


	    /**
	     * @builtin tointeger(float f) -> integer
	     * Converts a floating point number to an integer. Currently no checking
	     * of bounds and no rounding is implemented.
	     *
	     * Example <pre>
	     * tointeger(4.03) -> 4
	     * </pre>
	     */
#warning TODO: correct rounding. But this is locale dependend >:-P

	case YT_FLOAT:   return YCPInteger((long long)(value->asFloat()->value()));

	    /**
	     * @builtin tointeger(string s) -> integer
	     * Converts a string to an integer. Currently no checking
	     * of the syntax is performed and no warnings are printed.
	     * '0x' prefix for hexadecimal and '0' for octal is also supported.
	     *
	     * Example <pre>
	     * tointeger("42") -> 42
	     * tointeger("0x42") -> 66
	     * tointeger("042") -> 34
	     * </pre>
	     */

	case YT_STRING:  return YCPInteger(value->asString()->value_cstr());
	default: return YCPNull();
	}
    }
    else return YCPNull();
}


