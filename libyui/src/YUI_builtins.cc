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

		UI builtin commands


  Authors:	Mathias Kettner <kettner@suse.de>
		Stefan Hundhammer <sh@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>

  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/


#define VERBOSE_COMMANDS	// pretty verbose logging of each UI command
#define VERBOSE_REPLACE_WIDGET 0

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
#include <Y2.h>
#include <ycp/YCPVoid.h>

#include "YUI.h"
#include "YEvent.h"
#include "YUISymbols.h"
#include "YDialog.h"
#include "YWidget.h"
#include "YMacroRecorder.h"
#include "YMacroPlayer.h"
#include "YReplacePoint.h"
#include "YShortcut.h"
#include "YWizard.h"

using std::string;

// builtin HasSpecialWidget() -> YUI_special_widgets.cc


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

void YUI::evaluateSetModulename( const YCPString & name )
{
    _moduleName = name->value();
}


const char * YUI::moduleName()
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

YCPValue YUI::evaluateGetModulename( const YCPTerm & term )
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

void YUI::evaluateSetLanguage( const YCPString & language, const YCPString & encoding )
{
    string lang = language->value();
    if ( ! encoding.isNull() )
    {
	lang += ".";
	lang += encoding->value();
    }

    setenv( "LANG", lang.c_str(), 1 ); // 1 : replace
    setlocale( LC_NUMERIC, "C" );	// but always format numbers with "."
    YCPTerm newTerm = YCPTerm( "SetLanguage" );
    newTerm->add ( YCPString( lang ) );
    y2milestone ( "ui specific setLanguage( %s )", newTerm->toString().c_str() );

    setLanguage( newTerm );	// UI-specific setLanguage: returns YCPVoid() if OK, YCPNull() if error
}


/**
 * @builtin GetProductName() -> string
 *
 * Returns the current product name ("SuSE Linux", "United Linux", etc.) for
 * display in dialogs. This can be set with SetProductName().
 * <p>
 * Note: In help texts in RichText widgets, a predefined macro &amp;product;
 * can be used for the same purpose.
 *
 * @example sformat( "Welcome to %1", GetProductName() );
 **/
YCPString YUI::evaluateGetProductName()
{
    return YCPString( _productName );
}


/**
 * @builtin SetProductName( string prod ) -> void
 *
 * Set the current product name ("SuSE Linux", "United Linux", etc.) for
 * display in dialogs and in RichText widgets (for help text) with the RichText
 * &amp;product; macro.
 * <p>
 * This product name should be concise and meaningful to the user and not
 * cluttered with detailed version information. Don't use something like
 * "SuSE Linux 12.3-i786 Professional". Use something like "SuSE Linux"
 * instead.
 * <p>
 * This information can be retrieved with the GetProductName() builtin.
 *
 * @example SetProductName( "SuSE HyperWall" );
 **/
void YUI::evaluateSetProductName( const YCPString & name )
{
    _productName = name->value();
}


/*
 * Default UI-specific setLanguage()
 * Returns OK (YCPVoid() )
 */
YCPValue YUI::setLanguage( const YCPTerm & term )
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

void YUI::evaluateSetConsoleFont( const YCPString & console_magic, const YCPString & font,
    const YCPString & screen_map, const YCPString & unicode_map, const YCPString & encoding )
{
    setConsoleFont( console_magic, font, screen_map, unicode_map, encoding );
}

/**
 * @builtin SetKeyboard
 *
 *
 * @example SetKeyboard( )
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
 * Returns OK ( YCPVoid() )
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

YCPString YUI::evaluateGetLanguage( const YCPBoolean & strip )
{
    bool strip_encoding = strip->value();
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
YCPValue YUI::evaluateUserInput()
{
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
YCPValue YUI::evaluatePollInput()
{
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
YCPValue YUI::evaluateTimeoutUserInput( const YCPInteger & timeout )
{
    long timeout_millisec = timeout->value();

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
YCPValue YUI::evaluateWaitForEvent( const YCPInteger & timeout )
{
    long timeout_millisec = 0;

    if ( ! timeout.isNull() )
    {
	timeout_millisec = timeout->value();
    }

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
	if ( ! input->isVoid() || wait )	// Don't record empty PollInput() calls
	{
	    macroRecorder->beginBlock();
	    dialog->saveUserInput( macroRecorder );
	    macroRecorder->recordUserInput( input );
	    macroRecorder->endBlock();
	}
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
 * The option <tt>`centered</tt> centers the dialog to the desktop.
 * This has no effect for popup dialogs that are a child of a `defaultsize dialog
 * that is currently visible.
 * <p>
 * The option <tt>`decorated</tt> add a window border around the dialog, which
 * comes in handy if no window manager is running. This option may be ignored in
 * non-graphical UIs.
 * <p>
 * <tt>`smallDecorations</tt> tells the window manager to use only minimal
 * decorations - in particular, no title bar. This is useful for very small
 * popups (like only a one line label and no button). Don't overuse this.
 * This option is ignored for `defaultsize dialogs.
 * <p>
 * The option <tt>`warncolor</tt> displays the entire dialog in a bright
 * warning color.
 * <p>
 * The option <tt>`infocolor</tt> is a less intrusive color.
 *
 * @example OpenDialog( `opt( `defaultsize ), `Label( "Hi" ) )
 */

YCPBoolean YUI::evaluateOpenDialog( const YCPTerm & dialog_term, const YCPTerm & opts )
{
    YWidgetOpt opt;

    if ( ! opts.isNull() ) // evaluate `opt() contents
    {
	    YCPList optList = opts->args();

	    for ( int o=0; o < optList->size(); o++ )
	    {
		if ( optList->value(o)->isSymbol() )
		{
		    if      ( optList->value(o)->asSymbol()->symbol() == YUIOpt_defaultsize ) 		opt.hasDefaultSize.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_warncolor )		opt.hasWarnColor.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_infocolor )		opt.hasInfoColor.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_decorated )		opt.isDecorated.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_centered  )		opt.isCentered.setValue( true );
		    else if ( optList->value(o)->asSymbol()->symbol() == YUIOpt_smallDecorations )	opt.hasSmallDecorations.setValue( true );
		    else
			y2warning( "Unknown option %s for OpenDialog", opts->value(o)->toString().c_str() );
		}
	    }
    }

    blockEvents();	// We don't want self-generated events from UI builtins.
    YDialog *dialog = createDialog( opt );

    if ( dialog )
    {
	registerDialog( dialog ); // must be done first!
	YWidget *widget = createWidgetTree( dialog, 0, dialog_term );

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



/**
 * @builtin CloseDialog() -> boolean
 *
 * Closes the most recently opened dialog. It is an error
 * to call <tt>CloseDialog</tt> if no dialog is open.
 * <p>
 * Returns true on success.
 */

YCPValue YUI::evaluateCloseDialog()
{
    blockEvents();	// We don't want self-generated events from UI builtins.
    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	ycp2error ( "Can't CloseDialog: No dialog existing.");
	return YCPBoolean( false );
    }

    closeDialog( dialog );
    removeDialog();
    unblockEvents();

    return YCPBoolean( true );
}


void YUI::registerDialog( YDialog *dialog )
{
    dialogstack.push_back( dialog );
}


void YUI::removeDialog()
{
    delete currentDialog();
    dialogstack.pop_back();
}


YDialog * YUI::currentDialog() const
{
    if ( dialogstack.size() >= 1 ) return dialogstack.back();
    else return 0;
}


void YUI::showDialog( YDialog * )
{
    // dummy default implementation
}


void YUI::closeDialog( YDialog * )
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

YCPValue YUI::evaluateChangeWidget( const YCPValue & id_value, const YCPValue & property, const YCPValue & new_value )
{
    if ( ! isSymbolOrId( id_value ) )
    {
	return YCPNull();
    }

    YCPValue id = getId( id_value );
    YWidget *widget = widgetWithId( id, true );

    if ( ! widget )
	return YCPBoolean( false );

    if ( property->isSymbol() )
    {
	blockEvents();	// We don't want self-generated events from UI::ChangeWidget().
	YCPSymbol sym = property->asSymbol();
	YCPValue ret = widget->changeWidget( sym, new_value );

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
	YCPValue result = widget->changeWidgetTerm( property->asTerm(), new_value );
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

YCPValue YUI::evaluateQueryWidget( const YCPValue & id_value, const YCPValue & property )
{
    if ( ! isSymbolOrId( id_value ) )
    {
	return YCPNull();
    }

    YCPValue id = getId( id_value );
    YWidget *widget = widgetWithId( id, true ); // reports error

    if ( ! widget )
	return YCPVoid();

    if ( property->isSymbol() )
	return widget->queryWidget( property->asSymbol() );
    else
	return widget->queryWidgetTerm( property->asTerm() );
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

YCPBoolean YUI::evaluateReplaceWidget( const YCPValue & id_value, const YCPTerm & new_widget )
{
    if ( ! isSymbolOrId( id_value ) )
    {
	return YCPNull();
    }

    YCPValue id = getId( id_value );
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

#if VERBOSE_REPLACE_WIDGET
    rp->dumpDialogWidgetTree();
#endif

    YWidget::OptimizeChanges below( *currentDialog() ); // delay screen updates until this block is left

    rp->removeChildren();

    YWidget *widget = createWidgetTree( replpoint, rbg, new_widget );

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

	widget = createWidgetTree( replpoint, rbg, YCPTerm( YUIWidget_Empty, YCPList() ) );

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
 * @builtin WizardCommand( term wizardCommand ) -> boolean
 *
 * Issue a command to a wizard widget with ID 'wizardId'.
 * <p>
 * <b>This builtin is not for general use. Use the Wizard.ycp module instead.</b>
 * <p>
 * For available wizard commands see file YQWizard.cc .
 * If the current UI does not provide a wizard widget, 'false' is returned.
 * It is safe to call this even for UIs that don't provide a wizard widget. In
 * this case, all calls to this builtin simply return 'false'.
 * <p>
 * Returns true on success.
 */

YCPValue YUI::evaluateWizardCommand( const YCPTerm & command )
{
    if ( ! hasWizard() )
	return YCPBoolean( false );

    // A wizard widget always has ID `wizard
    YWidget * widget = widgetWithId( YCPSymbol( YWizardID ), false );

    if ( ! widget )
	return YCPBoolean( false );

    YWizard * wizard = dynamic_cast<YWizard *>( widget );

    if ( ! wizard )
	return YCPBoolean( false );

    blockEvents();	// Avoid self-generated events from builtins
    YCPValue ret = wizard->command( command );
    unblockEvents();

    return ret;
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

YCPBoolean YUI::evaluateSetFocus( const YCPValue & id_value )
{
    if ( ! isSymbolOrId( id_value ) )
	return YCPNull();

    YCPValue id = getId( id_value );
    YWidget *widget = widgetWithId( id, true );

    if ( ! widget )
	return YCPBoolean( false );

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

void YUI::evaluateBusyCursor()
{
    busyCursor();
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

void YUI::evaluateRedrawScreen()
{
    redrawScreen();
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

void YUI::evaluateNormalCursor()
{
    normalCursor();
}



/**
 * @builtin MakeScreenShot( string filename ) -> void
 *
 * Make a screen shot if the specific UI supports that.
 * The Qt UI opens a file selection box if filename is empty.
 */

void YUI::evaluateMakeScreenShot( const YCPString & filename )
{
    makeScreenShot( filename->value () );
}



/**
 * @builtin DumpWidgetTree() -> void
 *
 * Debugging function: Dump the widget tree of the current dialog to the log
 * file.
 */

void YUI::evaluateDumpWidgetTree()
{
    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "DumpWidgetTree: No dialog existing." );
	return;
    }

    dialog->dumpDialogWidgetTree();
}


/**
 * @builtin RecordMacro( string macroFileName ) -> void
 *
 * Begin recording a macro. Write the macro contents to file "macroFilename".
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
 * @builtin StopRecordingMacro() -> void
 *
 * Stop macro recording. This is only necessary if you don't wish to record
 * everything until the program terminates.
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
 * @builtin PlayMacro( string macroFileName ) -> void
 *
 * Execute everything in macro file "macroFileName".
 * Any errors are sent to the log file only.
 * The macro can be terminated only from within the macro file.
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
	    YCPValue result = macroPlayer->evaluateNextBlock();

	    if ( macroPlayer->error() || result.isNull() )
	    {
		y2error( "Macro aborted" );
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
 * @builtin FakeUserInput( any nextUserInput ) -> void
 * @builtin FakeUserInput() -> void
 *
 * Prepare a fake value for the next call to UserInput() -
 * i.e. the next UserInput() will return exactly this value.
 * This is only useful in connection with macros.
 * <p>
 * If called without a parameter, the next call to UserInput()
 * will return "nil".
 */
void YUI::evaluateFakeUserInput( const YCPValue & next_input )
{
    fakeUserInputQueue.push_back( next_input );
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
YCPString YUI::evaluateGlyph( const YCPSymbol & glyphSym )
{
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
void YUI::evaluateRecalcLayout()
{
    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "RecalcLayout(): No dialog existing" );
    }
    else
    {
	dialog->setInitialSize();
    }
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
void YUI::evaluatePostponeShortcutCheck()
{
    YDialog *dialog = currentDialog();

    if ( ! dialog )
    {
	y2error( "PostponeShortcutCheck(): No dialog existing" );
    }
    else
    {
	dialog->postponeShortcutCheck();
    }
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
void YUI::evaluateCheckShortcuts()
{
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
}


/**
 * @builtin WidgetExists( `id( any widgetId ) ) -> boolean
 * @builtin WidgetExists( symbol widgetId ) ) -> boolean
 *
 * Check whether or not a widget with the given ID currently exists in the
 * current dialog. Use this to avoid errors in the log file before changing the
 * properties of widgets that might or might not be there.
 */
YCPBoolean YUI::evaluateWidgetExists( const YCPValue & id_value )
{
    if ( ! isSymbolOrId( id_value ) ) return YCPNull();

    YCPValue id = getId( id_value );
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
YCPValue YUI::evaluateRunPkgSelection( const YCPValue & value_id )
{
    if ( ! isSymbolOrId( value_id ) )
    {
	y2error( "RunPkgSelection(): expecting `id( ... ), not '%s'", value_id->toString().c_str() );
	return YCPNull();
    }

    YCPValue id = getId( value_id );
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
YCPValue YUI::evaluateAskForExistingDirectory( const YCPString & startDir, const YCPString & headline )
{
    return askForExistingDirectory( startDir, headline );
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
YCPValue YUI::evaluateAskForExistingFile( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    return askForExistingFile( startWith, filter, headline );
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
YCPValue YUI::evaluateAskForSaveFileName( const YCPString & startWith, const YCPString & filter, const YCPString & headline )
{
    return askForSaveFileName( startWith, filter, headline );
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
void YUI::evaluateSetFunctionKeys( const YCPMap & new_fkeys )
{
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
}


int YUI::defaultFunctionKey( YCPString ylabel )
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
 * @builtin Recode ( string from, string to, string text ) -> any
 *
 * Recode encoding of string from or to "UTF-8" encoding.
 * One of from/to must be "UTF-8", the other should be an
 * iso encoding specifier (i.e. "ISO-8859-1" for western languages,
 * "ISO-8859-2" for eastern languages, etc. )
 */

YCPValue YUI::evaluateRecode( const YCPString & from, const YCPString & to, const YCPString & text )
{
    string outstr;
    if ( Recode ( text->value (), from->value (), to->value (), outstr ) != 0 )
    {
	static bool warned_about_recode = false;
	if ( ! warned_about_recode )
	{
	    y2error ( "Recode ( %s, %s, ... )", from->value().c_str(), to->value().c_str() );
	    warned_about_recode = true;
	}
	// return text as-is
	return ( text );
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

int YUI::Recode( const string & instr, const string & from,
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

    char * inptr  = (char *) instr.c_str();
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
