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

  File:	      YColoredLabel.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YColoredLabel.h"


YColoredLabel::YColoredLabel( YWidgetOpt & opt, YCPString text )
    : YWidget( opt )
    , text( text )
{
    // y2debug( "YColoredLabel( %s )", text->value_cstr() );
}


YCPValue YColoredLabel::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string s = property->symbol();

    /**
     * @property string Value the label text
     */
    if ( s == YUIProperty_Value )
    {
	if ( newValue->isString() )
	{
	    setLabel( newValue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "Label: Invalid parameter %s for Value property. Must be string",
		     newValue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YColoredLabel::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_Value )	return getLabel();
    else return YWidget::queryWidget( property );
}


void YColoredLabel::setLabel( const YCPString & label )
{
    text = label;
}


YCPString YColoredLabel::getLabel()
{
    return text;
}


