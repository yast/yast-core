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

   File:       YCPDeclType.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDeclType_h
#define YCPDeclType_h


#include "YCPDeclaration.h"

/**
 * @short YCPDeclarationRep, that restricts the type
 * A YCPDeclTypeRep restricts a value in a way, that its type
 * must be constructed by a certain type constructor.
 */
class YCPDeclTypeRep : public YCPDeclarationRep
{
    YCPValueType vt;

protected:
    friend class YCPDeclType;

    /**
     * Constructs a new YCPDeclTypeRep, that allows all values
     * whose type is contructed by the type vt.
     */
    YCPDeclTypeRep(YCPValueType vt);

public:

    /**
     * Returns YD_TYPE. See @ref YCPDeclarationRep#declarationtype.
     */
    YCPDeclarationType declarationtype() const;

     /**
     * Returns true, if value->valuetype() is the
     * type stored in this object.
     */
    bool allows(const YCPValue& value) const;

    /**
     * Returns propagated value, if value->valuetype() propagates
     * to the type stored in this object.
     * Returns YCPNull() else.
     */
    YCPValue propagateTo (const YCPValue& value) const;

    /**
     * Compares two DeclTypes for equality, greaterness or smallerness.
     * @param d DeclType to compare against
     * @return YO_LESS,    if this is smaller than d,
     *         YO_EQUAL,   if this is equal to d,
     *         YO_GREATER, if this is greater to d
     */
    YCPOrder compare(const YCPDeclType &d) const;

    /**
     * Returns a string representation of this object, that may
     * be parsed by the YCP parser. The string representation
     * of a YCPDeclTypeRep is one of the key words for the type
     * constructors, i.e. one out of ycp_typecons_name[].
     */
    string toString() const;

    /**
     * Return the YCPValueType of this declaration.
     */
    YCPValueType declarationvaluetype () const;
};

/**
 * @short Wrapper for YCPDeclTypeRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDeclTypeRep
 * with the arrow operator. See @ref YCPDeclTypeRep.
 */
class YCPDeclType : public YCPDeclaration
{
    DEF_COMMON(DeclType, Declaration);
public:
    YCPDeclType(YCPValueType vt) : YCPDeclaration(new YCPDeclTypeRep(vt)) {}
};

#endif   // YCP_DeclType_h
