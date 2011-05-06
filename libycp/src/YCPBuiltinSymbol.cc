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
#include "ycp/YCPBoolean.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPCodeCompare.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
s_tosymbol (const YCPString& v)
{
    /* @builtin tosymbol
     * @short Converts a string to a symbol.
     *
     * @param string VALUE
     * @return symbol
     *
     * @usage tosymbol("test") -> `test
     */

    return YCPSymbol(v->value());
}


YCPBuiltinSymbol::YCPBuiltinSymbol ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "tosymbol",	"symbol (const string)",		(void*) s_tosymbol },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinSymbol", declarations);
}
