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

   File:       YCPDeclList.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPDeclList data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPList.h"
#include "YCPDeclList.h"




// YCPDeclListRep

YCPDeclListRep::YCPDeclListRep(const YCPDeclaration& decl)
    : decl(decl)
{
}


YCPDeclarationType YCPDeclListRep::declarationtype() const
{
    return YD_LIST;
}


bool YCPDeclListRep::allows(const YCPValue& value) const
{
    // nil assignment is allowed
    if (value->valuetype() == YT_VOID) return true;

    if (!value->isList()) return false;
    const YCPList list = value->asList();

    for (int i=0; i<list->size(); i++)
	if (!decl->allows(list->value(i))) return false;

    return true;
}


YCPOrder YCPDeclListRep::compare(const YCPDeclList& d) const
{
    return decl->compare( d->decl );
}


string YCPDeclListRep::toString() const
{
    return "list(" + decl->toString() + ")";
}
