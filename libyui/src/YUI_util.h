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

  File:	      YUI_util.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YUI_util_h
#define YUI_util_h

#include <ycp/YCPString.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPFloat.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPBoolean.h>


/**
 * Check if a YCPValue is a numerical value (YCPInteger or YCPFloat).
 **/
bool isNum( const YCPValue & val );

/**
 * Convert a numerical YCPValue (YCPInteger or YCPFloat) to float.
 **/
float toFloat( const YCPValue & val );




#endif // YUI_util_h
