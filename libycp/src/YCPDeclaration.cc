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

   File:       YCPDeclaration.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPDeclaration data type
 *
 */

#include "ycp/y2log.h"

#include "ycp/YCPSymbol.h"

#include "ycp/YCPDeclAny.h"
#include "ycp/YCPDeclType.h"
#include "ycp/YCPDeclList.h"
#include "ycp/YCPDeclStruct.h"
#include "ycp/YCPDeclTerm.h"


// YCPDeclarationRep

YCPValueType YCPDeclarationRep::valuetype() const
{
    return YT_DECLARATION;
}


// declaration type checking

bool YCPDeclarationRep::isDeclAny()    const { return declarationtype() == YD_ANY; }
bool YCPDeclarationRep::isDeclType()   const { return declarationtype() == YD_TYPE; }
bool YCPDeclarationRep::isDeclList()   const { return declarationtype() == YD_LIST; }
bool YCPDeclarationRep::isDeclStruct() const { return declarationtype() == YD_STRUCT; }
bool YCPDeclarationRep::isDeclTerm()   const { return declarationtype() == YD_TERM; }


// declaration type conversions

YCPDeclAny YCPDeclarationRep::asDeclAny() const 
{ 
    if (!isDeclAny())
    {
	y2error ("Invalid cast of YCP declaration %s! Should be but is not DeclAny!", toString().c_str());
	abort();
    }
    return YCPDeclAny(static_cast<const YCPDeclAnyRep *>(this)); 
}


YCPDeclType YCPDeclarationRep::asDeclType() const 
{ 
    if (!isDeclType())
    {
	y2error ("Invalid cast of YCP declaration %s! Should be but is not DeclType!", toString().c_str());
	abort();
    }
    return YCPDeclType(static_cast<const YCPDeclTypeRep *>(this)); 
}


YCPDeclList YCPDeclarationRep::asDeclList() const 
{ 
    if (!isDeclList())
    {
	y2error ("Invalid cast of YCP declaration %s! Should be but is not DeclList!", toString().c_str());
	abort();
    }
    return YCPDeclList(static_cast<const YCPDeclListRep *>(this)); 
}


YCPDeclStruct YCPDeclarationRep::asDeclStruct() const 
{ 
    if (!isDeclStruct())
    {
	y2error ("Invalid cast of YCP declaration %s! Should be but is not DeclStruct!", toString().c_str());
	abort();
    }
    return YCPDeclStruct(static_cast<const YCPDeclStructRep *>(this)); 
}


YCPDeclTerm YCPDeclarationRep::asDeclTerm() const 
{ 
    if (!isDeclTerm())
    {
	y2error ("Invalid cast of YCP declaration %s! Should be but is not DeclTerm!", toString().c_str());
	abort();
    }
    return YCPDeclTerm(static_cast<const YCPDeclTermRep *>(this)); 
}


// comparison function
YCPOrder YCPDeclarationRep::compare(const YCPDeclaration& d) const
{
    if (declarationtype() == d->declarationtype())
    {
	switch (declarationtype())
	{
	case YD_ANY:        return this->asDeclAny()->compare(d->asDeclAny());
	case YD_TYPE:       return this->asDeclType()->compare(d->asDeclType());
	case YD_LIST:       return this->asDeclList()->compare(d->asDeclList());
	case YD_STRUCT:     return this->asDeclStruct()->compare(d->asDeclStruct());
	case YD_TERM:       return this->asDeclTerm()->compare(d->asDeclTerm());
	default: 
	    y2error ("Sorry, comparison of %s with %s not yet implemented", toString().c_str(), d->toString().c_str());
	    return YO_EQUAL;
	}
    }
    else
	return declarationtype() < d->declarationtype() ? YO_LESS : YO_GREATER;
}

