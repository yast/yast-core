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

   File:       YCPBasicInterpreter.cc

   Author:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

/*
 * Basic interpreter for YCP
 *
 */

static const char *YCPBasicInterpreterId = "$Id$";

#define INNER_DEBUG 0

#include <malloc.h>
#include <stdarg.h>
#include <signal.h>

#include "YCPBasicInterpreter.h"
#include "YCPDebugger.h"
#include "y2log.h"

static YCPValue evaluateIs (const YCPList& args);


// YCPBasicInterpreter


YCPDebugger* YCPBasicInterpreter::debugger = NULL;

void YCPBasicInterpreter::debuggerSignalHandler (int signum)
{
    if (!debugger)
    {
	debugger = new YCPDebugger (signum == SIGUSR2);
    }

    // debugger->add_interpreter is now called automatically in debugger->debug
}

YCPBasicInterpreter::YCPBasicInterpreter ()
{
    if (!debugger)
    {
	char *p = getenv ("Y2DEBUGGER");
	int i = p != NULL ? strtol (p, NULL, 10) : 0;
	if (i == 1 || i == 2)
	    debugger = new YCPDebugger (i == 2);
    }

    if (debugger)
	debugger->add_interpreter (this);

    // USR2 mimics Y2DEBUGGER=2 in pausing the execution.
    // Setting the handler multiple times does not hurt.
    // Sigaction is not necessary because we won't send multiple signals.
    void (*r)(int);
    r = signal (SIGUSR2, debuggerSignalHandler);
    if (r == SIG_ERR)
    {
	y2error ("Could not set debuggerSignalHandler");
    }
}


YCPBasicInterpreter::~YCPBasicInterpreter ()
{
    if (debugger)
    {
	debugger->delete_interpreter (this);
	if (debugger->number_of_interpreters () == 0)
	{
	    delete debugger;
	    debugger = NULL;
	}
    }
}


string
YCPBasicInterpreter::interpreter_name () const
{
    return "YCP";	// must be upper case
}


YCPValue YCPBasicInterpreter::callback (const YCPValue& value)
{
    y2internal ("dummy callback (%s)", value->toString().c_str());
    return YCPVoid();
}

YCPValue YCPBasicInterpreter::evaluateWFM (const YCPValue& value)
{
    y2internal ("dummy evaluateWFM (%s)", value->toString().c_str());
    return YCPVoid();
}

YCPValue YCPBasicInterpreter::evaluateUI (const YCPValue& value)
{
    y2internal ("dummy evaluateUI (%s)", value->toString().c_str());
    return YCPVoid();
}

YCPValue YCPBasicInterpreter::evaluateSCR (const YCPValue& value)
{
    y2internal ("dummy evaluateSCR (%s)", value->toString().c_str());
    return YCPVoid();
}


YCPValue YCPBasicInterpreter::evaluate(const YCPValue& value)
{
    if (value.isNull())
    {
	return value;
    }

#if INNER_DEBUG
    y2debug ("[%s] evaluate (%s)", interpreter_name().c_str(), value->toString().c_str());
#endif

    if (debugger)
	debugger->debug (this, YCPDebugger::BasicInterpreter, value);

    YCPValue v = YCPNull();
    switch (value->valuetype())
    {
	case YT_ERROR:
	{
#if INNER_DEBUG
	    y2debug ("Error");
#endif
	    YCPError err = value->asError();
	    ycp2error (current_file.c_str(), current_line, "%s\n", err->message().c_str());
	    v = err->value();
	}
	break;
	case YT_SYMBOL:
	{
#if INNER_DEBUG
	    y2debug ("Symbol");
#endif
	    v = evaluateSymbol (value->asSymbol());
	}
	break;
	case YT_IDENTIFIER:
	{
#if INNER_DEBUG
	y2debug ("Identifier");
#endif
	v = evaluateIdentifier (value->asIdentifier());
	}
	break;
	case YT_LIST:
	{
#if INNER_DEBUG
	    y2debug ("List");
#endif
	    v = evaluateList	   (value->asList());
	}
	break;
	case YT_MAP:
	{
#if INNER_DEBUG
	    y2debug ("Map");
#endif
	    v = evaluateMap	   (value->asMap());
	}
	break;
	case YT_LOCALE:
	{
#if INNER_DEBUG
	    y2debug ("Locale");
#endif
	    v = evaluateLocale	   (value->asLocale());
	}
	break;
	case YT_TERM:
	{
#if INNER_DEBUG
	    y2debug ("Term");
#endif
	    v = evaluateTerm	   (value->asTerm());
	}
	break;
	case YT_BUILTIN:
	{
#if INNER_DEBUG
	    y2debug ("Builtin");
#endif
	    v = evaluateBuiltin	   (value->asBuiltin());
	}
	break;
	case YT_BLOCK:
	{
#if INNER_DEBUG
	    y2debug ("Block");
#endif
	    v = evaluateBlock	   (value->asBlock());
	}
	break;

	// Simple value like integer or boolean are evaluated to
	// themselves.

	default:
	{
	    v = value;
	}
	break;
    }

#if INNER_DEBUG
    y2debug ("END [%s] evaluate (%s)", interpreter_name().c_str(), value->toString().c_str());
    y2debug ("[%s] result %d (%s)", interpreter_name().c_str(), scopeLevel, (v.isNull()) ? "NULL" : v->toString().c_str());
#endif
    if (!v.isNull() && v->isError())
    {
	v = evaluate (v);
    }
    return v;
}


YCPValue YCPBasicInterpreter::evaluateSymbol(const YCPSymbol& s)
{
#if INNER_DEBUG
    y2debug ("evaluateSymbol (%s)", s->toString().c_str());
#endif
    // A quoted symbol evaluates to itself
    if (s->isQuoted()) return s;

    // A non quoted symbol is looked up as variable
    string variablename = s->symbol();

    YCPValue v = lookupValue (variablename, "");

    if (v.isNull())
    {
	v = YCPError (string("[")
		      + interpreter_name()
		      + "] Undeclared variable "
		      + variablename);
    }
#if INNER_DEBUG
    y2debug ("evaluateSymbol (%s): %s", s->toString().c_str(), v->toString().c_str());
#endif
    return v;
}


YCPValue YCPBasicInterpreter::evaluateIdentifier(const YCPIdentifier& i) const
{
    if (i->symbol()->isQuoted())
	return i;

    string variablename = i->symbol()->symbol();
    const string name_space = i->module();
    YCPValue v = lookupValue (variablename, name_space);

    if (v.isNull())
    {
	v = YCPError (string("[")
		      + interpreter_name()
		      + "] Undeclared variable "
		      + ((name_space=="_") ? "" : name_space)
		      + "::"
		      + variablename);
    }
    return v;
}


YCPValue YCPBasicInterpreter::evaluateList(const YCPList& list)
{
    // eval([a,b,c,...]) -> [eval(a), eval(b), eval(c), ...]
    YCPList newlist;
    newlist->reserve (list->size());
    for (int i=0; i<list->size(); i++)
    {
	YCPValue v = evaluate(list->value(i));
	if (v.isNull())
	{
	    return v;
	}
	newlist->add(v);
    }
    return newlist;
}


YCPValue YCPBasicInterpreter::evaluateMap(const YCPMap& map)
{
    YCPMap newmap;

    for (YCPMapIterator pos = map->begin(); pos != map->end(); ++pos)
    {
	YCPValue v = evaluate(pos.value());
	if (v.isNull())
	{
	    return v;
	}
	YCPValue k = evaluate(pos.key());
	if (k.isNull())
	{
	    return k;
	}
	newmap->add( k, v);
    }

    return newmap;
}


YCPValue YCPBasicInterpreter::evaluateLocale(const YCPLocale& loc)
{
    ycp2milestone ("",0,"virtual evaluateLocale() !\n");
    // A locale evaluated to itself in the basic implementation.
    // Don't change locales here. This would break the translation
    // completely. The translator component would never see a locale,
    // if the Workflowamanger sees it before (which is true for all
    // .ycp script modules.
    return loc;
}


// we trust the syntax checker for the correct number of
// actual arguments

YCPValue YCPBasicInterpreter::evaluateBuiltin (const YCPBuiltin& builtin)
{
    if (builtin.isNull())
    {
	return YCPError ("evaluateBuiltin (NULL)");
    }
    builtin_t code = builtin->builtin_code();

#if INNER_DEBUG
    ycp2debug (interpreter_name().c_str(), -1, "evaluateBuiltin(%s)", builtin->toString().c_str());
#endif

    YCPValue ret = YCPNull();

    switch (code)
    {
	case YCPB_GETTEXTDOMAIN:
	{
	    // used by UI
	    return YCPString (getTextdomain());	// get current textdomain
	}
	break;
	case YCPB_TEXTDOMAIN:
	{
	    /**
	     * @builtin textdomain (optional_string_expression) -> string
	     * Returns the current textdomain and optionally sets a new one.
	     *
	     * It replaces the builtin variable textdomain.
	     *
	     * Compare with the texdomain declaration,
	     * which needs a string literal.
	     */
	    ret = YCPString (getTextdomain());	// get current textdomain
	    if (builtin->size () == 0)
		return ret;

	    string tmp;
	    if (builtin->value(0)->isString())
	    {
	    	tmp = builtin->value(0)->asString()->value();
	    }
	    else
	    {
		YCPValue v = evaluate(builtin->value(0));
		if (v.isNull() || !v->isString())
		{
		    return YCPError ("textdomain argument must be string", ret);
		}
		tmp = v->asString()->value();
	    }
	    if (!tmp.empty ())
		setTextdomain (tmp);
	    return ret;
	}
	break;
	case YCPB_LOCALDOMAIN:
	{
	    return setTextdomain (builtin->value(0)->asString()->value());
	}
	case YCPB_MODULE:
	case YCPB_INCLUDE:
	case YCPB_IMPORT:
	case YCPB_EXPORT:
	case YCPB_FULLNAME:
	    // skip builtin statements here, they're handled in YCPBlock.cc
	    return YCPVoid();
	break;
	case YCPB_ISNIL:
	case YCPB_NISNIL:
	case YCPB_IS:
	    // check (!)isnil (symbol) here _before_ evaluating the args
	    if (builtin->value(0)->isSymbol()
		&& (! builtin->value(0)->asSymbol()->isQuoted()))
	    {
		YCPValue symval = lookupValue (builtin->value(0)->asSymbol()->symbol(), "");

		if (symval.isNull())
		{
		    return YCPBoolean (false);	// symbol undefined
		}

		if (code == YCPB_ISNIL)
		{
		    return YCPBoolean (symval->valuetype() == YT_VOID);
		}
		else if (code == YCPB_NISNIL)
		{
		    return YCPBoolean (symval->valuetype() != YT_VOID);
		}
		// check YCPB_IS below after argument evaluation
	    }
	break;
	case YCPB_DEEPQUOTE: {
	    /**
	     * @builtin ``any -> any
	     * The double backquotes `` are called "deep quote". They are used
	     * to prevent the enclosed expression from evaluation. When a deepquoted
	     * <tt>``x</tt> expression is evaluated it evaluates to its contents <tt>x</tt>
	     * dropping the quotes.
	     *
	     * Example <pre>
	     * ``(1+2)		   -> (1+2)
	     * ``a		   -> a
	     * ``{ return 7; }	   -> { return 7; }
	     * { return 7; }	   -> 7
	     * </pre>
	     */
	    return builtin->value(0);
	}
	break;
	case YCPB_LOGAND: /*FALLTHRU*/
	case YCPB_LOGOR: {
	    // The logical and and or must be handled specially: The second parameter must
	    // not evaluated, if the first determines the result.
	    ret = evaluateLogicalAndOr(builtin->value(0), builtin->value(1), builtin->builtin_code() == YCPB_LOGAND);
	    if (!ret.isNull()) return ret;
	}
	break;
	case YCPB_WFM:		// WFM::...
	{
	    return evaluateWFM (builtin->value(0));
	}
	break;
	case YCPB_UI:		// UI::...
	{
	    return evaluateUI (builtin->value(0));
	}
	break;
	case YCPB_SCR:
	{
	    return evaluateSCR (builtin->value(0));
	}
	break;

	case YCPB_TRIPLE:
	{
	    YCPValue condition = evaluate (builtin->value(0));
	    if (condition->isBoolean())
	    {
		if (condition->asBoolean()->value() == true)
		    return evaluate (builtin->value(1));
		return evaluate (builtin->value(2));
	    }
	    else
	    {
		return YCPError ("Condition is not boolean");
	    }
	}
	break;
	case YCPB_BASSIGN:
	{
	    return evaluateBracketAssign (builtin);
	}
	break;
	case YCPB_BRACKET:
	{
	    return evaluateBracket (builtin);
	}
	break;

	default:
	break;
    }

    // Non-core builtins
    //
    // Evaluate arguments

    YCPList args = YCPList();

    for (int arg=0; arg<builtin->size(); arg++)
    {
	YCPValue v = evaluate(builtin->value(arg));
	if (v.isNull())
	{
	    return v;
	}

	args->add (v);
    }

#if INNER_DEBUG
    y2debug ("evaluateBuiltin %d args (%s)", code, args->toString().c_str());
#endif

    switch (code)
    {
	case YCPB_ASSIGN:
	{
	    ret = evaluateAssign (args);
	}
	break;
	case YCPB_LOCALDEFINE:
	{
	    ret = evaluateDefine (false, args);
	}
	break;
	case YCPB_LOCALDECLARE:
	{
	    ret = evaluateDeclare (false, args);
	}
	break;
	case YCPB_GLOBALDEFINE:
	{
	    ret = evaluateDefine (true, args);
	}
	break;
	case YCPB_GLOBALDECLARE:
	{
	    ret = evaluateDeclare (true, args);
	}
	break;
	case YCPB_DUMPSCOPE:
	{
	    ret = dumpScope (args);
	}
	break;
	case YCPB_MEMINFO:
	{
	    ret = dumpMeminfo(args);
	}
	break;
	case YCPB_IS:
	{
	    ret = evaluateIs (args);
	}
	break;
	case YCPB_ISNIL:
	{
	    ret = YCPBoolean (args->value(0).isNull());
	}
	break;
	case YCPB_EVAL:
	{
	    if (args->size() != 1)
	    {
		ret = YCPError ("eval() expects a single argument");
	    }
	    else
	    {
	    /**
	     * @builtin eval(any v) -> any
	     * Evaluate a YCP value. See also the builtin ``, which is
	     * kind of the counterpart to eval.
	     *
	     * Example <pre>
	     * eval(``(1+2)) -> 3
	     * { term a = ``add(); a = add(a, [1]); a = add(a, 4); return eval(a); } -> [1,4]
	     * </pre>
	     */
		ret = evaluate(args->value(0));
	    }
	}
	break;
	case YCPB_SYMBOLOF:
	{
	    if (args->size() != 1)
	    {
		ret = YCPError ("symbolof() expects a single argument");
	    }
	    else if (args->value(0)->isTerm())
	    {
	    /**
	     * @builtin symbolof(term t) -> symbol
	     * Returns the symbol of a term.
	     *
	     * Example <pre>
	     * symbolof(`hrombuch(18, false)) -> `hrombuch
	     * </pre>
	     */

		// syntax assures single argument is present
		ret = args->value(0)->asTerm()->symbol();
	    }
	    else
	    {
		ret = YCPNull();
	    }
	}
	break;
	default:	    // call subclass YCPInterpreter
	{
	    ret = evaluateBuiltinBuiltin (code, args);
	}
	break;
    }

    if (ret.isNull())
    {
	// Unknown builtin or unknown signature
	ret = YCPError ("Cannot evaluate unknown operation " + builtin->toString());
    }

    return ret;
}


YCPValue YCPBasicInterpreter::evaluateTerm(const YCPTerm& term)
{
    if (term.isNull())
    {
	reportError (LOG_ERROR, "evaluateTerm (NULL)");
	return YCPVoid();
    }
#if INNER_DEBUG
    y2debug ("evaluateTerm(%s)", term->toString().c_str());
#endif

    // Evaluate arguments
    YCPTerm evalterm = YCPTerm(term->symbol(), term->name_space());
    for (int arg=0; arg<term->size(); arg++)
    {
	YCPValue v = evaluate(term->value(arg));
	if (v.isNull())
	{
	    ycp2error (current_file.c_str(), current_line, "Term parameter is NULL\n");
	    return v;
	}
	evalterm->add(v);
    }

    // Don't evaluate quoted term
    if (evalterm->isQuoted()) return evalterm;

    // Look for a definition

    YCPValue ret = evaluateDefinition(evalterm);

    if (ret.isNull())
    {
	// Give subclassed Interpreter a chance
	ret = evaluateInstantiatedTerm(evalterm);
	if (ret.isNull())
	{
	    // Give subclassed YCPInterpreter a chance to know the term
	    // It is very important, that the parent interpreter handles
	    // the evaluation, since YCPInterpreter is subclassed from that.

	    ret = evaluateBuiltinTerm(evalterm);
	}
    }

    if (ret.isNull())
    {
	string symbol = evalterm->symbol()->symbol();
	string name_space = evalterm->name_space();
	ret = YCPError (string ("Undefined function ")
		+ (name_space.empty()?"":(name_space+"::"))
		+ symbol + "()");
    }

    return ret;
}

// -----------------------------------------------------------------------

static YCPValue evaluateIs (const YCPList& args)
{
    /**
     * @builtin is(any v, declaration d) -> boolean
     * Checks if the value <tt>v</tt> complies to the declaration <tt>d</tt>.
     * A special case is to check for a certain type.
     * <pre>
     * is(17, integer)			      -> true
     * is(0, float)			      -> false
     * is(0.0, float)			      -> true
     * is(x, any)			      -> true
     * is(`x, symbol)			      -> true
     * is([true, 8], [ boolean a, integer b ] -> true
     * </pre>
     */
    if (args->size() == 2 && args->value(1)->isDeclaration())
    {
	YCPDeclaration declaration = args->value(1)->asDeclaration();
	return YCPBoolean(declaration->allows(args->value(0)));
    }
    else return YCPError("Wrong arguments to is()");
}


YCPValue YCPBasicInterpreter::evaluateAssign(const YCPList& args)
{
    // The parser guarantees, that the args has two arguments,
    // the first of which is of type YCPSymbolRep or YCPIdentifierRep.
    string variablename;
    string scopename;
    if (args->value(0)->isSymbol())
    {
	variablename = args->value(0)->asSymbol()->symbol();
    }
    else if (args->value(0)->isIdentifier())
    {
	variablename = args->value(0)->asIdentifier()->symbol()->symbol();
	scopename    = args->value(0)->asIdentifier()->module();
    }
    else
    {
	return YCPError ("Bad assignment");
    }
    YCPValue newvalue	  = args->value(1);

    YCPValue result = assignSymbol (variablename, newvalue, scopename);
    if (result.isNull())
    {
	result = YCPError (string ("[")
			   + interpreter_name()
			   + "] Assignment not possible: variable "
			   + (scopename.empty()?"":(scopename+"::"))
			   + variablename
			   + " is not declared");
    }
    else if (result->isDeclaration())
    {
	result = YCPError (string ("Assignment not possible: invalid value ")
			   + newvalue->toString()
			   + " for variable "
			   + (scopename.empty()?"":(scopename+"::"))
			   + variablename
			   + ", which has been declared as "
			   + result->asDeclaration()->toString());
    }

    return result;
}


YCPValue YCPBasicInterpreter::evaluateBracketAssign(const YCPBuiltin& builtin)
{
y2debug ("evaluateBracketAssign(%s)", builtin->toString().c_str());
    // The parser guarantees, that the args has two arguments,
    // the first of which is of type YCPSymbolRep or YCPIndentifierRep.
    string variablename;
    string scopename;
    if (builtin->value(0)->isSymbol())
    {
	variablename = builtin->value(0)->asSymbol()->symbol();
    }
    else if (builtin->value(0)->isIdentifier())
    {
	variablename = builtin->value(0)->asIdentifier()->symbol()->symbol();
	scopename    = builtin->value(0)->asIdentifier()->module();
    }
    else
    {
	return YCPError ("Bad bracket assignment lvalue");
    }
    YCPList args = builtin->value(1)->asList();	// bracket args
    YCPValue newvalue = evaluate (builtin->value(2));
y2debug ("args(%s), newvalue (%s)", args->toString().c_str(), newvalue->toString().c_str());
    YCPValue result = lookupValue (variablename, scopename);
    if (result.isNull())
    {
	result = YCPError (string ("[")
			   + interpreter_name()
			   + "] Assignment not possible: variable "
			   + (scopename.empty()?"":(scopename+"::"))
			   + variablename
			   + " is not declared");
    }
    else
    {
	YCPList args = builtin->value(1)->asList();
	int i = 0;
	while (i < args->size())		// loop over all bracket args
	{
	    YCPValue v = evaluate (args->value(i));
	    if (v.isNull())
	    {
		result = YCPError ("Invalid bracket parameter");
		break;
	    }
	    else if (result->isList())
	    {
		YCPList l = result->asList();
y2debug("isList(%s<%s>", l->toString().c_str(), v->toString().c_str());
		i++;
		if (!v->isInteger())
		{
		    result = YCPError ("Invalid bracket parameter for list");
		    break;
		}

		long long idx = v->asInteger()->value();
		if (idx < 0)
		{
		    result = YCPError ("Invalid bracket list index");
		    break;
		}

		if (i < args->size())
		    result = l->value (idx);
		else
		{
		    result = YCPVoid();
		    l->set (idx, newvalue);
		}
	    }
	    else if (result->isMap())
	    {
		YCPMap m = result->asMap();
		i++;
y2debug("isMap(%s<%s>", m->toString().c_str(), v->toString().c_str());

		if (i < args->size())
		    result = m->value (v);
		else
		{
		    result = YCPVoid();
		    m->add (v, newvalue);
		}
	    }
	    else
	    {
		result = YCPNull();
	    }

	    if (result.isNull())
	    {
		result = YCPError ("Bracket assignment not list or map");
		break;
	    }
	} // while bracket args
    }
    return result;
}


YCPValue YCPBasicInterpreter::evaluateBracket(const YCPBuiltin& builtin)
{
    // builtin[0] == Symbol or Identifier
    // builtin[1] == YCPList (bracket args)
    // builtin[2] == Default (optional)
    YCPValue def = (builtin->size() > 2) ? evaluate (builtin->value(2)) : YCPNull();

    string variablename;
    string scopename;
    if (builtin->value(0)->isSymbol())
    {
	variablename = builtin->value(0)->asSymbol()->symbol();
    }
    else if (builtin->value(0)->isIdentifier())
    {
	variablename = builtin->value(0)->asIdentifier()->symbol()->symbol();
	scopename    = builtin->value(0)->asIdentifier()->module();
    }
    else
    {
	return YCPError ("Bad bracket assignment lvalue");
    }

    YCPValue result = lookupValue (variablename, scopename);
    if (result.isNull())
    {
	result = YCPError (string ("[")
			   + interpreter_name()
			   + "] variable "
			   + (scopename.empty()?"":(scopename+"::"))
			   + variablename
			   + " is not declared");
    }
    else if (result->isVoid())	// nil[...]:def
    {
	result = def;
    }
    else	// variable found
    {
	YCPList args = builtin->value(1)->asList();
	int i = 0;
	while (i < args->size())		// loop over all bracket args
	{
	    YCPValue v = evaluate (args->value(i));
	    if (v.isNull())
	    {
		result = YCPError ("Invalid bracket parameter");
		break;
	    }
	    else if (result->isList())
	    {
		YCPList l = result->asList();
		i++;
		if (!v->isInteger())
		{
		    result = YCPError ("Invalid bracket parameter for list");
		    break;
		}

		long long idx = v->asInteger()->value();
		if ((idx < 0) || (idx >= l->size()))
		{
		    result = def;
		    break;
		}
		result = l->value (idx);
	    }
	    else if (result->isMap())
	    {
		YCPMap m = result->asMap();
		i++;

		result = m->value (v);
	    }
	    else
	    {
		result = YCPError (string ("Bracket expression for "
			   + (scopename.empty()?"":(scopename+"::"))
			   + variablename
			   + " does not evaluate to a list or a map."));
		break;
	    }

	    if (result.isNull())
	    {
		result = def;
		break;
	    }
	} // while bracket args
    }

    return result;
}


YCPValue YCPBasicInterpreter::evaluateDefine(bool global, const YCPList& args)
{
    YCPDeclTerm declaration = args->value(0)->asDeclaration()->asDeclTerm();
    YCPValue value	    = args->value(1);
    string symbol	    = declaration->symbol()->symbol();

    declareSymbol (symbol, declaration, value, global);

    return YCPVoid();
}


YCPValue YCPBasicInterpreter::evaluateDefinition(const YCPTerm& term)
{
    string symbol = term->symbol()->symbol();
    string name_space = term->name_space();
#if INNER_DEBUG
    y2debug ("evaluateDefinition(%s)\n", term->toString().c_str());
#endif

    YCPDeclaration declaration = lookupDeclaration(symbol, name_space);

    if (declaration.isNull() || !declaration->isDeclTerm())
    {
	return YCPNull();
    }

    YCPDeclTerm declterm = declaration->asDeclTerm();

    if (!declterm->allows(term))
    {
	return YCPError(string("actual arguments don't match definition for '") + symbol + "'\n"
			+ "  Expected " + declterm->toString() + "\n"
			+ "  Seen " + term->toString());
    }

    if (!name_space.empty())
    {
	if (openInstance (name_space) == false)
	{
	    return YCPError (string("module \"")
			     + name_space
			     + "\" unknown. Missing import ?", YCPNull());
	}
    }

    // The term is evaluated in a new local variable scope. The
    // terms arguments are bound to variable in this new scope.

    openScope();

    // assign actual arguments

    for (int i=0; i<declterm->size(); i++)
    {
	declareSymbol ( declterm->argumentname(i)->symbol(),
			declterm->declaration(i),
			term->value(i),
			false,	// locally
			false,	// not const
			false); // dont warn
    }

    // save backtrace
    backtrace.push_back (CallFrame (current_func, current_line, current_file));
    current_func = name_space.empty () ? symbol : name_space + "::" + symbol;

    // evaluate in local scope (possibly switched by openInstance !)

    YCPValue ret = evaluate (lookupValue (symbol, ""));

    current_func = backtrace.back ().func;
    backtrace.pop_back ();

    closeScope ();

    if (!name_space.empty())
    {
	closeInstance (name_space);
    }

    return ret;
}


YCPValue
YCPBasicInterpreter::evaluateLogicalAndOr(const YCPValue& f1, const YCPValue& f2, bool op_is_and)
{
    YCPValue f1eval = evaluate(f1);
    if (f1eval.isNull())
    {
	return f1eval;
    }
    else if (!f1eval->isBoolean())
    {
	return YCPError (string ("Logical operator ")
			 + (op_is_and ? "&&" : "||")
			 + " not allowed with non boolean argument "
			 + f1eval->toString());
    }
    bool f1b = f1eval->asBoolean()->value();

    if (op_is_and && !f1b) return YCPBoolean(false);
    else if (!op_is_and && f1b) return YCPBoolean(true);

    YCPValue f2eval = evaluate(f2);
    if (f2eval.isNull())
    {
	return f2eval;
    }
    else if (!f2eval->isBoolean())
    {
	return YCPError (string ("Logical operator ")
			 + (op_is_and ? "&&" : "||")
			 + " not allowed with non boolean argument "
			 + f2eval->toString());
    }
    return f2eval;
}


YCPValue YCPBasicInterpreter::evaluateBlock(const YCPBlock& block)
{
    // If you have nested blocks, than you have to handle break,
    // continue and return statements in a special way, because they
    // may have to be honored by an more outlying block. Here we don't
    // expect any of such statements to be unhandled yet, because we
    // are going to evaluate an outmost block, ie, one, that is not
    // directly contained in another block.

    bool do_break, do_continue, do_return;
    YCPValue retval = block->evaluate(this, do_break, do_continue, do_return);

    /* FIXME: already checked in block->evaluate !  */
    if (do_break)
	reportError (LOG_ERROR, "BI break only allowed in loop blocks");
    else if (do_continue)
	reportError (LOG_ERROR, "BI continue only allowed in loop blocks");

    return retval;
}

YCPValue YCPBasicInterpreter::evaluateInstantiatedTerm(const YCPTerm& term)
{
    y2debug ("virtual evaluateInstantiatedTerm(%s)\n", term->toString().c_str());
    return YCPNull();
}


YCPValue YCPBasicInterpreter::evaluateBuiltinBuiltin (builtin_t code, const YCPList& args)
{
    y2debug ("virtual evaluateBuiltinBuiltin ()\n");
    return YCPNull();
}


YCPValue YCPBasicInterpreter::evaluateBuiltinTerm (const YCPTerm& term)
{
    y2debug ("virtual evaluateBuiltinTerm (%s)\n", term->toString().c_str());
    return YCPNull();
}

YCPValue YCPBasicInterpreter::setModuleName (const string& modulename)
{
    y2debug ("setModuleName (%s)\n", modulename.c_str());
    moduleName = modulename;
    return YCPVoid();
}

YCPValue YCPBasicInterpreter::setTextdomain (const string& domainname)
{
    y2debug ("virtual setTextdomain (%s)\n", domainname.c_str());
    return YCPNull();
}

string YCPBasicInterpreter::getTextdomain (void)
{
    y2debug ("virtual getTextdomain ()\n");
    return "yast2";
}

YCPValue YCPBasicInterpreter::includeFile (const string& filename)
{
    y2debug ("virtual includeFile (%s)\n", filename.c_str());
    return YCPNull();
}

YCPValue YCPBasicInterpreter::importModule (const string& modulename)
{
    y2debug ("virtual importModule (%s)\n", modulename.c_str());
    return YCPNull();
}

YCPValue YCPBasicInterpreter::dumpMeminfo(const YCPList& args) const
{
  if (!args->isEmpty())
    return YCPNull();

  struct mallinfo mi;
  mi = mallinfo ();

  // print "bytes from system" and "bytes in use"
  reportError (LOG_DEBUG, "meminfo: %d %d", mi.arena, mi.uordblks);

  return YCPVoid();
}


YCPValue YCPBasicInterpreter::evaluateDeclare(bool global, const YCPList& args)
{
    // The parser guarantees, that args is a term  with three
    // arguments of type YCPValueRep, YCPSymbolRep and YCPValueRep. I don't check it here a second
    // time.

    string variablename = args->value(1)->asSymbol()->symbol();

    YCPValue declaration = args->value(0);

    if (declaration.isNull() || declaration->isError())
    {
	return declaration;
    }
    else if (!declaration->isDeclaration())
    {
	return YCPError (string("Invalid variable declaration: ")
			 + args->value(0)->toString()
			 + " is no declaration");
    }

    YCPValue value = args->value(2);
    if (!value.isNull() && value->isError())
    {
	value = evaluate(value);    // trigger error log
    }

    YCPDeclaration as_decl = declaration->asDeclaration();
    // Test if the value fits to the declaration.
    if (value->isVoid()
	|| as_decl->allows(value))
    {
	declareSymbol (variablename, as_decl, value, global);
    }
    else
    {
	YCPValue v = as_decl->asDeclType()->propagateTo (value);
	if (! v.isNull())
	{
	    declareSymbol (variablename, as_decl, v, global);
	}
	else
	{
	    return YCPError (string ("Invalid initial value ")
			     + value->toString()
			     + " for variable "
			     + variablename
			     + ", which has been declared as "
			     + declaration->toString());
	    // DONT declare it anyway, as "any"
	    // declareSymbol (variablename, YCPDeclAny(), value, global);
	}
    }

    return YCPVoid();
}


void YCPBasicInterpreter::reportError (enum loglevel_t severity, const char *message, ...) const
{
    // Prepare info text
    va_list ap;
    va_start(ap, message);
    Y2Logging::y2_vlogger (severity, interpreter_name().c_str(), current_file.c_str(), current_line, "", message, ap);
    va_end(ap);
}
