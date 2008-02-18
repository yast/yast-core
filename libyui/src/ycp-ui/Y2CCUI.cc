/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	Y2CCUI.cc

   Authors:	Stanislav Visnovsky <visnov@suse.cz>
		Stefan Hundhammer <sh@suse.de>

/-*/


#include <string>

#define y2log_component "ui"
#include <ycp/y2log.h>

#include <y2/Y2Component.h>
#include "Y2CCUI.h"
#include "YUIComponent.h"

using std::string;


// Global instances of this class for the Y2ComponentBroker to find.
//
// The Y2Componentbroker will search for global symbols "g_y2cc" + component_name.

Y2CCUI g_y2ccUI;	// UI


Y2CCUI::Y2CCUI()
    : Y2ComponentCreator( Y2ComponentBroker::PLUGIN,
			  true ) // force_register
{
    // Since this component creator resides in the libpy2UI plug-in, it is
    // loaded as a plug-in, i.e. after component creator registration is closed
    // in the component broker: It sets its stop_register flag when the first
    // component or name space is created, so it rejects all attempts for
    // further component creators to register themselves.
    //
    // But since this component creator not only creates the "UI" name space
    // (which is normally the first component of the UIs to be created), but
    // also the UIs themselves ("qt", "ncurses", "gtk"), it needs to be
    // registered in the component broker, so the "force_registration" flag is
    // needed.

    // y2debug( "UI component creator %p constructor", this );
}


Y2Component *
Y2CCUI::provideNamespace( const char * cname )
{
    string name( cname );

    if ( name == "UI" )
    {
	y2debug ("UI library namespace provider tries for '%s'", cname);
	// implementation shortcut: we only provide the UI namespace and the UI component
	return create( cname );
    }
    else
    {
	return 0;
    }
}


Y2Component *
Y2CCUI::create( const char * cname ) const
{
    y2debug( "Requested \"%s\"", cname );
    string name( cname );

    if ( name == "UI" ||
	 name == "qt" ||
	 name == "ncurses"  ||
#if SUPPORT_GTK_UI
	 name == "gtk" ||
#endif
	 name == "ui" )
    {
	if ( name == "UI" || name == "ui" )
	    name = "";		// Automatically choose the appropriate UI

	YUIComponent* uiComponent = YUIComponent::uiComponent();

	if ( ! uiComponent )
	{
	    y2milestone("Creating UI component for \"%s\"", cname );
	    uiComponent = new YUIComponent( name );

	    if ( ! uiComponent )
	    {
		y2error( "Creating UI component \"%s\"s failed", cname );
	    }
	}
	else
	{
	    if ( uiComponent->requestedUIName().empty() && ! name.empty() )
	    {
		uiComponent->setRequestedUIName( name );
	    }

	    y2milestone( "Returning existing UI component for \"%s\"", cname );
	}

	return uiComponent;
    }
    else
    {
	return 0;
    }
}


Y2Component *
Y2CCUI::createInLevel( const char * name, int level, int currentLevel ) const
{
    return create( name );
}


