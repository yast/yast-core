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

   File:       YCPBoolean.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPBoolean_h
#define YCPBoolean_h


#include "YCPValue.h"


/**
 * @short YCPValueRep representing a boolean value
 *
 * YCP Syntax:
 * <pre>true | false</pre>
 */
class YCPBooleanRep : public YCPValueRep
{
    bool v;

protected:
    friend class YCPBoolean;

    /**
     * Creates a new YCPBooleanRep of value v.
     */
    YCPBooleanRep(bool v);

    /**
     * Creates a new YCPBooleanRep with a value given by
     * the string representation, i.e. either "true"
     * or "false".
     */
    YCPBooleanRep(const char *r);

public:
    /**
     * Returns the value of this YCPBooleanRep in form
     * of a bool value.
     */
    bool value() const;

    /**
     * Compares two YCPBooleans for equality, greaterness or smallerness.
     * @param v value to compare against
     * @return YO_LESS, if this is false and v is true,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is true and v is false.
     */
    YCPOrder compare(const YCPBoolean &) const;

    /**
     * Gives the ASCII representation of this value, i.e.
     * "true" or "false".
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_BOOLEAN. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPBooleanRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPBooleanRep
 * with the arrow operator. See @ref YCPBooleanRep.
 */
class YCPBoolean : public YCPValue
{
    DEF_COMMON(Boolean, Value);
    
    static YCPBoolean* trueboolean;
    static YCPBoolean* falseboolean;
    
public:
    YCPBoolean(bool v);
    YCPBoolean(const char *r) : YCPValue(new YCPBooleanRep(r)) {}
    YCPBoolean(bytecodeistream & str);
};

#endif	// YCPBoolean_h
