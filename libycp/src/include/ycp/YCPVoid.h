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

   File:       YCPVoid.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPVoid_h
#define YCPVoid_h


#include "YCPValue.h"

/**
 * @short YCPValueRep representing a void value
 *
 * YCP Syntax:
 * 
 *    <pre>nil</pre>
 */
class YCPVoidRep : public YCPValueRep
{
protected:
    friend class YCPVoid;

    /**
     * Creates a new YCPVoidRep
     */
    YCPVoidRep();

public:
    /**
     * Gives the ASCII representation of this value, i.e.
     * "nil"
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Returns YT_VOID. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;

    /**
     * Compares two void for equality, greaterness or smallerness.
     * returns always YO_EQUAL.
     */
    YCPOrder compare(const YCPVoid &v) const;
};

/**
 * @short Wrapper for YCPVoidRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPVoidRep
 * with the arrow operator. See @ref YCPVoidRep.
 */
class YCPVoid : public YCPValue
{
    DEF_COMMON(Void, Value);
public:
    YCPVoid() : YCPValue(new YCPVoidRep()) {}
    YCPVoid(bytecodeistream & str);
};
#endif   // YCPVoid_h
