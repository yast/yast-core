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

  File:		YUIInterpreter_core.cc

		Core functions of the UI interpreter
		

  Authors:	Mathias Kettner <kettner@suse.de>
		Stefan Hundhammer <sh@suse.de>
		
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

#include "YUIInterpreter.h"
#include "YUISymbols.h"
#include "hashtable.h"
#include "YWidget.h"

#include "YDialog.h"
#include "YRadioButton.h"
#include "YRadioButtonGroup.h"
#include "YReplacePoint.h"


bool YUIInterpreter::_reverseLayout = false;


YUIInterpreter::YUIInterpreter( bool with_threads, Y2Component *callback )
    : id_counter(0)
    , with_threads( with_threads )
    , box_in_the_middle( YCPNull() )
    , _moduleName( "yast2" )
    , macroRecorder(0)
    , macroPlayer(0)
    , callbackComponent( callback )
    , _events_blocked( false )
{
}


YUIInterpreter::~YUIInterpreter()
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


Y2Component *
YUIInterpreter::getCallback( void )
{
    y2debug ( "YUIInterpreter[%p]::getCallback() = %p", this, callbackComponent );
    return callbackComponent;
}


void
YUIInterpreter::setCallback( Y2Component *callback )
{
    y2debug ( "YUIInterpreter[%p]::setCallback( %p )", this, callback );
    callbackComponent = callback;
    return;
}


string
YUIInterpreter::interpreter_name() const
{
    return "UI";	// must be upper case
}


void
YUIInterpreter::internalError( const char *msg )
{
    fprintf( stderr, "YaST2 UI internal error: %s", msg );
}


void YUIInterpreter::topmostConstructorHasFinished()
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
}


void YUIInterpreter::createUIThread()
{
    pthread_attr_t attr;
    pthread_attr_init( & attr );
    pthread_create( & ui_thread, & attr, start_ui_thread, this );
}


void YUIInterpreter::terminateUIThread()
{
    y2debug( "Telling UI thread to shut down" );
    terminate_ui_thread = true;
    signalUIThread();
    pthread_join( ui_thread, 0 );
    y2debug( "UI thread shut down correctly" );
}


void YUIInterpreter::signalUIThread()
{
    static char arbitrary = 42;
    write ( pipe_to_ui[1], & arbitrary, 1 );

#ifdef VERBOSE_COMM
    y2debug( "Wrote byte to ui thread %d", pipe_to_ui[1] );
#endif
}


bool YUIInterpreter::waitForUIThread()
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


void YUIInterpreter::signalYCPThread()
{
    static char arbitrary;
    write( pipe_from_ui[1], & arbitrary, 1 );

#ifdef VERBOSE_COMM
    y2debug( "Wrote byte to ycp thread %d", pipe_from_ui[1] );
#endif
}


bool YUIInterpreter::waitForYCPThread()
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


void YUIInterpreter::uiThreadMainLoop()
{
    while ( true )
    {
	idleLoop ( pipe_to_ui[0] );

	// The pipe is non-blocking, so we have to check if
	// we really read a signal byte. Although idleLoop
	// already makes a select, this seems to be necessary.
	// Anyway: Why do we set the pipe to non-blocking if
	// we wait in idleLoop for it to become readable? It
	// is needed in YUIQt::idleLoop for QSocketNotifier.
	if ( ! waitForYCPThread () )
	    continue;

	if ( terminate_ui_thread ) return;

	YCPValue command = box_in_the_middle;
	if ( command.isNull() )
	{
	    y2error( "Command to ui is NULL" );
	    box_in_the_middle = YCPVoid();
	}
	else if ( command->isTerm() )
	{
	    box_in_the_middle = executeUICommand( command->asTerm() );
	}
	else
	{
	    y2error( "Command to ui is not a term: '%s'",
		     command->toString().c_str() );
	    box_in_the_middle = YCPVoid();
	}

	signalYCPThread();
    }
}


YCPValue YUIInterpreter::evaluateInstantiatedTerm( const YCPTerm & term )
{
    string symbol = term->symbol()->symbol();
    y2debug( "evaluateInstantiatedTerm( %s )", symbol.c_str() );

    if ( YUIInterpreter_in_word_set( symbol.c_str(), symbol.length() ) )
    {
	if ( macroPlayer )
	{
	    string command = term->symbol()->symbol();
	    
	    if( command == YUIBuiltin_UserInput		||
		command == YUIBuiltin_TimeoutUserInput	||
		command == YUIBuiltin_WaitForEvent )
	    {
		// This must be done in the YCP thread to keep the threads synchronized!
		playNextMacroBlock();
	    }
	}

	if ( with_threads )
	{
	    box_in_the_middle = term;
	    signalUIThread();
	    
	    while ( ! waitForUIThread() )
	    {
		// NOP
	    }

	    return box_in_the_middle;
	}
	else return executeUICommand( term );
    }
    else return YCPNull();
}


YCPValue YUIInterpreter::callback( const YCPValue & value )
{
    y2debug ( "YUIInterpreter::callback( %s )", value->toString().c_str() );
    if ( value->isBuiltin() )
    {
	YCPBuiltin b = value->asBuiltin();
	YCPValue v = b->value (0);

	if ( b->builtin_code() == YCPB_UI )
	{
	    return evaluate (v);
	}

	if ( callbackComponent )
	{
	    YCPValue v = YCPNull();
	    if ( b->builtin_code() == YCPB_WFM )		// if it goes to WFM, just send the value
	    {
		v = callbackComponent->evaluate (v);
	    }
	    else		// going to SCR, send the complete value
	    {
		v = callbackComponent->evaluate ( value );
	    }
	    return v;
	}
    }

    return YCPNull();
}


YCPValue YUIInterpreter::evaluateUI( const YCPValue & value )
{
    y2debug ( "evaluateUI( %s )\n", value->toString().c_str() );
    if ( value->isBuiltin() )
    {
	YCPBuiltin b = value->asBuiltin();
	if ( b->builtin_code() == YCPB_DEEPQUOTE )
	{
	    return evaluate ( b->value(0) );
	}
    }
    else if ( value->isTerm() && value->asTerm()->isQuoted() )
    {
	YCPTerm vt = value->asTerm();
	YCPTerm t( YCPSymbol( vt->symbol()->symbol(), false ), vt->name_space() );
	for ( int i=0; i<vt->size(); i++ )
	{
	    t->add( vt->value(i) );
	}
	return evaluate (t);
    }
    return evaluate ( value );
}


YCPValue YUIInterpreter::evaluateWFM( const YCPValue & value )
{
    y2debug ( "YUIInterpreter[%p]::evaluateWFM[%p]( %s )\n", this, callbackComponent, value->toString().c_str() );
    if ( callbackComponent )
    {
	if ( value->isBuiltin() )
	{
	    YCPBuiltin b = value->asBuiltin();
	    if ( b->builtin_code() == YCPB_DEEPQUOTE )
	    {
		return callbackComponent->evaluate ( b->value(0) );
	    }
	}
	else if ( value->isTerm() )
	{
	    YCPTerm vt = value->asTerm();
	    YCPTerm t( YCPSymbol( vt->symbol()->symbol(), false ), vt->name_space() );
	    for ( int i=0; i<vt->size(); i++ )
	    {
		YCPValue v = evaluate ( vt->value(i) );
		if ( v.isNull() )
		{
		    return YCPError ( "WFM parameter is NULL\n", YCPNull() );
		}
		t->add(v);
	    }
	    return callbackComponent->evaluate (t);
	}
	return callbackComponent->evaluate ( value );
    }
    y2error ( "YUIInterpreter[%p]: No callbackComponent", this );
    return YCPError ( "YUIInterpreter::evaluateWFM: No callback component WFM existing.", YCPNull() );
}



YCPValue YUIInterpreter::evaluateSCR( const YCPValue & value )
{
    y2debug ( "evaluateSCR( %s )\n", value->toString().c_str() );
    if ( callbackComponent )
    {
	if ( value->isBuiltin() )
	{
	    YCPBuiltin b = value->asBuiltin();
	    if ( b->builtin_code() == YCPB_DEEPQUOTE )
	    {
		return callbackComponent->evaluate ( YCPBuiltin ( YCPB_SCR, b->value(0) ) );
	    }
	}
	else if ( value->isTerm() )
	{
	    YCPTerm vt = value->asTerm();
	    YCPTerm t( YCPSymbol( vt->symbol()->symbol(), false ), vt->name_space() );
	    for ( int i=0; i<vt->size(); i++ )
	    {
		YCPValue v = evaluate ( vt->value(i) );
		if ( v.isNull() )
		{
		    return YCPError ( "SCR parameter is NULL\n", YCPNull() );
		}
		t->add(v);
	    }
	    return callbackComponent->evaluate ( YCPBuiltin ( YCPB_SCR, t ) );
	}
	return callbackComponent->evaluate ( YCPBuiltin ( YCPB_SCR, value ) );
    }
    return YCPError ( "YUIInterpreter::evaluateSCR: No callback component WFM existing.", YCPNull() );
}



YCPValue YUIInterpreter::evaluateLocale( const YCPLocale & l )
{
    y2debug( "evaluateLocale( %s )\n", l->value()->value().c_str() );

    // locales are evaluated by the WFM now. If a locale happens
    // to show up here, send it back. evaluateWFM() might return
    // YCPNull() if no WFM is available. Handle this also.

    YCPValue v = evaluateWFM(l);
    if ( v.isNull() )
    {
	return l->value();  // return YCPString()
    }
    return v;
}



YCPValue YUIInterpreter::setTextdomain( const string& textdomain )
{
    return evaluateWFM ( YCPBuiltin ( YCPB_LOCALDOMAIN, YCPString ( textdomain ) ) );
}



string YUIInterpreter::getTextdomain( void )
{
    YCPValue v = evaluateWFM ( YCPBuiltin ( YCPB_GETTEXTDOMAIN ) );
    if ( ! v.isNull() && v->isString() )
	return v->asString()->value();
    return "ui";
}



// ----------------------------------------------------------------------
// Default implementations for the virtual methods the deal with
// event processing


void YUIInterpreter::idleLoop( int fd_ycp )
{
    // Just wait for fd_ycp to become readable
    fd_set fdset;
    FD_ZERO( & fdset );
    FD_SET( fd_ycp, & fdset );
    // FIXME: check for EINTR
    select( fd_ycp+1, & fdset, 0, 0, 0 );
}


YRadioButtonGroup *YUIInterpreter::findRadioButtonGroup( YContainerWidget * root, YWidget * widget, bool * contains )
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


void *start_ui_thread( void *ui_int )
{
    YUIInterpreter *ui_interpreter = ( YUIInterpreter * ) ui_int;
    ui_interpreter->uiThreadMainLoop();
    return 0;
}



// EOF
