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

   File:       YCPDeclList.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDeclList_h
#define YCPDeclList_h


#include "YCPDeclaration.h"




/**
 * @short YCPDeclarationRep, that only allows homogenous lists.
 * A YCPDeclListRep allows only lists, for which every element
 * complies to a certain given declaration. For example
 * a YCPDeclListRep(new YCPDeclTypeRep(YT_INTEGER)) allows only lists
 * of integer.
 *
 * YCPSyntax: <tt>list(<i>type</i>)</tt>
 * <pre>list( integer )    list(any)</pre>
 */
class YCPDeclListRep : public YCPDeclarationRep
{
    const YCPDeclaration decl;

protected:
    friend class YCPDeclList;

    /**
     * Constructs a new_ YCPDeclListRep object. It allows lists,
     * that contains values fullfilling the declaration
     * rr.
     */
    YCPDeclListRep(const YCPDeclaration& decl);

public:

    /**
     * Returns YD_LIST. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPDeclarationType declarationtype() const;

    /**
     * Returns true, if value is a list whose elements all
     * comply to the stored declaration.
     */
    bool allows(const YCPValue& value) const;

    /**
     * Compares two DeclLists for equality, greaterness or smallerness.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPDeclList& v) const;

    /**
     * Returns a string representation of this object, that may
     * be parsed by the YCP parser. The string representation
     * of a YCPDeclTypeRep is one of the key words for the type
     * constructors, i.e. one out of ycp_typecons_name[].
     * The ASCII representation is list( RR ), where RR is
     * the ASCII representation of the declaration on
     * the list's elements.
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPDeclListRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDeclListRep
 * with the arrow operator. See @ref YCPDeclListRep.
 */
class YCPDeclList : public YCPDeclaration
{
    DEF_COMMON(DeclList, Declaration);
public:
    YCPDeclList(const YCPDeclaration& decl)
	: YCPDeclaration(new YCPDeclListRep(decl)) {}
};

#endif   //YCPDeclList_h
