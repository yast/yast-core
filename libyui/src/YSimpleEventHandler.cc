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

#include "YEvent.h"
#include "YSimpleEventHandler.h"

using std::string;


#define VERBOSE_EVENTS	1
#define VERBOSE_BLOCK	1


YSimpleEventHandler::YSimpleEventHandler()
{
    _pending_event	= 0;
    _events_blocked	= false;
}


YSimpleEventHandler::~YSimpleEventHandler()
{
    clear();
}


void YSimpleEventHandler::clear()
{
    if ( _pending_event )
    {
#if VERBOSE_EVENTS
	y2milestone( "Clearing pending event: %s", YEvent::toString( _pending_event->eventType() ) );
#endif
	delete _pending_event;
    }
}


YEvent * YSimpleEventHandler::consumePendingEvent()
{
    YEvent * event = _pending_event;
    _pending_event = 0;

    return event;
}


void YSimpleEventHandler::sendEvent( YEvent * event )
{
    if ( ! event )
    {
	y2error( "Ignoring NULL event" );
	return;
    }

    if ( eventsBlocked() )
    {
#if VERBOSE_BLOCK
	y2milestone( "Blocking %s event", YEvent::toString( event->eventType() ) );
#endif
	// Avoid memory leak: The event handler assumes ownership of the newly
	// created event, so we have to clean it up here.
	delete event;
	
	return;
    }
    
    if ( _pending_event )
    {
	/**
	 * This simple event handler keeps track of only the latest user event.
	 * If there is more than one, older events are automatically discarded.
	 * Since Events are created on the heap with the "new" operator,
	 * discarded events need to be deleted.
	 *
	 * Events that are not discarded are deleted later (after they are
	 * processed) by the generic UI.
	 **/

	delete _pending_event;
    }

#if VERBOSE_EVENTS
	y2milestone( "New pending event: %s", YEvent::toString( event->eventType() ) );
#endif
	
    _pending_event = event;
}


bool
YSimpleEventHandler::eventPendingFor( YWidget * widget ) const
{
    YWidgetEvent * event = dynamic_cast<YWidgetEvent *> (_pending_event);

    if ( ! event )
	return false;

    return event->widget() == widget;
}


void YSimpleEventHandler::blockEvents( bool block )
{
#if VERBOSE_BLOCK
    if ( block )	y2milestone( "Blocking events"   );
    else		y2milestone( "Unblocking events" );
#endif
    
    _events_blocked = block;
}
