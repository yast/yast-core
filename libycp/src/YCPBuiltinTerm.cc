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

   File:       YCPBuiltinTerm.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "YCPInterpreter.h"
#include "y2log.h"


YCPValue evaluateToTerm (YCPInterpreter* interpreter, const YCPList& args)
{
    /**
     * @builtin toterm (string s) -> term
     * Converts a string to a quoted term without arguments.
     *
     * Example: <pre>toterm ("foo") -> `foo ()</pre>
     */
    if (args->size() == 1 && args->value (0)->isString ())
    {
	string str = args->value (0)->asString ()->value ();
	YCPSymbol sym = YCPSymbol (str, true);
	YCPTerm term = YCPTerm (sym);
	return term;
    }

    return YCPNull ();
}


YCPValue evaluateTermOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    switch (code)
    {
	case YCPB_SELECT:
	    return evaluateSelect (interpreter, args);
	break;
	case YCPB_REMOVE:
	    return evaluateRemove (interpreter, args);
	break;
	case YCPB_SIZE:
	{
	    /**
	     * @builtin size (term t) -> integer
	     * Returns the number of arguments of the term <tt>t</tt>
	     */
	    return YCPInteger(args->value(0)->asTerm()->size());
	}
	break;
	case YCPB_ADD:
	{
	    /**
	     * @builtin add (term t, any v) -> term
	     * Adds the value <tt>v</tt> to the tern t and returns the newly
	     * created term. t is not modified.
	     *
	     * Example <pre> add (sym(a), b) -> sym (a, b) </pre>
	     */
	    if (args->size() == 2)
	    {
		return args->value(0)->asTerm()->functionalAdd(args->value(1));
	    }
	}
	break;
	default:
	break;
    }

    ycp2warning (interpreter->current_file.c_str(), interpreter->current_line, "evaluateTermOp unknown code %d", code);
    return YCPNull();
}

