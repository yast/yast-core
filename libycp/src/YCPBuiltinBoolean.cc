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

   File:	YCPBuiltinBoolean.cc

   Author:	Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include "ycp/YCPBuiltinBoolean.h"
#include "ycp/YCPBoolean.h"
#include "ycp/StaticDeclaration.h"


extern StaticDeclaration static_declarations;


static YCPValue
b_lnot (const YCPBoolean &b)
{
    /**
     * @builtin ! boolean b -> boolean
     * Logical not for booleans.
     *
     * Example: <pre>
     * !false -> true
     * </pre>
     */

    return YCPBoolean (!(b->value ()));
}


static YCPValue
b_or (const YCPBoolean &b1, const YCPBoolean &b2)
{
    /**
     * @builtin boolean b1 || boolean b2 -> boolean
     * Logical or for booleans.
     *
     * Example: <pre>
     * false || true -> true
     * </pre>
     */

    return YCPBoolean (b1->value () || b2->value ());
}


static YCPValue
b_and (const YCPBoolean &b1, const YCPBoolean &b2)
{
    /**
     * @builtin boolean b1 && boolean b2 -> boolean
     * Logical and for booleans.
     *
     * Example: <pre>
     * false && true -> false
     * </pre>
     */

    return YCPBoolean (b1->value () && b2->value ());
}


YCPBuiltinBoolean::YCPBuiltinBoolean ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "!",  "b|b",  0, (void *)b_lnot, 0 },
	{ "||", "b|bb", 0, (void *)b_or, 0 },
	{ "&&", "b|bb", 0, (void *)b_and, 0 },
	{ 0, 0, 0, 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinBoolean", declarations);
}
