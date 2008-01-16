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

  Authors:	Stefan Hundhammer <sh@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#define VERBOSE_COMM 0	// VERY verbose thread communication logging

#include <stdio.h>
#include <unistd.h> // pipe()
#include <fcntl.h>  // fcntl()
#include <errno.h>
#include <locale.h> // setlocale()
#include <pthread.h>

#define YUILogComponent "ui"
#include "YUILog.h"

#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPVoid.h>

#include "Y2UINamespace.h"
#include "YUI.h"
#include "YUISymbols.h"
#include "YWidget.h"
#include "YDialog.h"
#include "YApplication.h"


typedef YCPValue (*v2) ();
typedef YCPValue (*v2v)     (const YCPValue &);
typedef YCPValue (*v2vv)    (const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvv)   (const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvv)  (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);
typedef YCPValue (*v2vvvvv) (const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &, const YCPValue &);


bool	YUI::_reverseLayout	= false;
YUI *	YUI::_yui		= 0;

extern void *start_ui_thread( void * yui );

static void
yui_y2logger( YUILogLevel_t	logLevel,
	      const char *	logComponent,
	      const char *	sourceFileName,
	      int 	 	sourceLineNo,
	      const char * 	sourceFunctionName,
	      const char *	message );



YUI::YUI( bool withThreads )
    : _withThreads( withThreads )
    , _uiThread( 0 )
    , _moduleName( "yast2" )
    , _productName( "SUSE Linux" )
    , macroRecorder (0)
    , macroPlayer (0)
    , _events_blocked( false )
    , _callback( 0 )
{
    _yui = this;
    YUILog::setLoggerFunction( yui_y2logger );
}


YUI::~YUI()
{
    if ( _withThreads && _uiThread )
    {
	y2error( "shutdownThreads() was never called!" );
	y2error( "shutting down now - this might segfault" );
	shutdownThreads();
    }

    if ( YDialog::openDialogsCount() > 0 )
	y2error( "%d open dialogs left over", YDialog::openDialogsCount() );

    YDialog::deleteAllDialogs();

    deleteMacroRecorder();
    deleteMacroPlayer();
}


YUI *
YUI::ui()
{
    YUI_CHECK_PTR( _yui );
    return _yui;
}


YWidgetFactory *
YUI::widgetFactory()
{
    static YWidgetFactory * factory = 0;

    if ( ! factory )
	factory = ui()->createWidgetFactory();

    YUI_CHECK_PTR( factory );
    return factory;
}


YOptionalWidgetFactory *
YUI::optionalWidgetFactory()
{
    static YOptionalWidgetFactory * factory = 0;

    if ( ! factory )
	factory = ui()->createOptionalWidgetFactory();

    YUI_CHECK_PTR( factory );
    return factory;
}


YApplication *
YUI::app()
{
    static YApplication * app = 0;

    if ( ! app )
	app = ui()->createApplication();

    YUI_CHECK_PTR( app );
    return app;
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
    //
    // Therefore the creation of the thread is delayed and performed in this
    // method. It must be called at the end of the constructor of the specific
    // UI (the Qt UI or the NCurses UI).

    if ( _withThreads )
    {
	if ( pipe( pipe_from_ui ) == 0 &&
	     pipe( pipe_to_ui   ) == 0   )
	{

	    // Make fd non blockable the ui thread reads from
	    long arg;
	    arg = fcntl( pipe_to_ui[0], F_GETFL );
	    if ( fcntl( pipe_to_ui[0], F_SETFL, arg | O_NONBLOCK ) < 0 )
	    {
		y2error( "Couldn't set O_NONBLOCK: errno=%d: %m", errno );
		_withThreads = false;
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
	    y2error( "pipe() failed: errno=%d: %m", errno );
	    exit(2);
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
    pthread_create( & _uiThread, & attr, start_ui_thread, this );
}


void YUI::terminateUIThread()
{
    y2debug( "Telling UI thread to shut down" );
    terminate_ui_thread = true;
    signalUIThread();
    y2debug( "Waiting for UI thread to shut down" );
    waitForUIThread();
    pthread_join( _uiThread, 0 );
    y2debug( "UI thread shut down correctly" );
}


void YUI::shutdownThreads()
{
    if ( _uiThread )
    {
	terminateUIThread();
	_uiThread = 0;
	close( pipe_to_ui[0] );
	close( pipe_to_ui[1] );
	close( pipe_from_ui[0] );
	close( pipe_from_ui[1] );
    }
}


extern YCPValue UIUserInput ();
extern YCPValue UITimeoutUserInput( const YCPInteger& timeout );
extern YCPValue UIWaitForEvent();
extern YCPValue UIWaitForEventTimeout( const YCPInteger & timeout );


YCPValue YUI::callFunction( void * function, int argc, YCPValue argv[] )
{
    if ( function == 0 )
    {
	y2error( "NULL function pointer!" );
	return YCPNull();
    }

    // ensure YCPNull will be passed as YCPVoid
    for (int i = 0; i < argc ; i++)
    {
	if (argv[i].isNull ())
	{
	    argv[i] = YCPVoid ();
	}
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
    (void) write ( pipe_to_ui[1], & arbitrary, 1 );

#if VERBOSE_COMM
    y2debug( "Wrote byte to ui thread %d", pipe_to_ui[1] );
#endif
}


bool YUI::waitForUIThread()
{
    char arbitrary;
    int res;

    do {
#if VERBOSE_COMM
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

#if VERBOSE_COMM
    y2debug ( "Read byte from ui thread" );
#endif

    // return true if we really did get a signal byte
    return res != -1;
}


void YUI::signalYCPThread()
{
    static char arbitrary;
    (void) write( pipe_from_ui[1], & arbitrary, 1 );

#if VERBOSE_COMM
    y2debug( "Wrote byte to ycp thread %d", pipe_from_ui[1] );
#endif
}


bool YUI::waitForYCPThread()
{
    char arbitrary;

    int res;
    do {
#if VERBOSE_COMM
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

#if VERBOSE_COMM
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
	// signal byte. Although idleLoop already does a select(), this seems to
	// be necessary.  Anyway: Why do we set the pipe to non-blocking if we
	// wait in idleLoop for it to become readable? It is needed in
	// YUIQt::idleLoop for QSocketNotifier.

	if ( ! waitForYCPThread () )
	    continue;

	if ( terminate_ui_thread )
	{
	    y2debug( "Final sync with YCP thread" );
	    signalYCPThread();
	    y2debug( "Shutting down UI main loop" );
	    return;
	}

	_builtinCallData.result = _builtinCallData.function->evaluateCall_int();

	signalYCPThread();
    }
}


// ----------------------------------------------------------------------
// Default implementations for the virtual methods the deal with
// event processing

#if 0

void YUI::idleLoop( int fd_ycp )
{
    // Just wait for fd_ycp to become readable
    fd_set fdset;
    FD_ZERO( & fdset );
    FD_SET( fd_ycp, & fdset );
    // FIXME: check for EINTR
    select( fd_ycp+1, & fdset, 0, 0, 0 );
}

#endif

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


static void
yui_y2logger( YUILogLevel_t	logLevel,
	      const char *	logComponent,
	      const char *	sourceFileName,
	      int 	 	sourceLineNo,
	      const char * 	sourceFunctionName,
	      const char *	message )
{
    loglevel_t y2logLevel = LOG_DEBUG;
    
    switch ( logLevel )
    {
	case YUI_LOG_DEBUG:	y2logLevel = LOG_DEBUG;		break;
	case YUI_LOG_MILESTONE:	y2logLevel = LOG_MILESTONE;	break;
	case YUI_LOG_WARNING:	y2logLevel = LOG_WARNING;	break;
	case YUI_LOG_ERROR:	y2logLevel = LOG_ERROR;		break;
    }

    if ( ! logComponent )
	logComponent = "??";

    if ( ! sourceFileName )
	sourceFileName = "??";

    if ( ! sourceFunctionName )
	sourceFunctionName = "??";

    if ( ! message )
	message = "";

    y2_logger( y2logLevel, logComponent,
	       sourceFileName, sourceLineNo, sourceFunctionName,
	       "%s", message );
}




// EOF
