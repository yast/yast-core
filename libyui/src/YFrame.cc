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

  File:	      YFrame.cc

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>
#include "YUISymbols.h"
#include "YFrame.h"
#include "YShortcut.h"


YFrame::YFrame( YWidgetOpt &opt, const YCPString &newLabel)
    : YContainerWidget( opt)
    , label( YCPString( YShortcut::cleanShortcutString( newLabel->value( ) ) ) )
{
}


void YFrame::setLabel( const YCPString &newLabel )
{
    this->label = newLabel;
}


YCPString YFrame::getLabel( )
{
    return label;
}


YCPValue YFrame::changeWidget( const YCPSymbol & property, const YCPValue & newValue)
{
    string s = property->symbol( );
   
    /**
     * @property string Value the label text
     */
    if ( s == YUIProperty_Value )
    {
	if ( newValue->isString( ) )
	{
	    string new_label = newValue->asString( )->value();
	    new_label = YShortcut::cleanShortcutString( new_label );
	    setLabel( YCPString( new_label ) );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "Frame: Invalid parameter %s for Value property. Must be string",
		    newValue->toString( ).c_str());
	    return YCPBoolean( false);
	}
    }
    else return YWidget::changeWidget( property, newValue);
}


YCPValue YFrame::queryWidget( const YCPSymbol & property)
{
    string s = property->symbol( );
    if ( s == YUIProperty_Label) return getLabel( );
    else return YWidget::queryWidget( property);
}
