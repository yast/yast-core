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
   Summary:     YCP Term Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

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
     * @builtin size
     * @id size-term
     * @short Returns the number of arguments of the term <tt>TERM</tt>.
     *
     * @param term TERM 
     * @return integer Size of the <tt>TERM</tt>
     *
     * @usage size (`VBox(`Empty(),`Empty(),`Empty())) -> 3
     */

    if (term.isNull ())
	return YCPNull ();

    return YCPInteger (term->size ());
}


static YCPValue
t_add (const YCPTerm &term, const YCPValue &value)
{
    /**
     * @builtin add
     * @id add-term
     * @short Add value to term
     * @description
     * Adds the value <tt>VALUE</tt> to the term <tt>TERM</tt> and returns the
     * newly created term. As always in YCP, <tt>TERM</tt> is not modified.
     *
     * @param term TERM 
     * @param any VALUE 
     * @return term
     *
     * @see remove
     * @usage add (`VBox(`Empty()), `Label("a")) -> `VBox (`Empty (), `Label ("a"))
     * @usage add (`VBox(`Empty()), "a") -> `VBox (`Empty (), "a")
     */

    if (term.isNull () || value.isNull ())
	return YCPNull ();

    return term->functionalAdd (value);
}


static YCPValue
t_symbolof (const YCPTerm &term)
{
    /**
     * @builtin symbolof
     * @short Returns the symbol of the term <tt>TERM</tt>.
     *
     * @param term TERM
     * @return symbol
     *
     * @usage symbolof (`hrombuch (18, false)) -> `hrombuch
     * @usage symbolof (`Label("string")) -> `Label
     */

    if (term.isNull ())
	return YCPNull ();

    return YCPSymbol (term->name ());
}


// not static to get seen by l_select hack
//  parameters are YCPValue because we accept nil
YCPValue
t_select (const YCPValue &term, const YCPValue&i, const YCPValue &def)
{
    /**
     * @builtin select (deprecated, use TERM[ITEM]:DEFAULT)
     * @id select-term
     * @short Select item from term
     * @description
     * Gets the <tt>i</tt>'th value of the term <tt>t</tt>. The first value
     * has the index 0. The call <tt>select ([1, 2, 3], 1)</tt> thus returns
     * 2. Returns the <tt>default</tt> if the index is invalid or the found
     * value has a diffetent type that <tt>default</tt>.
     * Functionality replaced with syntax: <code>term a = `VBox(`VSpacing(2), `Label("string"), `VSpacing(2));
     * a[1]:`Empty() -> `Label ("string")
     * a[9]:`Empty() -> `Empty ()</code>
     *
     * @param term TERM
     * @param integer ITEM
     * @param any DEFAULT
     *
     * @usage select (`hirn (true, false), 33, true) -> true
     */

    if (term.isNull ()
	|| !term->isTerm()
	|| i.isNull ()
	|| !i->isInteger ())
    {
	return YCPNull ();
    }

    long idx = i->asInteger()->value ();

    YCPList list = term->asTerm()->args ();

    if (idx < 0 || idx >= list->size ())
    {
	return def;
    }

    return list->value (idx);
}


static YCPValue
t_toterm1 (const YCPValue &v)
{
    /**
     * @builtin toterm
     * @id toterm-any
     * @short Converts a value to a term.
     *
     * @description
     * If the value can't be converted to a term, nilterm is returned.
     *
     * @param any VALUE
     * @return term
     *
     * @usage toterm ("VBox") -> `VBox ()
     * @usage toterm (`VBox) -> `VBox ()
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
    else if (v->valuetype() == YT_SYMBOL)
    {
	return YCPTerm (v->asSymbol()->symbol());
    }
    return YCPNull();
}


static YCPValue
t_toterm2 (const YCPSymbol &s, const YCPList &l)
{
   /**
     * @builtin toterm
     * @id toterm-list
     * @short Constructs a term from a symbol and a list.
     *
     * @description
     * Constructs a term from a symbol and a list. Thus complement to symbolof
     * and argsof.
     *
     * @param symbol s
     * @param list l
     * @return term
     *
     * @usage toterm (`RadioButton, [ `id (`test), "Test" ]) -> `RadioButton (`id (`test), "Test")
     */

    if (s.isNull () || l.isNull ())
    {
	return YCPNull ();
    }

    YCPTerm ret = YCPTerm (s->symbol());

    for (int i = 0; i < l->size (); i++)
    {
	ret->add(l->value (i));
    }

    return ret;
}


static YCPValue
t_remove (const YCPTerm &term, const YCPInteger &i)
{
    /**
     * @builtin remove
     * @id remove-term
     * @short Remove item from term
     *
     * @description
     * Remove the i'th value from a term. The first value has the index 1 (!).
     * (The index counting is for compatibility reasons with the 'old'
     * remove which allowed 'remove(`term(1,2,3), 0) = [1,2,3]'
     * Use 'argsof (term) -> list' for this kind of transformation.)
     *
     * The yast2-core version < 2.17.16 returns nil if the index is invalid. This behavior
     * has changed in version 2.17.16 to return unchanged term.
     *
     * @param term TERM
     * @param integer i
     * @return term
     *
     * @see add
     * @usage remove (`fun(1, 2), 1) -> `fun(2)
     * @usage remove (`VBox(`Label("a"), `Label("b")), 2) -> `VBox (`Label ("a"))
     */

    if (term.isNull () || i.isNull ())
	return YCPNull ();

    long idx = i->value ();
    YCPList args = term->args();

    if (idx <= 0 || idx > args->size ())
    {
	ycp2error ("Index %s for remove () out of range", toString (idx).c_str ());
	return term;
    }

    args->remove (idx-1);

    return YCPTerm (term->name(), args);
}


static YCPValue
t_argsof (const YCPTerm &term)
{
    /**
     * @builtin argsof
     * @short Returns the arguments of a term.
     *
     * @param term TERM
     * @return list
     *
     * @usage argsof (`fun(1, 2)) -> [1, 2]
     * @usage argsof(`TextEntry(`id("text"), "Label", "value")) -> [`id ("text"), "Label", "value"]
     */

    if (term.isNull ())
	return YCPNull ();

    return term->args();
}


YCPBuiltinTerm::YCPBuiltinTerm ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
#define ETC 0, NULL, constTypePtr(), NULL
#define ETCf   NULL, constTypePtr(), NULL
	{ "add",	"term (term, const any)",		(void *)t_add,      ETC },
	{ "size",	"integer (term)",			(void *)t_size,	    ETC },
	{ "symbolof",	"symbol (term)",			(void *)t_symbolof, ETC },
	{ "select",	"flex (term, integer, const flex)",	(void *)t_select, DECL_NIL|DECL_FLEX, ETCf },
	{ "toterm",	"term (any)",				(void *)t_toterm1,  ETC },
	{ "toterm",	"term (symbol, const list <any>)",	(void *)t_toterm2,  ETC },
	{ "remove",	"term (term, integer)",			(void *)t_remove,   ETC },
	{ "argsof",	"list <any> (term)",			(void *)t_argsof,   ETC },
	{ NULL, NULL, NULL, ETC }
#undef ETC
#undef ETCf
    };

    static_declarations.registerDeclarations ("YCPBuiltinTerm", declarations);
}
