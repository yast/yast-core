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

   File:	YCPBuiltinTerm.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include "ycp/YCPBuiltinTerm.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPString.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
t_size (const YCPTerm &term)
{
    /**
     * @builtin size (term t) -> integer
     * Returns the number of arguments of the term <tt>t</tt>.
     */

    if (term.isNull ())
	return YCPNull ();

    return YCPInteger (term->size ());
}


static YCPValue
t_add (const YCPTerm &term, const YCPValue &value)
{
    /**
     * @builtin add (term t, any v) -> term
     * Adds the value <tt>v</tt> to the term <tt>t</tt> and returns the
     * newly created term. As always in YCP, <tt>t</tt> is not modified.
     *
     * Example: <pre>
     * add (sym (a), b) -> sym (a, b)
     * </pre>
     */

    if (term.isNull () || value.isNull ())
	return YCPNull ();

    return term->functionalAdd (value);
}


static YCPValue
t_symbolof (const YCPTerm &term)
{
    /**
     * @builtin symbolof (term t) -> symbol
     * Returns the symbol of the term <tt>t</tt>.
     *
     * Example: <pre>
     * symbolof (`hrombuch (18, false)) -> `hrombuch
     * </pre>
     */

    if (term.isNull ())
	return YCPNull ();

    return YCPSymbol (term->name ());
}


// not static to get seen by l_select hack
YCPValue
t_select (const YCPTerm &term, const YCPInteger &i, const YCPValue &def)
{
    /**
     * @builtin select (term t, integer i, any default) -> any
     * Gets the <tt>i</tt>'th value of the term <tt>t</tt>. The first value
     * has the index 0. The call <tt>select ([1, 2, 3], 1)</tt> thus returns
     * 2. Returns the <tt>default</tt> if the index is invalid or the found
     * value has a diffetent type that <tt>default</tt>.
     *
     * Example: <pre>
     * select (`hirn (true, false), 33, true) -> true
     * </pre>
     */

    if (term.isNull () || i.isNull ())
	return YCPNull ();

    long idx = i->value ();

    YCPList list = term->args ();

    if (idx < 0 || idx >= list->size ())
    {
	return def;
    }

    return list->value (idx);
}


static YCPValue
t_toterm (const YCPValue &v)
{
    /**
     * @builtin toterm (any value) -> term
     * Converts a value to a term.
     * If the value can't be converted to a term, nilterm is returned.
     *
     */

    if (v.isNull())
    {
	return v;
    }
    if (v->valuetype() == YT_TERM)
    {
	return v->asTerm();
    }
    else if (v->valuetype() == YT_STRING)
    {
	return YCPTerm (v->asString()->value());
    }
    return YCPNull();
}


static YCPValue
t_remove (const YCPTerm &term, const YCPInteger &i)
{
    /**
     * @builtin remove (term t, integer i) -> term
     * Remove the i'th value from a term. The first value has the index 1 (!).
     * (The index counting is for compatibility reasons with the 'old'
     *  remove which allowed 'remove(`term(1,2,3), 0) = [1,2,3]'
     *  Use 'argsof (term) -> list' for this kind of transformation.)
     *
     * Example: <pre>
     * remove (`fun(1, 2), 1) -> `fun(2)
     * </pre>
     */

    if (term.isNull () || i.isNull ())
	return YCPNull ();

    long idx = i->value ();
    YCPList args = term->args();

    if (idx <= 0 || idx > args->size ())
    {
	ycp2error ("Index %s for remove () out of range", toString (idx).c_str ());
	return YCPNull ();
    }

    args->remove (idx-1);

    return YCPTerm (term->name(), args);
}


static YCPValue
t_argsof (const YCPTerm &term)
{
    /**
     * @builtin argsof (term t) -> list
     * Returns the arguments of a term.
     *
     * Example: <pre>
     * argsof (`fun(1, 2)) -> [1, 2]
     * </pre>
     */

    if (term.isNull ())
	return YCPNull ();

    return term->args();
}


YCPBuiltinTerm::YCPBuiltinTerm ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "add",	"term (term, const any)",		(void *)t_add	},
	{ "size",	"integer (term)",			(void *)t_size	},
	{ "symbolof",	"symbol (term)",			(void *)t_symbolof },
	{ "select",	"flex (term, integer, const flex)",	(void *)t_select, DECL_NIL|DECL_FLEX },
	{ "toterm",	"term (any)",				(void *)t_toterm },
	{ "remove",	"term (term, integer)",			(void *)t_remove },
	{ "argsof",	"list <any> (term)",			(void *)t_argsof },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinTerm", declarations);
}
