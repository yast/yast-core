/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|      	                        core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

  File:       YLogView.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include <ycp/YCPSymbol.h>
#include <ycp/YCPBoolean.h>
#define y2log_component "ui"
#include <ycp/y2log.h>

#include "YUISymbols.h"
#include "YLogView.h"


YLogView::YLogView( YWidgetOpt & opt, const YCPString & label, int visibleLines, int maxLines)
    : YWidget( opt)
    , _label( label)
    , _visibleLines( visibleLines)
    , _maxLines( maxLines)
{
    setDefaultStretchable( YD_HORIZ, true);
    setDefaultStretchable( YD_VERT,  true);
}


void YLogView::setLabel( const YCPString & newLabel)
{
    _label = newLabel;
}


void YLogView::clearText( )
{
    _logText.clear( );
}


void YLogView::appendText( const YCPString & newText )
{
    string		text 	= newText->value( );
    string::size_type 	from	= 0;
    string::size_type 	to	= 0;


    // Split the text into single lines

    while ( to < text.size( ) )
    {
	from = to;
	to   = text.find( '\n', from );
	if ( to == string::npos )		// no more newline
	    to = text.size( );
	else
	    to++;				// include the newline

	// Output one single line
	appendLine( text.substr( from, to - from ) );
    }

    if ( to < text.size( ) )		// anything left over?
    {
	// Output the rest
	appendLine( text.substr( to, text.size( ) - to ) );
    }

    setLogText( YCPString( logText( ) ) );// pass the entire text to the specific UI's widget
}


void YLogView::appendLine( const string & line )
{
    _logText.push_back( line );

    if ( maxLines( ) > 0 && _logText.size( ) > ( unsigned) maxLines( ) )
    {
	_logText.pop_front( );
    }
}


string YLogView::logText( )
{
    string text;

    for ( unsigned i=0; i < _logText.size( ); i++ )
    {
	text += _logText[i];
    }

    return text;
}



YCPValue YLogView::changeWidget( const YCPSymbol & property, const YCPValue & newValue)
{
    string s = property->symbol( );

    /**
     * @property string Value
     * All log lines. Set this property to replace or clear the entire contents.
     * Can only be set, not queried.
     */
    if ( s == YUIProperty_Value)
    {
	if ( newValue->isString())
	{
	    clearText( );
	    appendText( newValue->asString( ) );
	    return YCPBoolean( true);
	}
	else
	{
	    y2error( "LogView: Invalid Value property - string expected, not %s",
		     newValue->toString( ).c_str() );

	    return YCPBoolean( false);
	}
    }
    /**
     * @property string LastLine
     * The last log line. Set this property to append one or more line( s) to the log.
     * Can only be set, not queried.
     */
    if ( s == YUIProperty_LastLine)
    {
	if ( newValue->isString())
	{
	    appendText( newValue->asString( ) );
	    return YCPBoolean( true);
	}
	else
	{
	    y2error( "LogView: Invalid LastLine property - string expected, not %s",
		     newValue->toString( ).c_str() );

	    return YCPBoolean( false);
	}
    }
    /**
     * @property string Label The label above the log text.
     */
    else if ( s == YUIProperty_Label)
    {
	if ( newValue->isString())
	{
	    setLabel( newValue->asString( ) );
	    return YCPBoolean( true);
	}
	else
	{
	    y2error( "LogView: Invalid Label property - string expected, not %s",
		     newValue->toString( ).c_str() );

	    return YCPBoolean( false);
	}
    }
    else return YWidget::changeWidget( property, newValue );
}



YCPValue YLogView::queryWidget( const YCPSymbol & property)
{
    string s = property->symbol( );
    if ( s == YUIProperty_Label) return label( );
    else return YWidget::queryWidget( property);
}
