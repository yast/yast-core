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

   File:	YCPValue.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "ycp/y2log.h"

#include "ycp/YCPVoid.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPString.h"
#include "ycp/YCPByteblock.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPDeclaration.h"
#include "ycp/YCPLocale.h"
#include "ycp/YCPList.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPBlock.h"
#include "ycp/YCPBuiltin.h"
#include "ycp/YCPIdentifier.h"
#include "ycp/YCPError.h"


// YCPValueRep


// value type checking

bool YCPValueRep::isVoid()        const { return valuetype() == YT_VOID; }
bool YCPValueRep::isBoolean()     const { return valuetype() == YT_BOOLEAN; }
bool YCPValueRep::isInteger()     const { return valuetype() == YT_INTEGER; }
bool YCPValueRep::isFloat()       const { return valuetype() == YT_FLOAT; }
bool YCPValueRep::isString()      const { return valuetype() == YT_STRING; }
bool YCPValueRep::isByteblock()   const { return valuetype() == YT_BYTEBLOCK; }
bool YCPValueRep::isPath()        const { return valuetype() == YT_PATH; }
bool YCPValueRep::isSymbol()      const { return valuetype() == YT_SYMBOL; }
bool YCPValueRep::isDeclaration() const { return valuetype() == YT_DECLARATION; }
bool YCPValueRep::isLocale()      const { return valuetype() == YT_LOCALE; }
bool YCPValueRep::isList()        const { return valuetype() == YT_LIST; }
bool YCPValueRep::isTerm()        const { return valuetype() == YT_TERM; }
bool YCPValueRep::isMap()         const { return valuetype() == YT_MAP; }
bool YCPValueRep::isBlock()       const { return valuetype() == YT_BLOCK; }
bool YCPValueRep::isBuiltin()     const { return valuetype() == YT_BUILTIN; }
bool YCPValueRep::isIdentifier()  const { return valuetype() == YT_IDENTIFIER; }
bool YCPValueRep::isError()	  const { return valuetype() == YT_ERROR; }


// value type conversions

YCPVoid YCPValueRep::asVoid() const
{
    if (!isVoid())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Void!",
	      toString().c_str());
	abort();
    }
    return YCPVoid(static_cast<const YCPVoidRep *>(this));
}

YCPBoolean YCPValueRep::asBoolean() const
{
    if (!isBoolean())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Boolean!",
	      toString().c_str());
	abort();
    }
    return YCPBoolean(static_cast<const YCPBooleanRep *>(this));
}

YCPInteger YCPValueRep::asInteger() const
{
    if (!isInteger())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Integer!",
	      toString().c_str());
	abort();
    }
    return YCPInteger(static_cast<const YCPIntegerRep *>(this));
}

YCPFloat YCPValueRep::asFloat() const
{
    if (!isFloat())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Float!",
	      toString().c_str());
	abort();
    }
    return YCPFloat(static_cast<const YCPFloatRep *>(this));
}

YCPString YCPValueRep::asString() const
{
    if (!isString())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not String!",
	      toString().c_str());
	abort();
    }
    return YCPString(static_cast<const YCPStringRep *>(this));
}

YCPByteblock YCPValueRep::asByteblock() const
{
    if (!isByteblock())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Byteblock!",
	      toString().c_str());
	abort();
    }
    return YCPByteblock(static_cast<const YCPByteblockRep *>(this));
}


YCPPath YCPValueRep::asPath() const
{
    if (!isPath())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Path!",
	      toString().c_str());
	abort();
    }
    return YCPPath(static_cast<const YCPPathRep *>(this));
}

YCPSymbol YCPValueRep::asSymbol() const
{
    if (!isSymbol())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Symbol!",
	      toString().c_str());
	abort();
    }
    return YCPSymbol(static_cast<const YCPSymbolRep *>(this));
}

YCPDeclaration YCPValueRep::asDeclaration() const
{
    if (!isDeclaration())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Declaration!",
	      toString().c_str());
	abort();
    }
    return YCPDeclaration(static_cast<const YCPDeclarationRep *>(this));
}

YCPLocale YCPValueRep::asLocale() const
{
    if (!isLocale())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Locale!",
	      toString().c_str());
	abort();
    }
    return YCPLocale(static_cast<const YCPLocaleRep *>(this));
}

YCPList YCPValueRep::asList() const
{
    if (!isList())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not List!",
	      toString().c_str());
	abort();
    }
    return YCPList(static_cast<const YCPListRep *>(this));
}

YCPTerm YCPValueRep::asTerm() const
{
    if (!isTerm())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Term!",
	      toString().c_str());
	abort();
    }
    return YCPTerm(static_cast<const YCPTermRep *>(this));
}

YCPMap YCPValueRep::asMap() const
{
    if (!isMap())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Map!",
	      toString().c_str());
	abort();
    }
    return YCPMap(static_cast<const YCPMapRep *>(this));
}

YCPBlock YCPValueRep::asBlock() const
{
    if (!isBlock())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Block!",
	      toString().c_str());
	abort();
    }
    return YCPBlock(static_cast<const YCPBlockRep *>(this));
}

YCPBuiltin YCPValueRep::asBuiltin() const
{
    if (!isBuiltin())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Builtin!",
	      toString().c_str());
	abort();
    }
    return YCPBuiltin(static_cast<const YCPBuiltinRep *>(this));
}

YCPIdentifier YCPValueRep::asIdentifier() const
{
    if (!isIdentifier())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Identifier!",
	      toString().c_str());
	abort();
    }
    return YCPIdentifier(static_cast<const YCPIdentifierRep *>(this));
}

YCPError YCPValueRep::asError() const
{
    if (!isError())
    {
	y2error("Invalid cast of YCP value '%s'! Should be but is not Error!",
	      toString().c_str());
	abort();
    }
    return YCPError(static_cast<const YCPErrorRep *>(this));
}

bool YCPValueRep::equal(const YCPValue& v) const
{
    return compare( v ) == YO_EQUAL;
}


YCPOrder YCPValueRep::compare(const YCPValue& v, bool rl) const
{
    if (valuetype() == v->valuetype())
    {
	switch (valuetype()) {
	case YT_VOID:        return this->asVoid()->compare(v->asVoid());
	case YT_BOOLEAN:     return this->asBoolean()->compare(v->asBoolean());
	case YT_INTEGER:     return this->asInteger()->compare(v->asInteger());
	case YT_FLOAT:       return this->asFloat()->compare(v->asFloat());
	case YT_STRING:      return this->asString()->compare(v->asString(), rl);
	case YT_BYTEBLOCK:   return this->asByteblock()->compare(v->asByteblock());
	case YT_PATH:        return this->asPath()->compare(v->asPath());
	case YT_SYMBOL:      return this->asSymbol()->compare(v->asSymbol());
	case YT_DECLARATION: return this->asDeclaration()->compare(v->asDeclaration());
	case YT_LOCALE:      return this->asLocale()->compare(v->asLocale(), rl);
	case YT_LIST:        return this->asList()->compare(v->asList());
	case YT_TERM:        return this->asTerm()->compare(v->asTerm());
	case YT_MAP:         return this->asMap()->compare(v->asMap());
	case YT_BLOCK:       return this->asBlock()->compare(v->asBlock());
	case YT_BUILTIN:     return this->asBuiltin()->compare(v->asBuiltin());
	case YT_IDENTIFIER:  return this->asIdentifier()->compare(v->asIdentifier());
	case YT_ERROR:
	{
	     return this->asError()->value()->compare(v);
	}
	default:
	    y2error("Sorry, comparison of '%s' with '%s' not yet implemented",
		  toString().c_str(), v->toString().c_str());
	    return YO_EQUAL;
	}
    }

#if 0
    y2warning ("Comparison of different type",
	   toString().c_str(), v->toString().c_str());
#endif
    return valuetype() < v->valuetype() ? YO_LESS : YO_GREATER;
}
