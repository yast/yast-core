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

   File:       YCPDeclType.cc

		YCPDeclType data type

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "y2log.h"
#include "YCPDeclType.h"
#include "YCPInteger.h"
#include "YCPFloat.h"
#include "YCPString.h"
#include "YCPLocale.h"


// YCPDeclTypeRep

YCPDeclTypeRep::YCPDeclTypeRep(YCPValueType vt)
    : vt(vt)
{
}


YCPDeclarationType YCPDeclTypeRep::declarationtype() const
{
    return YD_TYPE;
}


bool YCPDeclTypeRep::allows(const YCPValue& value) const
{
    // void is allowed for every type
    // float allows integer, but not vice versa
    return ((value->valuetype() == vt)
	    || (value->valuetype() == YT_VOID));
}


YCPValue YCPDeclTypeRep::propagateTo (const YCPValue& value) const
{
    if ((vt == YT_FLOAT) && (value->valuetype() == YT_INTEGER))
    {
	return YCPFloat (value->asInteger()->value());
    }
    else if ((vt == YT_LOCALE) && (value->valuetype() == YT_STRING))
    {
	return YCPLocale (value->asString()->value());
    }
    else
    {
	return YCPNull();
    }
}


YCPOrder YCPDeclTypeRep::compare(const YCPDeclType& d) const
{
    if ( vt == d->vt ) return YO_EQUAL;
    else return vt < d->vt ? YO_LESS : YO_GREATER;
}


YCPValueType YCPDeclTypeRep::declarationvaluetype () const
{
    return vt;
}


string YCPDeclTypeRep::toString() const
{
    switch (vt) {
    case YT_VOID:        return "void";
    case YT_BOOLEAN:     return "boolean";
    case YT_INTEGER:     return "integer";
    case YT_FLOAT:       return "float";
    case YT_STRING:      return "string";
    case YT_BYTEBLOCK:   return "byteblock";
    case YT_PATH:        return "path";
    case YT_SYMBOL:      return "symbol";
    case YT_DECLARATION: return "declaration";
    case YT_LOCALE:      return "locale";
    case YT_LIST:        return "list";
    case YT_TERM:        return "term";
    case YT_MAP:         return "map";
    case YT_BLOCK:       return "block";
    default:             return "";
    }
}

