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

  File:	      YRichText.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YRichText.h"


YRichText::YRichText( YWidgetOpt & opt, YCPString text )
    : YWidget( opt )
    , text( text )
    , autoScrollDown( false )
{
    // Derived classes need to check opt.plainTextMode and opt.shrinkable!

    if ( opt.autoScrollDown.value() )
	autoScrollDown = true;
    
    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


YCPValue YRichText::changeWidget( const YCPSymbol & property, const YCPValue & newvalue )
{
    string s = property->symbol();
    /*
     * @property string Value the RichText's text contents
     */
    if ( s == YUIProperty_Value )
    {
	if ( newvalue->isString() )
	{
	    setText( newvalue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "RichText: Invalid parameter %s for Value property. Must be string",
		     newvalue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newvalue );
}



YCPValue YRichText::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_Value ) return getText();
    else return YWidget::queryWidget( property );
}


void YRichText::setText( const YCPString & RichText )
{
    text = RichText;
}


YCPString YRichText::getText()
{
    return text;
}

