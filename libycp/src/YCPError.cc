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

   File:       YCPError.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#include "ycp/y2log.h"
#include "ycp/YCPValue.h"
#include "ycp/YCPError.h"


YCPErrorRep::YCPErrorRep()
	: error_message("")
	, error_value (YCPVoid())
{}


YCPErrorRep::YCPErrorRep(const string& message, const YCPValue& value)
	: error_message(message)
	, error_value (value)
{}


string YCPErrorRep::message() const
{
  return error_message;
}


const YCPValue YCPErrorRep::value() const
{
  return error_value;
}


string YCPErrorRep::toString() const
{
  return string ("*** ") + error_message;
}


YCPValueType YCPErrorRep::valuetype() const
{
  return YT_ERROR;
}
