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

  File:	      YCheckBox.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YCheckBox.h"


YCheckBox::YCheckBox( const YWidgetOpt & opt, const YCPString & label )
    : YWidget( opt )
    , label( label )
{
}


void YCheckBox::setLabel( const YCPString & label )
{
    this->label = label;
}


YCPString YCheckBox::getLabel()
{
    return label;
}


YCPValue YCheckBox::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();

    if ( s == YUIProperty_Value )
    {
	if ( newvalue->isBoolean() ||
	     newvalue->isVoid()      ) // -> tri-state - neither on nor off
	{
	    setValue( newvalue );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "CheckBox: Invalid parameter %s for property `Value. Must be boolean or nil.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else if ( s == YUIProperty_Label )
    {
	if ( newvalue->isString() )
	{
	    setLabel( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "CheckBox: Invalid parameter %s for property `Label. Must be string.",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newvalue );
}



YCPValue YCheckBox::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if	   ( s == YUIProperty_Value ) return getValue();
    else if ( s == YUIProperty_Label ) return getLabel();
    else return YWidget::queryWidget( property );
}


void YCheckBox::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}

