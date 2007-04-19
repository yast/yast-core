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

using std::string;
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
	KeyEvent,
	CancelEvent,
	TimeoutEvent,
	DebugEvent
    };


    enum EventReason
    {
	UnknownReason = 0,
	Activated,
	SelectionChanged,
	ValueChanged
    };
    

    /**
     * Constructor.
     **/
    YEvent( EventType eventType = UnknownEvent );

    /**
     * Virtual desctructor to force a polymorph object
     * so dynamic_cast can be used  
     **/
    virtual ~YEvent();

    /**
     * Returns the event type.
     **/
    EventType eventType() const { return _eventType; }

    /**
     * Returns the unique serial no. of this event.
     * This is mainly useful for debugging.
     **/
    unsigned long serial() const { return _serial; }

    /**
     * Returns the character representation of an event type.
     **/
    static const char * toString( EventType eventType );
    
    /**
     * Returns the character representation of an event reason.
     **/
    static const char * toString( EventReason reason );
    
    /**
     * Constructs a YCP map to be returned upon UI::WaitForEvent().
     **/
    virtual YCPMap ycpEvent();

    /**
     * Returns the ID to be returned upon UI::UserInput().
     *
     * This is the same as the "id" field of the ycpEvent() map
     * (if this type of event has any such field in its map).
     * It may also be YCPVoid() (nil).
     **/
    virtual YCPValue userInput();

    
protected:

    EventType 			_eventType;
    unsigned long		_serial;
    
    static unsigned long	_nextSerial;
    static int			_activeEvents;
};



class YWidgetEvent: public YEvent
{
public:
    
    /**
     * Constructor.
     **/
    YWidgetEvent( YWidget *	widget		= 0,
		  EventReason	reason		= Activated, 
		  EventType 	eventType	= WidgetEvent );

    /**
     * Returns the widget that caused this event. This might as well be 0 if
     * this is not a widget event.
     **/
    YWidget * widget() const { return _widget; }

    /**
     * Returns the reason for this event. This very much like an event sub-type.
     **/
    EventReason reason() const { return _reason; }

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

    YWidget * 	_widget;
    EventReason	_reason;
};


class YKeyEvent: public YEvent
{
public:
    
    /**
     * Constructor.
     *
     * Create a key event with a specified key symbol (a text describing the
     * key, such as "CursorLeft", "F1", etc.) and optionally the widget that
     * currently has the keyboard focus.
     **/
    YKeyEvent( const string &	keySymbol,
	       YWidget *	focusWidget = 0 );

    /**
     * Returns the key symbol - a text describing the
     * key, such as "CursorLeft", "F1", "a", "A", etc.
     **/
    string keySymbol() const { return _keySymbol; }

    /**
     * Returns the widget that currently has the keyboard focus.
     *
     * This might as well be 0 if no widget has the focus or if the creator of 
     * this event could not obtain that information.
     **/
    YWidget * focusWidget() const { return _focusWidget; }

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

    string	_keySymbol;
    YWidget * 	_focusWidget;
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
    YSimpleEvent( EventType eventType, const string &		id );

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
public:
    
    YMenuEvent( const YCPValue & id )	: YSimpleEvent( MenuEvent, id )	{}
    YMenuEvent( const char *     id )	: YSimpleEvent( MenuEvent, id ) {}
    YMenuEvent( const string & 	 id )	: YSimpleEvent( MenuEvent, id ) {}
};


/**
 * Event to be returned upon closing a dialog with the window manager close
 * button (or Alt-F4)
 **/
class YCancelEvent: public YSimpleEvent
{
public:
    
    YCancelEvent() : YSimpleEvent( CancelEvent, "cancel" ) {}
};


/**
 * Event to be returned upon closing a dialog with the window manager close
 * button (or Alt-F4)
 **/
class YDebugEvent: public YSimpleEvent
{
public:
    
    YDebugEvent() : YSimpleEvent( DebugEvent, "debugHotkey" ) {}
};


/**
 * Event to be returned upon timeout
 * (i.e. no event available in the specified timeout)
 **/
class YTimeoutEvent: public YSimpleEvent
{
public:
    
    YTimeoutEvent() : YSimpleEvent( TimeoutEvent, "timeout" ) {}
};




#endif // YEvent_h
