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

   File:	UI.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "ycp/UI.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPString.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

UI::UI()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "UI",			"",	DECL_NAMESPACE,	0, 0 },
	{ "OpenDialog",		"v|w",	0,		0, 0 },
	{ "CloseDialog",	"v|",	0,		0, 0 },
	{ "UserInput",		"a|",	0,		0, 0 },
	{ 0, 0, 0, 0 }
    };

    extern StaticDeclaration static_declarations;
    static_declarations.registerDeclarations ("UI", declarations);
}
