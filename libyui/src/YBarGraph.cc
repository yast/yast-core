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

  File:		YBarGraph.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#include <stdio.h>

#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YBarGraph.h"

#define CHECK_INDEX(index)						\
    do									\
    {									\
	if ( (index) < 0  ||						\
	     (index) >= (int) priv->segments.size() )			\
	{								\
	    YUI_THROW( YUIIndexOutOfRangeException(			\
                           (index), /* current */			\
			   0,	    /* min */				\
		           (int) priv->segments.size() - 1  ) ); /* max */ \
	}								\
    } while( 0 )



struct YBarGraphPrivate
{
    YBarGraphPrivate()
	: updatesPending( false )
	, postponeUpdates( false )
	{}

    vector<YBarGraphSegment>	segments;
    bool			updatesPending;
    bool			postponeUpdates;
};




YBarGraph::YBarGraph( YWidget * parent )
    : YWidget( parent )
    , priv( new YBarGraphPrivate() )
{
    YUI_CHECK_NEW( priv );
    setDefaultStretchable( YD_HORIZ, true );
}


YBarGraph::~YBarGraph()
{
    // NOP
}


void
YBarGraph::updateDisplay()
{
    priv->updatesPending = true;

    if ( ! priv->postponeUpdates )
    {
	doUpdate();
	priv->updatesPending = false;
    }
}


void
YBarGraph::addSegment( const YBarGraphSegment & segment )
{
    priv->segments.push_back( segment );
    updateDisplay();
}


void
YBarGraph::deleteAllSegments()
{
    priv->segments.clear();
    updateDisplay();
}


const YBarGraphSegment &
YBarGraph::segment( int segmentIndex ) const
{
    CHECK_INDEX( segmentIndex );

    return priv->segments[ segmentIndex ];
}


int
YBarGraph::segments()
{
    return (int) priv->segments.size();
}


void
YBarGraph::setValue( int segmentIndex, int newValue )
{
    CHECK_INDEX( segmentIndex );

    priv->segments[ segmentIndex ].setValue( newValue );
    updateDisplay();
}


void
YBarGraph::setLabel( int segmentIndex, const string & newLabel )
{
    CHECK_INDEX( segmentIndex );

    priv->segments[ segmentIndex ].setLabel( newLabel );
    updateDisplay();
}


void
YBarGraph::setSegmentColor( int segmentIndex, const YColor & color )
{
    CHECK_INDEX( segmentIndex );

    if ( color.isUndefined() )
	YUI_THROW( YUIException( "Invalid YColor" ) );

    priv->segments[ segmentIndex ].setSegmentColor( color );
    updateDisplay();
}


void
YBarGraph::setTextColor( int segmentIndex, const YColor & color )
{
    CHECK_INDEX( segmentIndex );

    if ( color.isUndefined() )
	YUI_THROW( YUIException( "Invalid YColor" ) );

    priv->segments[ segmentIndex ].setTextColor( color );
    updateDisplay();
}


const YPropertySet &
YBarGraph::propertySet()
{
    static YPropertySet propSet;

    if ( propSet.isEmpty() )
    {
	/*
	 * @property list<integer> Values	The numerical value for each segment.
	 * @property list<string>  Labels	Text label for each segment ('\n' allowed).
	 *					Use %1 as a placeholder for the current value.
	 */
	propSet.add( YProperty( YUIProperty_Values,		YOtherProperty	 ) );
	propSet.add( YProperty( YUIProperty_Labels,		YOtherProperty	 ) );
	propSet.add( YWidget::propertySet() );
    }

    return propSet;
}


bool
YBarGraph::setProperty( const string & propertyName, const YPropertyValue & val )
{
    propertySet().check( propertyName, val.type() ); // throws exceptions if not found or type mismatch

    if      ( propertyName == YUIProperty_Values )	return false; // Needs special handling
    else if ( propertyName == YUIProperty_Labels )	return false; // Needs special handling
    else
    {
	YWidget::setProperty( propertyName, val );
    }

    return true; // success -- no special handling necessary
}


YPropertyValue
YBarGraph::getProperty( const string & propertyName )
{
    propertySet().check( propertyName ); // throws exceptions if not found

    if	    ( propertyName == YUIProperty_Values	)	return YPropertyValue( YOtherProperty );
    else if ( propertyName == YUIProperty_Labels	)	return YPropertyValue( YOtherProperty );
    else
    {
	return YWidget::getProperty( propertyName );
    }
}




YBarGraphMultiUpdate::YBarGraphMultiUpdate( YBarGraph * barGraph )
    : _barGraph ( barGraph )
{
    YUI_CHECK_PTR( barGraph );

    _barGraph->priv->postponeUpdates = true;
}


YBarGraphMultiUpdate::~YBarGraphMultiUpdate()
{
    _barGraph->priv->postponeUpdates = false;

    if ( _barGraph->priv->updatesPending )
	_barGraph->updateDisplay();
}





















#if 0



YCPValue YBarGraph::changeWidget( const YCPSymbol & property, const YCPValue & newValue )
{
    string s = property->symbol();

    /**
     * @property integer-list Values
     * The numerical values of each segment.
     */
    if ( s == YUIProperty_Values )
    {
	if ( newValue->isList() )
	{
	    parseValuesList ( newValue->asList() );
	    doUpdate();

	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YBarGraph::changeWidget( `Values ): list of integers expected, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    /**
     * @property string-list Labels
     * The labels for each segment. "\n" allowed.
     * Use "%1" as a placeholder for the current value.
     */
    if ( s == YUIProperty_Labels )
    {
	if ( newValue->isList() )
	{
	    parseLabelsList ( newValue->asList() );
	    doUpdate();

	    return YCPBoolean( true );
	}
	else
	{
	    y2error( "YBarGraph::changeWidget( `Labels ): list of strings expected, not %s",
		     newValue->toString().c_str() );

	    return YCPBoolean( false );
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



void YBarGraph::parseValuesList( const YCPList & newVal )
{
    _values.clear();

    for ( int i=0; i < newVal->size(); i++ )
    {
	YCPValue val( newVal->value(i) );

	if ( val->isInteger() )
	{
	    _values.push_back( val->asInteger()->value() );
	}
	else
	{
	    y2error( "YBarGraph::parseValueList(): Syntax error: "
		     "Expecting integer, you passed \"%s\"",
		     val->toString().c_str() );

	    _values.push_back(0);
	}
    }
}


void YBarGraph::parseLabelsList( const YCPList & newLabels )
{
    _labels.clear();

    if ( newLabels->size() != segments() )
    {
	y2warning( "YBarGraph::parseLabelsList(): Warning: "
		   "The number of labels ( %d ) is not equal "
		   "to the number of values ( %d )!",
		   segments(), newLabels->size() );
    }

    for ( int i=0; i < newLabels->size(); i++ )
    {
	YCPValue val( newLabels->value(i) );

	if ( i < segments() )
	{
	    if ( val->isString() )
	    {
		_labels.push_back( val->asString()->value() );
	    }
	    else
	    {
		y2error( "YBarGraph::parseLabelsList(): Syntax error: "
			 "Expecting string, you passed \"%s\"",
			 val->toString().c_str() );

		_labels.push_back( "" );
	    }
	}
	else
	{
	    y2error( "YBarGraph::parseLabelsList(): Warning: "
		     "Ignoring excess label \"%s\"",
		     val->toString().c_str() );
	}
    }
}


#endif
