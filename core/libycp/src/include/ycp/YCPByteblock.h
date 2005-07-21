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

   File:       YCPByteblock.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPByteblock_h
#define YCPByteblock_h


#include "YCPValue.h"


/**
 * @short YCPValueRep representing a block of bytes
 */
class YCPByteblockRep : public YCPValueRep
{
    /**
     * The byte block
     */
    const unsigned char *bytes;

    /**
     * Length of the byte block
     */ 
    long len;

protected:
    friend class YCPByteblock;

    /**
     * Creates a new YCPByteblockRep object.
     * @param bytes pointer to a buffer containing the bytes. I'll make me a copy of this,
     * please free the memory yourself, if you need to.
     * @param length length of the byte block.
     */
    YCPByteblockRep(const unsigned char *bytes, long len);

    /**
     * Creates a new YCPByteblockRep object from a stream.
     * See YCPByteblock (bytecodeistream &) implementation.
     */
    YCPByteblockRep (bytecodeistream & str, long len);

    /**
     * Cleans up
     */
    ~YCPByteblockRep();

public:
    /**
     * Returns the bytes of the block.
     */
    const unsigned char *value() const;

    /**
     * Returns the number of bytes in the block.
     */
    long size() const;

    /**
     * Returns a string representation of this objects value.
     * Byteblock values are represented in YCP #(byteblockstring), where
     * byteblockstring is some yet to be defined but typical byteblock
     * and date representation.
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;

    /**
     * Compares two bytes blocks.
     */
    YCPOrder compare(const YCPByteblock& s) const;

    /**
     * Returns YT_BYTEBLOCK. See @ref YCPValueRep#type.
     */
    YCPValueType valuetype() const;
};


/**
 * @short Wrapper for YCPByteblockRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPByteblockRep
 * with the arrow operator. See @ref YCPByteblockRep.
 */
class YCPByteblock : public YCPValue
{
    DEF_COMMON(Byteblock, Value);
public:
    YCPByteblock(const unsigned char *r, long l) : YCPValue(new YCPByteblockRep(r, l)) {}
    YCPByteblock(bytecodeistream & str);
};

   
#endif   // YCPByteblock_h
