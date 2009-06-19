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
   Summary:     YCP Float Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <sstream>

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
     * @operator float f1 + float f2 -> float
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
     * @operator float f1 - float f2 -> float
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
     * @operator float f1 * float f2 -> float
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
     * @operator float f1 * float f2 -> float
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
     * @operator - float i -> float
     * Negative of float.
     */

    if (f1.isNull ())
	return YCPNull ();

    return YCPFloat (-(f1->value ()));
}


static YCPValue
f_abs(const YCPFloat &f)
{
    /**
     * @builtin float::abs
     * @short absolute value
     * @description
     * Returns absolute value of f.
     * @param f FLOAT
     * @return FLOAT
     * @usage trunc (+1.6) -> +1.6
     * @usage trunc (-1.6) -> +1.6
     */

    if (f.isNull ())
	return YCPNull ();

    return YCPFloat(fabs(f->value()));
}


static YCPValue
f_floor(const YCPFloat &f)
{
    /**
     * @builtin float::floor
     * @short round downwards to integer
     * @description
     * Returns f rounded downwards to the nearest integer.
     * @param f FLOAT
     * @return FLOAT
     * @usage trunc (+1.6) -> +1.0
     * @usage trunc (-1.6) -> -2.0
     */

    if (f.isNull ())
	return YCPNull ();

    return YCPFloat(floor(f->value()));
}


static YCPValue
f_ceil(const YCPFloat &f)
{
    /**
     * @builtin float::ceil
     * @short round upwards to integer
     * @description
     * Returns f rounded upwards to the nearest integer.
     * @param f FLOAT
     * @return FLOAT
     * @usage trunc (+1.6) -> +2.0
     * @usage trunc (-1.6) -> -1.0
     */

    if (f.isNull ())
	return YCPNull ();

    return YCPFloat(ceil(f->value()));
}


static YCPValue
f_trunc(const YCPFloat &f)
{
    /**
     * @builtin float::trunc
     * @short round to integer, towards zero
     * @description
     * Returns f rounded to the nearest integer, towards zero.
     * @param f FLOAT
     * @return FLOAT
     * @usage trunc (+1.6) -> +1.0
     * @usage trunc (-1.6) -> -1.0
     */

    if (f.isNull ())
	return YCPNull ();

    return YCPFloat(trunc(f->value()));
}


static YCPValue
f_pow(const YCPFloat &f1, const YCPFloat &f2)
{
    /**
     * @builtin float::pow
     * @short power function
     * @description
     * Returns the value of f1 raised to the power of f2.
     * @param f1 FLOAT
     * @param f2 FLOAT
     * @return FLOAT
     * @usage pow (10.0, 3.0) -> 1000.0
     */

    if (f1.isNull () || f2.isNull ())
	return YCPNull ();

    return YCPFloat(pow(f1->value(), f2->value()));
}


static YCPValue
f_tostring (const YCPFloat &f, const YCPInteger &precision)
{
    /**
     * @builtin tostring
     * @id tostring-float
     * @short Converts a floating point number to a string
     * @description
     * Converts a floating point number to a string, using the
     * specified precision.
     * @param float FLOAT
     * @param integer PRECISION
     * @return string
     * @usage tostring (0.12345, 4) -> 0.1235
     */

    if (f.isNull () || precision.isNull ())
	return YCPNull ();

    char *buffer;

    if (asprintf (&buffer, "%.*f", int (precision->value ()), f->value ()) == -1)
	return YCPNull (); // malloc error
    YCPValue ret = YCPString (buffer);
    free (buffer);
    return ret;
}


// bnc#460094
static YCPValue
f_tolstring (const YCPFloat &f, const YCPInteger &precision)
{
    /**
     * @builtin float::tolstring
     * @short Converts a floating point number to a localized string
     * @description
     * Converts a floating point number to a localized string, using the
     * specified precision.
     * @param float FLOAT
     * @param integer PRECISION
     * @return string
     * @usage tostring (0.12345, 4) -> 0,1235
     */

    if (f.isNull () || precision.isNull ())
	return YCPNull ();

    std::ostringstream ss;
    ss.imbue (std::locale (""));
    ss.precision (precision->value ());
    ss << fixed<< f->value ();
    YCPString ret (ss.str ());

    return ret;
}


static YCPValue
f_tofloat (const YCPValue &v)
{
    /**
     * @builtin tofloat
     * @short Converts a value to a floating point number.
     * @description
     * If the value can't be converted to a float, nilfloat is returned.
     * @param any VALUE
     * @return float
     *
     * @usage tofloat (4) -> 4.0
     * @usage tofloat ("42") -> 42.0
     * @usage tofloat ("3.14") -> 3.14
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
#define ETC 0, NULL, constTypePtr(), NULL
#define ETCf   NULL, constTypePtr(), NULL

    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "+",	     "float (float, float)",	(void *)f_plus,     ETC },
	{ "-",	     "float (float, float)",	(void *)f_minus,    ETC },
	{ "-",	     "float (float)",		(void *)f_neg,      ETC },
	{ "*",	     "float (float, float)",	(void *)f_mult,     ETC },
	{ "/",	     "float (float, float)",	(void *)f_div,      ETC },
	{ "tofloat", "float (const any)",	(void *)f_tofloat,  ETC },
	{ "tostring","string (float, integer)",	(void *)f_tostring, ETC },
	{ NULL, NULL, NULL, ETC }
    };

    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations_ns[] = {
	{ "float",   "",			NULL,    DECL_NAMESPACE, ETCf },
	{ "abs",     "float (float)",		(void *)f_abs,       ETC },
	{ "floor",   "float (float)",		(void *)f_floor,     ETC },
	{ "ceil",    "float (float)",		(void *)f_ceil,      ETC },
	{ "trunc",   "float (float)",		(void *)f_trunc,     ETC },
	{ "pow",     "float (float, float)",	(void *)f_pow,       ETC },
	{ "tolstring","string (float, integer)",(void *)f_tolstring, ETC },
	{ NULL, NULL, NULL, ETC }
    };

    static_declarations.registerDeclarations ("YCPBuiltinFloat", declarations);
    static_declarations.registerDeclarations ("YCPBuiltinFloat", declarations_ns);
#undef ETC
#undef ETCf
}
