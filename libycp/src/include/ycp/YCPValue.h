/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       YCPValue.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPValue_h
#define YCPValue_h


#include "YCPElement.h"


/**
 * @short Value Type
 * Defines constants for the Value types. The Value type specifies the class
 * the YCPValueRep object belongs to.
 */
enum YCPValueType {
    YT_UNDEFINED   = -1,
    YT_VOID        = 0,
    YT_BOOLEAN     = 1,
    YT_INTEGER     = 2,
    YT_FLOAT       = 3,
    YT_STRING      = 4,
    YT_BYTEBLOCK   = 5,
    YT_PATH        = 6,
    YT_SYMBOL      = 7,
    YT_DECLARATION = 8,
    YT_LOCALE      = 9,
    YT_LIST        = 10,
    YT_TERM        = 11,
    YT_MAP         = 12,
    YT_BLOCK       = 13,
    YT_BUILTIN	   = 14,
    YT_IDENTIFIER  = 15,
    YT_ERROR	   = 16
};

enum YCPOrder {
    YO_LESS        = -1,
    YO_EQUAL       =  0,
    YO_GREATER     =  1
};

/**
 * @short Abstract base class of all YCP values.
 * Abstract base class of all YCP elements that can be used as primary
 * data, that can be stored in the SCR and that can be parsed as a
 * whole. An important property of a value is, that its ASCII
 * representation uniquely specifies its type. The parser's output is
 * a stream of YCPValues, not of YCPElements.
 **/
class YCPValueRep : public YCPElementRep
{
public:
    /**
     * Returns the type of the value. If you just want to check, whether it is
     * legal to cast an object of the YCPValueRep to a certain more specific
     * object, you should use one of the is... methods.
     */
    virtual YCPValueType valuetype() const = 0;

    /**
     * Checks, if the type of this value is YT_VOID.
     */
    bool isVoid() const;

    /**
     * Checks, if the type of this value is YT_BOOLEAN.
     */
    bool isBoolean() const;

    /**
     * Checks, if the type of this value is YT_INTEGER.
     */
    bool isInteger() const;

    /**
     * Checks, if the type of this value is YT_FLOAT.
     */
    bool isFloat() const;

    /**
     * Checks, if the type of this value is YT_STRING.
     */
    bool isString() const;

    /**
     * Checks, if the type of this value is YT_BYTEBLOCK.
     */
    bool isByteblock() const;

    /**
     * Checks, if the type of this value is YT_PATH.
     */
    bool isPath() const;

    /**
     * Checks, if the type of this value is YT_SYMBOL.
     */
    bool isSymbol() const;

    /**
     * Checks, if the type of this value is YT_DECLARATION.
     */
    bool isDeclaration() const;

    /**
     * Checks, if the type of this value is YT_LOCALE
     */
    bool isLocale() const;

    /**
     * Checks, if the type of this value is YT_LIST
     */
    bool isList() const;

    /**
     * Checks, if the type of this value is YT_TERM. Note that a YCPTermRep
     * also is a YCPListRep and has always also the type VT_LIST.
     */
    bool isTerm() const;

    /**
     * Checks, if the type of this value is YT_MAP.
     */
    bool isMap() const;

    /**
     * Checks, if the type of this value is YT_BLOCK.
     */
    bool isBlock() const;

    /**
     * Checks, if the type of this value is YT_BUILTIN.
     */
    bool isBuiltin() const;

    /**
     * Checks, if the type of this value is YT_IDENTIFIER.
     */
    bool isIdentifier() const;

    /**
     * Checks, if the type of this value is YT_ERROR.
     */
    bool isError() const;

    /**
     * Casts this value into a pointer of type const YCPVoidRep *.
     */
    YCPVoid asVoid() const;

    /**
     * Casts this value into a pointer of type const YCPBooleanRep *.
     */
    YCPBoolean asBoolean() const;

    /**
     * Casts this value into a pointer of type const YCPIntegerRep *.
     */
    YCPInteger asInteger() const;

    /**
     * Casts this value into a pointer of type const YCPFloat .
     */
    YCPFloat asFloat() const;

    /**
     * Casts this value into a pointer of type const YCPString .
     */
    YCPString asString() const;

    /**
     * Casts this value into a pointer of type const YCPByteblock .
     */
    YCPByteblock asByteblock() const;

    /**
     * Casts this value into a pointer of type const YCPPath .
     */
    YCPPath asPath() const;

    /**
     * Casts this value into a pointer of type const YCPSymbol .
     */
    YCPSymbol asSymbol() const;

    /**
     * Casts this value into a pointer of type const YCPDeclaration .
     */
    YCPDeclaration asDeclaration() const;

    /**
     * Casts this value into a pointer of type const YCPLocale .
     */
    YCPLocale asLocale() const;

    /**
     * Casts this value into a pointer of type const YCPList .
     */
    YCPList asList() const;

    /**
     * Casts this value into a pointer of type const YCPTerm .
     */
    YCPTerm asTerm() const;

    /**
     * Casts this value into a pointer of type const YCPMap .
     */
    YCPMap asMap() const;

    /**
     * Casts this value into a pointer of type const YCPBlock .
     */
    YCPBlock asBlock() const;

    /**
     * Casts this value into a pointer of type const YCPBuiltin .
     */
    YCPBuiltin asBuiltin() const;

    /**
     * Casts this value into a pointer of type const YCPIdentifier .
     */
    YCPIdentifier asIdentifier() const;

    /**
     * Casts this value into a pointer of type const YCPError.
     */
    YCPError asError() const;

    /**
     * Compares two YCP values for equality. Two values are equal if
     * they have the same type and the same contents.
     */
    bool equal(const YCPValue&) const;

    /**
     * Compares two YCP values for equality, greaterness or smallerness.
     * You should not compare values of different types.
     * @param v value to compare against
     * @param rl respect locale
     * @return YO_LESS, if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPValue &v, bool rl = false) const;
};


/**
 * @short Wrapper for YCPValueRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPValueRep
 * with the arrow operator. See @ref YCPValueRep.
 */
class YCPValue : public YCPElement
{
    DEF_COMMON(Value, Element);
public:
};

#endif   // YCPValue_h
