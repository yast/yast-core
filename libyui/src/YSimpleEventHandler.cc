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


YSimpleEventHandler::YSimpleEventHandler()
{
    _pending_event = 0;
}


YSimpleEventHandler::~YSimpleEventHandler()
{
    clear();
}


void YSimpleEventHandler::clear()
{
    if ( _pending_event )
	delete _pending_event;
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

    if ( _pending_event )
    {
	/**
	 * This simple event handler keeps track of only the latest user event.
	 * If there is more than one, older events are automatically discarded.
	 * Since Events are created on the heap with the "new" operator, discarded
	 * events need to be deleted.
	 *
	 * Events that are not discarded are deleted later (after they are
	 * processed) by the generic UI.
	 **/

	delete _pending_event;
    }

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

