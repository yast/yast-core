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

  File:		YWidgetID.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#include "YWidgetID.h"


YStringWidgetID::YStringWidgetID( const string & val )
    : _value( val )
{
    // NOP
}


YStringWidgetID::~YStringWidgetID()
{
    // NOP
}


bool
YStringWidgetID::isEqual( YWidgetID * otherID ) const
{
    YStringWidgetID * otherStringID =
	dynamic_cast<YStringWidgetID *> (otherID);

    if ( otherStringID )
	return ( this->valueConstRef() == otherStringID->valueConstRef() );
    else
	return false;
}


string
YStringWidgetID::toString() const
{
    return _value;
}


string
YStringWidgetID::value() const
{
    return _value;
}


const string &
YStringWidgetID::valueConstRef() const
{
    return _value;
}

