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

   File:       YCPDeclStruct.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPDeclStruct data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include "y2log.h"
#include "YCPDeclStruct.h"

using std::make_pair;


// YCPDeclStructRep

YCPDeclStructRep::YCPDeclStructRep()
{
    declarations.reserve(16);
}


YCPDeclarationType YCPDeclStructRep::declarationtype() const
{
    return YD_STRUCT;
}


bool YCPDeclStructRep::add(const YCPSymbol& sym, const YCPValue& decl)
{
    declarations.push_back( make_pair( decl->asDeclaration(),sym ));
    return true; // TODO: Check for multiple names
}


bool YCPDeclStructRep::checkSignature(const YCPList& list) const
{
    if (list->size() != (int)(declarations.size())) return false;

    for (int i=0; i<list->size(); i++)
	if (!declarations[i].first->allows(list->value(i))) return false;

    return true;
}


bool YCPDeclStructRep::allows(const YCPValue& value) const
{
    // nil assignment is allowed
    return ((value->valuetype() == YT_VOID)
	    || (value->isList()
		&& checkSignature(value->asList())));
}


int YCPDeclStructRep::size() const
{
    return declarations.size();
}


YCPSymbol YCPDeclStructRep::argumentname(int n) const
{
    return declarations[n].second;
}


YCPDeclaration YCPDeclStructRep::declaration(int n) const
{
    return declarations[n].first;
}


YCPOrder YCPDeclStructRep::compare(const YCPDeclStruct& d) const
{
    int size_this  = size();
    int size_d     = d->size();
    int i          = 0;
    YCPOrder order = YO_EQUAL;


    if ( size_this == size_d )   // (0 == 0) .. (n == n)
    {
	for ( i = 0; i < size_this; i++ )
	{
	    order = declaration(i)->compare( d->declaration(i) );
	    if ( order == YO_LESS || order == YO_GREATER ) return order;
	}

	return YO_EQUAL; // no difference found
    }
    else if ( size_this < size_d ) return YO_LESS;
    else                           return YO_GREATER;
}


string YCPDeclStructRep::toString() const
{
    return "[" + commaList() + "]";
}


string YCPDeclStructRep::commaList() const
{
    if (declarations.size() == 0) return "";

    string ret;

    for (unsigned index=0; index<declarations.size(); index++)
    {
	if (index != 0) ret += ", ";
	ret += declarations[index].first->toString();
	ret += " ";
	ret += argumentname(index)->toString();
    }
    return ret;
}



