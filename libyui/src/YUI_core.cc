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

  File:		YUI_core.cc

		Core functions of the UI component
		

  Authors:	Mathias Kettner <kettner@suse.de>
		Stefan Hundhammer <sh@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>
		
  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#define noVERBOSE_COMM		// VERY verbose thread communication logging
#define noVERBOSE_FIND_RADIO_BUTTON_GROUP

#include <stdio.h>
#include <unistd.h> // pipe()
#include <fcntl.h>  // fcntl()
#include <errno.h> 
#include <locale.h> // setlocale()
#include <pthread.h>
#include <assert.h>
#include <string.h>

#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPVoid.h>

#include "YUI.h"
#include "YUISymbols.h"
#include "YWidget.h"

#include "YDialog.h"
#include "YRadioButton.h"
#include "YRadioButtonGroup.h"
#include "YReplacePoint.h"


typedef YCPValue (*v2) ();
typedef YCPValue (*v2v)     (const YCPValue &);
typedef YCPValue (*v2vv)    (const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvv)   (const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvv)  (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);


bool YUI::_reverseLayout = false;


YUI::YUI( bool with_threads )
    : id_counter(0)
    , with_threads( with_threads )
    , _moduleName( "yast2" )
    , _productName( "SuSE Linux" )
    , macroRecorder (0)
    , macroPlayer (0)
    , _events_blocked( false )
    , _callback( 0 )
{
}


YUI::~YUI()
{
    if ( with_threads )
    {
	terminateUIThread();
	close( pipe_to_ui[0] );
	close( pipe_to_ui[1] );
	close( pipe_from_ui[0] );
	close( pipe_from_ui[1] );
    }

    while ( dialogstack.size() > 0 )
    {
	removeDialog();
    }

    deleteMacroRecorder();
    deleteMacroPlayer();
}


void
YUI::internalError( const char *msg )
{
    fprintf( stderr, "YaST2 UI internal error: %s", msg );
}


void YUI::topmostConstructorHasFinished()
{
    // The ui thread must not be started before the constructor
    // of the actual user interface is finished. Otherwise there
    // is a race condition. The ui thread would go into idleLoop()
    // before the ui is really setup. For example the Qt interface
    // does a processOneEvent in the idleLoop(). This may do a
    // repaint operation on the dialog that is just under construction!

    // Therefore the creation of the thread is delayed and
    // performed in this method. It must be called at the end of the constructor
    // of the ui ( YUIQt, YUINcurses ).

    if ( with_threads )
    {
	pipe( pipe_from_ui );
	pipe( pipe_to_ui );

	// Make fd non blockable the ui thread reads from
	long arg;
	arg = fcntl( pipe_to_ui[0], F_GETFL );
	if ( fcntl( pipe_to_ui[0], F_SETFL, arg | O_NONBLOCK ) < 0 )
	{
	    y2error( "Couldn't set O_NONBLOCK: errno=%d: %m", errno );
	    with_threads = false;
	    close( pipe_to_ui[0] );
	    close( pipe_to_ui[1] );
	    close( pipe_from_ui[0] );
	    close( pipe_from_ui[1] );
	}
	else
	{
	    terminate_ui_thread = false;
	    createUIThread();
	}
    }
    else
    {
	y2milestone( "Running without threads" );
    }
}


void YUI::createUIThread()
{
    pthread_attr_t attr;
    pthread_attr_init( & attr );
    pthread_create( & ui_thread, & attr, start_ui_thread, this );
}


void YUI::terminateUIThread()
{
    y2debug( "Telling UI thread to shut down" );
    terminate_ui_thread = true;
    signalUIThread();
    pthread_join( ui_thread, 0 );
    y2debug( "UI thread shut down correctly" );
}


YCPValue YUI::callBuiltin( void * function, int argc, YCPValue argv[] )
{
    YCPValue ret = YCPVoid();
    
    if ( with_threads )
    {
	_builtinCallData.function	= function;
	_builtinCallData.argc		= argc;
	_builtinCallData.argv		= argv;
	
	signalUIThread();
	
	while ( ! waitForUIThread() )
	{
	    // NOP
	}

	ret = _builtinCallData.result;
    }
    else
    {
	ret = callFunction( function, argc, argv );
    }

    return ret;
}


YCPValue YUI::callFunction( void * function, int argc, YCPValue argv[] )
{
    if ( function == 0 )
    {
	y2error( "NULL function pointer!" );
	return YCPNull();
    }

    YCPValue ret = YCPVoid();

    switch ( argc )
    {
	case 0:
	    ret = (*(v2) function) ();
	    break;
	case 1:
	    ret = (*(v2v) function) (argv[0]);
	    break;
	case 2:
	    ret = (*(v2vv) function) (argv[0], argv[1]);
	    break;
	case 3:
	    ret = (*(v2vvv) function) (argv[0], argv[1], argv[2]);
	    break;
	case 4:
	    ret = (*(v2vvvv) function) (argv[0], argv[1], argv[2], argv[3]);
	    break;
	case 5:
	    ret = (*(v2vvvvv) function) (argv[0], argv[1], argv[2], argv[3], argv[4]);
	    break;
	default:
	    {
		y2error( "Bad builtin: Arg count %d", argc );
		ret = YCPNull();
	    }
	    break;
    }
    
    return ret;
}


void YUI::signalUIThread()
{
    static char arbitrary = 42;
    write ( pipe_to_ui[1], & arbitrary, 1 );

#ifdef VERBOSE_COMM
    y2debug( "Wrote byte to ui thread %d", pipe_to_ui[1] );
#endif
}


bool YUI::waitForUIThread()
{
    char arbitrary;
    int res;

    do {
#ifdef VERBOSE_COMM
	y2debug ( "Waiting for ui thread..." );
#endif
	res = read( pipe_from_ui[0], & arbitrary, 1 );
	if ( res == -1 )
	{
	    if ( errno == EINTR || errno == EAGAIN )
		continue;
	    else
		y2error ( "waitForUIThread: %m" );
	}
    } while ( res == 0 );

#ifdef VERBOSE_COMM
    y2debug ( "Read byte from ui thread" );
#endif

    // return true if we really did get a signal byte
    return res != -1;
}


void YUI::signalYCPThread()
{
    static char arbitrary;
    write( pipe_from_ui[1], & arbitrary, 1 );

#ifdef VERBOSE_COMM
    y2debug( "Wrote byte to ycp thread %d", pipe_from_ui[1] );
#endif
}


bool YUI::waitForYCPThread()
{
    char arbitrary;

    int res;
    do {
#ifdef VERBOSE_COMM
	y2debug ( "Waiting for ycp thread..." );
#endif
	res = read( pipe_to_ui[0], & arbitrary, 1 );
	if ( res == -1 )
	{
	    if ( errno == EINTR || errno == EAGAIN )
		continue;
	    else
		y2error ( "waitForYCPThread: errno=%d: %m", errno );
	}
    } while ( res == 0 );

#ifdef VERBOSE_COMM
    y2debug ( "Read byte from ycp thread" );
#endif

    // return true if we really did get a signal byte
    return res != -1;
}


void YUI::uiThreadMainLoop()
{
    while ( true )
    {
	idleLoop ( pipe_to_ui[0] );

	// The pipe is non-blocking, so we have to check if we really read a
	// signal byte. Although idleLoop already makes a select, this seems to
	// be necessary.  Anyway: Why do we set the pipe to non-blocking if we
	// wait in idleLoop for it to become readable? It is needed in
	// YUIQt::idleLoop for QSocketNotifier.
	
	if ( ! waitForYCPThread () )
	    continue;

	if ( terminate_ui_thread )
	    return;

	// callFunction() checks for NULL function pointers

	_builtinCallData.result = callFunction( _builtinCallData.function, 
						_builtinCallData.argc, 
						_builtinCallData.argv );
	signalYCPThread();
    }
}


// ----------------------------------------------------------------------
// Default implementations for the virtual methods the deal with
// event processing


void YUI::idleLoop( int fd_ycp )
{
    // Just wait for fd_ycp to become readable
    fd_set fdset;
    FD_ZERO( & fdset );
    FD_SET( fd_ycp, & fdset );
    // FIXME: check for EINTR
    select( fd_ycp+1, & fdset, 0, 0, 0 );
}


YRadioButtonGroup *YUI::findRadioButtonGroup( YContainerWidget * root, YWidget * widget, bool * contains )
{
    YCPValue root_id = root->id();

#ifdef VERBOSE_FIND_RADIO_BUTTON_GROUP
    y2debug( "findRadioButtonGroup( %s, %s )",
	     root_id.isNull() ? "__" : root_id->toString().c_str(),
	     widget->id()->toString().c_str() );
#endif

    bool is_rbg = root->isRadioButtonGroup();
    if ( widget == root ) *contains = true;
    else
    {
	for ( int i=0; i<root->numChildren(); i++ )
	{
	    if ( root->child(i)->isContainer() )
	    {
		YRadioButtonGroup *rbg =
		    findRadioButtonGroup( dynamic_cast <YContainerWidget *> ( root->child(i) ), widget, contains );
		if ( rbg ) return rbg; // Some other lower rbg is it.
	    }
	    else if ( root->child(i) == widget ) *contains = true;
	}
    }
    if ( is_rbg && *contains ) return dynamic_cast <YRadioButtonGroup *> ( root );
    else return 0;
}


// ----------------------------------------------------------------------


void *start_ui_thread( void * yui )
{
    YUI * ui= (YUI *) yui;

#if VERBOSE_COMM
    y2debug( "Starting UI thread" );
#endif
    
    if ( ui )
	ui->uiThreadMainLoop();
    return 0;
}



// EOF
