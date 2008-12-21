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

   File:	YCPBuiltinSymbol.cc
   Summary:     YCP Symbol Builtins

   Authors:	Arvin Schnell <aschnell@suse.de>
   Maintainer:	Arvin Schnell <aschnell@suse.de>

/-*/

#include "ycp/YCPBuiltinSymbol.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPString.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPCodeCompare.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
s_tosymbol1 (const YCPString& v)
{
    /* @builtin tosymbol
     * @id tosymbol-string
     * @short Converts a string to a symbol.
     *
     * @param string VALUE
     * @return symbol
     *
     * @usage tosymbol("test") -> `test
     */

    return YCPSymbol(v->value());
}


static YCPValue
s_tosymbol2 (const YCPInteger& v)
{
    /* @builtin tosymbol
     * @id tosymbol-integer
     * @short Converts a integer to a symbol.
     *
     * @param string VALUE
     * @return symbol
     *
     * @usage tosymbol(69) -> `69
     */

    return YCPSymbol(v->toString());
}


YCPBuiltinSymbol::YCPBuiltinSymbol ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "tosymbol",	"symbol (const string)",				(void*) s_tosymbol1 },
	{ "tosymbol",	"symbol (const integer)",				(void*) s_tosymbol2 },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinSymbol", declarations);
}
