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

  File:	      YIntField.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMacroRecorder.h"
#include "YIntField.h"


YIntField::YIntField( YWidgetOpt & opt,
		      const YCPString & label,
		      int minValue,
		      int maxValue,
		      int initialValue )
    : YWidget( opt )
    , _label( label )
    , _minValue( minValue )
    , _maxValue( maxValue )
    , _value( initialValue )
{
    setDefaultStretchable( YD_HORIZ, true );
    setStretchable( YD_VERT, false );
}


void YIntField::setLabel( const YCPString & newLabel )
{
    _label = newLabel;
}

void YIntField::setValue( int newValue )
{
    _value = newValue;
}


YCPValue YIntField::changeWidget( const YCPSymbol & property,
				  const YCPValue  & newValue )
{
    string sym = property->symbol();

    /**
     * @property integer Value the numerical value
     */
    if ( sym == YUIProperty_Value )
    {
	if ( newValue->isInteger() )
	{
	    int val = newValue->asInteger()->value();

	    if ( val < minValue() )
	    {
		y2warning( "YIntField::changeWidget( `Value ): "
			   "Warning: New value %d below minValue ( %d )",
			   val, minValue() );
		setValue( minValue() );
	    }
	    else if ( val > maxValue() )
	    {
		y2warning( "YIntField::changeWidget( `Value ): "
			   "Warning: New value %d above maxValue ( %d )",
			   val, maxValue() );
		setValue( maxValue() );
	    }
	    else
	    {
		setValue( val );
	    }

	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YIntField::changeWidget( `Value ): "
		     "Error: Expecting integer value, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    /**
     * @property string Label the slider label
     */
    else if ( sym == YUIProperty_Label )
    {
	if ( newValue->isString() )
	{
	    setLabel( newValue->asString() );
	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YIntField::changeWidget( `Value ): "
		     "Error: Expecting string, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YIntField::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if 		( s == YUIProperty_Value )	return YCPInteger( value() );
    else if	( s == YUIProperty_Label ) 	return label();
    else return YWidget::queryWidget( property );
}


void YIntField::saveUserInput( YMacroRecorder *macroRecorder )
{
    macroRecorder->recordWidgetProperty( this, YUIProperty_Value );
}

