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

   File:	YCPBuiltinFloat.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include <unistd.h>
#include <stdio.h>

#include "ycp/YCPBuiltinFloat.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPString.h"
#include "ycp/YCPInteger.h"
#include "ycp/StaticDeclaration.h"

#include "y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
f_plus (const YCPFloat &f1, const YCPFloat &f2)
{
    /**
     * @builtin float f1 + float f2 -> float
     * Addition of floats.
     *
     * Example: <pre>
     * 1.5 + 2.5 -> 4.0
     * </pre>
     */
     
    if (f1.isNull () || f2.isNull ())
	return YCPNull ();

    return YCPFloat (f1->value () + f2->value ());
}


static YCPValue
f_minus (const YCPFloat &f1, const YCPFloat &f2)
{
    /**
     * @builtin float f1 - float f2 -> float
     * Subtraction of floats.
     *
     * Example: <pre>
     * 1.5 - 2.5 -> -1.0
     * </pre>
     */

    if (f1.isNull () || f2.isNull ())
	return YCPNull ();

    return YCPFloat (f1->value () - f2->value ());
}


static YCPValue
f_mult (const YCPFloat &f1, const YCPFloat &f2)
{
    /**
     * @builtin float f1 * float f2 -> float
     * Multiplication of floats.
     *
     * Example: <pre>
     * 1.5 * 2.5 -> 3.75
     * </pre>
     */

    if (f1.isNull () || f2.isNull ())
	return YCPNull ();

    return YCPFloat (f1->value () * f2->value ());
}


static YCPValue
f_div (const YCPFloat &f1, const YCPFloat &f2)
{
    /**
     * @builtin float f1 * float f2 -> float
     * Division of floats.
     *
     * Example: <pre>
     * 1.5 / 2.5 -> 0.6
     * </pre>
     */

    if (f1.isNull () || f2.isNull ())
	return YCPNull ();

    if (f2->value() == 0.0)
    {
	ycp2error ("division by zero");
	return YCPNull ();
    }

    return YCPFloat (f1->value () / f2->value ());
}


static YCPValue
f_neg (const YCPFloat &f1)
{
    /**
     * @builtin - float i -> float
     * Negative of float.
     */

    if (f1.isNull ())
	return YCPNull ();

    return YCPFloat (-(f1->value ()));
}


static YCPValue
f_tostring (const YCPFloat &f, const YCPInteger &precision)
{
    /**
     * @builtin tostring (float f, integer precision) -> string
     * Converts a floating point number to a string, using the
     * specified precision.
     *
     * Example: <pre>
     * tostring (0.12345, 4) -> 0.1235
     * </pre>
     */

    if (f.isNull () || precision.isNull ())
	return YCPNull ();

    char *buffer;

    asprintf (&buffer, "%.*f", int (precision->value ()), f->value ());
    YCPValue ret = YCPString (buffer);
    free (buffer);
    return ret;
}


static YCPValue
f_tofloat (const YCPValue &v)
{
    /**
     * @builtin tofloat (any value) -> float
     * Converts a value to a floating point number.
     * If the value can't be converted to a float, nilfloat is returned.
     *
     * Example: <pre>
     * tofloat (4) -> 4.0
     * tofloat ("42") -> 42.0
     * tofloat ("3.14") -> 3.14
     * </pre>
     */

    if (v.isNull())
    {
	return v;
    }

    switch (v->valuetype())
    {
	case YT_INTEGER:
	    return YCPFloat (double (v->asInteger()->value ()));
	break;
	case YT_FLOAT:
	    return v->asFloat();
	break;
	case YT_STRING:
	    return YCPFloat (v->asString()->value_cstr ());
	break;
	default:
	break;
    }
    return YCPNull();
}


YCPBuiltinFloat::YCPBuiltinFloat ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "+",	     "float (float, float)",	(void *)f_plus },
	{ "-",	     "float (float, float)",	(void *)f_minus },
	{ "-",	     "float (float)",		(void *)f_neg },
	{ "*",	     "float (float, float)",	(void *)f_mult },
	{ "/",	     "float (float, float)",	(void *)f_div },
	{ "tofloat", "float (const any)",	(void *)f_tofloat },
	{ "tostring","string (float, integer)",	(void *)f_tostring },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinFloat", declarations);
}
