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

  File:	      YMultiProgressMeter.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YMultiProgressMeter.h"



YMultiProgressMeter::YMultiProgressMeter( const YWidgetOpt & 	opt,
					  bool 			horizontal,
					  const YCPList & 	maxValues )
    : YWidget( opt )
    , _horizontal( horizontal )
{
    for ( int i=0; i < maxValues->size(); i++ )
    {
	Value_t val = maxValues->value( i )->asInteger()->value();
	_maxValues.push_back( val );
	_currentValues.push_back( val );
    }

    y2milestone( "maxValues: %s", maxValues->toString().c_str() );

    setDefaultStretchable( YD_HORIZ, _horizontal   );
    setDefaultStretchable( YD_VERT,  ! _horizontal );
}


YCPValue YMultiProgressMeter::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string s = property->symbol();

    /**
     * @property string Values the current values for all segments
     */
    if ( s == YUIProperty_Values )
    {
	if ( newValue->isList() )
	{
	    YCPList valList = newValue->asList();

	    if ( valList->size() == segments() )
	    {
		for ( int i=0; i < segments(); i++ )
		{
		    setCurrentValue( i, valList->value( i )->asInteger()->value() );
		}

		y2debug( "Setting values: %s", valList->toString().c_str() );
		doUpdate();	// notify derived classes
		return YCPBoolean( true );
	    }
	    else
	    {
		y2error( "Expected %d values for MultiProgressMeter, not %d (%s)",
			 segments(), valList->size(), valList->toString().c_str() );
	    }
	    
	    return YCPBoolean( false );
	}
	else
	{
	    y2error( "Expected list<integer> for MultiProgressMeter values, not %s",
		     newValue->toString().c_str() );
	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YMultiProgressMeter::queryWidget( const YCPSymbol & property )
{
    string s = property->symbol();
    if ( s == YUIProperty_Values )
    {
	YCPList values;

	for ( int i=0; i < segments(); i++ )
	{
	    Value_t val = ( i < (int) _currentValues.size() ) ? _currentValues[ i ] : -1;
	    values->add( YCPInteger( val ) );
	}

	return values;
    }
    else return YWidget::queryWidget( property );
}


YMultiProgressMeter::Value_t
YMultiProgressMeter::maxValue( int segment ) const
{
    if ( segment >= 0 && segment <= (int) _maxValues.size() )
	return _maxValues[ segment ];
    else
    {
	y2error( "Index %d out of range (0..%zu)", segment, _maxValues.size() -1 );
	return -1;
    }
}


YMultiProgressMeter::Value_t
YMultiProgressMeter::currentValue( int segment ) const
{
    if ( segment >= 0 && segment <= (int) _currentValues.size() )
	return _currentValues[ segment ];
    else
    {
	y2error( "Index %d out of range (0..%zu)", segment, _currentValues.size() -1 );
	return -1;
    }
}


void YMultiProgressMeter::setCurrentValue( int segment, Value_t value )
{
    if ( segment >= 0 && segment <= (int) _currentValues.size() )
    {
	if ( value < 0 )	// Allow -1 etc. as marker values
	    value = 0;
	
	if ( value > maxValue( segment ) )
	{
	    y2error( "Current value %lld for segment #%d exceeds maximum of %lld",
		     value, segment, maxValue( segment ) );

	    value = maxValue( segment );
	}

	_currentValues[ segment ] = value;
	// y2debug( "Segment[ %d ]: %d", segment, value );
    }
    else
    {
	y2error( "Index %d out of range (0..%zu)", segment, _currentValues.size() -1 );
    }
}

