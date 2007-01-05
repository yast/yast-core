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

  File:	      YCheckBoxFrame.cc

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YCheckBoxFrame.h"


YCheckBoxFrame::YCheckBoxFrame( const YWidgetOpt & opt,
				const YCPString  & label )
    : YContainerWidget( opt )
      , _label( label )
{
    _autoEnable 	= ! opt.noAutoEnable.value();
    _invertAutoEnable	= opt.invertAutoEnable.value();
    _debugLabelWidget   = this;
}



void YCheckBoxFrame::setLabel( const YCPString & label )
{
    _label = label;
}


YCPString YCheckBoxFrame::getLabel()
{
    return _label;
}


void YCheckBoxFrame::handleChildrenEnablement( bool enabled )
{
    if ( autoEnable() )
    {
	if ( invertAutoEnable() )
	    enabled = ! enabled;

	setChildrenEnabling( enabled );
    }
}


YCPValue YCheckBoxFrame::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string sym = property->symbol();

    if ( sym == YUIProperty_Value )
    {
	if ( newValue->isBoolean() )
	{
	    setValue( newValue->asBoolean()->value() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "CheckBoxFrame: Invalid parameter %s for property `Value. Must be boolean.",
		     newValue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else if ( sym == YUIProperty_Label )
    {
	if ( newValue->isString() )
	{
	    setLabel( newValue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "CheckBoxFrame: Invalid parameter %s for property `Label. Must be string.",
		     newValue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YCheckBoxFrame::queryWidget( const YCPSymbol & property )
{
    string sym = property->symbol();
    if	    ( sym == YUIProperty_Value ) return YCPBoolean( getValue() );
    else if ( sym == YUIProperty_Label ) return getLabel();
    else return YWidget::queryWidget( property );
}


void YCheckBoxFrame::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}


