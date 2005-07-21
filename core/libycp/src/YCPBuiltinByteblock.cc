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

   File:	YCPBuiltinByteblock.cc
   Summary:     YCP Byteblock Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "ycp/YCPBuiltinByteblock.h"
#include "ycp/YCPByteblock.h"
#include "ycp/YCPInteger.h"
#include "ycp/StaticDeclaration.h"


extern StaticDeclaration static_declarations;


static YCPValue
b_tobyteblock (const YCPValue &v)
{
    /**
     * @builtin tobyteblock
     * @short Converts a value to a byteblock.
     *
     * @description
     * If the value can't be converted to a byteblock, nilbyteblock is returned.
     *
     * @param any VALUE
     * @return byteblock
     */

    if (v.isNull())
    {
	return v;
    }
    if (v->valuetype() == YT_BYTEBLOCK)
    {
	return v->asByteblock();
    }
    return YCPNull();
}

static YCPValue
b_size (const YCPByteblock &v)
{
    /**
     * @builtin size
     * @short  Returns a size of a byteblock in bytes.
     * @param byteblock VALUE
     * @return integer
     * @usage size(#[03A6f298B5]) -> 5
     *
     */

    if (v.isNull())
    {
	return v;
    }

    return YCPInteger (v->size ());
}

YCPBuiltinByteblock::YCPBuiltinByteblock ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "tobyteblock","byteblock (const any)", 	(void *)b_tobyteblock },
	{ "size",   "integer (const byteblock)",	(void *)b_size },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinByteblock", declarations);
}
