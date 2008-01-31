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

  File:		YUI_builtins.cc

  Authors:	Mathias Kettner <kettner@suse.de>
		Stefan Hundhammer <sh@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#define VERBOSE_REPLACE_WIDGET 		0
#define VERBOSE_EVENTS			0
#define VERBOSE_DISCARDED_EVENTS	0

#include <stdio.h>
#include <unistd.h> 	// pipe()
#include <fcntl.h>  	// fcntl()
#include <errno.h>  	// strerror()
#include <pthread.h>
#include <assert.h>
#include <string.h>
#include <iconv.h>

#define y2log_component "ui"
#include <ycp/y2log.h>	// ycperror()

#define YUILogComponent "ui"
#include "YUILog.h"

#include <Y2.h>
#include <ycp/YCPVoid.h>

#include "YUI.h"
#include "YUI_util.h"
#include "YApplication.h"
#include "YWidget.h"
#include "YEvent.h"
#include "YUIException.h"
#include "YUISymbols.h"
#include "YDialog.h"
#include "YMacroRecorder.h"
#include "YMacroPlayer.h"
#include "YReplacePoint.h"
#include "YShortcut.h"
#include "YWizard.h"
#include "YWidgetFactory.h"
#include "YCPErrorDialog.h"
#include "YCPEvent.h"
#include "YCPValueWidgetID.h"
#include "YCPDialogParser.h"
#include "YCPItemParser.h"
#include "YCPPropertyHandler.h"
#include "YCPWizardCommandParser.h"
#include "YOptionalWidgetFactory.h"
#include "YCheckBox.h"

using std::string;


/**
 * @builtin HasSpecialWidget
 * @short Checks for support of a special widget type.
 * @description
 * Checks for support of a special widget type. Use this prior to creating a
 * widget of this kind. Do not use this to check for ordinary widgets like
 * PushButton etc. - just the widgets where the widget documentation explicitly
 * states it is an optional widget not supported by all UIs.
 *
 * Returns true if the UI supports the special widget and false if not.
 */

YCPValue YUI::evaluateHasSpecialWidget( const YCPSymbol & widget )
{
    YOptionalWidgetFactory * fact = YUI::optionalWidgetFactory();

    if ( ! fact )
	return YCPBoolean( false );

    bool   hasWidget = false;
    string symbol    = widget->symbol();

    if	    ( symbol == YUISpecialWidget_DummySpecialWidget	)	hasWidget = fact->hasDummySpecialWidget();
    else if ( symbol == YUISpecialWidget_BarGraph		)	hasWidget = fact->hasBarGraph();
    else if ( symbol == YUISpecialWidget_DumbTab		)	hasWidget = fact->hasDumbTab();
    else if ( symbol == YUISpecialWidget_DownloadProgress	)	hasWidget = fact->hasDownloadProgress();
    else if ( symbol == YUISpecialWidget_HMultiProgressMeter	)	hasWidget = fact->hasMultiProgressMeter();
    else if ( symbol == YUISpecialWidget_VMultiProgressMeter	)	hasWidget = fact->hasMultiProgressMeter();
    else if ( symbol == YUISpecialWidget_Slider			)	hasWidget = fact->hasSlider();
    else if ( symbol == YUISpecialWidget_PatternSelector	)	hasWidget = fact->hasPatternSelector();
    else if ( symbol == YUISpecialWidget_PartitionSplitter	)	hasWidget = fact->hasPartitionSplitter();
    else if ( symbol == YUISpecialWidget_SimplePatchSelector	)	hasWidget = fact->hasSimplePatchSelector();
    else if ( symbol == YUISpecialWidget_Wizard			)	hasWidget = fact->hasWizard();
    else if ( symbol == YUISpecialWidget_Date			)	hasWidget = fact->hasDateField();
    else if ( symbol == YUISpecialWidget_Time			)	hasWidget = fact->hasTimeField();
    else if ( symbol == YUISpecialWidget_TimezoneSelector	)	hasWidget = fact->hasTimezoneSelector();
    else
    {
	yuiError() << "HasSpecialWidget(): Unknown special widget: " << symbol << endl;
	return YCPNull();
    }

    return YCPBoolean( hasWidget );
}



/**
 * @builtin SetModulename
 * @short Sets Module Name
 * @description
 * Does nothing. The SetModulename command is introduced for
 * the translator. But the translator sends all commands
 * to the ui. So the ui shouldn't complain about this
 * command.
 * @param string module
 * @return void
 * @usage SetModulename( "inst_environment" )
 */

void YUI::evaluateSetModulename( const YCPString & name )
{
    _moduleName = name->value();
}


const char * YUI::moduleName()
{
    return _moduleName.c_str();
}



/**
 * @builtin GetModulename
 * @short Gets the name of a Module
 * @description
 * This is tricky. The UI doesn't care about the current module
 * name, only the translator does. However, since the translator
 * acts as a filter between a client and the UI, it cant directly
 * return the module name. The current implementation inserts the
 * modules name in the translator and it arrives here as the term
 * argument. So the example has no arguments, but the internal code
 * checks for a string argument.
 * @return string
 *
 * @usage GetModulename()
 */

YCPValue YUI::evaluateGetModulename( const YCPTerm & term )
{
    if ( ( term->size() == 1 ) && ( term->value(0)->isString() ) )
    {
	return term->value(0);
    }
    else return YCPNull();
}



/**
 * @builtin SetLanguage
 * @short Sets the language of the UI
 * @description
 * Tells the UI that the user has selected another language.
 * If the UI has any language dependend output that language
 * setting is honored. <tt>lang</tt> is an ISO language string,
 * such as <tt>de</tt> or <tt>de_DE</tt>. It is required
 * to specify an encoding scheme, since not all user interfaces
 * are capable of UTF-8.
 * @param string lang Language selected by user
 * @optarg string encoding
 * @return void
 *
 * @usage SetLanguage( "de_DE@euro" )
 * @usage SetLanguage( "en_GB" )
 */

void YUI::evaluateSetLanguage( const YCPString & language, const YCPString & encoding )
{
    YUI::app()->setLanguage( language->value(), encoding->value() );
}


/**
 * @builtin GetProductName
 * @short Gets Product Name
 * @description
 * Returns the current product name ("SuSE Linux", "United Linux", etc.) for
 * display in dialogs. This can be set with SetProductName().
 *
 * Note: In help texts in RichText widgets, a predefined macro &amp;product;
 * can be used for the same purpose.
 *
 * @return string Product Name
 * @usage sformat( "Welcome to %1", GetProductName() );
 **/
YCPString YUI::evaluateGetProductName()
{
    return YCPString( _productName );
}


/**
 * @builtin SetProductName
 * @short Sets Product Name
 * @description
 * Sets the current product name ("SuSE Linux", "United Linux", etc.) for
 * displaying in dialogs and in RichText widgets (for help text) with the RichText
 * &amp;product; macro.
 *
 * This product name should be concise and meaningful to the user and not
 * cluttered with detailed version information. Don't use something like
 * "SuSE Linux 12.3-i786 Professional". Use something like "SuSE Linux"
 * instead.
 *
 * This information can be retrieved with the GetProductName() builtin.
 * @param string prod
 * @return void
 *
 * @usage SetProductName( "SuSE HyperWall" );
 **/
void YUI::evaluateSetProductName( const YCPString & name )
{
    _productName = name->value();
}


/**
 * @builtin SetConsoleFont
 * @short Sets Console Font
 * @description
 * Switches the text console to the specified font.
 * See the setfont(8) command and the console HowTo for details.
 * @see setfont(8)
 * @param string console_magic
 * @param string font
 * @param string screen_map
 * @param string unicode_map
 * @param string encoding
 * @return void
 *
 * @usage SetConsoleFont( "( K", "lat2u-16.psf", "latin2u.scrnmap", "lat2u.uni", "latin1" )
 */

void YUI::evaluateSetConsoleFont( const YCPString & console_magic, const YCPString & font,
    const YCPString & screen_map, const YCPString & unicode_map, const YCPString & encoding )
{
    setConsoleFont( console_magic, font, screen_map, unicode_map, encoding );
}


/**
 * @builtin RunInTerminal
 * @short runs external program in the same terminal
 * @description
 * Use this builtin if you want to run external program from ncurses UI
 * as a separate process. It saves current window layout to the stack and
 * runs the external program in the same terminal. When done, it restores
 * the original window layout and returns exit code of the external program
 * (an integer value returned by system() call). When called from the Qt UI,
 * an error message is printed to the log.
 * @param string external_program
 * return integer
 *
 * @usage RunInTerminal("/bin/bash")
 */

YCPInteger YUI::evaluateRunInTerminal(const YCPString & module )
{
    int ret = runInTerminal( module );

    return YCPInteger ( ret );

}

int YUI::runInTerminal ( const YCPString & module )
{
    yuiError() << "Not in text mode: Cannot run external program in terminal." << endl;

    return -1;
}

/**
 * @builtin SetKeyboard
 * @short Sets Keyboard
 *
 * @return void
 * @usage SetKeyboard()
 */
void YUI::evaluateSetKeyboard( )
{
    setKeyboard( );
}


/*
 * Default UI-specific setKeyboard()
 * Returns OK ( YCPVoid() )
 */
YCPValue YUI::setKeyboard(  )
{
    // NOP

    return YCPVoid();	// OK ( YCPNull() would mean error )
}


/*
 * Default UI-specific setConsoleFont()
 * Returns OK (YCPVoid())
 */
YCPValue YUI::setConsoleFont( const YCPString & console_magic,
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
void YUI::busyCursor()
{
    // NOP
}


/**
 * Default UI-specific normalCursor() - does nothing
 */
void YUI::normalCursor()
{
    // NOP
}


/**
 * Default UI-specific redrawScreen() - does nothing
 */
void YUI::redrawScreen()
{
    // NOP
}


/**
 * Default UI-specific makeScreenShot() - does nothing
 */
void YUI::makeScreenShot( string filename )
{
    // NOP
}


/**
 * Default UI-specific beep() - does nothing
 */
void YUI::beep()
{
    // NOP
}


/**
 * @builtin GetLanguage
 * @short Gets Language
 * @description
 * Retrieves the current language setting from of the user interface.  Since
 * YaST2 is a client server architecture, we distinguish between the language
 * setting of the user interface and that of the configuration modules. If the
 * module or the translator wants to know which language the user currently
 * uses, it can call <tt>GetLanguage</tt>. The return value is an ISO language
 * code, such as "de" or "de_DE".
 *
 * If "strip_encoding" is set to "true", all encoding or similar information is
 * cut off, i.e. everything from the first "." or "@" on. Otherwise the current
 * contents of the "LANG" environment variable is returned (which very likely
 * ends with ".UTF-8" since this is the encoding YaST2 uses internally).
 *
 * @param boolean strip_encoding
 * @return string
 *
 */

YCPString YUI::evaluateGetLanguage( const YCPBoolean & strip )
{
    return YCPString( YUI::app()->language( strip->value() ) );
}



/**
 * @builtin UserInput
 * @short User Input
 * @description
 * Waits for the user to click some button, close the window or
 * activate some widget that has the <tt>`notify</tt> option set.
 * The return value is the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicit cancel
 * button (for example he closes the window).
 *
 * @return any
 */
YCPValue YUI::evaluateUserInput()
{
#if VERBOSE_EVENTS
    yuiDebug() << "UI::UserInput()" << endl;
#endif

    return doUserInput( YUIBuiltin_UserInput,
			0,		// timeout_millisec
			true,		// wait
			false );	// detailed
}


YCPValue YUI::waitForUserInput()
{
#if VERBOSE_EVENTS
    yuiDebug() << "Waiting for user input..." << endl;
#endif
    
    return doUserInput( YUIBuiltin_UserInput,
			0,		// timeout_millisec
			true,		// wait
			false );	// detailed
}


/**
 * @builtin PollInput
 * @short Poll Input
 * @description
 * Doesn't wait but just looks if the user has clicked some
 * button, has closed the window or has activated
 * some widget that has the <tt>`notify</tt> option set. Returns
 * the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicit cancel
 * button (for example he closes the window). Returns nil if no
 * user input has occured.
 *
 * @return any
 *
 */
YCPValue YUI::evaluatePollInput()
{
#if VERBOSE_EVENTS
    yuiDebug() << "UI::PollInput()" << endl;
#endif

    return doUserInput( YUIBuiltin_PollInput,
			0,		// timeout_millisec
			false,		// wait
			false );	// detailed
}


/**
 * @builtin TimeoutUserInput
 * @short User Input with Timeout
 * @description
 * Waits for the user to click some button, close the window or
 * activate some widget that has the <tt>`notify</tt> option set
 * or until the specified timeout is expired.
 * The return value is the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicit cancel
 * button (for example he closes the window).
 * Upon timeout, <tt>`timeout</tt> is returned.
 *
 * @param integer timeout_millisec
 * @return any
 */
YCPValue YUI::evaluateTimeoutUserInput( const YCPInteger & timeout )
{
    long timeout_millisec = timeout->value();

#if VERBOSE_EVENTS
    yuiDebug() << "UI::TimeoutUserInput( " << timeout_millisec << " )" << endl;
#endif

    return doUserInput( YUIBuiltin_TimeoutUserInput,
			timeout_millisec,
			true,			// wait
			false );		// detailed
}


/**
 * @builtin WaitForEvent
 * @short Waits for Event
 * @description
 * Extended event handling - very much like UserInput(), but returns much more
 * detailed information about the event that occured in a map.
 *
 * @optarg timeout_millisec
 * @return map
 */
YCPValue YUI::evaluateWaitForEvent( const YCPInteger & timeout )
{
    long timeout_millisec = 0;

    if ( ! timeout.isNull() )
    {
	timeout_millisec = timeout->value();
    }

#if VERBOSE_EVENTS
    yuiDebug() << "UI::WaitForEvent( " << timeout_millisec << " )" << endl;
#endif

    return doUserInput( YUIBuiltin_WaitForEvent,
			timeout_millisec,
			true,			// wait
			true );			// detailed
}




YCPValue YUI::doUserInput( const char * 	builtin_name,
			   long 		timeout_millisec,
			   bool 		wait,
			   bool 		detailed )
{
    // Plausibility check for timeout

    if ( timeout_millisec < 0 )
    {
	yuiError() << builtin_name << "(): Invalid value " << timeout_millisec
		   << " for timeout - assuming 0"
		   << endl;

	timeout_millisec = 0;
    }

    YEvent *	event = 0;
    YCPValue 	input = YCPVoid();

    try
    {
	YDialog * dialog = YDialog::currentDialog();

	// Check for leftover postponed shortcut check

	if ( dialog->shortcutCheckPostponed() )
	{
	    yuiError() << "Missing CheckShortcuts() before " << builtin_name
		       << "() after PostponeShortcutCheck()!"
		       << endl;

	    dialog->checkShortcuts( true );
	}


	// Handle events

	if ( fakeUserInputQueue.empty() )
	{
	    if ( wait )
	    {
		do
		{
		    // Get an event from the specific UI. Wait if there is none.

#if VERBOSE_EVENTS
		    yuiDebug() << "SpecificUI::userInput()" << endl;
#endif
		    event = filterInvalidEvents( userInput( (unsigned long) timeout_millisec ) );

		    // If there was no event or if filterInvalidEvents() discarded
		    // an invalid event, go back and get the next one.
		} while ( ! event );
	    }
	    else
	    {
		// Get an event from the specific UI. Don't wait if there is none.

#if VERBOSE_EVENTS
		yuiDebug() << "SpecificUI::pollInput()" << endl;
#endif
		event = filterInvalidEvents( pollInput() );

		// Nevermind if filterInvalidEvents() discarded an invalid event.
		// PollInput() is called very often (in a loop) anyway, and most of
		// the times it returns 'nil' anyway, so there is no need to care
		// for just another 'nil' that is returned in this exotic case.
	    }

	    if ( event )
	    {
		YCPEvent ycpEvent( event );
		
		if ( detailed )
		    input = ycpEvent.eventMap();
		else
		    input = ycpEvent.eventId();

#if VERBOSE_EVENTS
		yuiDebug() << "Got regular event from keyboard / mouse: " << input << endl;
#endif
	    }
	}
	else // fakeUserInputQueue contains elements -> use the first one
	{
	    // Handle macro playing

	    input = fakeUserInputQueue.front();
	    yuiDebug() << "Using event from fakeUserInputQueue: "<< input << endl;
	    fakeUserInputQueue.pop_front();
	}

	// Handle macro recording

	if ( macroRecorder )
	{
	    if ( ! input->isVoid() || wait )	// Don't record empty PollInput() calls
	    {
		macroRecorder->beginBlock();
		dialog->saveUserInput( macroRecorder );
		macroRecorder->recordUserInput( input );
		macroRecorder->endBlock();
	    }
	}
    }
    catch ( YUIException & exception )
    {
	YUI_CAUGHT( exception );
	YCPErrorDialog::exceptionDialog( "Internal Error", exception );
	YUI_RETHROW( exception );
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
YUI::filterInvalidEvents( YEvent * event )
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

	    // yuiDebug() << "Discarding event for widget that has become invalid" << endl;

	    delete widgetEvent;
	    return 0;
	}

	if ( widgetEvent->widget()->findDialog() != YDialog::currentDialog() )
	{
	    /**
	     * Silently discard events from all but the current (topmost) dialog.
	     *
	     * This may happen even here even though the specific UI should have
	     * taken care about that: Events may still be in the queue. They might
	     * have been valid (i.e. belonged to the topmost dialog) when they
	     * arrived, but maybe simply nobody has evaluated them.
	     **/

	    // Yes, really yuiDebug() - this may legitimately happen.
	    yuiDebug() << "Discarding event from widget from foreign dialog" << endl;

#if VERBOSE_DISCARDED_EVENTS
	    yuiDebug() << "Expected: "   << YDialog::currentDialog()
		       << ", received: " << widgetEvent->widget()->findDialog()
		       << endl;

	    yuiDebug() << "Event widget: "  << widgetEvent->widget() << endl;
	    yuiDebug() << "From:" << endl;
	    widgetEvent->widget()->findDialog()->dumpWidgetTree();
	    yuiDebug() << "Current dialog:" << endl;
	    YDialog::currentDialog()->dumpWidgetTree();
#endif

	    YDialog::currentDialog()->activate();

	    delete widgetEvent;
	    return 0;
	}

    }

    return event;
}


/**
 * @builtin OpenDialog
 * @id OpenDialog_with_options
 * @short Opens a Dialog with options
 * @description
 * Same as the OpenDialog with one argument, but you can specify options
 * with a term of the form <tt><b>`opt</b></tt>.
 *
 * The <tt>`mainDialog</tt> option creates a "main window" dialog:
 * The dialog will get a large "default size". In the Qt UI, this typically
 * means 800x600 pixels large (or using a -geometry command line argument if
 * present) or full screen. In the NCurses UI, this is always full screen.
 *
 * <tt>`defaultsize</tt> is an alias for <tt>`mainDialog</tt>.
 *
 * The <tt>`warncolor</tt> option displays the entire dialog in a bright
 * warning color.
 *
 * The <tt>`infocolor</tt> option displays the dialog in a color scheme that is
 * distinct from the normal colors, but not as bright as warncolor.
 *
 * The <tt>`decorated</tt> option is now obsolete, but still accepted to keep
 * old code working.
 *
 * The <tt>`centered</tt> option is now obsolete, but still accepted to keep
 * old code working.
 *
 * @param term options
 * @param term widget
 * @return boolean true if success, false if error
 *
 * @usage OpenDialog( `opt( `defaultsize ), `Label( "Hello, World!" ) )
 */

YCPBoolean YUI::evaluateOpenDialog( const YCPTerm & opts, const YCPTerm & dialogTerm )
{
    YDialogType		dialogType = YPopupDialog;
    YDialogColorMode	colorMode  = YDialogNormalColor;

    if ( ! opts.isNull() ) // evaluate `opt() contents
    {
	    YCPList optList = opts->args();

	    for ( int o=0; o < optList->size(); o++ )
	    {
		if ( optList->value(o)->isSymbol() )
		{
		    if      ( optList->value(o)->asSymbol()->symbol() == YUIOpt_mainDialog  ) 		dialogType = YMainDialog;
		    if      ( optList->value(o)->asSymbol()->symbol() == YUIOpt_defaultsize ) 		dialogType = YMainDialog;
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_infocolor )		colorMode  = YDialogInfoColor;
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_warncolor )		colorMode  = YDialogWarnColor;
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_decorated ) 		{} // obsolete
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_centered  )		{} // obsolete
		    else
			yuiWarning() << "Unknown option " << opts->value(o) << " for OpenDialog" << endl;
		}
	    }
    }

    blockEvents();	// Prevent self-generated events from UI built-ins.

    bool ok = true;

    try
    {
	YDialog * dialog = YUI::widgetFactory()->createDialog( dialogType, colorMode );
	YUI_CHECK_NEW( dialog );

	YCPDialogParser::parseWidgetTreeTerm( dialog, dialogTerm );
	dialog->open();
    }
    catch ( YUIException & exception )
    {
	YUI_CAUGHT( exception );

	// Delete this half-created dialog:
	// Some widgets are in a very undefined state (no children etc.)
	YDialog::deleteTopmostDialog();

	ycperror( "UI::OpenDialog() failed" );
	ok = false;

	YCPErrorDialog::exceptionDialog( "UI Syntax Error", exception );
    }

    unblockEvents();

    return YCPBoolean( ok );
}



/**
 * @builtin CloseDialog()
 * @short Closes an open dialog
 * @description
 * Closes the most recently opened dialog. It is an error
 * to call <tt>CloseDialog</tt> if no dialog is open.
 *
 * @return boolean Returns true on success.
 */

YCPValue YUI::evaluateCloseDialog()
{
    blockEvents();	// We don't want self-generated events from UI builtins.
    YDialog::deleteTopmostDialog();
    unblockEvents();

    return YCPBoolean( true );
}



/**
 * @builtin ChangeWidget
 * @short Changes widget contents
 * @description
 * Changes a property of a widget of the topmost dialog. <tt>id</tt> specified
 * the widget to change, <tt>property</tt> specifies the property that should
 * be changed, <tt>newvalue</tt> gives the new value.
 *
 * For example in order to change the label of an InputField with id `name to
 * "anything", you write <tt>ChangeWidget( `id(`name), `Label, "anything" )</tt>.
 * @param symbol widgetId Can also be specified as `id( any widgetId )
 * @param symbol property
 * @param any newValue
 *
 * @return boolean Returns true on success.
 */

YCPValue YUI::evaluateChangeWidget( const YCPValue & idValue, const YCPValue & property, const YCPValue & newValue )
{
    YCPValue ret = YCPVoid();

    try
    {
	blockEvents();	// We don't want self-generated events from UI::ChangeWidget().

	if ( ! YCPDialogParser::isSymbolOrId( idValue ) )
	{
	    YUI_THROW( YUISyntaxErrorException( string( "Expected `id(...) or `symbol, not " ) +
						idValue->toString().c_str() ) );
	}

	YCPValue	id	= YCPDialogParser::parseIdTerm( idValue );
	YWidget *	widget 	= YCPDialogParser::findWidgetWithId( id,
								     true ); // throw if not found

	YPropertySet propSet = widget->propertySet();

	if ( property->isSymbol() )
	{
	    string oldShortcutString = widget->shortcutString();
	    string propertyName	 = property->asSymbol()->symbol();

	    YPropertyValue val;

	    if		( newValue->isString()  )	val = YPropertyValue( newValue->asString()->value()  );
	    else if	( newValue->isInteger() )	val = YPropertyValue( newValue->asInteger()->value() );
	    else if	( newValue->isBoolean() )	val = YPropertyValue( newValue->asBoolean()->value() );
	    else
		val = YPropertyValue( false ); // Dummy value, will be rejected anyway

	    bool success = widget->setProperty( propertyName, val );

	    if ( ! success )
	    {
		// Try again with the known special cases
		success = YCPPropertyHandler::setComplexProperty( widget, propertyName, newValue );
	    }

	    ret = YCPBoolean( success );

	    if ( oldShortcutString != widget->shortcutString() )
		YDialog::currentDialog()->checkShortcuts();
	}
	else if ( property->isTerm() )
	{
	    bool success	= YCPPropertyHandler::setComplexProperty( widget, property->asTerm(), newValue );
	    ret		= YCPBoolean( success );
	}
	else
	{
	    YUI_THROW( YUISyntaxErrorException( string( "Bad UI::ChangeWidget args: " )
						+ property->toString() ) );
	}
    }
    catch( YUIException & exception )
    {
	YUI_CAUGHT( exception );
	ycperror( "UI::ChangeWidget failed: UI::ChangeWidget( %s, %s, %s )",
		  idValue->toString().c_str(),
		  property->toString().c_str(),
		  newValue->toString().c_str() );
	ret = YCPNull();
    }

    unblockEvents();

    return ret;
}



/**
 * @builtin QueryWidget
 * @short Queries Widget contents
 * @description
 * Queries a property of a widget of the topmost dialog.  For example in order
 * to query the current text of an InputField with id `name, you write
 * <tt>QueryWidget( `id(`name), `Value )</tt>. In some cases the propery can be given
 * as term in order to further specify it. An example is
 * <tt>QueryWidget( `id( `table ), `Item( 17 ) )</tt> for a table where you query a
 * certain item.
 *
 * @param  symbol widgetId Can also be specified as `id( any id )
 * @param symbol|term property
 * @return any
 */

YCPValue YUI::evaluateQueryWidget( const YCPValue & idValue, const YCPValue & property )
{
    YCPValue ret = YCPVoid();

    try
    {
	if ( ! YCPDialogParser::isSymbolOrId( idValue ) )
	{
	    YUI_THROW( YUISyntaxErrorException( string( "Expected `id(...) or `symbol, not " ) +
						idValue->toString().c_str() ) );
	}

	YCPValue id = YCPDialogParser::parseIdTerm( idValue );
	YWidget *widget = YCPDialogParser::findWidgetWithId( id,
							     true ); // throw if not found

	YPropertySet propSet = widget->propertySet();

	if ( property->isSymbol() )		// The normal case: UI::QueryWidget(`myWidget, `SomeProperty)
	{
	    string propertyName = property->asSymbol()->symbol();
	    YPropertyValue val  = widget->getProperty( propertyName );

	    switch ( val.type() )
	    {
		case YStringProperty:	return YCPString ( val.stringVal()  );
		case YBoolProperty:	return YCPBoolean( val.boolVal()    );
		case YIntegerProperty:	return YCPInteger( val.integerVal() );
		case YOtherProperty:	return YCPPropertyHandler::getComplexProperty( widget, propertyName );

		default:
		    ycperror( "Unknown result for setProperty( %s )", propertyName.c_str() );
		    return YCPVoid();
	    }
	}
	else if ( property->isTerm() )	// Very rare: UI::QueryWidget(`myTable, `Item("abc", 3) )
	{
	    return YCPPropertyHandler::getComplexProperty( widget, property->asTerm() );
	}
	else
	{
	    YUI_THROW( YUISyntaxErrorException( string( "Bad UI::QueryWidget args: " )
						+ property->toString() ) );
	}
    }
    catch( YUIException & exception )
    {
	YUI_CAUGHT( exception );
	ycperror( "UI::QueryWidget failed: UI::QueryWidget( %s, %s )",
		  idValue->toString().c_str(),
		  property->toString().c_str() );
	ret = YCPNull();
    }

    return ret;
}


/**
 * @builtin ReplaceWidget
 *
 * @description
 * Replaces a complete widget (or widget subtree) with an other widget
 * (or widget tree). You can only replace the widget contained in
 * a <tt>ReplacePoint</tt>. As parameters to <tt>ReplaceWidget</tt>
 * specify the id of the ReplacePoint and the new widget.
 *
 * @param symbol id
 * @param term newWidget
 * @return true if success, false if failed
 */

YCPBoolean YUI::evaluateReplaceWidget( const YCPValue & idValue, const YCPTerm & newContentTerm )
{
    bool success = true;

    try
    {
	if ( ! YCPDialogParser::isSymbolOrId( idValue ) )
	{
	    YUI_THROW( YUISyntaxErrorException( string( "Expected `id(...) or `symbol, not " ) +
						idValue->toString().c_str() ) );
	}

	blockEvents();	// Prevent self-generated events
	YCPValue  id     = YCPDialogParser::parseIdTerm( idValue );
	YWidget * widget = YCPDialogParser::findWidgetWithId( id,
							      true ); // throw if not found
	if ( ! widget ) return YCPBoolean( false );

	YReplacePoint * replacePoint = dynamic_cast<YReplacePoint *> (widget);

	if ( ! replacePoint )
	    YUI_THROW( YUIException( string( "Widget with ID " ) + id->toString() + " is not a ReplacePoint" ) );

#if VERBOSE_REPLACE_WIDGET
	replacePoint->dumpDialogWidgetTree();
#endif
	YDialog * dialog = YDialog::currentDialog();

	YWidget::OptimizeChanges below( *dialog ); // delay screen updates until this block is left
	replacePoint->deleteChildren();

	YCPDialogParser::parseWidgetTreeTerm( replacePoint, newContentTerm );
	replacePoint->showChild();

#if VERBOSE_REPLACE_WIDGET
	replacePoint->dumpDialogWidgetTree();
#endif

	dialog->setInitialSize();
	dialog->checkShortcuts();
    }
    catch( YUIException & exception )
    {
	YUI_CAUGHT( exception );
	success = false;

	ycperror( "UI::ReplaceWidget() failed: UI::ReplaceWidget( %s, %s )",
		  idValue->toString().c_str(),
		  newContentTerm->toString().c_str() );

	YCPErrorDialog::exceptionDialog( "UI Syntax Error", exception );
    }

    unblockEvents();

    return YCPBoolean( success );
}



/**
 * @builtin WizardCommand
 * @short Runs a wizard command
 * @description
 * Issues a command to a wizard widget with ID 'wizardId'.
 *
 * <b>This builtin is not for general use. Use the Wizard.ycp module instead.</b>
 *
 * For available wizard commands see file YCPWizardCommandParser.cc .
 * If the current UI does not provide a wizard widget, 'false' is returned.
 * It is safe to call this even for UIs that don't provide a wizard widget. In
 * this case, all calls to this builtin simply return 'false'.
 *
 * @param term wizardCommand
 *
 * @return boolean  Returns true on success.
 */

YCPValue YUI::evaluateWizardCommand( const YCPTerm & command )
{
    if ( ! YUI::optionalWidgetFactory()->hasWizard() )
	return YCPBoolean( false );

    // A wizard widget always has ID `wizard
    YWidget * widget = YCPDialogParser::findWidgetWithId( YCPSymbol( YWizardID ),
							  false ); // don't throw if not found

    if ( ! widget )
	return YCPBoolean( false );

    YWizard * wizard = dynamic_cast<YWizard *>( widget );

    if ( ! wizard )
	return YCPBoolean( false );

    blockEvents();	// Avoid self-generated events from builtins
    bool ret = YCPWizardCommandParser::parseAndExecute( wizard, command );
    unblockEvents();

    return YCPBoolean( ret );
}



/**
 * @builtin SetFocus
 * @short Sets Focus to the specified widget
 * @description
 * Sets the keyboard focus to the specified widget.  Notice that not all
 * widgets can accept the keyboard focus; this is limited to interactive
 * widgets like PushButtton, InputField, SelectionBox etc. - manager widgets
 * like VBox, HBox etc. will not accept the keyboard focus. They will not
 * propagate the keyboard focus to some child widget that accepts the
 * focus. Instead, an error message will be emitted into the log file.
 * @param symbol widgetId
 * @return boolean Returns true on success (i.e. the widget accepted the focus).
 */

YCPBoolean YUI::evaluateSetFocus( const YCPValue & idValue )
{
    if ( ! YCPDialogParser::isSymbolOrId( idValue ) )
	return YCPNull();

    YCPValue id = YCPDialogParser::parseIdTerm( idValue );
    YWidget *widget = YCPDialogParser::findWidgetWithId( id );

    if ( ! widget )
	return YCPBoolean( false );

    return YCPBoolean( widget->setKeyboardFocus() );
}



/**
 * @builtin BusyCursor
 * @short Sets the mouse cursor to the busy cursor
 * @description
 * Sets the mouse cursor to the busy cursor, if the UI supports such a feature.
 *
 * This should normally not be necessary. The UI handles mouse cursors itself:
 * When input is possible (i.e. inside UserInput() ), there is automatically a
 * normal cursor, otherwise, there is the busy cursor. Override this at your
 * own risk.
 *
 * @return void
 */

void YUI::evaluateBusyCursor()
{
    busyCursor();
}



/**
 * @builtin RedrawScreen
 * @short Redraws the screen
 * @description
 * Redraws the screen after it very likely has become garbled by some other output.
 *
 * This should normally not be necessary: The (specific) UI redraws the screen
 * automatically whenever required. Under rare circumstances, however, the
 * screen might have changes due to circumstances beyond the UI's control: For
 * text based UIs, for example, system commands that cause output to every tty
 * might make this necessary. Call this in the YCP code after such a command.
 *
 * @return void
 */

void YUI::evaluateRedrawScreen()
{
    redrawScreen();
}


/**
 * @builtin NormalCursor
 * @short Sets the mouse cursor to the normal cursor
 * @description
 * Sets the mouse cursor to the normal cursor (after BusyCursor), if the UI
 * supports such a feature.
 *
 * This should normally not be necessary. The UI handles mouse cursors itself:
 * When input is possible (i.e. inside UserInput() ), there is automatically a
 * normal cursor, otherwise, there is the busy cursor. Override this at your
 * own risk.
 *
 * @return void
 */

void YUI::evaluateNormalCursor()
{
    normalCursor();
}



/**
 * @builtin MakeScreenShot
 * @short Makes Screen Shot
 * @description
 * Makes a screen shot if the specific UI supports that.
 * The Qt UI opens a file selection box if filename is empty.
 *
 * @param string filename
 * @return void
 */

void YUI::evaluateMakeScreenShot( const YCPString & filename )
{
    makeScreenShot( filename->value () );
}



/**
 * @builtin DumpWidgetTree
 * @short Debugging function
 * @description
 * Debugging function: Dumps the widget tree of the current dialog to the log
 * file.
 *
 * @return void
 */

void YUI::evaluateDumpWidgetTree()
{
    YDialog::currentDialog()->dumpDialogWidgetTree();
}

/**
 * @builtin Beep
 * @short Beeps the system bell
 * @description
 * Beeps the system bell. This is implemented by the frontend, which may do
 * a visual beep if the system is set up that way (eg. for accessiblity
 * purposes).
 *
 * @return void
 */
void YUI::evaluateBeep()
{
    beep();
}

/**
 * @builtin RecordMacro
 * @short Records Macro into a file
 * @description
 * Begins recording a macro. Write the macro contents to file "macroFilename".
 * @param string macroFileName
 * @return void
 */
void YUI::evaluateRecordMacro( const YCPString & filename )
{
    recordMacro( filename->value () );
}


void YUI::recordMacro( string filename )
{
    deleteMacroRecorder();
    macroRecorder = new YMacroRecorder( filename );
}


void YUI::deleteMacroRecorder()
{
    if ( macroRecorder )
    {
	delete macroRecorder;
	macroRecorder = 0;
    }
}



/**
 * @builtin StopRecordingMacro
 * @short Stops recording macro
 * @description
 * Stops macro recording. This is only necessary if you don't wish to record
 * everything until the program terminates.
 *
 * @return void
 */
void YUI::evaluateStopRecordMacro()
{
    stopRecordMacro();
}


void YUI::stopRecordMacro()
{
    deleteMacroRecorder();
}


/**
 * @builtin PlayMacro
 * @short Plays a recorded macro
 * @description
 * Executes everything in macro file "macroFileName".
 * Any errors are sent to the log file only.
 * The macro can be terminated only from within the macro file.
 *
 * @param string macroFileName
 * @return void
 */
void YUI::evaluatePlayMacro( const YCPString & filename )
{
    playMacro( filename->value() );
}


void YUI::playMacro( string filename )
{
    deleteMacroPlayer();
    macroPlayer = new YMacroPlayer( filename );
}


void YUI::playNextMacroBlock()
{
    if ( ! macroPlayer )
    {
	yuiError() << "No macro player active." << endl;
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
	    YCPValue result = macroPlayer->evaluateNextBlock();

	    if ( macroPlayer->error() || result.isNull() )
	    {
		yuiError() << "Macro aborted" << endl;
		deleteMacroPlayer();
	    }
	}
    }
}


void YUI::deleteMacroPlayer()
{
    if ( macroPlayer )
    {
	delete macroPlayer;
	macroPlayer = 0;
    }
}


/**
 * @builtin FakeUserInput
 * @short Fakes User Input
 * @description
 * Prepares a fake value for the next call to UserInput() -
 * i.e. the next UserInput() will return exactly this value.
 * This is only useful in connection with macros.
 *
 * If called without a parameter, the next call to UserInput()
 * will return "nil".
 *
 * @optarg any nextUserInput
 * @return void
 */
void YUI::evaluateFakeUserInput( const YCPValue & next_input )
{
    yuiDebug() << "UI::FakeUserInput( " << next_input << " )" << endl;
    fakeUserInputQueue.push_back( next_input );
}



/**
 * @builtin Glyph
 * @short Returns a special character (a 'glyph')
 * @description
 * Returns a special character (a 'glyph') according to the symbol specified.
 *
 * Not all UIs may be capable of displaying every glyph; if a specific UI
 * doesn't support it, a textual representation (probably in plain ASCII) will
 * be returned.
 *
 * This is also why there is only a limited number of predefined
 * glyphs: An ASCII equivalent is required which is sometimes hard to find for
 * some characters defined in Unicode / UTF-8.
 *
 * Please note the value returned may consist of more than one character; for
 * example, Glyph( `ArrowRight ) may return something like "-&gt;".
 *
 * If an unknown glyph symbol is specified, 'nil' is returned.
 *
 * @param symbol glyph
 * @return string
 */
YCPString YUI::evaluateGlyph( const YCPSymbol & glyphSym )
{
    return YCPString( YUI::app()->glyph( glyphSym->symbol() ) );
}


/**
 * @builtin GetDisplayInfo
 * @short Gets Display Info
 * @description
 * Gets information about the current display and the UI's capabilities.
 *
 * Example in Qt:
 * <code>
 * GetDisplayInfo() -> $[
 *	"Colors":65536,
 *	"DefaultHeight":735,
 *	"DefaultWidth":1176,
 *	"Depth":16,
 *	"HasAnimationSupport":true,
 *	"HasFullUtf8Support":true,
 *	"HasIconSupport":false,
 *	"HasImageSupport":true,
 *	"HasLocalImageSupport":true,
 *	"Height":1050,
 *	"LeftHandedMouse":false,
 *	"RichTextSupportsTable":true,
 *	"TextMode":false,
 *	"Width":1680
 * ]
 * </code>
 *
 * Example in ncurses:
 * <code>
 * GetDisplayInfo() -> $[
 *	"Colors":8,
 *	"DefaultHeight":54,
 *	"DefaultWidth":151,
 *	"Depth":-1,
 *	"HasAnimationSupport":false,
 *	"HasFullUtf8Support":true,
 *	"HasIconSupport":false,
 *	"HasImageSupport":false,
 *	"HasLocalImageSupport":true,
 *	"Height":54,
 *	"LeftHandedMouse":false,
 *	"RichTextSupportsTable":false,
 *	"TextMode":true,
 *	"Width":151
 * ]
 * </code>
 *
 * Function output might differ according to the system where called.
 *
 * @return map <string any>
 *
 */
YCPMap YUI::evaluateGetDisplayInfo()
{
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
    info_map->add( YCPString( YUICap_RichTextSupportsTable	), YCPBoolean( richTextSupportsTable()	) );
    info_map->add( YCPString( YUICap_LeftHandedMouse		), YCPBoolean( leftHandedMouse()	) );

    return info_map;
}


/**
 * @builtin RecalcLayout
 * @short Recalculates Layout
 * @description
 * Recompute the layout of the current dialog.
 *
 * <b>This is a very expensive operation.</b>
 *
 * Use this after changing widget properties that might affect their size -
 * like the a Label widget's value. Call this once (!) after changing all such
 * widget properties.
 *
 * @return void
 */
void YUI::evaluateRecalcLayout()
{
    YDialog::currentDialog()->setInitialSize();
}


/**
 * @builtin PostponeShortcutCheck
 * @short Postpones Shortcut Check
 * @description
 * Postpone keyboard shortcut checking during multiple changes to a dialog.
 *
 * Normally, keyboard shortcuts are checked automatically when a dialog is
 * created or changed. This can lead to confusion, however, when multiple
 * changes to a dialog (repeated ReplaceWidget() calls) cause unwanted
 * intermediate states that may result in shortcut conflicts while the dialog
 * is not final yet. Use this function to postpone this checking until all
 * changes to the dialog are done and then explicitly check with
 * <tt>CheckShortcuts()</tt>. Do this before the next call to
 * <tt>UserInput()</tt> or <tt>PollInput()</tt> to make sure the dialog doesn't
 * change "on the fly" while the user tries to use one of those shortcuts.
 *
 * The next call to <tt>UserInput()</tt> or <tt>PollInput()</tt> will
 * automatically perform that check if it hasn't happened yet, any an error
 * will be issued into the log file.
 *
 * Use only when really necessary. The automatic should do well in most cases.
 *
 * The normal sequence looks like this:
 *
 * <code>
 * PostponeShortcutChecks();
 * ReplaceWidget( ... );
 * ReplaceWidget( ... );
 * ...
 * ReplaceWidget( ... );
 * CheckShortcuts();
 * ...
 * UserInput();
 * </code>
 *
 * @return void
 */
void YUI::evaluatePostponeShortcutCheck()
{
    YDialog::currentDialog()->postponeShortcutCheck();
}


/**
 * @builtin CheckShortcuts
 * @short Performs an explicit shortcut check after postponing shortcut checks.
 * @description
 * Performs an explicit shortcut check after postponing shortcut checks.
 * Use this after calling <tt>PostponeShortcutCheck()</tt>.
 *
 * The normal sequence looks like this:
 *
 * <code>
 * PostponeShortcutChecks();
 * ReplaceWidget( ... );
 * ReplaceWidget( ... );
 * ...
 * ReplaceWidget( ... );
 * CheckShortcuts();
 * ...
 * UserInput();
 * </code>
 *
 * @return void
 */
void YUI::evaluateCheckShortcuts()
{
    YDialog * dialog = YDialog::currentDialog();

    if ( ! dialog->shortcutCheckPostponed() )
    {
	yuiWarning() << "Use UI::CheckShortcuts() only after UI::PostponeShortcutCheck() !" << endl;
    }

    dialog->checkShortcuts( true );
}


/**
 * @builtin WidgetExists
 * @short Checks whether or not a widget with the given ID currently exists
 * @description
 * Checks whether or not a widget with the given ID currently exists in the
 * current dialog. Use this to avoid errors in the log file before changing the
 * properties of widgets that might or might not be there.
 *
 * @param symbol widgetId
 * @return boolean
 */
YCPBoolean YUI::evaluateWidgetExists( const YCPValue & idValue )
{
    if ( ! YCPDialogParser::isSymbolOrId( idValue ) ) return YCPNull();

    YCPValue id = YCPDialogParser::parseIdTerm( idValue );
    YWidget *widget = YCPDialogParser::findWidgetWithId( id,
							 false ); // Don't throw if not found
    return widget ? YCPBoolean( true ) : YCPBoolean( false );
}


/**
 * @builtin RunPkgSelection
 * @short Initializes and run the PackageSelector widget
 * @description
 * <b>Not to be used outside the package selection</b>
 *
 * Initialize and run the PackageSelector widget identified by 'pkgSelId'.
 *
 * Black magic to everybody outside. ;- )
 *
 * @param any pkgSelId
 * @return any Returns `cancel if the user wishes to cancel his selections.
 *
 */
YCPValue YUI::evaluateRunPkgSelection( const YCPValue & value_id )
{
    YCPValue result = YCPNull();

    try
    {
	if ( ! YCPDialogParser::isSymbolOrId( value_id ) )
	{
	    yuiError() << "RunPkgSelection(): expecting `id( ... ), not " << value_id << endl;
	    return YCPNull();
	}

	YCPValue id = YCPDialogParser::parseIdTerm( value_id );
	YWidget * selector = YCPDialogParser::findWidgetWithId( id );

	yuiMilestone() << "Running package selection..." << endl;
	YEvent * event = runPkgSelection( selector );

	if ( event )
	{
	    YCPEvent ycpEvent( event );
	    result = ycpEvent.eventId();

	    if ( result->isString() )				
		result = YCPSymbol( result->asString()->value() ); // "accept" -> `accept

	    yuiMilestone() << "Package selection done. Returning with " << result << endl;

	    delete event;
	}
    }
    catch ( YUIException & exception )
    {
	YUI_CAUGHT( exception );
	ycperror( "RunPkgSelection() failed" );
	YDialog::currentDialog()->dumpWidgetTree();
    }

    return result;
}



/**
 * @builtin AskForExistingDirectory
 * @short Ask user for existing directory
 * @description
 * Opens a directory selection box and prompt the user for an existing directory.
 *
 * @param string startDir is the initial directory that is displayed.
 * @param string headline is an explanatory text for the directory selection box.
 * Graphical UIs may omit that if no window manager is running.
 * @return string  Returns the selected directory name or <i>nil</i> if the
 * user canceled the operation.
 */
YCPValue
YUI::evaluateAskForExistingDirectory( const YCPString & startDir, const YCPString & headline )
{
    string ret = app()->askForExistingDirectory( startDir->value(), headline->value() );

    if ( ret.empty() )
	return YCPVoid();
    else
	return YCPString( ret );
}



/**
 * @builtin AskForExistingFile
 * @short Ask user for existing file
 * @description
 * Opens a file selection box and prompt the user for an existing file.
 *
 * @param string startWith is the initial directory or file.
 * @param string filter is one or more blank-separated file patterns, e.g. "*.png *.jpg"
 * @param string headline is an explanatory text for the file selection box.
 * Graphical UIs may omit that if no window manager is running.
 * @return string Returns the selected file name or <i>nil</i> if the user
 * canceled the operation.
 */
YCPValue YUI::evaluateAskForExistingFile( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    string ret = app()->askForExistingFile( startWith->value(), filter->value(), headline->value() );

    if ( ret.empty() )
	return YCPVoid();
    else
	return YCPString( ret );
}


/**
 * @builtin AskForSaveFileName
 * @short Ask user for a file to save data to.
 * @description
 * Opens a file selection box and prompt the user for a file to save data to.
 * Automatically asks for confirmation if the user selects an existing file.
 *
 * @param string startWith is the initial directory or file.
 * @param string filter is one or more blank-separated file patterns, e.g. "*.png *.jpg"
 * @param string headline is an explanatory text for the file selection box.
 * Graphical UIs may omit that if no window manager is running.
 *
 * @return string Returns the selected file name or <i>nil</i> if the user canceled the operation.
 */
YCPValue YUI::evaluateAskForSaveFileName( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    string ret = app()->askForSaveFileName( startWith->value(), filter->value(), headline->value() );

    if ( ret.empty() )
	return YCPVoid();
    else
	return YCPString( ret );
}

/**
 * @builtin SetFunctionKeys
 * @short Sets the (default) function keys for a number of buttons.
 * @description
 * This function receives a map with button labels and the respective function
 * key number that should be used if on other `opt( `key_F.. ) is specified.
 *
 * Any keyboard shortcuts in those labels are silently ignored so this is safe
 * to use even if the UI's internal shortcut manager rearranges shortcuts.
 *
 * Each call to this function overwrites the data of any previous calls.
 *
 * @param map fkeys
 * @return void
 * @usage SetFunctionKeys( $[ "Back": 8, "Next": 10, ... ] );
 */
void YUI::evaluateSetFunctionKeys( const YCPMap & new_fkeys )
{
    for ( YCPMapIterator it = new_fkeys->begin(); it != new_fkeys->end(); ++it )
    {
	if ( it.key()->isString() && it.value()->isInteger() )
	{
	    string label = YShortcut::cleanShortcutString( it.key()->asString()->value() );
	    int	   fkey  = it.value()->asInteger()->value();

	    if ( fkey > 0 && fkey <= 24 )
	    {
		yuiDebug() << "Mapping \"" << label << "\"\t-> F" << fkey << endl;
		app()->setDefaultFunctionKey( label, fkey );
	    }
	    else
	    {
		ycperror( "SetFunctionKeys(): Function key %d out of range for \"%s\"",
			  fkey, label.c_str() );
	    }
	}
	else
	{
	    ycperror( "SetFunctionKeys(): Invalid map element: "
		      "Expected <string>: <integer>, not %s: %s",
		      it.key()->toString().c_str(), it.value()->toString().c_str() );
	}
    }
}


/**
 * @builtin WFM/SCR
 * @id WFM_SCR
 * @short callback
 * @description
 * This is used for a callback mechanism. The expression will
 * be sent to the WFM interpreter and evaluated there.
 * USE WITH CAUTION.
 *
 * @param block expression
 * @return any
 */

YCPValue YUI::evaluateCallback( const YCPTerm & term, bool to_wfm )
{
    if ( term->size() != 1 )	// must have 1 arg - anything allowed
    {
	return YCPNull();
    }

    if ( _callback )
    {
	YCPValue v = YCPNull();
	if ( to_wfm )		// if it goes to WFM, just send the value
	{
	    v = _callback->evaluate ( term->value(0) );
	}
	else		// going to SCR, send the complete term
	{
	    v = _callback->evaluate( term );
	}
	return v;
    }

    return YCPVoid();
}


/**
 * Default conversion from logical layout spacing units
 * to device dependent units.
 *
 * This default function assumes 80x25 units.
 * Derived UIs may want to reimplement this.
 **/
int YUI::deviceUnits( YUIDimension dim, float layout_units )
{
    return (int) ( layout_units + 0.5 );
}


/**
 * Default conversion from device dependent layout spacing units
 * to logical layout units.
 *
 * This default function assumes 80x25 units.
 * Derived UIs may want to reimplement this.
 **/
float YUI::layoutUnits( YUIDimension dim, int device_units )
{
    return (float) device_units;
}



/**
 * @builtin Recode
 * @short Recodes encoding of string from or to "UTF-8" encoding.
 * @description
 * Recodes encoding of string from or to "UTF-8" encoding.
 * One of from/to must be "UTF-8", the other should be an
 * iso encoding specifier (i.e. "ISO-8859-1" for western languages,
 * "ISO-8859-2" for eastern languages, etc. )
 *
 * @param string from
 * @param string to
 * @param string text
 * @return any
 */

YCPValue YUI::evaluateRecode( const YCPString & from, const YCPString & to, const YCPString & text )
{
    string outstr;
    if ( YUI::Recode ( text->value (), from->value (), to->value (), outstr ) != 0 )
    {
	static bool warned_about_recode = false;
	if ( ! warned_about_recode )
	{
	    yuiError() << "Recode( " << from << ", " << to << " )" << endl;
	    warned_about_recode = true;
	}
	// return text as-is
	return ( text );
    }
    return YCPString ( outstr );
}




// EOF
