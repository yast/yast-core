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

   File:       YCPFloat.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPFloat_h
#define YCPFloat_h


#include "YCPValue.h"

    
/**
 * @short YCPValueRep representing a floating point number.
 * The precision of the floating point number a YCPFloatRep
 * is representing is not specified, because it would be
 * very difficult to guarantee a certain precision platform
 * independently and because this is not neccessary.
 *
 * YCP Syntax: Like in C. The decimal point is obligatory.
 * <pre>1.0, -0.6, 0.9e-16, ...</pre>
 */
class YCPFloatRep : public YCPValueRep
{
    double v;

protected:
    friend class YCPFloat;

    /**
     * Creates a new YCPFloatRep object with the value given
     * in v.
     */
    YCPFloatRep(double v);

    /**
     * Creates a new YCPFloatRep object from its ASCII representation
     * @param r string like '18.8e-17'
     */
    YCPFloatRep(const char *r);

public:
    /**
     * Returns the value of this object in form of a
     * C value of type double.
     */
    double value() const;

    /**
     * Compares two YCPFloats for equality, greaterness or smallerness.
     * @param v value to compare against
     * @return YO_LESS, if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPFloat &v) const;

    /**
     * Returns an ASCII representation of this value.
     * Note that this must alway contain either a decimal
     * point, or an exponent symbol in order to keep up
     * the axiom, that the syntactical representation of
     * a YCP value uniquely describes its type. Examples: 
     * 0.0, 1e10, -17.0e8
     */ 
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_FLOAT. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPFloatRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPFloatRep
 * with the arrow operator. See @ref YCPFloatRep.
 */
class YCPFloat : public YCPValue
{
    DEF_COMMON(Float, Value);
public:
    YCPFloat(double v) : YCPValue(new YCPFloatRep(v)) {}
    YCPFloat(const char *r) : YCPValue(new YCPFloatRep(r)) {}
    YCPFloat(std::istream & str);
};

#endif   // YCPFloat_h
