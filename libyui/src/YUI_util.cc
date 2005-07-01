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

  File:		YUI_util.cc

  Summary:      utility functions


  Authors:	Stefan Hundhammer <sh@suse.de>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/



#include "YUI_util.h"

#define y2log_component "ui"
#include <ycp/y2log.h>


using std::string;


bool isNum( const YCPValue & val )
{
    if ( val.isNull() )
	return false;

    return ( val->isInteger() || val->isFloat() );
}


float toFloat( const YCPValue & val )
{
    if ( val->isInteger() )
	return (float) val->asInteger()->value();

    if ( val->isFloat() )
	return val->asFloat()->value();

    y2error( "Can't convert this to float: %s",
	     val->toString().c_str() );

    return -1.0;
}


// EOF
