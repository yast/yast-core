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
#include "YWidget.h"
#include "YEvent.h"


YEvent::YEvent( EventType eventType )
	: _eventType( eventType )
{
}

const char *
YEvent::format( EventType eventType )
{
    switch ( eventType )
    {
        case NoEvent:			return "NoEvent";
	case UnknownEvent:		return "UnknownEvent";
	case WidgetEvent:		return "WidgetEvent";
	case MenuEvent:			return "MenuEvent";
	case KeyboardEvent:		return "KeyboardEvent";
	case CancelEvent:		return "CancelEvent";
	case TimeoutEvent:		return "TimeoutEvent";
	case DebugEvent:		return "DebugEvent";

	// Intentionally omitting "default" branch so the compiler can
	// detect unhandled enums
    }

    return "<Unknown - internal error>";
}


YCPValue YEvent::userInput()
{
    // TO DO
    return YCPVoid();
}


YCPMap YEvent::ycpEvent()
{
    // TO DO
    return YCPMap();
}






YWidgetEvent::YWidgetEvent( YWidget *	widget,
			    EventType 	eventType	)
    : YEvent( eventType )
    , _widget( widget )
{
}

YCPMap YWidgetEvent::ycpEvent()
{

}


YCPValue YWidgetEvent::userInput()
{

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
    , _id( YCPSymbol( id, true ) )
{

}


YSimpleEvent::YSimpleEvent( EventType 		eventType,
			    const std::string 	id )
    : YEvent( eventType )
    , _id( YCPSymbol( id.c_str(), true ) )
{

}


YCPMap YSimpleEvent::ycpEvent()
{

}


YCPValue YSimpleEvent::userInput()
{

}



