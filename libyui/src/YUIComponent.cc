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
#define y2log_component "ui-component"
#include <ycp/y2log.h>

#include "YUIComponent.h"
#include "YUI.h"


YUI * YUIComponent::_ui = 0;


YUIComponent::YUIComponent()
{
}


YUIComponent::~YUIComponent()
{
    if ( _ui )
	delete _ui;
}


void YUIComponent::setServerOptions( int argc, char **argv )
{
    if ( ! _ui )
    {
	// Evaluate some command line arguments
	
	bool with_threads = true;
	const char * macro_file = 0;

	for ( int i=0; i < argc; i++ )
	{
	    if ( strcmp( argv[i], "--nothreads" ) == 0 )
	    {
		with_threads = false;
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
		    macro_file = argv[++i];
		    y2milestone( "Playing macro '%s' from command line",
				 macro_file ? macro_file : "<NULL>" );
		}
	    }
	}
	
	_ui = createUI( argc, argv, with_threads, macro_file );
    }
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
