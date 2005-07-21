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

  File:	      YDate.cc

  Author:     Anas Nashif <nashif@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YDate.h"


YDate::YDate( const YWidgetOpt & opt,
               const YCPString  & label)
    : YWidget( opt )
    , label ( label )
{
    y2debug( "YDate( %s )", label->value_cstr() );
}


YCPValue YDate::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string s = property->symbol();

    /**
     * @property string Value the date 
     */
    if ( s == YUIProperty_Value )
    {
	if ( newValue->isString() )
	{
	    setNewDate( newValue->asString() );
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


void YDate::setLabel( const YCPString & label )
{
    this->label = label;
}



YCPValue YDate::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_Value )	return getDate();
    else return YWidget::queryWidget( property );
}



