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

   File:       YCPDeclaration.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPDeclaration_h
#define YCPDeclaration_h


#include "YCPValue.h"


/**
 * @short Declaration Type
 * Defines constants for the Declaration types. The Declaration type specifies
 * the class the YCPDeclarationRep object belongs to.
 */
enum YCPDeclarationType
{
    YD_ANY         = 0,
    YD_TYPE        = 1,
    YD_LIST        = 2,
    YD_STRUCT      = 3,
    YD_TERM        = 4
};

/**
 * @short Type, range and other restriction on a YCPValueRep
 * This is the abstract base class of all declarations. A declaration
 * is an object that represents a restriction on the type and value of a
 * YCPValueRep. It is a more general concept than a mere type declaration.
 * A type declaration is only a special case of a declaration.
 * A YCPDeclarationRep can decide, whether a given YCPValueRep and its implicit
 * type have a certain proberty.
 */
class YCPDeclarationRep : public YCPValueRep
{
public:
    /**
     * @short Declaration Type
     * Defines constants for the Declaration types. The Declaration type specifies
     * the class the YCPDeclarationRep object belongs to.
     */
    virtual YCPDeclarationType declarationtype() const = 0;

    /**
     * Checks, whether the given value v fullfills this range
     * restriction.
     */
    virtual bool allows(const YCPValue&) const = 0;
    
    /**
     * Returns YT_DECLARATION. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;
    
    /**
     * Returns true, if this object is of the type YCPDeclAnyRep.
     */
    bool isDeclAny() const;
    
    /**
     * Returns true, if this object is of the type YCPDeclTypeRep.
     */
    bool isDeclType() const;
    
    /**
     * Returns true, if this object is of the type YCPDeclListRep.
     */
    bool isDeclList() const;
    
    /**
     * Returns true, if this object is of the type YCPDeclStructRep.
     */
    bool isDeclStruct() const;

    /**
     * Returns true, if this object is of the type YCPDeclTermRep.
     */
    bool isDeclTerm() const;

    /**
     * Casts this declaration into a pointer of type const YCPDeclAnyRep *.
     */
    YCPDeclAny asDeclAny() const;

    /**
     * Casts this declaration into a pointer of type const YCPDeclTypeRep *.
     */
    YCPDeclType asDeclType() const;

    /**
     * Casts this declaration into a pointer of type const YCPDeclListRep *.
     */
    YCPDeclList asDeclList() const;

    /**
     * Casts this declaration into a pointer of type const YCPDeclStructRep *.
     */
    YCPDeclStruct asDeclStruct() const;

    /**
     * Casts this declaration into a pointer of type const YCPDeclTermRep *.
     */
    YCPDeclTerm asDeclTerm() const;

    /**
     * Compares two declarations for equality, greaterness or smallerness.
     * @param d declaration to compare against
     * @return YO_LESS,    if this is smaller than d,
     *         YO_EQUAL,   if this is equal to d,
     *         YO_GREATER, if this is greater to d
     */
    YCPOrder compare(const YCPDeclaration &d) const;
};

/**
 * @short Wrapper for YCPDeclarationRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPDeclarationRep
 * with the arrow operator. See @ref YCPDeclarationRep.
 */
class YCPDeclaration : public YCPValue
{
    DEF_COMMON(Declaration, Value);
public:
};

#endif   // YCPDeclaration_h
