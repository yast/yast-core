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

  File:		YUIInterpreter_builtins.cc

		UI builtin commands


  Authors:	Mathias Kettner <kettner@suse.de>
		Stefan Hundhammer <sh@suse.de>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#define VERBOSE_COMMANDS	// pretty verbose logging of each UI command
#define VERBOSE_REPLACE_WIDGET

#include <stdio.h>
#include <unistd.h> // pipe()
#include <fcntl.h>  // fcntl()
#include <errno.h>  // strerror()
#include <locale.h> // setlocale()
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <iconv.h>

#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPVoid.h>

#include "YUIInterpreter.h"
#include "YEvent.h"
#include "YUISymbols.h"
#include "hashtable.h"
#include "YDialog.h"
#include "YWidget.h"
#include "YMacroRecorder.h"
#include "YMacroPlayer.h"
#include "YReplacePoint.h"
#include "YShortcut.h"

using std::string;



YCPValue YUIInterpreter::executeUICommand( const YCPTerm & term )
{
    string symbol = term->symbol()->symbol();
    YCPValue ret = YCPNull();
#ifdef VERBOSE_COMMANDS
    y2debug( "YUIInterpreter::executeUICommand( %s )", symbol.c_str() );
#endif

    if	    ( symbol == YUIBuiltin_AskForExistingDirectory	) ret = evaluateAskForExistingDirectory ( term );
    else if ( symbol == YUIBuiltin_AskForExistingFile		) ret = evaluateAskForExistingFile	( term );
    else if ( symbol == YUIBuiltin_AskForSaveFileName		) ret = evaluateAskForSaveFileName	( term );
    else if ( symbol == YUIBuiltin_BusyCursor			) ret = evaluateBusyCursor		( term );
    else if ( symbol == YUIBuiltin_ChangeWidget			) ret = evaluateChangeWidget		( term );
    else if ( symbol == YUIBuiltin_CheckShortcuts		) ret = evaluateCheckShortcuts		( term );
    else if ( symbol == YUIBuiltin_CloseDialog			) ret = evaluateCloseDialog		( term );
    else if ( symbol == YUIBuiltin_DumpWidgetTree		) ret = evaluateDumpWidgetTree		( term );
    else if ( symbol == YUIBuiltin_FakeUserInput		) ret = evaluateFakeUserInput		( term );
    else if ( symbol == YUIBuiltin_GetDisplayInfo		) ret = evaluateGetDisplayInfo		( term );
    else if ( symbol == YUIBuiltin_GetLanguage			) ret = evaluateGetLanguage		( term );
    else if ( symbol == YUIBuiltin_GetModulename		) ret = evaluateGetModulename		( term );
    else if ( symbol == YUIBuiltin_Glyph			) ret = evaluateGlyph			( term );
    else if ( symbol == YUIBuiltin_HasSpecialWidget		) ret = evaluateHasSpecialWidget	( term );
    else if ( symbol == YUIBuiltin_MakeScreenShot		) ret = evaluateMakeScreenShot		( term );
    else if ( symbol == YUIBuiltin_NormalCursor			) ret = evaluateNormalCursor		( term );
    else if ( symbol == YUIBuiltin_OpenDialog			) ret = evaluateOpenDialog		( term );
    else if ( symbol == YUIBuiltin_PlayMacro			) ret = evaluatePlayMacro		( term );
    else if ( symbol == YUIBuiltin_PollInput			) ret = evaluatePollInput		( term );
    else if ( symbol == YUIBuiltin_PostponeShortcutCheck	) ret = evaluatePostponeShortcutCheck	( term );
    else if ( symbol == YUIBuiltin_QueryWidget			) ret = evaluateQueryWidget		( term );
    else if ( symbol == YUIBuiltin_RecalcLayout			) ret = evaluateRecalcLayout		( term );
    else if ( symbol == YUIBuiltin_Recode			) ret = evaluateRecode			( term );
    else if ( symbol == YUIBuiltin_RecordMacro			) ret = evaluateRecordMacro		( term );
    else if ( symbol == YUIBuiltin_RedrawScreen			) ret = evaluateRedrawScreen		( term );
    else if ( symbol == YUIBuiltin_ReplaceWidget		) ret = evaluateReplaceWidget		( term );
    else if ( symbol == YUIBuiltin_RunPkgSelection		) ret = evaluateRunPkgSelection		( term );
    else if ( symbol == YUIBuiltin_SCR				) ret = evaluateCallback		( term, false );
    else if ( symbol == YUIBuiltin_SetConsoleFont		) ret = evaluateSetConsoleFont		( term );
    else if ( symbol == YUIBuiltin_SetFocus			) ret = evaluateSetFocus		( term );
    else if ( symbol == YUIBuiltin_SetFunctionKeys		) ret = evaluateSetFunctionKeys		( term );
    else if ( symbol == YUIBuiltin_SetLanguage			) ret = evaluateSetLanguage		( term );
    else if ( symbol == YUIBuiltin_SetModulename		) ret = evaluateSetModulename		( term );
    else if ( symbol == YUIBuiltin_StopRecordMacro		) ret = evaluateStopRecordMacro		( term );
    else if ( symbol == YUIBuiltin_UserInput			) ret = evaluateUserInput		( term );
    else if ( symbol == YUIBuiltin_TimeoutUserInput		) ret = evaluateTimeoutUserInput	( term );
    else if ( symbol == YUIBuiltin_WaitForEvent			) ret = evaluateWaitForEvent		( term );
    else if ( symbol == YUIBuiltin_WFM				) ret = evaluateCallback		( term, true );
    else if ( symbol == YUIBuiltin_WidgetExists			) ret = evaluateWidgetExists		( term );
    else
    {
	y2internal( "Unknown term symbol in executeUICommand: %s",
		    symbol.c_str() );
	return YCPVoid();
    }

    if ( ret.isNull() )
    {
	return YCPError ( string ( "Invalid arguments for ui command: " )+term->toString() );
    }
    else return ret;
}



// builtin HasSpecialWidget() -> YUIInterpreter_special_widgets.cc


/**
 * @builtin SetModulename( string name ) -> void
 *
 * Does nothing. The SetModulename command is introduced for
 * the translator. But the translator sends all commands
 * to the ui. So the ui shouldn't complain about this
 * command.
 *
 * @example SetModulename( "inst_environment" )
 */

YCPValue YUIInterpreter::evaluateSetModulename( const YCPTerm & term )
{
    if ( term->size() == 1 && term->value(0)->isString() )
    {
	_moduleName = term->value(0)->asString()->value();
	return YCPVoid();
    }
    else return YCPNull();
}


const char *YUIInterpreter::moduleName()
{
    return _moduleName.c_str();
}



/**
 * @builtin GetModulename( <string> ) -> string
 *
 * This is tricky. The UI doesn't care about the current module
 * name, only the translator does. However, since the translator
 * acts as a filter between a client and the UI, it cant directly
 * return the module name. The current implementation inserts the
 * modules name in the translator and it arrives here as the term
 * argument. So the example has no arguments, but the internal code
 * checks for a string argument.
 *
 * @example GetModulename()
 */

YCPValue YUIInterpreter::evaluateGetModulename( const YCPTerm & term )
{
    if ( ( term->size() == 1 ) && ( term->value(0)->isString() ) )
    {
	return term->value(0);
    }
    else return YCPNull();
}



/**
 * @builtin SetLanguage( string lang, [ string encoding ] ) -> nil
 *
 * Tells the ui that the user has selected another language.
 * If the ui has any language dependend output that language
 * setting is honored. <tt>lang</tt> is an ISO language string,
 * such as <tt>de</tt> or <tt>de_DE</tt>. It is required
 * to specify an encoding scheme, since not all user interfaces
 * are capable of UTF-8.
 *
 * @example SetLanguage( "de_DE@euro" )
 * @example SetLanguage( "en_GB" )
 */

YCPValue YUIInterpreter::evaluateSetLanguage( const YCPTerm & term )
{
    if ( term->size() > 0 && term->value(0)->isString() )
    {
	string lang = term->value(0)->asString()->value();
	if ( term->size() > 1 && term->value(1)->isString() )
	{
	    lang += ".";
	    lang += term->value(1)->asString()->value();
	}

	setenv( "LANG", lang.c_str(), 1 ); // 1 : replace
#ifdef WE_HOPEFULLY_DONT_NEED_THIS_ANY_MORE
	setlocale( LC_ALL, "" );	// switch char set mapping in glibc
#endif
	setlocale( LC_NUMERIC, "C" );	// but always format numbers with "."
	YCPTerm newTerm = YCPTerm( term->symbol() );
	newTerm->add ( YCPString( lang ) );
	y2milestone ( "ui specific setLanguage( %s )", newTerm->toString().c_str() );

	return setLanguage( newTerm );	// UI-specific setLanguage: returns YCPVoid() if OK, YCPNull() if error
    }
    else return YCPNull();
}


/*
 * Default UI-specific setLanguage()
 * Returns OK (YCPVoid() )
 */

YCPValue YUIInterpreter::setLanguage( const YCPTerm & term )
{
    // NOP

    return YCPVoid();	// OK (YCPNull() would mean error)
}


/**
 * @builtin SetConsoleFont( string console_magic, string font, string screen_map, string unicode_map, string encoding ) -> nil
 *
 * Switches the text console to the specified font.
 * See the setfont(8) command and the console HowTo for details.
 *
 * @example SetConsoleFont( "( K", "lat2u-16.psf", "latin2u.scrnmap", "lat2u.uni", "latin1" )
 */

YCPValue YUIInterpreter::evaluateSetConsoleFont( const YCPTerm & term )
{
    if ( term->size() == 5 &&
	 term->value(0)->isString() &&
	 term->value(1)->isString() &&
	 term->value(2)->isString() &&
	 term->value(3)->isString() &&
	 term->value(4)->isString()   )
    {
	return setConsoleFont( term->value(0)->asString(),	// console magic
			       term->value(1)->asString(),	// font
			       term->value(2)->asString(),	// screen_map
			       term->value(3)->asString(),	// unicode_map
			       term->value(4)->asString() );	// encoding
    }
    else return YCPNull();
}


/*
 * Default UI-specific setConsoleFont()
 * Returns OK ( YCPVoid() )
 */

YCPValue YUIInterpreter::setConsoleFont( const YCPString & console_magic,
					 const YCPString & font,
					 const YCPString & screen_map,
					 const YCPString & unicode_map,
					 const YCPString & encoding )
{
    // NOP

    return YCPVoid();	// OK ( YCPNull() would mean error )
}


/**
 * Default UI-specific busyCursor() - does nothing
 */
void YUIInterpreter::busyCursor()
{
    // NOP
}


/**
 * Default UI-specific normalCursor() - does nothing
 */
void YUIInterpreter::normalCursor()
{
    // NOP
}


/**
 * Default UI-specific redrawScreen() - does nothing
 */
void YUIInterpreter::redrawScreen()
{
    // NOP
}


/**
 * Default UI-specific makeScreenShot() - does nothing
 */
void YUIInterpreter::makeScreenShot( string filename )
{
    // NOP
}



/**
 * @builtin GetLanguage( boolean strip_encoding ) -> string
 *
 * Retrieves the current language setting from of the user interface.  Since
 * YaST2 is a client server architecture, we distinguish between the language
 * setting of the user interface and that of the configuration modules. If the
 * module or the translator wants to know which language the user currently
 * uses, it can call <tt>GetLanguage</tt>. The return value is an ISO language
 * code, such as "de" or "de_DE".
 *
 * If "strip_encoding" is set to "true", all encoding or similar information is
 * cut off, i.e. everything from the first "." or "@" on. Otherwise the current
 * contents of the "LANG" environment variable is returned ( which very likely
 * ends with ".UTF-8" since this is the encoding YaST2 uses internally).
 */

YCPValue YUIInterpreter::evaluateGetLanguage( const YCPTerm & term )
{
    if ( term->size() == 1 && term->value(0)->isBoolean() )
    {
	bool strip_encoding = term->value(0)->asBoolean()->value();
	const char *lang_cstr = getenv( "LANG" );
	string lang = "";		// Fallback if $LANG not set

	if ( lang_cstr )		// only if environment variable set
	{
	    lang = lang_cstr;

	    if ( strip_encoding )
	    {
		y2milestone( "Stripping encoding" );
		string::size_type pos = lang.find_first_of( ".@" );

		if ( pos != string::npos )		// if encoding etc. specified
		{
		    lang = lang.substr( 0, pos );	// remove it
		}
	    }
	}

	return YCPString( lang );
    }
    else return YCPNull();
}



/**
 * @builtin UserInput() -> any
 *
 * Waits for the user to click some button, close the window or
 * activate some widget that has the <tt>`notify</tt> option set.
 * The return value is the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicit cancel
 * button (for example he closes the window).
 * <p>
 * Read more details and usage example in the
 * <a href="events/event-builtins.html#UserInput">
 * YaST2 UI Event Handling Documentation</a>.
 */
YCPValue YUIInterpreter::evaluateUserInput( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    return doUserInput( YUIBuiltin_UserInput,
			0,		// timeout_millisec
			true,		// wait
			false );	// detailed
}


/**
 * @builtin PollInput() -> any
 *
 * Doesn't wait but just looks if the user has clicked some
 * button, has closed the window or has activated
 * some widget that has the <tt>`notify</tt> option set. Returns
 * the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicite cancel
 * button ( for example he closes the window). Returns nil if no
 * user input has occured.
 * <p>
 * Read more details and usage example in the
 * <a href="events/event-builtins.html#PollInput">
 * YaST2 UI Event Handling Documentation</a>.
 */
YCPValue YUIInterpreter::evaluatePollInput( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    return doUserInput( YUIBuiltin_PollInput,
			0,		// timeout_millisec
			false,		// wait
			false );	// detailed
}


/**
 * @builtin TimeoutUserInput( integer timeout_millisec ) -> any
 *
 * Waits for the user to click some button, close the window or
 * activate some widget that has the <tt>`notify</tt> option set
 * or until the specified timeout is expired.
 * The return value is the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicit cancel
 * button (for example he closes the window).
 * Upon timeout, <tt>`timeout</tt> is returned.
 * <p>
 * Read more details and usage example in the
 * <a href="events/event-builtins.html#TimeoutUserInput">
 * YaST2 UI Event Handling Documentation</a>.
 */
YCPValue YUIInterpreter::evaluateTimeoutUserInput( const YCPTerm & term )
{
    if ( term->size() != 1 || ! term->value(0)->isInteger() )
	return YCPNull();

    long timeout_millisec = term->value(0)->asInteger()->value();

    return doUserInput( YUIBuiltin_TimeoutUserInput,
			timeout_millisec,
			true,			// wait
			false );		// detailed
}


/**
 * @builtin WaitForEvent() -> map
 * @builtin WaitForEvent( integer timeout_millisec ) -> map
 *
 * Extended event handling - very much like UserInput(), but returns much more
 * detailed information about the event that occured in a map.
 * <p>
 * Read more details and usage example in the
 * <a href="events/event-builtins.html#WaitForEvent">
 * YaST2 UI Event Handling Documentation</a>.
 */
YCPValue YUIInterpreter::evaluateWaitForEvent( const YCPTerm & term )
{
    long timeout_millisec = 0;

    if ( term->size() != 0 )
    {
	if ( term->size() != 1 || ! term->value(0)->isInteger() )
	    return YCPNull();

	timeout_millisec = term->value(0)->asInteger()->value();
    }

    return doUserInput( YUIBuiltin_WaitForEvent,
			timeout_millisec,
			true,			// wait
			true );			// detailed
}




YCPValue YUIInterpreter::doUserInput( const char * 	builtin_name,
				      long 		timeout_millisec,
				      bool 		wait,
				      bool 		detailed )
{
    // Plausibility check for timeout

    if ( timeout_millisec < 0 )
    {
	y2error( "%s(): Invalid value %ld for timeout - assuming 0",
		 builtin_name, timeout_millisec );
	timeout_millisec = 0;
    }


    // Check environment: Do we have a dialog to receive input from?

    YDialog * dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "%s(): No dialog existing", builtin_name );
	internalError( "No dialog existing during UserInput().\n"
		       "\n"
		       "Please check the log file!" );
	return YCPNull();
    }


    // Check for leftover postponed shortcut check

    if ( dialog->shortcutCheckPostponed() )
    {
	y2error( "Missing CheckShortcuts() before %s() after PostponeShortcutCheck()!", builtin_name );
	dialog->checkShortcuts( true );
    }


    // Handle events

    YEvent *	event = 0;
    YCPValue 	input = YCPVoid();

    if ( fakeUserInputQueue.empty() )
    {
	if ( wait )
	{
	    do
	    {
		// Get an event from the specific UI. Wait if there is none.

		event = filterInvalidEvents( userInput( (unsigned long) timeout_millisec ) );

		// If there was no event or if filterInvalidEvents() discarded
		// an invalid event, go back and get the next one.
	    } while ( ! event );
	}
	else
	{
	    // Get an event from the specific UI. Don't wait if there is none.

	    event = filterInvalidEvents( pollInput() );

	    // Nevermind if filterInvalidEvents() discarded an invalid event.
	    // PollInput() is called very often (in a loop) anyway, and most of
	    // the times it returns 'nil' anyway, so there is no need to care
	    // for just another 'nil' that is returned in this exotic case.
	}

	if ( event )
	{
	    if ( detailed )
		input = event->ycpEvent();	// The event map
	    else
		input = event->userInput();	// Only one single ID (or 'nil')
	}
    }
    else // fakeUserInputQueue contains elements -> use the first one
    {
	// Handle macro playing

	input = fakeUserInputQueue.front();
	fakeUserInputQueue.pop_front();
    }


    // Handle macro recording

    if ( macroRecorder )
    {
	macroRecorder->beginBlock();
	dialog->saveUserInput( macroRecorder );
	macroRecorder->recordUserInput( input );
	macroRecorder->endBlock();
    }


    // Clean up.
    //
    // The generic UI interpreter assumes ownership of events delivered by
    // userInput() / pollInput(), so now (after it is processed) is the time to
    // delete that event.

    if ( event )
	delete event;

    return input;
}


YEvent *
YUIInterpreter::filterInvalidEvents( YEvent * event )
{
    if ( ! event )
	return 0;

    YWidgetEvent * widgetEvent = dynamic_cast<YWidgetEvent *> (event);

    if ( widgetEvent && widgetEvent->widget() )
    {
	if ( ! widgetEvent->widget()->isValid() )
	{
	    /**
	     * Silently discard events from widgets that have become invalid.
	     *
	     * This may legitimately happen if some widget triggered an event yet
	     * nobody cared for that event (i.e. called UserInput() or PollInput() )
	     * and the widget has been destroyed meanwhile.
	     **/

	    // y2debug( "Discarding event for widget that has become invalid" );

	    delete widgetEvent;
	    return 0;
	}

	if ( widgetEvent->widget()->yDialog() != currentDialog() )
	{
	    /**
	     * Silently discard events from all but the current ( topmost ) dialog.
	     *
	     * This may happen even here even though the specific UI should have
	     * taken care about that: Events may still be in the queue. They might
	     * have been valid (i.e. belonged to the topmost dialog ) when they
	     * arrived, but maybe simply nobody has evaluated them.
	     **/

	    // Yes, really y2debug() - this may legitimately happen.
	    y2debug( "Discarding event from widget from foreign dialog" );

	    delete widgetEvent;
	    return 0;
	}

    }

    return event;
}


/**
 * @builtin OpenDialog( term widget ) -> boolean
 *
 * Opens a new dialog. <tt>widget</tt> is a term representation of the widget
 * being displayed.
 * See <a href="YCP-UI-widgets.html">the widget documentation</a> for details
 * what widgets are available.	All open dialogs are arranged in a stack. A
 * newly opened dialog is put on top of the stack. All operations implicitely
 * refer to the topmost dialog. The user can interact only with that dialog.
 * The application does not terminate if the last dialog is closed.
 * <p>
 * Returns true on success.
 *
 * @example OpenDialog( `Label( "Please wait..." ) )
 */

/**
 * @builtin OpenDialog( term options, term widget ) -> boolean
 *
 * Same as the OpenDialog with one argument, but you can specify options
 * with a term of the form <tt><b>`opt</b></tt>.
 * <p>
 * The option <tt>`defaultsize</tt> makes the dialog be resized to the default
 * size, for example for the Qt interface the -geometry option is honored and
 * for ncurses the dialog fills the whole window.
 * <p>
 * The option <tt>`decorated</tt> add a window border around the dialog, which
 * comes in handy if no window manager is running. This option may be ignored in
 * non-graphical UIs.
 * <p>
 * The option <tt>`warncolor</tt> displays the entire dialog in a bright
 * warning color.
 * <p>
 * The option <tt>`infocolor</tt> is a less intrusive color.
 *
 * @example OpenDialog( `opt( `defaultsize ), `Label( "Hi" ) )
 */

YCPValue YUIInterpreter::evaluateOpenDialog( const YCPTerm & term )
{
    int argc = term->size();

    if ( ( argc == 1 && term->value(0)->isTerm() ) ||	// Trivial case: No `opt(), only the dialog description term
	 ( argc == 2
	   && term->value(0)->asTerm()->symbol()->symbol() == YUISymbol_opt	// `opt(...)
	   && term->value(1)->isTerm() ) )					// The actual dialog
    {
	YWidgetOpt opt;

	if ( argc == 2 ) // evaluate `opt() contents
	{
	    YCPList optList = term->value(0)->asTerm()->args();

	    for ( int o=0; o < optList->size(); o++ )
	    {
		if ( optList->value(o)->isSymbol() )
		{
		    if      ( optList->value(o)->asSymbol()->symbol() == YUIOpt_defaultsize ) 	opt.hasDefaultSize.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_warncolor )	opt.hasWarnColor.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_infocolor )	opt.hasInfoColor.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_decorated )	opt.isDecorated.setValue( true );
		    else
			y2warning( "Unknown option %s for OpenDialog", term->value(o)->toString().c_str() );
		}
	    }
	}

	blockEvents();	// We don't want self-generated events from UI builtins.
	YDialog *dialog = createDialog( opt );

	if ( dialog )
	{
	    registerDialog( dialog ); // must be done first!
	    YWidget *widget = createWidgetTree( dialog, 0, term->value( argc-1 )->asTerm() );

	    if ( widget )
	    {
		dialog->addChild( widget );
		dialog->setInitialSize();
		dialog->checkShortcuts();
		showDialog( dialog );

		unblockEvents();
		return YCPBoolean( true );
	    }
	    else removeDialog();
	}

	unblockEvents();
	return YCPBoolean( false );
    }
    else
    {
	y2error( "Bad arguments for UI::OpenDialog()." );
	return YCPNull();
    }
}



/**
 * @builtin CloseDialog() -> boolean
 *
 * Closes the most recently opened dialog. It is an error
 * to call <tt>CloseDialog</tt> if no dialog is open.
 * <p>
 * Returns true on success.
 */

YCPValue YUIInterpreter::evaluateCloseDialog( const YCPTerm & term )
{
    if ( term->size() == 0 )
    {
	blockEvents();	// We don't want self-generated events from UI builtins.
	YDialog *dialog = currentDialog();

	if ( ! dialog )
	    return YCPError ( "Can't CloseDialog: No dialog existing.", YCPBoolean( false ) );

	closeDialog( dialog );
	removeDialog();
	unblockEvents();

	return YCPBoolean( true );
    }
    else return YCPNull();
}


void YUIInterpreter::registerDialog( YDialog *dialog )
{
    dialogstack.push_back( dialog );
}


void YUIInterpreter::removeDialog()
{
    delete currentDialog();
    dialogstack.pop_back();
}


YDialog *YUIInterpreter::currentDialog() const
{
    if ( dialogstack.size() >= 1 ) return dialogstack.back();
    else return 0;
}


void YUIInterpreter::showDialog( YDialog * )
{
    // dummy default implementation
}


void YUIInterpreter::closeDialog( YDialog * )
{
    // dummy default implementation
}



/**
 * @builtin ChangeWidget( `id( any widgetId ), symbol property, any newValue ) -> boolean
 * @builtin ChangeWidget( symbol widgetId, symbol property, any newValue ) -> boolean
 *
 * Changes a property of a widget of the topmost dialog. <tt>id</tt> specified
 * the widget to change, <tt>property</tt> specifies the property that should
 * be changed, <tt>newvalue</tt> gives the new value.
 * <p>
 * For example in order to change the label of a TextEntry with id `name to
 * "anything", you write <tt>ChangeWidget( `id(`name), `Label, "anything" )</tt>.
 * <p>
 * Returns true on success.
 */

YCPValue YUIInterpreter::evaluateChangeWidget( const YCPTerm & term )
{
    if ( term->size() != 3
	 || ( ! isSymbolOrId( term->value(0) ) )
	 || ( ! term->value(1)->isSymbol() &&
	      ! term->value(1)->isTerm()	) )
    {
	return YCPNull();
    }

    YCPValue id = getId( term->value(0) );
    YWidget *widget = widgetWithId( id, true );

    if ( ! widget )
	return YCPVoid();

    if ( term->value(1)->isSymbol() )
    {
	blockEvents();	// We don't want self-generated events from UI::ChangeWidget().
	YCPSymbol sym = term->value(1)->asSymbol();
	YCPValue ret = widget->changeWidget( sym, term->value(2) );

	if ( widget->shortcutProperty()				// The widget has a shortcut property
	     && sym->symbol() == widget->shortcutProperty()	// and this is what should be changed
	     && ret->isBoolean()				// and the change didn't return 'nil' (error)
	     && ret->asBoolean()->value() )			// and was successful
	{
	    // The shortcut property has just successfully been changed
	    // -> time for a new check

	    currentDialog()->checkShortcuts();
	}

	unblockEvents();
	return ret;
    }
    else
    {
	blockEvents();	// We don't want self-generated events from UI::ChangeWidget().
	YCPValue result = widget->changeWidget( term->value(1)->asTerm(), term->value(2) );
	unblockEvents();

	return result;
    }
}



/**
 * @builtin QueryWidget( `id( any id ), symbol | term property ) -> any
 * @builtin QueryWidget( symbol widgetId ), symbol | term property ) -> any
 *
 * Queries a property of a widget of the topmost dialog.  For example in order
 * to query the current text of a TextEntry with id `name, you write
 * <tt>QueryWidget( `id(`name), `Value )</tt>. In some cases the propery can be given
 * as term in order to further specify it. An example is
 * <tt>QueryWidget( `id( `table ), `Item( 17 ) )</tt> for a table where you query a
 * certain item.
 */

YCPValue YUIInterpreter::evaluateQueryWidget( const YCPTerm & term )
{
    if ( term->size() != 2
	 || ( ! isSymbolOrId( term->value(0) )
	 || ( ! term->value(1)->isSymbol() &&
	      ! term->value(1)->isTerm()     ) ) )
    {
	return YCPNull();
    }

    YCPValue id = getId( term->value(0) );
    YWidget *widget = widgetWithId( id, true ); // reports error

    if ( ! widget )
	return YCPVoid();

    if ( term->value(1)->isSymbol() )
	return widget->queryWidget( term->value(1)->asSymbol() );
    else
	return widget->queryWidget( term->value(1)->asTerm() );
}



/**
 * @builtin ReplaceWidget( `id( any id ), term newWidget ) -> boolean
 * @builtin ReplaceWidget( symbol id, term newWidget ) -> boolean
 *
 * Replaces a complete widget (or widget subtree) with an other widget
 * (or widget tree). You can only replace the widget contained in
 * a <tt>ReplacePoint</tt>. As parameters to <tt>ReplaceWidget</tt>
 * specify the id of the ReplacePoint and the new widget.
 * <p>
 * This is an example:
 * <pre>
 * OpenDialog( `ReplacePoint( `id( `rp ), `PushButton( "OK" ) ) );
 * ...
 * ReplaceWidget( `id( `rp ), `Label( "Label" ) )
 * </pre>
 */

YCPValue YUIInterpreter::evaluateReplaceWidget( const YCPTerm & term )
{
    if ( term->size() != 2
	 || ! isSymbolOrId( term->value(0) )
	 || ! term->value(1)->isTerm()
	 )
    {
	return YCPNull();
    }

    YCPValue id = getId( term->value(0) );
    YWidget *replpoint = widgetWithId( id, true ); // reports error
    if ( ! replpoint ) return YCPBoolean( false );

    if ( ! replpoint->isReplacePoint() )
    {
	y2error( "ReplaceWidget: widget %s is not a ReplacePoint",
		 id->toString().c_str() );
	return YCPBoolean( false );
    }

    YReplacePoint *rp = dynamic_cast <YReplacePoint *> ( replpoint );
    assert( rp );

    // What if the widget tree to be inserted contains radiobuttons, but the
    // radiobutton group is in the unchanged rest? We must find the radio button
    // group belonging to the new subtree.

    bool contains;
    YRadioButtonGroup *rbg = findRadioButtonGroup( currentDialog(), replpoint, & contains );

    // I must _first_ remove the old widget and then create the new ones. The reason
    // is: Otherwise you couldn't use the same widget ids in the old and new widget tree.

#ifdef VERBOSE_REPLACE_WIDGET
    rp->dumpDialogWidgetTree();
#endif

    YWidget::OptimizeChanges below( *currentDialog() ); // delay screen updates until this block is left

    rp->removeChildren();

    YWidget *widget = createWidgetTree( replpoint, rbg, term->value(1)->asTerm() );

    if ( widget )
    {
	blockEvents();	// We don't want self-generated events from UI builtins.
	rp->addChild( widget );
	currentDialog()->setInitialSize();
	currentDialog()->checkShortcuts();
	unblockEvents();

	return YCPBoolean( true );
    }
    else
    {
	blockEvents();	// We don't want self-generated events from UI builtins.

	widget = createWidgetTree( replpoint, rbg, YCPTerm( YCPSymbol( YUIWidget_Empty, true ), YCPList() ) );

	if ( widget )
	{
	    rp->addChild( widget );
	    currentDialog()->setInitialSize();
	    currentDialog()->checkShortcuts();
	}
	else // Something bad will happen
	    y2error( "Severe problem: can't create Empty widget" );

	unblockEvents();

	return YCPBoolean( false );
    }
}




/**
 * @builtin SetFocus( `id( any widgetId ) ) -> boolean
 * @builtin SetFocus( symbol widgetId ) ) -> boolean
 *
 * Sets the keyboard focus to the specified widget.  Notice that not all
 * widgets can accept the keyboard focus; this is limited to interactive
 * widgets like PushButtton, TextEntry, SelectionBox etc. - manager widgets
 * like VBox, HBox etc. will not accept the keyboard focus. They will not
 * propagate the keyboard focus to some child widget that accepts the
 * focus. Instead, an error message will be emitted into the log file.
 * <p>
 * Returns true on success (i.e. the widget accepted the focus).
 */

YCPValue YUIInterpreter::evaluateSetFocus( const YCPTerm & term )
{
    if ( term->size() != 1 || ! isSymbolOrId( term->value(0) ) )
	return YCPNull();

    YCPValue id = getId( term->value(0) );
    YWidget *widget = widgetWithId( id, true );

    if ( ! widget )
	return YCPVoid();

    return YCPBoolean( widget->setKeyboardFocus() );
}



/**
 * @builtin BusyCursor() -> void
 *
 * Sets the mouse cursor to the busy cursor, if the UI supports such a feature.
 * <p>
 * This should normally not be necessary. The UI handles mouse cursors itself:
 * When input is possible (i.e. inside UserInput() ), there is automatically a
 * normal cursor, otherwise, there is the busy cursor. Override this at your
 * own risk.
 */

YCPValue YUIInterpreter::evaluateBusyCursor( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    busyCursor();
    return YCPVoid();
}



/**
 * @builtin RedrawScreen() -> void
 *
 * Redraws the screen after it very likely has become garbled by some other output.
 * <p>
 * This should normally not be necessary: The ( specific ) UI redraws the screen
 * automatically whenever required. Under rare circumstances, however, the
 * screen might have changes due to circumstances beyond the UI's control: For
 * text based UIs, for example, system commands that cause output to every tty
 * might make this necessary. Call this in the YCP code after such a command.
 */

YCPValue YUIInterpreter::evaluateRedrawScreen( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    redrawScreen();
    return YCPVoid();
}


/**
 * @builtin NormalCursor() -> void
 *
 * Sets the mouse cursor to the normal cursor ( after BusyCursor ), if the UI
 * supports such a feature.
 * <p>
 * This should normally not be necessary. The UI handles mouse cursors itself:
 * When input is possible (i.e. inside UserInput() ), there is automatically a
 * normal cursor, otherwise, there is the busy cursor. Override this at your
 * own risk.
 */

YCPValue YUIInterpreter::evaluateNormalCursor( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    normalCursor();
    return YCPVoid();
}



/**
 * @builtin MakeScreenShot( string filename ) -> void
 *
 * Make a screen shot if the specific UI supports that.
 * The Qt UI opens a file selection box if filename is empty.
 */

YCPValue YUIInterpreter::evaluateMakeScreenShot( const YCPTerm & term )
{
    if ( term->size() != 1 || ! term->value(0)->isString() )
	return YCPNull();

    makeScreenShot( term->value(0)->asString()->value() );
    return YCPVoid();
}



/**
 * @builtin DumpWidgetTree() -> void
 *
 * Debugging function: Dump the widget tree of the current dialog to the log
 * file.
 */

YCPValue YUIInterpreter::evaluateDumpWidgetTree( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "DumpWidgetTree: No dialog existing." );
	return YCPVoid();
    }

    dialog->dumpDialogWidgetTree();

    return YCPVoid();
}


/**
 * @builtin RecordMacro( string macroFileName ) -> void
 *
 * Begin recording a macro. Write the macro contents to file "macroFilename".
 */
YCPValue YUIInterpreter::evaluateRecordMacro( const YCPTerm & term )
{
    if ( term->size() == 1 && term->value(0)->isString() )
    {
	recordMacro( term->value(0)->asString()->value() );

	return YCPVoid();
    }
    else
    {
	return YCPNull();
    }
}


void YUIInterpreter::recordMacro( string filename )
{
    deleteMacroRecorder();
    macroRecorder = new YMacroRecorder( filename );
}


void YUIInterpreter::deleteMacroRecorder()
{
    if ( macroRecorder )
    {
	delete macroRecorder;
	macroRecorder = 0;
    }
}



/**
 * @builtin StopRecordingMacro() -> void
 *
 * Stop macro recording. This is only necessary if you don't wish to record
 * everything until the program terminates.
 */
YCPValue YUIInterpreter::evaluateStopRecordMacro( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    stopRecordMacro();

    return YCPVoid();
}


void YUIInterpreter::stopRecordMacro()
{
    deleteMacroRecorder();
}


/**
 * @builtin PlayMacro( string macroFileName ) -> void
 *
 * Execute everything in macro file "macroFileName".
 * Any errors are sent to the log file only.
 * The macro can be terminated only from within the macro file.
 */
YCPValue YUIInterpreter::evaluatePlayMacro( const YCPTerm & term )
{
    if ( term->size() == 1 && term->value(0)->isString() )
    {
	playMacro( term->value(0)->asString()->value() );

	return YCPVoid();
    }
    else
    {
	return YCPNull();
    }
}


void YUIInterpreter::playMacro( string filename )
{
    deleteMacroPlayer();
    macroPlayer = new YMacroPlayer( filename );
}


void YUIInterpreter::playNextMacroBlock()
{
    if ( ! macroPlayer )
    {
	y2error( "No macro player active." );
	return;
    }

    if ( macroPlayer->error() || macroPlayer->finished() )
    {
	deleteMacroPlayer();
    }
    else
    {
	if ( ! macroPlayer->finished() )
	{
	    YCPBlock macroBlock = macroPlayer->nextBlock();

	    if ( macroPlayer->error() || macroBlock.isNull() )
	    {
		y2error( "Macro aborted" );
		deleteMacroPlayer();
	    }
	    else
	    {
		y2milestone( "Evaluating macro block:\n%s", macroBlock->toString().c_str() );
		evaluate( macroBlock );
	    }
	}
    }
}


void YUIInterpreter::deleteMacroPlayer()
{
    if ( macroPlayer )
    {
	delete macroPlayer;
	macroPlayer = 0;
    }
}


/**
 * @builtin FakeUserInput( any nextUserInput ) -> void
 *
 * Prepare a fake value for the next call to UserInput() -
 * i.e. the next UserInput() will return exactly this value.
 * This is only useful in connection with macros.
 * If called from within a macro, this will relinquish control from the macro
 * to the YCP code until the next UserInput.
 * <p>
 * "nil" is a legal value.
 */
YCPValue YUIInterpreter::evaluateFakeUserInput( const YCPTerm & term )
{
    if ( term->size() != 1 )	// must have 1 arg - anything allowed
	return YCPNull();

    fakeUserInputQueue.push_back( term->value(0) );

    return YCPVoid();
}



/**
 * @builtin Glyph( symbol glyph ) -> string
 *
 * Return a special character ( a 'glyph' ) according to the symbol specified.
 * <p>
 * Not all UIs may be capable of displaying every glyph; if a specific UI
 * doesn't support it, a textual representation ( probably in plain ASCII ) will
 * be returned.
 * <p>
 * This is also why there is only a limited number of predefined
 * glyphs: An ASCII equivalent is required which is sometimes hard to find for
 * some characters defined in Unicode / UTF-8.
 * <p>
 * Please note the value returned may consist of more than one character; for
 * example, Glyph( `ArrowRight ) may return something like "-&gt;".
 * <p>
 * Predefined glyphs include:
 * <ul>
 *     <li>`ArrowLeft
 *     <li>`ArrowRight
 *     <li>`ArrowUp
 *     <li>`ArrowDown
 *     <li>`CheckMark
 *     <li>`BulletArrowRight
 *     <li>`BulletCircle
 *     <li>`BulletSquare
 * </ul>
 * <p>
 * If an unknown glyph symbol is specified, 'nil' is returned.
 * <p>
 * See also the <a href="examples/Glyphs.ycp">Glyphs.ycp</a>
 * UI example:
 * <table border=2><tr><td>
 * <img src="examples/screenshots/Glyphs.png">
 * </td></tr></table>
 * <i>Glyphs in the Qt UI</i>
 * <p>
 * <img src="examples/screenshots/Glyphs-ncurses.png">
 * <p><i>Glyphs in the NCurses UI</i>
 */
YCPValue YUIInterpreter::evaluateGlyph( const YCPTerm & term )
{
    if ( term->size() != 1 || ! term->value(0)->isSymbol() )
    {
	y2error( "Expected symbol, not %s", term->toString().c_str() );
	return YCPNull();
    }

    YCPSymbol glyphSym = term->value(0)->asSymbol();


    YCPString glyphText = glyph( glyphSym );	// ask specific UI

    if ( glyphText->value().length() == 0 )	// specific UI doesn't have a suitable representation
    {
	string sym = glyphSym->symbol();

	if	( sym == YUIGlyph_ArrowLeft		)	glyphText = YCPString( "<-"  );
	else if ( sym == YUIGlyph_ArrowRight		)	glyphText = YCPString( "->"  );
	else if ( sym == YUIGlyph_ArrowUp		)	glyphText = YCPString( "^"   );
	else if ( sym == YUIGlyph_ArrowDown		)	glyphText = YCPString( "v"   );
	else if ( sym == YUIGlyph_CheckMark		)	glyphText = YCPString( "[x]" );
	else if ( sym == YUIGlyph_BulletArrowRight	)	glyphText = YCPString( "=>"  );
	else if ( sym == YUIGlyph_BulletCircle		)	glyphText = YCPString( "o"   );
	else if ( sym == YUIGlyph_BulletSquare		)	glyphText = YCPString( "[]"  );
	else	// unknown glyph symbol
	{
	    y2error( "Unknown glyph `%s", sym.c_str() );
	    return YCPNull();
	}
    }

    return glyphText;
}



/**
 * @builtin GetDisplayInfo() -> map
 *
 * Get information about the current display and the UI's capabilities.
 * <p>
 * Returns a map with the following contents:
 * <p>
 * <table>
 *	<tr>
 *		<td>	Width			</td>	<td>integer</td>
 *		<td>
 *			The overall display width.
 *			<p>Unit: characters for text mode UIs,
 *			pixels for graphical UIs.
 *			<p>-1 means unknown.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	Height			</td>	<td>integer</td>
 *		<td>
 *			The overall display height.
 *			<p>-1 means unknown.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	Depth			</td>	<td>integer</td>
 *		<td>
 *			The display color depth.
 *			Meaningful only for graphical displays.
 *			<p>-1 means unknown.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	Colors			</td	<td>integer</td>
 *		<td>
 *			The number of colors that can be displayed
 *			simultaneously at any one time.
 *			<p>-1 means unknown.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	DefaultWidth		</td>	<td>integer</td>
 *		<td>	The width of a `opt( `defaultsize ) dialog.</td>
 *	</tr>
 *	<tr>
 *		<td>	DefaultHeight		</td>	<td>integer</td>
 *		<td>	The height of a `opt( `defaultsize ) dialog.</td>
 *	</tr>
 *	<tr>
 *		<td>	TextMode		</td>	<td>boolean</td>
 *		<td>
 *			<i>true</i> if text mode only supported.
 *			<b>Don't misuse this!</b> See remark below.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	HasImageSupport		</td>	<td>boolean</td>
 *		<td>	<i>true</i> if images can be displayed.</td>
 *	</tr>
 *	<tr>
 *		<td>	HasLocalImageSupport	</td>	<td>boolean</td>
 *		<td>	<i>true</i> if images can be loaded from local files rather than
 *			having to use <tt>SCR::Read( .target.byte, ... )</tt>.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	HasAnimationSupport	</td>	<td>boolean</td>
 *		<td>	<i>true</i> if animations can be displayed,
 *			i.e. if the Image widget supports <tt>`opt( `animated )</tt>.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	HasIconSupport		</td>	<td>boolean</td>
 *		<td>	<i>true</i> if icons can be displayed.</td>
 *	</tr>
 *	<tr>
 *		<td>	HasFullUtf8Support	</td>	<td>boolean</td>
 *		<td>	<i>true</i> if all UTF-8 characters can be displayed simultaneously.</td>
 *	</tr>
 * </table>
 *
 * <p>
 * <b>Important:</b> Don't misuse this function to simply not support the
 * NCurses UI properly! Always think twice before checking for <i>text
 * mode</i>. If you think there is no proper layout etc. solution for NCurses,
 * it might be time to reconsider the complexity or even the concept of your
 * dialog.
 *
 */
YCPValue YUIInterpreter::evaluateGetDisplayInfo( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    YCPMap info_map;

    info_map->add( YCPString( YUICap_Width			), YCPInteger( getDisplayWidth()	) );
    info_map->add( YCPString( YUICap_Height			), YCPInteger( getDisplayHeight()	) );
    info_map->add( YCPString( YUICap_Depth			), YCPInteger( getDisplayDepth()	) );
    info_map->add( YCPString( YUICap_Colors			), YCPInteger( getDisplayColors()	) );
    info_map->add( YCPString( YUICap_DefaultWidth		), YCPInteger( getDefaultWidth()	) );
    info_map->add( YCPString( YUICap_DefaultHeight		), YCPInteger( getDefaultHeight()	) );
    info_map->add( YCPString( YUICap_TextMode			), YCPBoolean( textMode()		) );
    info_map->add( YCPString( YUICap_HasImageSupport		), YCPBoolean( hasImageSupport()	) );
    info_map->add( YCPString( YUICap_HasLocalImageSupport	), YCPBoolean( hasLocalImageSupport()	) );
    info_map->add( YCPString( YUICap_HasAnimationSupport	), YCPBoolean( hasAnimationSupport()	) );
    info_map->add( YCPString( YUICap_HasIconSupport		), YCPBoolean( hasIconSupport()		) );
    info_map->add( YCPString( YUICap_HasFullUtf8Support		), YCPBoolean( hasFullUtf8Support()	) );

    return info_map;
}


/**
 * @builtin RecalcLayout() -> void
 *
 * Recompute the layout of the current dialog.
 * <p>
 * <b>This is a very expensive operation.</b>
 * <p>
 * Use this after changing widget properties that might affect their size -
 * like the a Label widget's value. Call this once ( ! ) after changing all such
 * widget properties.
 */
YCPValue YUIInterpreter::evaluateRecalcLayout( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "RecalcLayout(): No dialog existing" );
    }
    else
    {
	dialog->setInitialSize();
    }

    return YCPVoid();
}


/**
 * @builtin PostponeShortcutCheck() -> void
 *
 * Postpone keyboard shortcut checking during multiple changes to a dialog.
 * <p>
 * Normally, keyboard shortcuts are checked automatically when a dialog is
 * created or changed. This can lead to confusion, however, when multiple
 * changes to a dialog ( repeated ReplaceWidget() calls ) cause unwanted
 * intermediate states that may result in shortcut conflicts while the dialog
 * is not final yet. Use this function to postpone this checking until all
 * changes to the dialog are done and then explicitly check with
 * <tt>CheckShortcuts()</tt>. Do this before the next call to
 * <tt>UserInput()</tt> or <tt>PollInput()</tt> to make sure the dialog doesn't
 * change "on the fly" while the user tries to use one of those shortcuts.
 * <p>
 * The next call to <tt>UserInput()</tt> or <tt>PollInput()</tt> will
 * automatically perform that check if it hasn't happened yet, any an error
 * will be issued into the log file.
 * <p>
 * Use only when really necessary. The automatic should do well in most cases.
 * <p>
 * The normal sequence looks like this:
 * <p>
 * <pre>
 * PostponeShortcutChecks();
 * ReplaceWidget( ... );
 * ReplaceWidget( ... );
 * ...
 * ReplaceWidget( ... );
 * CheckShortcuts();
 * ...
 * UserInput();
 * </pre>
 */
YCPValue YUIInterpreter::evaluatePostponeShortcutCheck( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "PostponeShortcutCheck(): No dialog existing" );
    }
    else
    {
	dialog->postponeShortcutCheck();
    }

    return YCPVoid();
}


/**
 * @builtin CheckShortcuts() -> void
 *
 * Perform an explicit shortcut check after postponing shortcut checks.
 * Use this after calling <tt>PostponeShortcutCheck()</tt>.
 * <p>
 * The normal sequence looks like this:
 * <p>
 * <pre>
 * PostponeShortcutChecks();
 * ReplaceWidget( ... );
 * ReplaceWidget( ... );
 * ...
 * ReplaceWidget( ... );
 * CheckShortcuts();
 * ...
 * UserInput();
 * </pre>
 */
YCPValue YUIInterpreter::evaluateCheckShortcuts( const YCPTerm & term )
{
    if ( term->size() != 0 )
	return YCPNull();

    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "CheckShortcuts(): No dialog existing" );
    }
    else
    {
	if ( ! dialog->shortcutCheckPostponed() )
	{
	    y2warning( "Use UI::CheckShortcuts() only after UI::PostponeShortcutCheck() !" );
	}

	dialog->checkShortcuts( true );
    }

    return YCPVoid();
}


/**
 * @builtin WidgetExists( `id( any widgetId ) ) -> boolean
 * @builtin WidgetExists( symbol widgetId ) ) -> boolean
 *
 * Check whether or not a widget with the given ID currently exists in the
 * current dialog. Use this to avoid errors in the log file before changing the
 * properties of widgets that might or might not be there.
 */
YCPValue YUIInterpreter::evaluateWidgetExists( const YCPTerm & term )
{
    if ( term->size() != 1
	 || ! isSymbolOrId( term->value(0) ) ) return YCPNull();

    YCPValue id = getId( term->value(0) );
    YWidget *widget = widgetWithId( id, false ); // reports error
    return widget ? YCPBoolean( true ) : YCPBoolean( false );
}


/**
 * @builtin RunPkgSelection( `id( any pkgSelId ) -> any
 *
 * <b>Not to be used outside the package selection</b>
 * Initialize and run the PackageSelector widget identified by 'pkgSelId'.
 * Black magic to everybody outside. ;- )
 * <p>
 * Returns `cancel if the user wishes to cancel his selections.
 */
YCPValue YUIInterpreter::evaluateRunPkgSelection( const YCPTerm & term )
{
    if ( term->size() != 1
	 || ! isSymbolOrId( term->value( 0 ) ) )
    {
	y2error( "RunPkgSelection(): expecting `id( ... ), not '%s'", term->toString().c_str() );
	return YCPNull();
    }

    YCPValue id = getId( term->value(0) );
    YWidget * selector = widgetWithId( id, true );

    if ( ! selector )
    {
	y2error( "RunPkgSelection(): No PackageSelector widget with ID '%s'",
		 id->toString().c_str() );

	return YCPVoid();
    }

    // call overloaded method from specific UI
    return runPkgSelection( selector );
}



/**
 * @builtin AskForExistingDirectory( string startDir, string headline ) -> string
 *
 * Open a directory selection box and prompt the user for an existing directory.
 * <p>
 * <i>startDir</i> is the initial directory that is displayed.
 * <p>
 * <i>headline</i> is an explanatory text for the directory selection box.
 * Graphical UIs may omit that if no window manager is running.
 * <p>
 * Returns the selected directory name or <i>nil</i> if the user canceled the operation.
 */
YCPValue YUIInterpreter::evaluateAskForExistingDirectory( const YCPTerm & term )
{
    if ( term->size() == 2 &&
	 term->value(0)->isString() &&
	 term->value(1)->isString()   )
    {
	return askForExistingDirectory( term->value(0)->asString(),
					term->value(1)->asString() );
    }

    return YCPNull();
}



/**
 * @builtin AskForExistingFile( string startWith, string filter, string headline ) -> string
 *
 * Open a file selection box and prompt the user for an existing file.
 * <p>
 * <i>startWith</i> is the initial directory or file.
 * <p>
 * <i>filter</i> is one or more blank-separated file patterns, e.g. "*.png *.jpg"
 * <p>
 * <i>headline</i> is an explanatory text for the file selection box.
 * Graphical UIs may omit that if no window manager is running.
 * <p>
 * Returns the selected file name or <i>nil</i> if the user canceled the operation.
 */
YCPValue YUIInterpreter::evaluateAskForExistingFile( const YCPTerm & term )
{
    if ( term->size() == 3 &&
	 term->value(0)->isString() &&
	 term->value(1)->isString() &&
	 term->value(2)->isString()   )
    {
	return askForExistingFile( term->value(0)->asString(),
				   term->value(1)->asString(),
				   term->value(2)->asString() );

    }

    return YCPNull();
}


/**
 * @builtin AskForSaveFileName( string startWith, string filter, string headline ) -> string
 *
 * Open a file selection box and prompt the user for a file to save data to.
 * Automatically asks for confirmation if the user selects an existing file.
 * A
 * <p>
 * <i>startWith</i> is the initial directory or file.
 * <p>
 * <i>filter</i> is one or more blank-separated file patterns, e.g. "*.png *.jpg"
 * <p>
 * <i>headline</i> is an explanatory text for the file selection box.
 * Graphical UIs may omit that if no window manager is running.
 * <p>
 * Returns the selected file name or <i>nil</i> if the user canceled the operation.
 */
YCPValue YUIInterpreter::evaluateAskForSaveFileName( const YCPTerm & term )
{
    if ( term->size() == 3 &&
	 term->value(0)->isString() &&
	 term->value(1)->isString() &&
	 term->value(2)->isString()   )
    {
	return askForSaveFileName( term->value(0)->asString(),
				   term->value(1)->asString(),
				   term->value(2)->asString() );

    }

    return YCPNull();
}


/**
 * @builtin SetFunctionKeys( map fkeys ) -> void
 *
 * Set the ( default ) function keys for a number of buttons.
 * <p>
 * This function receives a map with button labels and the respective function
 * key number that should be used if on other `opt( `key_F.. ) is specified.
 * <p>
 * Any keyboard shortcuts in those labels are silently ignored so this is safe
 * to use even if the UI's internal shortcut manager rearranges shortcuts.
 * <p>
 * Each call to this function overwrites the data of any previous calls.
 * <p>
 * @example SetFunctionKeys( $[ "Back": 8, "Next": 10, ... ] );
 */
YCPValue YUIInterpreter::evaluateSetFunctionKeys( const YCPTerm & term )
{
    if ( term->size() == 1 && term->value(0)->isMap() )
    {
	YCPMap new_fkeys( term->value(0)->asMap() );
	default_fkeys = YCPMap();

	for ( YCPMapIterator it = new_fkeys->begin(); it != new_fkeys->end(); ++it )
	{
	    if ( it.key()->isString() && it.value()->isInteger() )
	    {
		string label = YShortcut::cleanShortcutString( it.key()->asString()->value() );
		int fkey = it.value()->asInteger()->value();

		if ( fkey > 0 && fkey <= 24 )
		{
		    y2milestone( "Mapping \"%s\"\t-> F%d", label.c_str(), fkey );
		    default_fkeys->add( YCPString( label ), it.value()->asInteger() );
		}
		else
		{
		    y2error( "SetFunctionKeys(): Function key %d out of range for \"%s\"",
			     fkey, label.c_str() );
		}
	    }
	    else
	    {
		y2error( "SetFunctionKeys(): Invalid map element: "
			 "Expected <string>: <integer>, not %s: %s",
			 it.key()->toString().c_str(), it.value()->toString().c_str() );
	    }
	}
	return YCPVoid();
    }
    else
	return YCPNull();
}


int YUIInterpreter::defaultFunctionKey( YCPString ylabel )
{
    int fkey = 0;

    string label = YShortcut::cleanShortcutString( ylabel->value() );

    if ( label.size() > 0 )
    {
	YCPValue val = default_fkeys->value( YCPString( label ) );

	if ( ! val.isNull() && val->isInteger() )
	    fkey = val->asInteger()->value();
    }

    return fkey;
}


/**
 * @builtin WFM/SCR ( expression ) -> any
 *
 * This is used for a callback mechanism. The expression will
 * be sent to the WFM interpreter and evaluated there.
 * USE WITH CAUTION.
 */

YCPValue YUIInterpreter::evaluateCallback( const YCPTerm & term, bool to_wfm )
{
    y2debug ( "( %s ), callback @ %p", term->toString().c_str(), callbackComponent );
    if ( term->size() != 1 )	// must have 1 arg - anything allowed
    {
	return YCPNull();
    }

    if ( callbackComponent )
    {
	YCPValue v = YCPNull();
	if ( to_wfm )		// if it goes to WFM, just send the value
	{
	    v = callbackComponent->evaluate ( term->value(0) );
	}
	else		// going to SCR, send the complete term
	{
	    v = callbackComponent->evaluate( term );
	}
	y2debug ( "callback returns ( %s )", v->toString().c_str() );
	return v;
    }

    return YCPVoid();
}




/**
 * @builtin Recode ( string from, string to, string text ) -> any
 *
 * Recode encoding of string from or to "UTF-8" encoding.
 * One of from/to must be "UTF-8", the other should be an
 * iso encoding specifier (i.e. "ISO-8859-1" for western languages,
 * "ISO-8859-2" for eastern languages, etc. )
 */

YCPValue YUIInterpreter::evaluateRecode( const YCPTerm & term )
{
    if ( ( term->size() != 3 )
	 || ! (term->value(0)->isString())
	 || ! (term->value(1)->isString())
	 || ! (term->value(2)->isString()))
    {
	y2error( "Wrong number or type of arguments for Recode ( string from, string to, string text )" );
	return YCPVoid();
    }

    string outstr;
    if ( Recode ( term->value(2)->asString()->value(),
		  term->value(0)->asString()->value(),
		  term->value(1)->asString()->value(),
		  outstr ) != 0 )
    {
	static bool warned_about_recode = false;
	if ( ! warned_about_recode )
	{
	    y2error ( "Recode ( %s, %s, ... )", term->value(0)->asString()->value().c_str(), term->value( 1)->asString()->value().c_str() );
	    warned_about_recode = true;
	}
	// return text as-is
	return ( term->value(2)->asString() );
    }
    return YCPString ( outstr );
}



static iconv_t fromutf8_cd   = ( iconv_t )( -1 );
static string  fromutf8_name = "";

static iconv_t toutf8_cd     = ( iconv_t )( -1 );
static string  toutf8_name   = "";

static iconv_t fromto_cd     = ( iconv_t )( -1 );
static string  from_name     = "";
static string  to_name	     = "";

static const unsigned recode_buf_size = 1024;
static char	      recode_buf[ recode_buf_size ];

int YUIInterpreter::Recode( const string & instr, const string & from,
			    const string & to, string & outstr )
{
    if ( from == to
	 || instr.empty())
    {
	outstr = instr;
	return 0;
    }

    outstr.clear();
    iconv_t cd = ( iconv_t )( -1 );

    if ( from == "UTF-8" )
    {
	if ( fromutf8_cd == ( iconv_t )( -1 )
	     || fromutf8_name != to)
	{
	    if ( fromutf8_cd != ( iconv_t )( -1 ) )
	    {
		iconv_close ( fromutf8_cd );
	    }
	    fromutf8_cd = iconv_open ( to.c_str(), from.c_str() );
	    fromutf8_name = to;
	}
	cd = fromutf8_cd;
    }
    else if ( to == "UTF-8" )
    {
	if ( toutf8_cd == ( iconv_t )( -1 )
	     || toutf8_name != from)
	{
	    if ( toutf8_cd != ( iconv_t )( -1 ) )
	    {
		iconv_close ( toutf8_cd );
	    }
	    toutf8_cd = iconv_open ( to.c_str(), from.c_str() );
	    toutf8_name = from;
	}
	cd = toutf8_cd;
    }
    else
    {
	if ( fromto_cd == ( iconv_t )( -1 )
	     || from_name != from
	     || to_name != to)
	{
	    if ( fromto_cd != ( iconv_t )( -1 ) )
	    {
		iconv_close ( fromto_cd );
	    }
	    fromto_cd = iconv_open ( to.c_str(), from.c_str() );
	    from_name = from;
	    to_name   = to;
	}
	cd = fromto_cd;
    }

    if ( cd == ( iconv_t )( -1 ) )
    {
	static bool complained = false;
	if ( ! complained )
	{
	    // glibc-locale is not necessarily installed so only complain once
	    y2error ( "Recode: ( errno %d ) failed conversion '%s' to '%s'", errno, from.c_str(), to.c_str() );
	    complained = true;
	}
	outstr = instr;
	return 1;
    }

    size_t inbuf_len  = instr.length();
    size_t outbuf_len = recode_buf_size-1;
    char * outbuf = recode_buf;

    char * inptr  = ( char * ) instr.c_str();
    char * outptr = outbuf;
    char * l	  = NULL;

    size_t iconv_ret = ( size_t )( -1 );

    do
    {
	iconv_ret = iconv ( cd, ( & inptr ), & inbuf_len, & outptr, & outbuf_len );

	if ( iconv_ret == ( size_t )( -1 ) )
	{

	    if ( errno == EILSEQ )	// Illegal multibyte sequence?
	    {
		if ( l != outptr )
		{
		    *outptr++ = '?';	// Insert '?' for the illegal character
		    outbuf_len--;	// Account for that '?'
		    l = outptr;
		}
		inptr++;
		continue;
	    }
	    else if ( errno == EINVAL )
	    {
		inptr++;
		continue;
	    }
	    else if ( errno == E2BIG )	// Buffer overflow?
	    {
		*outptr = '\0';		// Terminate converted buffer contents
		outstr += recode_buf;	// Append buffer to output string

		// Set up buffer for the next chunk and start over
		outptr = recode_buf;
		outbuf_len = recode_buf_size-1;
		continue;
	    }
	}

    } while ( inbuf_len != ( size_t )(0) );

    *outptr = '\0';			// Terminate converted buffer contents
    outstr += recode_buf;		// Append buffer to output string

    return 0;
}



// EOF
