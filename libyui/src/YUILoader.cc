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

  File:		YUILoader.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/

#include <stdlib.h>		// getenv()
#include <unistd.h>		// isatty()

#include "YUILoader.h"
#include "YUIPlugin.h"
#include "YUIException.h"


void YUILoader::loadUI()
{
    const char * envDisplay = getenv( "DISPLAY" );

    if ( envDisplay )
    {
	//
	// Qt UI
	//
	
	try
	{
	    loadPlugin( YUIPlugin_Qt );
	    return;
	}
	catch ( YUIException & ex)
	{
	    YUI_CAUGHT( ex );
	}
    }

    if ( isatty( STDOUT_FILENO ) )
    {
	//
	// NCurses UI
	//
	
	try
	{
	    loadPlugin( YUIPlugin_NCurses );
	    return;
	}
	catch ( YUIException & ex)
	{
	    YUI_CAUGHT( ex );
	    YUI_RETHROW( ex ); // what else to do here?
	}
    }
    else
    {
	YUI_THROW( YUIException( "No $DISPLAY and stdout is not a tty" ) );
    }
}


void YUILoader::loadPlugin( const string & name )
{
    YUIPlugin uiPlugin( name.c_str() );

    if ( uiPlugin.success() )
    {
	createUIFunction_t createUI = (createUIFunction_t) uiPlugin.locateSymbol( "createUI" );

	if ( createUI )
	{
	    YUI * ui = createUI( false ); // no threads

	    if ( ui )
		return;
	}
    }

    YUI_THROW( YUIPluginException( name ) );
}
