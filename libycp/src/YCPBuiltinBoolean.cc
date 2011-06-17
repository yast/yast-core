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

   File:	YCPBuiltinBoolean.cc

   Author:	Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "ycp/YCPBuiltinBoolean.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

extern ExecutionEnvironment ee;

extern StaticDeclaration static_declarations;


static YCPValue
b_lnot (const YCPBoolean &b)
{
    /**
     * @operator ! boolean b -> boolean
     * Logical not for booleans.
     *
     * Example: <pre>
     * !false -> true
     * </pre>
     */
     
    if (b.isNull ()) 
	return YCPNull ();

    return YCPBoolean (!(b->value ()));
}


static YCPValue
b_or (const YCPCode &b1, const YCPCode &b2)
{
    /**
     * @operator boolean b1 || boolean b2 -> boolean
     * Logical or for booleans.
     *
     * Example: <pre>
     * false || true -> true
     * </pre>
     */
     
    YCPValue b1_v = b1->code ()->evaluate ();
    
    if (b1_v.isNull () || b1_v->isVoid ())
    {
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "First operand of '||' evaluates to nil, using 'false' instead.");
    }
    else if (b1_v->asBoolean ()->value ())
    {
	return YCPBoolean (true);
    }

    YCPValue b2_v = b2->code ()->evaluate ();
    if (b2_v.isNull () || b2_v->isVoid ())
    {
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "Second operand of '||' evaluates to nil, using 'false' instead.");
	return YCPBoolean (false);
    }
    
    return YCPBoolean (b2_v->asBoolean ()->value ());
}


static YCPValue
b_and (const YCPCode &b1, const YCPCode &b2)
{
    /**
     * @operator boolean b1 && boolean b2 -> boolean
     * Logical and for booleans.
     *
     * Example: <pre>
     * false && true -> false
     * </pre>
     */
     
    YCPValue b1_v = b1->code ()->evaluate ();
    
    if (b1_v.isNull () || b1_v->isVoid ())
    {
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "First operand of '&&' evaluates to nil, using 'false' instead.");
	return YCPBoolean (false);
    }

    if (! b1_v->asBoolean ()->value ()) 
	return YCPBoolean (false);

    YCPValue b2_v = b2->code ()->evaluate ();
    if (b2_v.isNull () || b2_v->isVoid ())
    {
	ycp2warning (ee.filename().c_str(), ee.linenumber(), "Second operand of '&&' evaluates to nil, using 'false' instead.");
	return YCPBoolean (false);
    }
    
    return YCPBoolean (b2_v->asBoolean ()->value ());
}


YCPBuiltinBoolean::YCPBuiltinBoolean ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "!",  "boolean (boolean)",          (void *)b_lnot,   0,           0, 0, 0 },
	{ "||", "boolean (boolean, boolean)", (void *)b_or, 	DECL_NOEVAL, 0, 0, 0 },
	{ "&&", "boolean (boolean, boolean)", (void *)b_and, 	DECL_NOEVAL, 0, 0, 0 },
	{ 0,    0,                            0,                0,           0, 0, 0 },
    };

    static_declarations.registerDeclarations ("YCPBuiltinBoolean", declarations);
}
