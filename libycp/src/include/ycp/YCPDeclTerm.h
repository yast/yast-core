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

   File:       YCPDeclTerm.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDeclTerm_h
#define YCPDeclTerm_h


#include "YCPDeclStruct.h"



/**
 * @short Object representing a term declaration
 * A term declaration is a structure declaration plus
 * a symbol name.
 */
class YCPDeclTermRep : public YCPDeclarationRep
{
    const YCPSymbol s;
    YCPDeclStruct st;

protected:
    friend class YCPDeclTerm;

    /**
     * Creates a new and empty term declaration. The term's
     * symbol is given with s.
     */
    YCPDeclTermRep(const YCPSymbol& s);

    /**
     * Deletes all components.
     */
    ~YCPDeclTermRep() {}

public:

    /**
     * Returns YD_ANY. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPDeclarationType declarationtype() const;

    /**
     * Alters this DeclTerm's declaration such that it allows
     * a list that is one element larger and that element
     * complies to rr. If symbol is not 0, then the new
     * element can be accessed by the given symbol name.
     */
    bool add(const YCPSymbol& sym, const YCPValue& decl);

    /**
     * @return the number of arguments of the declaration.
     */
    int size() const;

    /**
     * Returns the symbol of argument n of the DeclTerm
     * @param n
     */
    YCPSymbol argumentname(int n) const;

    /**
     * Returns the DeclTerm's declaration of argument n
     * @param n
     */
    YCPDeclaration declaration(int n) const;

    /**
     * Returns true, if value is a term, whose symbol and whose
     * elements exactly match this declaration.
     */
    bool allows(const YCPValue& value) const;

    /**
     * Returns the symbol of this term declaration
     */
    YCPSymbol symbol() const;

    /**
     * Compares two YCPDeclTerms for equality, greaterness or smallerness.
     * Comparison is done as follows:
     *   Compare the symbols of the DeclTerms.
     *   If the symbols are equal compare the DeclStructs of the DeclTerms.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater than v
     * 
     */
    YCPOrder compare(const YCPDeclTerm &v) const;

    /**
     * Returns a string representation of this object, that may
     * be parsed by the YCP parser. The string representation
     * of a YCPDeclTermRep is a symbol name followed by the
     * ASCII representation of the underlying YCPDeclStructRep,
     * for example WizardWindow(title: text, size: integer(1..3)).
     */
    string toString() const;
};

/**
 * @short Wrapper for YCPDeclTermRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDeclTermRep
 * with the arrow operator. See @ref YCPDeclTermRep.
 */
class YCPDeclTerm : public YCPDeclaration
{
    DEF_COMMON(DeclTerm, Declaration);
public:
    YCPDeclTerm(const YCPSymbol& s) : YCPDeclaration(new YCPDeclTermRep(s)) {}
};

#endif   // YCPDeclTerm_h
