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

  File:	      YBarGraph.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <stdio.h>

#define y2log_component "ui"
#include <stdio.h>
#include <ycp/y2log.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPList.h>
#include <ycp/YCPString.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>

#include "YUISymbols.h"
#include "YBarGraph.h"


YBarGraph::YBarGraph( const YWidgetOpt & opt )
    :YWidget( opt )
{
    setDefaultStretchable( YD_HORIZ, true );
}


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


int YBarGraph::segments()
{
    return _values.size();
}


int YBarGraph::value( int n )
{
    if ( n >= 0 && n < (int) _values.size() )
    {
	return _values[n];
    }
    else
    {
	y2error( "YBarGraph::value(): Invalid index %d ( 0 <= n <= %zd",
		 n, _values.size() );

	return -1;
    }
}


string YBarGraph::label( int n )
{
    if ( n >= 0 && n < (int) _labels.size() )
    {
	return _labels[n];
    }
    else
    {
	if ( n >= 0 && n < (int) _values.size() )
	{
	    // If an existing segment doesn't have a label, use its value
	    // as fallback

	    char buffer[ 20 ];
	    sprintf( buffer, "%d", _values[n] );

	    return string( buffer );
	}
	else
	{
	    y2error( "YBarGraph::label(): Invalid index %d ( 0 <= n <= %zd",
		     n, _labels.size() );
	}

	return string();
    }
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


/*
 * Default update method - does nothing.
 * Overwrite this to do the actual visual update.
 */

void YBarGraph::doUpdate()
{
    // NOP
}

