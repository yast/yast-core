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

   File:       YCPDeclTerm.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * YCPDeclTerm data type
 *
 */

#include "y2log.h"
#include "YCPTerm.h"
#include "YCPDeclTerm.h"



// YCPDeclTermRep

YCPDeclTermRep::YCPDeclTermRep(const YCPSymbol& s)
    : s(s)
{
}



YCPDeclarationType YCPDeclTermRep::declarationtype() const
{
    return YD_TERM;
}


// get the DeclTerm's symbol
YCPSymbol YCPDeclTermRep::symbol() const
{
    return s;
}


// add a declaration to the DeclTerm's declarations
bool YCPDeclTermRep::add(const YCPSymbol& sym, const YCPValue& decl)
{
    return st->add( sym, decl );
}


// get the size of the DeclTerm's declarations
int YCPDeclTermRep::size() const
{
    return st->size();
}


// get the symbol of argument n of the DeclTerm
YCPSymbol YCPDeclTermRep::argumentname(int n) const
{
    return st->argumentname(n);
}


// get the DeclTerm's declaration of number n
YCPDeclaration YCPDeclTermRep::declaration(int n) const
{
    return st->declaration(n);
}


// check if value is valid for this DeclTerm
bool YCPDeclTermRep::allows(const YCPValue& value) const
{
    // nil assignment is allowed
    return ((value->valuetype() == YT_VOID)
	   || (value->isTerm()
	      && (value->asTerm())->symbol()->symbol() == s->symbol()
	      && st->allows(value->asTerm()->args())));
}


//compare DeclTerms
YCPOrder YCPDeclTermRep::compare(const YCPDeclTerm& d) const
{
    YCPOrder order = YO_EQUAL;

    order = s->compare( d->s );

    if ( order == YO_EQUAL ) order = st->compare( d->st );

    return order;
}

// get the DeclTerm's declaration as string
string YCPDeclTermRep::toString() const
{
    return s->toString() + "(" + st->commaList() + ")";
}
