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

   File:       YCPError.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPError_h
#define YCPError_h

#include "ycp/YCPVoid.h"

/**
 * @short YCPValueRep representing an error.
 * A YCPErrorRep is a YCPValue containing an error message
 * to be printed by the interpreter along with the current
 * file and line information.
 */
class YCPErrorRep : public YCPValueRep
{
    /**
     * The error messsage
     */
    string error_message;

    /**
     * the value to be returned after error reporting
     */
    const YCPValue error_value;

protected:
    friend class YCPError;

    /**
     * Creates a new error.
     */
    YCPErrorRep();

    /**
     * Creates a new error with a message.
     */
    YCPErrorRep (const string& message, const YCPValue& value = YCPVoid());

    /**
     * Cleans up
     */
    ~YCPErrorRep() {}

public:
    /**
     * Returns the error message
     */

    string message() const;

    /**
     * Returns the error value
     */

    const YCPValue value() const;

    /**
     * Returns an ASCII representation of the error.
     */
    string toString() const;

    /**
     * Returns YT_ERROR. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPErrorRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPErrorRep
 * with the arrow operator. See @ref YCPErrorRep.
 */
class YCPError : public YCPValue
{
    DEF_COMMON(Error, Value);
public:
    YCPError() : YCPValue(new YCPErrorRep()) {}
    YCPError(const string& message, const YCPValue& value = YCPVoid())
	: YCPValue(new YCPErrorRep(message, value)) {}
};

#endif   // YCPError_h
