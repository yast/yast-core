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

  File:		YUIComponent.cc

  Authors:	Stefan Hundhammer <sh@suse.de>
		
  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#include <string.h>
#include <stdio.h>
#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPValue.h>
#include <ycp/YCPVoid.h>

#include "YUIComponent.h"
#include "YUI.h"


// Most class variables are static so they can be accessed from static methods.

YUI *		YUIComponent::_ui			= 0;
YUIComponent *	YUIComponent::_uiComponent		= 0;


YUIComponent::YUIComponent()
{
    _argc			= 0;
    _argv			= 0;
    _macro_file			= 0;
    _with_threads		= false;
    _have_server_options	= false;

    if ( _uiComponent )
    {
	y2error( "Can't create multiple instances of UI component!" );
	return;
    }
    
    _uiComponent	= this;
}


YUIComponent::~YUIComponent()
{
    if ( _ui )
	delete _ui;
}


YUIComponent * YUIComponent::uiComponent()
{
    return _uiComponent;
}


void YUIComponent::createUI()
{
    if ( ! _have_server_options )
    {
	y2error( "createUI() called before setServerOptions() !" );
	return;
    }

    if ( _ui )
    {
	y2error( "can't create multiple UIs!" );
	return;
    }

    y2debug( "Creating UI" );
    _ui = createUI( _argc, _argv, _with_threads, _macro_file );
}


YCPValue YUIComponent::callBuiltin( void * function, int fn_argc, YCPValue fn_argv[] )
{
    if ( ! _ui )
    {
	y2debug( "Late creation of UI instance" );
	createUI();
    }

    if ( _ui )
	return _ui->callBuiltin( function, fn_argc, fn_argv );
    else
	return YCPVoid();
}


void YUIComponent::setServerOptions( int argc, char **argv )
{
    // Evaluate some command line arguments
	
    _with_threads = true;
    _macro_file	  = 0;

    for ( int i=0; i < argc; i++ )
    {
	if ( strcmp( argv[i], "--nothreads" ) == 0 )
	{
	    _with_threads = false;
	}
	else if ( strcmp( argv[i], "--macro" ) == 0 )
	{
	    if ( i+1 >= argc )
	    {
		y2error( "Missing arg for '--macro'" );
		fprintf( stderr, "y2base: Missing argument for --macro\n" );
		exit( 1 );
	    }
	    else
	    {
		_macro_file = argv[++i];
		y2milestone( "Playing macro '%s' from command line",
			     _macro_file ? _macro_file : "<NULL>" );
	    }
	}
    }

    _argc = argc;
    _argv = argv;
    _have_server_options = true;

    // We only save those values for now. The UI gets instantiated upon the
    // first call to YUIComponent::ui() which will usually happen when the
    // first UI builtin is due to be executed via the call handler (see
    // YUI_bindings.cc).
}


void YUIComponent::result( const YCPValue & result )
{
    if ( _ui )
	delete _ui;
    
    _ui = 0;
}


void YUIComponent::setCallback( Y2Component * callback )
{
    if ( _ui )
	_ui->setCallback( callback );
}


Y2Component * YUIComponent::getCallback() const
{
    if ( _ui )
	return _ui->getCallback();
    else
	return 0;
}


// EOF
