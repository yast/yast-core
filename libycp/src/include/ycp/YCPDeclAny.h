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

   File:       YCPDeclAny.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDeclAny_h
#define YCPDeclAny_h


#include "YCPDeclaration.h"


/**
 * @short declaration that allows any value
 * 
 * YCPSyntax: <pre>any</pre>
 */
class YCPDeclAnyRep : public YCPDeclarationRep
{
public:

    /**
     * Returns YD_ANY. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPDeclarationType declarationtype() const;

    /**
     * Returns always true, because this declaration
     * allows any value.
     */
    bool allows(const YCPValue&) const;

    /**
     * Compares two YCPDeclAnys for equality, greaterness or smallerness.
     * @param v value to compare against
     * @return (always YO_EQUAL)
     */
    YCPOrder compare(const YCPDeclAny &v) const;

    /**
     * Returns "any".
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPDeclAnyRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDeclAnyRep
 * with the arrow operator. See @ref YCPDeclAnyRep.
 */
class YCPDeclAny : public YCPDeclaration
{
    DEF_COMMON(DeclAny, Declaration);
public:
    YCPDeclAny() : YCPDeclaration(new YCPDeclAnyRep()) {}
};

#endif   // YCPDeclAny_h
