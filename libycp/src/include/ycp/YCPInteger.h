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

   File:       YCPInteger.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPInteger_h
#define YCPInteger_h


#include "YCPValue.h"

    
/**
 * @short YCPValueRep representing a 64 bit signed integer
 *
 * YCPSyntax:<pre>-2, 0, 4711, ...<pre>
 */
class YCPIntegerRep : public YCPValueRep
{
    long long v;

protected:
    friend class YCPInteger;

    /**
     * Constructs a new YCPIntegerRep from the value given in v.
     */
    YCPIntegerRep(long long v);

    /**
     * Constructs a new YCPIntegerRep from its ASCII representation.
     *  if valid != NULL, returns validity of string (if it really represents an integer)
     */
    YCPIntegerRep(const char *r, bool *valid);

public:
    /**
     * Returns the value of this object in form of a long long
     * C value.
     */
    long long value() const;

    /**
     * Compares two YCPIntegers for equality, greaterness or smallerness.
     * @param v value to compare against
     * @return YO_LESS,    if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPInteger &v) const;

    /**
     * Gives the ASCII representation of this value, i.e.
     * "1" or "-17" or "327698"
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_INTEGER. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;

};

/**
 * @short Wrapper for YCPIntegerRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPIntegerRep
 * with the arrow operator. See @ref YCPIntegerRep.
 */
class YCPInteger : public YCPValue
{
    DEF_COMMON(Integer, Value);
public:
    YCPInteger(long long v) : YCPValue(new YCPIntegerRep(v)) {}
    YCPInteger(const char *r, bool *valid = NULL) : YCPValue(new YCPIntegerRep(r, valid)) {}
    YCPInteger(bytecodeistream & str);
};

#endif   // YCPInteger_h
