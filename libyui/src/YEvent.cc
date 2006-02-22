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

  File:	      YEvent.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/



#define y2log_component "ui-events"
#include <ycp/y2log.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPString.h>
#include <ycp/YCPInteger.h>
#include "YWidget.h"
#include "YEvent.h"

using std::string;


unsigned long 	YEvent::_nextSerial	= 0;
int		YEvent::_activeEvents	= 0;


YEvent::YEvent( EventType eventType )
	: _eventType( eventType )
{
    _serial = _nextSerial++;

    if ( ++_activeEvents > 3 )
    {
	y2error( "Memory leak? %d active events", _activeEvents );
    }
}


YEvent::~YEvent()
{
    if ( --_activeEvents < 0 )
    {
	y2error( "FATAL: More events deleted than destroyed" );
	abort();
    }
}


const char *
YEvent::toString( EventType eventType )
{
    switch ( eventType )
    {
        case NoEvent:			return "NoEvent";
	case UnknownEvent:		return "UnknownEvent";
	case WidgetEvent:		return "WidgetEvent";
	case MenuEvent:			return "MenuEvent";
	case KeyEvent:			return "KeyEvent";
	case CancelEvent:		return "CancelEvent";
	case TimeoutEvent:		return "TimeoutEvent";
	case DebugEvent:		return "DebugEvent";

	// Intentionally omitting "default" branch so the compiler can
	// detect unhandled enums
    }

    return "<Unknown event type - internal error>";
}


const char *
YEvent::toString( EventReason reason )
{
    switch ( reason )
    {
	case UnknownReason:		return "Unknown";
	case Activated:			return "Activated";
	case SelectionChanged:		return "SelectionChanged";
	case ValueChanged:		return "ValueChanged";

	// Intentionally omitting "default" branch so the compiler can
	// detect unhandled enums
    }

    return "<Unknown event reason - internal error>";
}


YCPValue YEvent::userInput()
{
    return YCPVoid();
}


YCPMap YEvent::ycpEvent()
{
    YCPMap map;

    map->add( YCPString( "EventType" 		), YCPString ( toString( eventType() )	) );
    map->add( YCPString( "EventSerialNo" 	), YCPInteger( serial()			) );

    return map;
}






YWidgetEvent::YWidgetEvent( YWidget *	widget,
			    EventReason	reason, 
			    EventType 	eventType )
    : YEvent( eventType )
    , _widget( widget )
    , _reason( reason )
{
}


YCPMap YWidgetEvent::ycpEvent()
{
    // Use the generic YEvent's map as a base

    YCPMap map = YEvent::ycpEvent();
    map->add( YCPString( "EventReason" ), YCPString ( toString( reason() )	) );

    if ( _widget )
    {
	// Add widget specific info:
	// Add WidgetID

	map->add( YCPString( "ID"	), _widget->id() );
	map->add( YCPString( "WidgetID"	), _widget->id() );	// This is just an alias


	// Add WidgetClass

	const char * widgetClass = _widget->widgetClass();

	if ( widgetClass )
	{
	    if ( *widgetClass == 'Y' )	// skip leading "Y" (YPushButton, YTextEntry, ...)
		widgetClass++;

	    map->add( YCPString( "WidgetClass" ), YCPSymbol( widgetClass ) );
	}


	// Add the Widget's debug label.
	// This is usually the label (translated to the user's locale).

	string debugLabel = _widget->debugLabel();

	if ( ! debugLabel.empty() )
	    map->add( YCPString( "WidgetDebugLabel" ), YCPString( debugLabel ) );
    }

    return map;
}


YCPValue YWidgetEvent::userInput()
{
    return _widget->id();
}






YKeyEvent::YKeyEvent( const string &	keySymbol,
		      YWidget *		focusWidget )
    : YEvent( KeyEvent )
    , _keySymbol( keySymbol )
    , _focusWidget( focusWidget )
{
}


YCPMap YKeyEvent::ycpEvent()
{
    // Use the generic YEvent's map as a base

    YCPMap map = YEvent::ycpEvent();

    if ( ! _keySymbol.empty() )
    {
	map->add( YCPString( "KeySymbol" ), YCPString( _keySymbol ) );
	map->add( YCPString( "ID"	 ), YCPString( _keySymbol ) ); // just an alias
    }

    if ( _focusWidget )
    {
	// Add widget specific info:
	// Add ID of the focus widget

	map->add( YCPString( "FocusWidgetID" ), _focusWidget->id() ); // just an alias


	// Add WidgetClass

	const char * widgetClass = _focusWidget->widgetClass();

	if ( widgetClass )
	{
	    if ( *widgetClass == 'Y' )	// skip leading "Y" (YPushButton, YTextEntry, ...)
		widgetClass++;

	    map->add( YCPString( "FocusWidgetClass" ), YCPSymbol( widgetClass ) );
	}


	// Add the Widget's shortcut property.
	// This is usually the label (translated to the user's locale).

	string debugLabel = _focusWidget->debugLabel();

	if ( ! debugLabel.empty() )
	    map->add( YCPString( "FocusWidgetDebugLabel" ), YCPString( debugLabel ) );
    }

    return map;
}


YCPValue YKeyEvent::userInput()
{
    return YCPString( _keySymbol );
}






YSimpleEvent::YSimpleEvent( EventType 		eventType,
			    const YCPValue &	id )
    : YEvent( eventType )
    , _id( id )
{
}


YSimpleEvent::YSimpleEvent( EventType 		eventType,
			    const char * 	id )
    : YEvent( eventType )
    , _id( YCPSymbol( id ) )
{
}


YSimpleEvent::YSimpleEvent( EventType 		eventType,
			    const string & 	id )
    : YEvent( eventType )
    , _id( YCPSymbol( id.c_str() ) )
{
}


YCPMap YSimpleEvent::ycpEvent()
{
    // Use the generic YEvent's map as a base

    YCPMap map = YEvent::ycpEvent();

    // Add ID
    map->add( YCPString( "ID" ), _id );

    return map;
}


YCPValue YSimpleEvent::userInput()
{
    return _id;
}



