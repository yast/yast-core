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

  File:	      YEvent.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YEvent_h
#define YEvent_h


#include <string>
#include <ycp/YCPValue.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPSymbol.h>

class YWidget;


/**
 * @short Abstract base class for events to be returned upon UI::UserInput()
 * and related.
 **/
class YEvent
{
public:
    
    enum EventType
    {
	NoEvent = 0,
	UnknownEvent,
	WidgetEvent,
	MenuEvent,
	KeyboardEvent,
	CancelEvent,
	TimeoutEvent,
	DebugEvent
    };
    

    /**
     * Constructor.
     **/
    YEvent( EventType eventType = UnknownEvent );

    /**
     * Virtual desctructor to force a polymorph object
     * so dynamic_cast can be used  
     **/
    virtual ~YEvent() {}

    /**
     * Returns the event type.
     **/
    EventType eventType() const { return _eventType; }

    /**
     * Returns the character representation of an event type.
     **/
    static const char * format( EventType eventType );
    
    /**
     * Constructs a YCP map to be returned upon UI::WaitForEvent().
     **/
    virtual YCPMap ycpEvent();

    /**
     * Returns the ID to be returned upon UI::UserInput().
     *
     * This is the same as the "id" field of the ycpEvent() map.
     **/
    virtual YCPValue userInput();

    
protected:

    EventType _eventType;
};



class YWidgetEvent: public YEvent
{
public:
    
    /**
     * Constructor.
     *
     * If this is a widget event, the widget that caused the event can be
     * passed here. 
     **/
    YWidgetEvent( YWidget *	widget	= 0,
		  EventType 	eventType	= WidgetEvent );

    /**
     * Returns the widget that caused this event. This might as well be 0 if
     * this is not a widget event.
     **/
    YWidget * widget() const { return _widget; }

    /**
     * Constructs a YCP map to be returned upon UI::WaitForEvent().
     *
     * Reimplemented from YEvent.
     **/
    virtual YCPMap ycpEvent();

    /**
     * Returns the ID to be returned upon UI::UserInput().
     * This is the same as the "id" field of the ycpEvent() map.
     *
     * Reimplemented from YEvent.
     **/
    virtual YCPValue userInput();
    
protected:

    YWidget * _widget;
};


/**
 * @short abstract base class for events that just deal with an ID.
 **/
class YSimpleEvent: public YEvent
{
public:
    
    /**
     * Constructors.
     **/
    YSimpleEvent( EventType eventType, const YCPValue & 	id );
    YSimpleEvent( EventType eventType, const char * 		id );
    YSimpleEvent( EventType eventType, const std::string	id );

    /**
     * Returns the ID associated with this event.
     **/
    YCPValue id() const { return _id; }

    /**
     * Constructs a YCP map to be returned upon UI::WaitForEvent().
     *
     * Reimplemented from YEvent.
     **/
    virtual YCPMap ycpEvent();

    /**
     * Returns the ID to be returned upon UI::UserInput().
     * This is the same as the "id" field of the ycpEvent() map.
     *
     * Reimplemented from YEvent.
     **/
    virtual YCPValue userInput();

    
protected:

    YCPValue _id;
};


/**
 * @short Event to be returned upon menu selection
 **/
class YMenuEvent: public YSimpleEvent
{
    YMenuEvent( const YCPValue & id )
	: YSimpleEvent( MenuEvent, id )
	{}
};


/**
 * Event to be returned upon closing a dialog with the window manager close
 * button (or Alt-F4)
 **/
class YCancelEvent: public YSimpleEvent
{
    YCancelEvent( const YCPValue & id ) 
	: YSimpleEvent( CancelEvent, "cancel" )
	{}
};


/**
 * Event to be returned upon closing a dialog with the window manager close
 * button (or Alt-F4)
 **/
class YDebugEvent: public YSimpleEvent
{
    YDebugEvent( const YCPValue & id ) 
	: YSimpleEvent( CancelEvent, "debugHotkey" )
	{}
};




#endif // YEvent_h
