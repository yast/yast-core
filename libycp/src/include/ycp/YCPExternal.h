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

   File:       YCPExternal.h

   Author:     Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef YCPExternal_h
#define YCPExternal_h


#include "YCPValue.h"


/**
 * @short A blackbox wrapper for handling external values in ycp.
 * The payload is handled as a blackbox and is not owned by
 * the wrapper.
 */
class YCPExternalRep : public YCPValueRep
{
    void * m_payload;
    string m_magic;
    void (*m_destructor)(void *, string);

protected:
    friend class YCPExternal;

    /**
     * Creates a new blackbox
     *
     * @param payload	the data stored
     * @param magic	the magic identification for external entity to
     * 			identify its payload
     */
    YCPExternalRep(void * payload, string magic, void (*destructor)(void *, string) = 0);

    /**
     * Cleans up
     */
    ~YCPExternalRep();

public:
    /**
     */
    void * payload() const;

    /**
     */ 
    string magic () const;

    /**
     * Returns an ASCII representation of the payload.
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     * Generates an error, because it's not possible to
     * store the data in persistent stream.
     */
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;

    /**
     * Returns YT_EXTERNAL. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPExternalRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPExternalRep
 * with the arrow operator. See @ref YCPExternalRep.
 */
class YCPExternal : public YCPValue
{
    DEF_COMMON(External, Value);
public:
    YCPExternal(void * payload, string magic, void (*destructor)(void*, string) = 0) 
	: YCPValue(new YCPExternalRep(payload, magic, destructor)) {}
    YCPExternal(bytecodeistream & str);
};

#endif   // YCPExternal_h
