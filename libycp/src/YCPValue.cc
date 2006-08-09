/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       YCPValue.cc
		YCPValue data type

   Authors:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

#include "ycp/YCPVoid.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPString.h"
#include "ycp/YCPByteblock.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPList.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPExternal.h"


// YCPValueRep

static
const char * names[] = {
    "VOID", "BOOLEAN", "INTEGER", "FLOAT", "STRING", "BYTEBLOCK", "PATH",
    "SYMBOL", "LIST", "TERM", "MAP", "CODE", "RETURN", "BREAK",
    "ENTRY", "ERROR", "REFERENCE", "EXTERNAL",
};

const char *
YCPValueRep::valuetype_str () const
{
    YCPValueType vt = valuetype ();
    if (vt < 0 || (unsigned)vt >= sizeof (names) / sizeof( names[0]))
	return "UNKNOWN";
    return names[vt];
}

// value type checking

bool YCPValueRep::isVoid()        const { return valuetype() == YT_VOID 
						|| valuetype() == YT_RETURN; }
bool YCPValueRep::isBoolean()     const { return valuetype() == YT_BOOLEAN; }
bool YCPValueRep::isInteger()     const { return valuetype() == YT_INTEGER; }
bool YCPValueRep::isFloat()       const { return valuetype() == YT_FLOAT; }
bool YCPValueRep::isString()      const { return valuetype() == YT_STRING; }
bool YCPValueRep::isByteblock()   const { return valuetype() == YT_BYTEBLOCK; }
bool YCPValueRep::isPath()        const { return valuetype() == YT_PATH; }
bool YCPValueRep::isSymbol()      const { return valuetype() == YT_SYMBOL; }
bool YCPValueRep::isList()        const { return valuetype() == YT_LIST; }
bool YCPValueRep::isTerm()        const { return valuetype() == YT_TERM; }
bool YCPValueRep::isMap()         const { return valuetype() == YT_MAP; }
bool YCPValueRep::isCode()	  const { return valuetype() == YT_CODE; }
bool YCPValueRep::isBreak()	  const { return valuetype() == YT_BREAK; }
bool YCPValueRep::isReturn()	  const { return valuetype() == YT_RETURN; }
bool YCPValueRep::isEntry()	  const { return valuetype() == YT_ENTRY; }
bool YCPValueRep::isReference()	  const { return valuetype() == YT_REFERENCE; }
bool YCPValueRep::isExternal()	  const { return valuetype() == YT_EXTERNAL; }


// value type conversions

YCPVoid
YCPValueRep::asVoid() const
{
    if (!isVoid())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Void!",
	      toString().c_str());
	abort();
    }
    return YCPVoid (static_cast<const YCPVoidRep *>(this));
}

YCPBoolean
YCPValueRep::asBoolean() const
{
    if (!isBoolean())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Boolean!",
	      toString().c_str());
	abort();
    }
    return YCPBoolean (static_cast<const YCPBooleanRep *>(this));
}

YCPInteger
YCPValueRep::asInteger() const
{
    if (!isInteger())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Integer!",
	      toString().c_str());
	abort();
    }
    return YCPInteger (static_cast<const YCPIntegerRep *>(this));
}

YCPFloat
YCPValueRep::asFloat() const
{
    if (!isFloat())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Float!",
	      toString().c_str());
	abort();
    }
    return YCPFloat (static_cast<const YCPFloatRep *>(this));
}

YCPString
YCPValueRep::asString() const
{
    if (!isString())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not String!",
	      toString().c_str());
	abort();
    }
    return YCPString (static_cast<const YCPStringRep *>(this));
}

YCPByteblock
YCPValueRep::asByteblock() const
{
    if (!isByteblock())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Byteblock!",
	      toString().c_str());
	abort();
    }
    return YCPByteblock (static_cast<const YCPByteblockRep *>(this));
}


YCPPath
YCPValueRep::asPath() const
{
    if (!isPath())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Path!",
	      toString().c_str());
	abort();
    }
    return YCPPath (static_cast<const YCPPathRep *>(this));
}

YCPSymbol
YCPValueRep::asSymbol() const
{
    if (!isSymbol())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Symbol!",
	      toString().c_str());
	abort();
    }
    return YCPSymbol (static_cast<const YCPSymbolRep *>(this));
}

YCPList
YCPValueRep::asList() const
{
    if (!isList())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not List!",
	      toString().c_str());
	abort();
    }
    return YCPList (static_cast<const YCPListRep *>(this));
}

YCPTerm
YCPValueRep::asTerm() const
{
    if (!isTerm())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Term!",
	      toString().c_str());
	abort();
    }
    return YCPTerm (static_cast<const YCPTermRep *>(this));
}

YCPMap
YCPValueRep::asMap() const
{
    if (!isMap())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Map!",
	      toString().c_str());
	abort();
    }
    return YCPMap (static_cast<const YCPMapRep *>(this));
}

YCPCode
YCPValueRep::asCode() const
{
    if (!isCode())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Code !",
	      toString().c_str());
	abort();
    }
    return YCPCode (static_cast<const YCPCodeRep*>(this));
}

YCPEntry
YCPValueRep::asEntry() const
{
    if (!isEntry())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Entry !",
	      toString().c_str());
	abort();
    }
    return YCPEntry (static_cast<const YCPEntryRep*>(this));
}

YCPReference
YCPValueRep::asReference() const
{
    if (!isReference())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not Reference!",
	      toString().c_str());
	abort();
    }
    return YCPReference (static_cast<const YCPReferenceRep *>(this));
}

YCPExternal
YCPValueRep::asExternal() const
{
    if (!isExternal())
    {
	ycp2error("Invalid cast of YCP value '%s'! Should be but is not External!",
	      toString().c_str());
	abort();
    }
    return YCPExternal (static_cast<const YCPExternalRep *>(this));
}

bool
YCPValueRep::equal (const YCPValue& v) const
{
    return compare (v) == YO_EQUAL;
}


YCPOrder
YCPValueRep::compare (const YCPValue& v, bool rl) const
{
    if (v.isNull())
    {
	ycp2error ("Internal error: YCPValueRep::compare(NULL)");
	return YO_EQUAL;
    }

    if (valuetype() == v->valuetype())
    {
	switch (valuetype()) {
	case YT_VOID:        return this->asVoid()->compare (v->asVoid());
	case YT_BOOLEAN:     return this->asBoolean()->compare (v->asBoolean());
	case YT_INTEGER:     return this->asInteger()->compare (v->asInteger());
	case YT_FLOAT:       return this->asFloat()->compare (v->asFloat());
	case YT_STRING:      return this->asString()->compare (v->asString(), rl);
	case YT_BYTEBLOCK:   return this->asByteblock()->compare (v->asByteblock());
	case YT_PATH:        return this->asPath()->compare (v->asPath());
	case YT_SYMBOL:      return this->asSymbol()->compare (v->asSymbol());
	case YT_LIST:        return this->asList()->compare (v->asList());
	case YT_TERM:        return this->asTerm()->compare (v->asTerm());
	case YT_MAP:         return this->asMap()->compare (v->asMap());
	case YT_CODE:	     return this->asCode()->compare (v->asCode());
	case YT_REFERENCE:   return this->asReference()->compare (v->asReference());
	default:
	    ycp2error("Sorry, comparison of '%s' with '%s' not yet implemented",
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

/**
 * Default constructor, sets the value to YCPNull().
 */
YCPValue::YCPValue ()
    : YCPElement ()
{}

// FIXME: remove this in the future
YCPValue YCPError (string message, const YCPValue & ret)
{
    ycp2error ("%s", message.c_str ());
    return ret;
}
