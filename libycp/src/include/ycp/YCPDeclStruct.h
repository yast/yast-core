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

   File:       YCPDeclStruct.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDeclStruct_h
#define YCPDeclStruct_h


#include "YCPSymbol.h"
#include "YCPList.h"
#include "YCPDeclaration.h"




/**
 * @short A declaration, that allows structs/tuples.
 * As you see in @ref YCPListRep, structs and tuples are the same
 * in YCP. With a declaration of the type YCPDeclStructRep, you
 * can restrict the value to be a list with a fixed number of
 * elements that comply to a fixed list of declarations.
 * Furthermore a YCPDeclStructRep may be used to define a mapping
 * from element names to list indices. This allows you to
 * access list member with names and thus have a struct with
 * named elements. Strictly spoken, all elements _must_ have
 * a name.
 */
class YCPDeclStructRep : public YCPDeclarationRep
{
    vector< pair<YCPDeclaration,YCPSymbol> > declarations;

protected:
    friend class YCPDeclStruct;

    /**
     * Constructs a declaration, that allows
     * empty lists.
     */
    YCPDeclStructRep();

    /**
     * Deletes all components
     */
    ~YCPDeclStructRep() {}

public:

    /**
     * Returns YD_STRUCT. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPDeclarationType declarationtype() const;

    /**
     * Alters this declaration such that it allows
     * a list that is one element larger and that element
     * complies to rr. If symbol is not 0, then the new
     * element can be accessed by the given symbol name.
     *
     * The struct declaration ( age: integer(0..), name: string) will
     * yield the followind code:
     *
     * YCPDeclStruct rrs = new YCPDeclStructRep();
     *
     * rrs->add(new YCPDeclTypeRep(YT_STRING), new YCPSymbolRep("name"));
     *
     * @param decl A value of type declaration. If decl is no declaration
     *  then a runtime error is triggered
     * @return false, if the symbol is already used. The element name must be unique.
     */
    bool add(const YCPSymbol& sym, const YCPValue& decl);

    /**
     * Gives access to the struct member of list that has a given name.
     */
    YCPValue member(const YCPList& list, string name) const;

    /**
     * @return the number of arguments of this declaration.
     */
    int size() const;

    /**
     * Returns the symbol of argument n
     * @param n
     */
    YCPSymbol argumentname(int n) const;

    /**
     * Returns the declaration of argument n
     * @param n
     */
    YCPDeclaration declaration(int n) const;

    /**
     * Returns true, value is a list, whose elements excacly
     * match the declaration stored in the object.
     */
    bool allows(const YCPValue& value) const;

    /**
     * Compares two DeclStructs for equality, greaterness or smallerness.
     * Comparison is done as follows:
     * Shorter length is smaller.
     * On equal lengths the declaration components (first) are compared.
     * The argument component is ignored since it is only a syntax dummy.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     *
     */
    YCPOrder compare(const YCPDeclStruct &v) const;

    /**
     * Returns a string representation of this object, that may
     * be parsed by the YCP parser. The string representation
     * of a YCPDeclStructRep is a comma separated list of range
     * restriction representations enclosed by brackets, for
     * example ( boolean, content: list(any) )
     */
    string toString() const;

    /**
     * Creates a comma separated string representation of
     * the member names and declarations.
     */
    string commaList() const;

protected:
    /**
     * Helper function used by this class and by @ref YCPDeclTermRep
     * that checks, whether the elements of a list fulfill the declarations
     * stored in this object.
     */
    bool checkSignature(const YCPList& list) const;
};

/**
 * @short Wrapper for YCPDeclStructRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDeclStructRep
 * with the arrow operator. See @ref YCPDeclStructRep.
 */
class YCPDeclStruct : public YCPDeclaration
{
    DEF_COMMON(DeclStruct, Declaration);
public:
    YCPDeclStruct() : YCPDeclaration(new YCPDeclStructRep()) {}
};

#endif   // YCPDeclStruct
