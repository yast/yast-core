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

   File:       YCP.h

   Author:     Mathias Kettner <kettner@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Basic YCP library
 */

#ifndef YCP_h
#define YCP_h

// include all basic types

/**
 * \page ycpvalues YCP Values
 *
 * YCP values are the primary way of communicating of YaST components. The following values are implemented:
 *
 * <em>Simple values:</em>
 *
 * - \ref YCPNull		no value - typically error during internal communication
 * - \ref YCPVoid		nil value
 * - \ref YCPBoolean		boolean value
 * - \ref YCPInteger		signed 64-bit integer
 * - \ref YCPFloat		double floating point
 * - \ref YCPString		UTF-8 encoded string
 *
 * <em>Special values:</em>
 * - \ref YCPByteblock		array of bytes (e.g. image)
 * - \ref YCPCode		executable code written in YCP language
 * - \ref YCPExternal		pointer
 *
 * <em>Complex values:</em>
 * - \ref YCPList		list/array of YCP values
 * - \ref YCPMap		map/hash of YCP values, key/value pairs
 * - \ref YCPPath		special ordered list of YCP values
 * - \ref YCPTerm		constant structure of YCP values 
 */
#include <ycp/YCPBoolean.h>
#include <ycp/YCPByteblock.h>
#include <ycp/YCPCode.h>
#include <ycp/YCPFloat.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPPath.h>
#include <ycp/YCPString.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPExternal.h>

#endif // YCP_h
