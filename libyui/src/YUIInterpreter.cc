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

  File:	      YUIInterpreter.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#define VERBOSE_COMMANDS	// pretty verbose logging of each UI command
#define noVERBOSE_COMM		// VERY verbose thread communication logging
#define noVERBOSE_FIND_RADIO_BUTTON_GROUP
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

#include "YUIInterpreter.h"
#include "YUISymbols.h"
#include "hashtable.h"
#include "YWidget.h"
#include "YMacroRecorder.h"
#include "YMacroPlayer.h"

#include "YAlignment.h"
#include "YBarGraph.h"
#include "YCheckBox.h"
#include "YComboBox.h"
#include "YDialog.h"
#include "YEmpty.h"
#include "YFrame.h"
#include "YImage.h"
#include "YIntField.h"
#include "YLabel.h"
#include "YLogView.h"
#include "YMenuButton.h"
#include "YProgressBar.h"
#include "YPushButton.h"
#include "YRadioButton.h"
#include "YRadioButtonGroup.h"
#include "YReplacePoint.h"
#include "YRichText.h"
#include "YSelectionBox.h"
#include "YMultiSelectionBox.h"
#include "YSpacing.h"
#include "YSplit.h"
#include "YSquash.h"
#include "YTable.h"
#include "YTextEntry.h"
#include "YTree.h"


void *start_ui_thread(void *ui_int)
{
    YUIInterpreter *ui_interpreter = (YUIInterpreter *) ui_int;
    ui_interpreter->uiThreadMainLoop();
    return 0;
}

// ----------------------------------------------------------------------


YUIInterpreter::YUIInterpreter(bool with_threads, Y2Component *callback)
    : id_counter(0)
    , with_threads(with_threads)
    , box_in_the_middle(YCPNull())
    , _moduleName("yast2")
    , macroRecorder(0)
    , macroPlayer(0)
    , callbackComponent(callback)
    , menuSelection( YCPVoid() )
{
}


YUIInterpreter::~YUIInterpreter()
{
    if (with_threads)
    {
	terminateUIThread();
	close( pipe_to_ui[0] );
	close( pipe_to_ui[1] );
	close( pipe_from_ui[0] );
	close( pipe_from_ui[1] );
    }

    while (dialogstack.size() > 0)
    {
	removeDialog();
    }

    deleteMacroRecorder();
    deleteMacroPlayer();
}


Y2Component *
YUIInterpreter::getCallback (void)
{
    y2debug ("YUIInterpreter[%p]::getCallback() = %p", this, callbackComponent);
    return callbackComponent;
}


void
YUIInterpreter::setCallback (Y2Component *callback)
{
    y2debug ("YUIInterpreter[%p]::setCallback(%p)", this, callback);
    callbackComponent = callback;
    return;
}


string
YUIInterpreter::interpreter_name () const
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
    // of the ui (YUIQt, YUINcurses).

    if (with_threads)
    {
	pipe(pipe_from_ui);
	pipe(pipe_to_ui);

	// Make fd non blockable the ui thread reads from
	long arg;
	arg = fcntl(pipe_to_ui[0], F_GETFL);
	if (fcntl(pipe_to_ui[0], F_SETFL, arg | O_NONBLOCK) < 0)
	{
	    y2error("Couldn't set O_NONBLOCK: %s\n", strerror(errno));
	    with_threads = false;
	    close(pipe_to_ui[0]);
	    close(pipe_to_ui[1]);
	    close(pipe_from_ui[0]);
	    close(pipe_from_ui[1]);
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
    pthread_attr_init(&attr);
    pthread_create(&ui_thread, &attr, start_ui_thread, this);
}


void YUIInterpreter::terminateUIThread()
{
    y2debug("Telling UI thread to shut down");
    terminate_ui_thread = true;
    signalUIThread();
    pthread_join(ui_thread, 0);
    y2debug("UI thread shut down correctly");
}


void YUIInterpreter::signalUIThread()
{
    static char arbitrary = 42;
    write (pipe_to_ui[1], &arbitrary, 1);

#ifdef VERBOSE_COMM
    y2debug("Wrote byte to ui thread %d", pipe_to_ui[1]);
#endif
}


bool YUIInterpreter::waitForUIThread ()
{
    char arbitrary;
    int res;

    do {
#ifdef VERBOSE_COMM
	y2debug ("Waiting for ui thread...");
#endif
	res = read (pipe_from_ui[0], &arbitrary, 1);
	if (res == -1) {
	    y2error ("waitForUIThread: %m");
	    if (errno == EINTR)
		continue;
	}
    } while (res == 0);

#ifdef VERBOSE_COMM
    y2debug ("Read byte from ui thread");
#endif

    // return true if we really did get a signal byte
    return res != -1;
}


void YUIInterpreter::signalYCPThread()
{
    static char arbitrary;
    write(pipe_from_ui[1], &arbitrary, 1);

#ifdef VERBOSE_COMM
    y2debug("Wrote byte to ycp thread %d", pipe_from_ui[1]);
#endif
}


bool YUIInterpreter::waitForYCPThread()
{
    char arbitrary;

    int res;
    do {
#ifdef VERBOSE_COMM
	y2debug ("Waiting for ycp thread...");
#endif
	res = read (pipe_to_ui[0], &arbitrary, 1);
	if (res == -1) {
	    y2error ("waitForYCPThread: %m");
	    if (errno == EINTR)
		continue;
	}
    } while (res == 0);

#ifdef VERBOSE_COMM
    y2debug ("Read byte from ycp thread");
#endif

    // return true if we really did get a signal byte
    return res != -1;
}


void YUIInterpreter::uiThreadMainLoop()
{
    while (true)
    {
	idleLoop (pipe_to_ui[0]);

	// The pipe is non-blocking, so we have to check if
	// we really read a signal byte. Although idleLoop
	// already makes a select, this seems to be necessary.
	// Anyway: Why do we set the pipe to non-blocking if
	// we wait in idleLoop for it to become readable? It
	// is needed in YUIQt::idleLoop for QSocketNotifier.
	if (!waitForYCPThread ())
	    continue;

	if (terminate_ui_thread) return;

	YCPValue command = box_in_the_middle;
	if (command.isNull())
	{
	    y2error("Command to ui is NULL");
	    box_in_the_middle = YCPVoid();
	}
	else if (command->isTerm())
	{
	    box_in_the_middle = executeUICommand(command->asTerm());
	}
	else
	{
	    y2error("Command to ui is not a term: '%s'",
		    command->toString().c_str() );
	    box_in_the_middle = YCPVoid();
	}

	signalYCPThread();
    }
}


YCPValue YUIInterpreter::evaluateInstantiatedTerm(const YCPTerm& term)
{
    string symbol = term->symbol()->symbol();
    y2debug("evaluateInstantiatedTerm(%s)", symbol.c_str());

    if (YUIInterpreter_in_word_set (symbol.c_str(), symbol.length()))
    {
	if ( macroPlayer && term->symbol()->symbol() == YUIBuiltin_UserInput )
	{
	    // This must be done in the YCP thread to keep the threads synchronized!
	    playNextMacroBlock();
	}

	if ( with_threads )
	{
	    box_in_the_middle = term;
	    signalUIThread();
	    while (!waitForUIThread());

	    return box_in_the_middle;
	}
	else return executeUICommand(term);
    }
    else return YCPNull();
}


YCPValue YUIInterpreter::callback(const YCPValue& value)
{
    y2debug ("YUIInterpreter::callback (%s)", value->toString().c_str());
    if (value->isBuiltin())
    {
	YCPBuiltin b = value->asBuiltin();
	YCPValue v = b->value (0);

	if (b->builtin_code() == YCPB_UI)
	{
	    return evaluate (v);
	}

	if (callbackComponent)
	{
	    YCPValue v = YCPNull();
	    if (b->builtin_code() == YCPB_WFM)		// if it goes to WFM, just send the value
	    {
		v = callbackComponent->evaluate (v);
	    }
	    else		// going to SCR, send the complete value
	    {
		v = callbackComponent->evaluate (value);
	    }
	    return v;
	}
    }

    return YCPNull();
}


YCPValue YUIInterpreter::evaluateUI(const YCPValue& value)
{
    y2debug ("YUIInterpreter::evaluateUI(%s)\n", value->toString().c_str());
    if (value->isBuiltin())
    {
	YCPBuiltin b = value->asBuiltin();
	if (b->builtin_code() == YCPB_DEEPQUOTE)
	{
	    return evaluate (b->value(0));
	}
    }
    else if (value->isTerm())
    {
	YCPTerm vt = value->asTerm();
	YCPTerm t (YCPSymbol (vt->symbol()->symbol(), false), vt->name_space());
	for (int i = 0; i < vt->size(); i++)
	{
	    YCPValue v = evaluate (vt->value (i));
	    if (v.isNull ())
	    {
		return YCPError ("YUI parameter is NULL\n", YCPNull ());
	    }
	    t->add (v);
	}
	return evaluateInstantiatedTerm (t);
    }
    return YCPError ("Unknown UI:: operation");
}


YCPValue YUIInterpreter::evaluateWFM(const YCPValue& value)
{
    y2debug ("YUIInterpreter[%p]::evaluateWFM[%p](%s)\n", this, callbackComponent, value->toString().c_str());
    if (callbackComponent)
    {
	if (value->isBuiltin())
	{
	    YCPBuiltin b = value->asBuiltin();
	    if (b->builtin_code() == YCPB_DEEPQUOTE)
	    {
		return callbackComponent->evaluate (b->value(0));
	    }
	}
	else if (value->isTerm())
	{
	    YCPTerm vt = value->asTerm();
	    YCPTerm t(YCPSymbol(vt->symbol()->symbol(), false), vt->name_space());
	    for (int i=0; i<vt->size(); i++)
	    {
		YCPValue v = evaluate (vt->value(i));
		if (v.isNull())
		{
		    return YCPError ("WFM parameter is NULL\n", YCPNull());
		}
		t->add(v);
	    }
	    return callbackComponent->evaluate (t);
	}
	return callbackComponent->evaluate (value);
    }
    y2error ("YUIInterpreter[%p]: No callbackComponent", this);
    return YCPError ("YUIInterpreter::evaluateWFM: No callback component WFM existing.", YCPNull());
}


YCPValue YUIInterpreter::evaluateSCR(const YCPValue& value)
{
    y2debug ("evaluateSCR(%s)\n", value->toString().c_str());
    if (callbackComponent)
    {
	if (value->isBuiltin())
	{
	    YCPBuiltin b = value->asBuiltin();
	    if (b->builtin_code() == YCPB_DEEPQUOTE)
	    {
		return callbackComponent->evaluate (YCPBuiltin (YCPB_SCR, b->value(0)));
	    }
	}
	else if (value->isTerm())
	{
	    YCPTerm vt = value->asTerm();
	    YCPTerm t(YCPSymbol(vt->symbol()->symbol(), false), vt->name_space());
	    for (int i=0; i<vt->size(); i++)
	    {
		YCPValue v = evaluate (vt->value(i));
		if (v.isNull())
		{
		    return YCPError ("SCR parameter is NULL\n", YCPNull());
		}
		t->add(v);
	    }
	    return callbackComponent->evaluate (YCPBuiltin (YCPB_SCR, t));
	}
	return callbackComponent->evaluate (YCPBuiltin (YCPB_SCR, value));
    }
    return YCPError ("YUIInterpreter::evaluateSCR: No callback component WFM existing.", YCPNull());
}


YCPValue YUIInterpreter::evaluateLocale(const YCPLocale& l)
{
    y2debug("evaluateLocale(%s)\n", l->value()->value().c_str());

    // locales are evaluated by the WFM now. If a locale happens
    // to show up here, send it back. evaluateWFM() might return
    // YCPNull() if no WFM is available. Handle this also.

    YCPValue v = evaluateWFM(l);
    if (v.isNull())
    {
	return l->value();  // return YCPString()
    }
    return v;
}


YCPValue YUIInterpreter::setTextdomain (const string& textdomain)
{
    return evaluateWFM (YCPBuiltin (YCPB_LOCALDOMAIN, YCPString (textdomain)));
}

string YUIInterpreter::getTextdomain (void)
{
    YCPValue v = evaluateWFM (YCPBuiltin (YCPB_GETTEXTDOMAIN));
    if (!v.isNull() && v->isString())
	return v->asString()->value();
    return "ui";
}

// ----------------------------------------------------------------------
// Default implementations for the virtual methods the deal with
// event processing


void YUIInterpreter::idleLoop(int fd_ycp)
{
    // Just wait for fd_ycp to become readable
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd_ycp, &fdset);
    // FIXME: check for EINTR
    select(fd_ycp+1, &fdset, 0, 0, 0);
}


YWidget *YUIInterpreter::pollInput(YDialog *, EventType *event)
{
    y2internal("default pollInput function called");
    // Default implementation: no input.
    *event = ET_NONE;
    return 0;
}


YWidget *YUIInterpreter::userInput(YDialog *, EventType *event)
{
    y2internal("default userInput function called");
    // Default implementation: cancel
    *event = ET_CANCEL;
    return 0;
}

bool YUIInterpreter::waitForEvent(int fd_ycp)
{
    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(fd_ycp, &fdset);
    // FIXME: check for EINTR
    select(fd_ycp+1, &fdset, 0, 0, 0);
    return false;
}


// ----------------------------------------------------------------------


YCPValue YUIInterpreter::executeUICommand(const YCPTerm &term)
{
    string symbol = term->symbol()->symbol();
    YCPValue ret = YCPNull();
#ifdef VERBOSE_COMMANDS
    y2debug("YUIInterpreter::executeUICommand(%s)", symbol.c_str() );
#endif

    if	    (symbol == YUIBuiltin_SetModulename		)	ret = evaluateSetModulename	(term);
    else if (symbol == YUIBuiltin_GetModulename		)	ret = evaluateGetModulename	(term);
    else if (symbol == YUIBuiltin_SetLanguage		)	ret = evaluateSetLanguage	(term);
    else if (symbol == YUIBuiltin_GetLanguage		)	ret = evaluateGetLanguage	(term);
    else if (symbol == YUIBuiltin_SetConsoleFont	)	ret = evaluateSetConsoleFont	(term);
    else if (symbol == YUIBuiltin_UserInput		)	ret = evaluateUserInput		(term, false);
    else if (symbol == YUIBuiltin_PollInput		)	ret = evaluateUserInput		(term, true);
    else if (symbol == YUIBuiltin_OpenDialog		)	ret = evaluateOpenDialog	(term);
    else if (symbol == YUIBuiltin_CloseDialog		)	ret = evaluateCloseDialog	(term);
    else if (symbol == YUIBuiltin_ChangeWidget		)	ret = evaluateChangeWidget	(term);
    else if (symbol == YUIBuiltin_QueryWidget		)	ret = evaluateQueryWidget	(term);
    else if (symbol == YUIBuiltin_ReplaceWidget		)	ret = evaluateReplaceWidget	(term);
    else if (symbol == YUIBuiltin_HasSpecialWidget	)	ret = evaluateHasSpecialWidget	(term);
    else if (symbol == YUIBuiltin_SetFocus		)	ret = evaluateSetFocus		(term);
    else if (symbol == YUIBuiltin_BusyCursor		)	ret = evaluateBusyCursor	(term);
    else if (symbol == YUIBuiltin_NormalCursor		)	ret = evaluateNormalCursor	(term);
    else if (symbol == YUIBuiltin_RedrawScreen		)	ret = evaluateRedrawScreen	(term);
    else if (symbol == YUIBuiltin_DumpWidgetTree	)	ret = evaluateDumpWidgetTree	(term);
    else if (symbol == YUIBuiltin_MakeScreenShot	)	ret = evaluateMakeScreenShot	(term);
    else if (symbol == YUIBuiltin_Recode         	)	ret = evaluateRecode		(term);
    else if (symbol == YUIBuiltin_RecordMacro		)	ret = evaluateRecordMacro	(term);
    else if (symbol == YUIBuiltin_StopRecordMacro 	)	ret = evaluateStopRecordMacro	(term);
    else if (symbol == YUIBuiltin_PlayMacro		)	ret = evaluatePlayMacro		(term);
    else if (symbol == YUIBuiltin_FakeUserInput		)	ret = evaluateFakeUserInput	(term);
    else if (symbol == YUIBuiltin_Glyph			)	ret = evaluateGlyph		(term);
    else if (symbol == YUIBuiltin_GetDisplayInfo	)	ret = evaluateGetDisplayInfo	(term);
    else if (symbol == YUIBuiltin_RecalcLayout		)	ret = evaluateRecalcLayout	(term);
    else if (symbol == YUIBuiltin_WidgetExists		)	ret = evaluateWidgetExists	(term);
    else if (symbol == YUIBuiltin_WFM			)	ret = evaluateCallback		(term, true);
    else if (symbol == YUIBuiltin_SCR			)	ret = evaluateCallback		(term, false);
    else
    {
	y2internal("Unknown term symbol in executeUICommand: %s",
		   symbol.c_str());
	return YCPVoid();
    }

    if (ret.isNull())
    {
	return YCPError (string ("Invalid arguments for ui command: ")+term->toString());
    }
    else return ret;
}



/**
 * @builtin SetModulename(string name) -> void
 *
 * Does nothing. The SetModulename command is introduced for
 * the translator. But the translator sends all commands
 * to the ui. So the ui shouldn't complain about this
 * command.
 *
 * @example SetModulename("inst_environment")
 */

YCPValue YUIInterpreter::evaluateSetModulename(const YCPTerm& term)
{
    if (term->size() == 1 && term->value(0)->isString())
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
 * @builtin GetModulename(<string>) -> string
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

YCPValue YUIInterpreter::evaluateGetModulename(const YCPTerm& term)
{
    if ((term->size() == 1) && (term->value(0)->isString()))
    {
	return term->value(0);
    }
    else return YCPNull();
}



/**
 * @builtin SetLanguage(string lang, [string encoding]) -> nil
 *
 * Tells the ui that the user has selected another language.
 * If the ui has any language dependend output that language
 * setting is honored. <tt>lang</tt> is an ISO language string,
 * such as <tt>de</tt> or <tt>de_DE</tt>. It is required
 * to specify an encoding scheme, since not all user interfaces
 * are capable of UTF-8.
 *
 * @example SetLanguage("de_DE@euro")
 * @example SetLanguage("en_GB")
 */

YCPValue YUIInterpreter::evaluateSetLanguage(const YCPTerm& term)
{
    if (term->size() > 0 && term->value(0)->isString())
    {
	string lang = term->value(0)->asString()->value();
	if (term->size() > 1 && term->value(1)->isString())
	{
	    lang += ".";
	    lang += term->value(1)->asString()->value();
	}

	setenv("LANG", lang.c_str(), 1); // 1 : replace
#ifdef WE_HOPEFULLY_DONT_NEED_THIS_ANY_MORE
	setlocale( LC_ALL, "" );	// switch char set mapping in glibc
#endif
	setlocale( LC_NUMERIC, "C");	// but always format numbers with "."
	YCPTerm newTerm = YCPTerm(term->symbol());
	newTerm->add (YCPString(lang));
	y2milestone ("ui specific setLanguage(%s)", newTerm->toString().c_str());
	return setLanguage(newTerm); 	// UI-specific setLanguage: returns YCPVoid() if OK, YCPNull() if error
    }
    else return YCPNull();
}


/*
 * Default UI-specific setLanguage()
 * Returns OK (YCPVoid())
 */

YCPValue YUIInterpreter::setLanguage(const YCPTerm &term)
{
    // NOP

    return YCPVoid();	// OK (YCPNull() would mean error)
}


/**
 * @builtin SetConsoleFont(string console_magic, string font, string screen_map, string unicode_map, string encoding) -> nil
 *
 * Switches the text console to the specified font.
 * See the setfont(8) command and the console HowTo for details.
 *
 * @example SetConsoleFont("(K", "lat2u-16.psf", "latin2u.scrnmap", "lat2u.uni", "latin1" )
 */

YCPValue YUIInterpreter::evaluateSetConsoleFont(const YCPTerm& term)
{
    if (term->size() == 5 &&
	term->value(0)->isString() &&
	term->value(1)->isString() &&
	term->value(2)->isString() &&
	term->value(3)->isString() &&
	term->value(4)->isString()   )
    {
	return setConsoleFont ( term->value(0)->asString(),	// console magic
				term->value(1)->asString(),	// font
				term->value(2)->asString(),	// screen_map
				term->value(3)->asString(),	// unicode_map
				term->value(4)->asString() );	// encoding
    }
    else return YCPNull();
}


/*
 * Default UI-specific setConsoleFont()
 * Returns OK (YCPVoid())
 */

YCPValue YUIInterpreter::setConsoleFont ( const YCPString &console_magic,
					  const YCPString &font,
					  const YCPString &screen_map,
					  const YCPString &unicode_map,
					  const YCPString &encoding )
{
    // NOP

    return YCPVoid();	// OK (YCPNull() would mean error)
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
void YUIInterpreter::makeScreenShot()
{
    // NOP
}



/**
 * @builtin GetLanguage(boolean strip_encoding) -> string
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
 * contents of the "LANG" environment variable is returned (which very likely
 * ends with ".UTF-8" since this is the encoding YaST2 uses internally).
 */

YCPValue YUIInterpreter::evaluateGetLanguage(const YCPTerm& term)
{
    if (term->size() == 1 && term->value(0)->isBoolean() )
    {
	bool strip_encoding = term->value(0)->asBoolean()->value();
	const char *lang_cstr = getenv("LANG");
	string lang = "";		// Fallback if $LANG not set

	if ( lang_cstr )		// only if environment variable set
	{
	    lang = lang_cstr;

	    if ( strip_encoding )
	    {
		y2milestone("Stripping encoding");
		string::size_type pos = lang.find_first_of(".@");

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
 */

/**
 * @builtin PollInput() -> any
 *
 * Doesn't wait but just looks if the user has clicked some
 * button, has closed the window or has activated
 * some widget that has the <tt>`notify</tt> option set. Returns
 * the id of the widget that has been selected
 * or <tt>`cancel</tt> if the user selected the implicite cancel
 * button (for example he closes the window). Returns nil if no
 * user input has occured.
 */

YCPValue YUIInterpreter::evaluateUserInput(const YCPTerm& term, bool poll)
{
    if (term->size() != 0)
	return YCPNull();

    YDialog *dialog = currentDialog();

    if (!dialog)
    {
	y2error("%s(): No dialog existing",
		poll ? YUIBuiltin_PollInput : YUIBuiltin_UserInput);
	internalError( "No dialog existing during UserInput().\n"
		       "\n"
		       "Please check the log file!" );
	return YCPSymbol("internalError", true);
    }

    EventType event_type;
    YWidget *event_widget = 0;
    YCPValue input = YCPNull();

    if ( fakeUserInputQueue.empty() )
    {
	if ( poll )
	{
	    event_widget = filterInvalidEvents( pollInput(dialog, &event_type) );
	}
	else
	{
	    do
	    {
		event_widget = filterInvalidEvents( userInput(dialog, &event_type) );
	    } while ( event_type != ET_CANCEL &&
		      event_type != ET_DEBUG  &&
		      event_type != ET_MENU   &&
		      ! event_widget );
	}

	switch (event_type)
	{
	    case ET_CANCEL:
		input = YCPSymbol("cancel", true);
		break;

	    case ET_DEBUG:
		input = YCPSymbol("debugHotkey", true);
		break;

	    case ET_WIDGET:
		input = event_widget ? event_widget->id() : YCPVoid();
		break;

	    case ET_MENU:
		input = getMenuSelection();
		setMenuSelection( YCPVoid() );
		y2debug( "Menu selection: '%s'", input->toString().c_str() );
		break;

	    case ET_NONE:
	    default:
		input = YCPVoid();
	}
    }
    else // fakeUserInputQueue contains elements -> use the first one
    {
	input = fakeUserInputQueue.front();
	fakeUserInputQueue.pop_front();
    }

    if ( macroRecorder )
    {
	macroRecorder->beginBlock();
	dialog->saveUserInput( macroRecorder );
	macroRecorder->recordUserInput( input );
	macroRecorder->endBlock();
    }

    return input;
}


YWidget *
YUIInterpreter::filterInvalidEvents( YWidget *event_widget )
{
    if ( ! event_widget )
	return 0;

    if ( ! event_widget->isValid() )
    {
	/**
	 * Silently discard events from widgets that have become invalid.
	 *
	 * This may legitimately happen if some widget triggered an event yet
	 * nobody cared for that event (i.e. called UserInput() or PollInput())
	 * and the widget has been destroyed meanwhile.
	 **/

	// y2debug("Discarding event for widget that has become invalid");

	return 0;
    }


    if ( event_widget->yDialog() != currentDialog() )
    {
	/**
	 * Silently discard events from all but the current (topmost) dialog.
	 *
	 * This may happen even here even though the specific UI should have
	 * taken care about that: Events may still be in the queue. They might
	 * have been valid (i.e. belonged to the topmost dialog) when they
	 * arrived, but maybe simply nobody has evaluated them.
	 **/

	// Yes, really y2debug() - this may legitimately happen.
	y2debug( "Discarding event from widget from foreign dialog" );
	return 0;
    }


    return event_widget;
}


/**
 * @builtin OpenDialog(term widget) -> boolean
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
 * @example OpenDialog(`Label("Please wait..."))
 */

/**
 * @builtin OpenDialog(term options, term widget) -> boolean
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
 * @example OpenDialog(`opt(`defaultsize), `Label("Hi"))
 */

YCPValue YUIInterpreter::evaluateOpenDialog(const YCPTerm& term)
{
    int s = term->size();
    if ((s == 1 || s == 2)
	&& term->value(s-1)->isTerm()
	&& (s == 1 || term->value(0)->isTerm() && term->value(0)->asTerm()->symbol()->symbol() == YUISymbol_opt))
    {
	YCPList optList;
	if (s == 2) optList = term->value(0)->asTerm()->args();
	YWidgetOpt opt;

	for (int o=0; o < optList->size(); o++)
	{
	    if (optList->value(o)->isSymbol())
	    {
		if	(optList->value(o)->asSymbol()->symbol() == YUIOpt_defaultsize)	opt.hasDefaultSize.setValue(true);
		else if (optList->value(o)->asSymbol()->symbol() == YUIOpt_warncolor)	opt.hasWarnColor.setValue(true);
		else if (optList->value(o)->asSymbol()->symbol() == YUIOpt_infocolor)	opt.hasInfoColor.setValue(true);
		else if (optList->value(o)->asSymbol()->symbol() == YUIOpt_decorated)	opt.isDecorated.setValue(true);
		else    y2warning("Unknown option %s for OpenDialog", term->value(o)->toString().c_str());
	    }
	}

	YDialog *dialog = createDialog(opt);
	if (dialog)
	{
	    registerDialog(dialog); // must be done first!
	    YWidget *widget = createWidgetTree(dialog, 0, term->value(s-1)->asTerm());

	    if (widget)
	    {
		dialog->addChild(widget);
		dialog->setInitialSize();
		dialog->checkKeyboardShortcuts();
		showDialog(dialog);
		return YCPBoolean(true);
	    }
	    else removeDialog();
	}
	return YCPBoolean(false);
    }
    else return YCPNull();
}



/**
 * @builtin CloseDialog() -> boolean
 *
 * Closes the most recently opened dialog. It is an error
 * to call <tt>CloseDialog</tt> if no dialog is open.
 * <p>
 * Returns true on success.
 */

YCPValue YUIInterpreter::evaluateCloseDialog(const YCPTerm& term)
{
    if (term->size() == 0)
    {
	YDialog *dialog = currentDialog();
	if (!dialog)
	{
	    return YCPError ("Can't CloseDialog: No dialog existing.", YCPBoolean(false));
	}
	closeDialog(dialog);
	removeDialog();
	return YCPBoolean(true);
    }
    else return YCPNull();
}



/**
 * @builtin ChangeWidget(`id(any id), symbol property, any newvalue) -> boolean
 *
 * Changes a property of a widget of the topmost dialog. <tt>id</tt> specified
 * the widget to change, <tt>property</tt> specifies the property that should
 * be changed, <tt>newvalue</tt> gives the new value.
 * <p>
 * For example in order to change the label of a TextEntry with id 8 to
 * "anything", you write <tt>ChangeWidget(`id(8), `Label, "anything")</tt>.
 * <p>
 * Returns true on success.
 */

YCPValue YUIInterpreter::evaluateChangeWidget(const YCPTerm& term)
{
    if (term->size() != 3
	|| !checkId(term->value(0))
	|| (!term->value(1)->isSymbol() && !term->value(1)->isTerm())) return YCPNull();

    YCPValue id = getId(term->value(0));

    YWidget *widget = widgetWithId(id, true);
    if (!widget) return YCPVoid();
    else if (term->value(1)->isSymbol())
    {
	YCPSymbol sym = term->value(1)->asSymbol();
	YCPValue ret = widget->changeWidget(sym, term->value(2));

	if ( widget->shortcutProperty()				// The widget has a shortcut property
	     && sym->symbol() == widget->shortcutProperty()	// and this is what should be changed
	     && ret->isBoolean()				// and the change didn't return 'nil' (error)
	     && ret->asBoolean()->value() )			// and was successful
	{
	    // The shortcut property has just successfully been changed
	    // -> time for a new check

	    currentDialog()->checkKeyboardShortcuts();
	}

	return ret;
    }
    else
	return widget->changeWidget(term->value(1)->asTerm(), term->value(2));
}



/**
 * @builtin QueryWidget(`id(any id), symbol|term property) -> any
 *
 * Queries a property of a widget of the topmost dialog.  For example in order
 * to query the current text of a TextEntry with id 8, you write
 * <tt>QueryWidget(`id(8), `Value)</tt>. In some cases the propery can be given
 * as term in order to further specify it. An example is
 * <tt>QueryWidget(`id(`table), `Item(17))</tt> for a table where you query a
 * certain item.
 */

/**
 * @builtin QueryWidget(`id(any id), term property) -> any
 *
 * Queries a property of a widget of the topmost dialog. For example in order
 * to query the current text of a TextEntry with id 8, you write
 * <tt>QueryWidget(`id(8), `Value)</tt>
 */

YCPValue YUIInterpreter::evaluateQueryWidget(const YCPTerm& term)
{
    if (term->size() != 2
	|| !checkId(term->value(0))
	|| (!term->value(1)->isSymbol() && !term->value(1)->isTerm())) return YCPNull();

    YCPValue id = getId(term->value(0));

    YWidget *widget = widgetWithId(id, true); // reports error
    if (!widget) return YCPVoid();
    else if (term->value(1)->isSymbol())
	return widget->queryWidget(term->value(1)->asSymbol());
    else
	return widget->queryWidget(term->value(1)->asTerm());
}



/**
 * @builtin ReplaceWidget(`id(any id), term newwidget) -> boolean
 *
 * Replaces a complete widget (or widget subtree) with an other widget
 * (or widget tree). You can only replace the widget contained in
 * a <tt>ReplacePoint</tt>. As parameters to <tt>ReplaceWidget</tt>
 * specify the id of the ReplacePoint and the new widget.
 * <p>
 * This is an example:
 * <pre>
 * OpenDialog(`ReplacePoint(`id(`rp), `PushButton("OK")));
 * ...
 * // sometimes later...
 * ...
 * ReplaceWidget(`id(`rp), `Label("Label"))
 * </pre>
 */

YCPValue YUIInterpreter::evaluateReplaceWidget(const YCPTerm& term)
{
    if (term->size() != 2
	|| !checkId(term->value(0))
	|| !term->value(1)->isTerm()) return YCPNull();

    YCPValue id = getId(term->value(0));
    YWidget *replpoint = widgetWithId(id, true); // reports error
    if (!replpoint) return YCPBoolean(false);

    if (!replpoint->isReplacePoint())
    {
	y2error("ReplaceWidget: widget %s is not a ReplacePoint",
		id->toString().c_str());
	return YCPBoolean(false);
    }

    YReplacePoint *rp = dynamic_cast <YReplacePoint *> ( replpoint );
    assert(rp);

    // What if the widget tree to be inserted contains radiobuttons, but the
    // radiobutton group is in the unchanged rest? We must find the radio button
    // group belonging to the new subtree.

    bool contains;
    YRadioButtonGroup *rbg = findRadioButtonGroup(currentDialog(), replpoint, &contains);

    // I must _first_ remove the old widget and then create the new ones. The reason
    // is: Otherwise you couldn't use the same widget ids in the old and new widget tree.

#ifdef VERBOSE_REPLACE_WIDGET
    rp->dumpDialogWidgetTree();
#endif

    YWidget::OptimizeChanges below( *currentDialog() ); // delay screen updates until this block is left

    rp->removeChildren();

    YWidget *widget = createWidgetTree(replpoint, rbg, term->value(1)->asTerm());

    if (widget)
    {
	rp->addChild(widget);
	currentDialog()->setInitialSize();
	currentDialog()->checkKeyboardShortcuts();
	return YCPBoolean(true);
    }
    else
    {
	widget = createWidgetTree(replpoint, rbg, YCPTerm(YCPSymbol(YUIWidget_Empty, true), YCPList()));
	if (widget)
	{
	    rp->addChild(widget);
	    currentDialog()->setInitialSize();
	    currentDialog()->checkKeyboardShortcuts();
	}
	else // Something bad will happen
	    y2error("Severe problem: can't create Empty widget");
	return YCPBoolean(false);
    }
}



/**
 * @builtin HasSpecialWidget(`symbol widget) -> boolean
 *
 * Checks for support of a special widget type. Use this prior to creating a
 * widget of this kind. Do not use this to check for ordinary widgets like
 * PushButton etc. - just the widgets where the widget documentation explicitly
 * states it is an optional widget not supported by all UIs.
 * <p>
 * Returns true if the UI supports the special widget and false if not.
 */

YCPValue YUIInterpreter::evaluateHasSpecialWidget(const YCPTerm& term)
{
    bool hasWidget = false;

    if (term->size() != 1 || ! term->value(0)->isSymbol() )
	return YCPNull();

    string symbol = term->value(0)->asSymbol()->symbol();

    if	    (symbol == YUISpecialWidget_DummySpecialWidget	)	hasWidget = hasDummySpecialWidget();
    else if (symbol == YUISpecialWidget_BarGraph		)	hasWidget = hasBarGraph();
    else if (symbol == YUISpecialWidget_ColoredLabel		)	hasWidget = hasColoredLabel();
    else if (symbol == YUISpecialWidget_DownloadProgress	)	hasWidget = hasDownloadProgress();
    else if (symbol == YUISpecialWidget_Slider			)	hasWidget = hasSlider();
    else if (symbol == YUISpecialWidget_PartitionSplitter	)	hasWidget = hasPartitionSplitter();
    else
    {
	y2error("HasSpecialWidget(): Unknown special widget: %s", symbol.c_str() );
	return YCPNull();
    }

    return YCPBoolean(hasWidget);
}



/**
 * @builtin SetFocus(`id(any id)) -> boolean
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

YCPValue YUIInterpreter::evaluateSetFocus(const YCPTerm& term)
{
    if (term->size() != 1 || !checkId(term->value(0)) )
	return YCPNull();

    YCPValue id = getId(term->value(0));
    YWidget *widget = widgetWithId(id, true);

    if ( !widget )
	return YCPVoid();

    return YCPBoolean( widget->setKeyboardFocus() );
}



/**
 * @builtin BusyCursor() -> void
 *
 * Sets the mouse cursor to the busy cursor, if the UI supports such a feature.
 * <p>
 * This should normally not be necessary. The UI handles mouse cursors itself:
 * When input is possible (i.e. inside UserInput()), there is automatically a
 * normal cursor, otherwise, there is the busy cursor. Override this at your
 * own risk.
 */

YCPValue YUIInterpreter::evaluateBusyCursor(const YCPTerm& term)
{
    if (term->size() != 0 )
	return YCPNull();

    busyCursor();
    return YCPVoid();
}



/**
 * @builtin RedrawScreen() -> void
 *
 * Redraws the screen after it very likely has become garbled by some other output.
 * <p>
 * This should normally not be necessary: The (specific) UI redraws the screen
 * automatically whenever required. Under rare circumstances, however, the
 * screen might have changes due to circumstances beyond the UI's control: For
 * text based UIs, for example, system commands that cause output to every tty
 * might make this necessary. Call this in the YCP code after such a command.
 */

YCPValue YUIInterpreter::evaluateRedrawScreen(const YCPTerm& term)
{
    if (term->size() != 0 )
	return YCPNull();

    redrawScreen();
    return YCPVoid();
}


/**
 * @builtin NormalCursor() -> void
 *
 * Sets the mouse cursor to the normal cursor (after BusyCursor), if the UI
 * supports such a feature.
 * <p>
 * This should normally not be necessary. The UI handles mouse cursors itself:
 * When input is possible (i.e. inside UserInput()), there is automatically a
 * normal cursor, otherwise, there is the busy cursor. Override this at your
 * own risk.
 */

YCPValue YUIInterpreter::evaluateNormalCursor(const YCPTerm& term)
{
    if (term->size() != 0 )
	return YCPNull();

    normalCursor();
    return YCPVoid();
}



/**
 * @builtin MakeScreenShot() -> void
 *
 * Make a screen shot if the specific UI supports that.	 The Qt UI creates PNG
 * files in the /tmp directory.
 */

YCPValue YUIInterpreter::evaluateMakeScreenShot(const YCPTerm& term)
{
    if (term->size() != 0 )
	return YCPNull();

    makeScreenShot();
    return YCPVoid();
}



/**
 * @builtin DumpWidgetTree() -> void
 *
 * Debugging function: Dump the widget tree of the current dialog to the log
 * file.
 */

YCPValue YUIInterpreter::evaluateDumpWidgetTree(const YCPTerm& term)
{
    if (term->size() != 0 )
	return YCPNull();

    YDialog *dialog = currentDialog();

    if (!dialog)
    {
	y2error("DumpWidgetTree: No dialog existing.");
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
YCPValue YUIInterpreter::evaluateRecordMacro (const YCPTerm& term)
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
YCPValue YUIInterpreter::evaluateStopRecordMacro (const YCPTerm& term)
{
    if (term->size() != 0 )
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
YCPValue YUIInterpreter::evaluatePlayMacro (const YCPTerm& term)
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
YCPValue YUIInterpreter::evaluateFakeUserInput (const YCPTerm& term)
{
    if ( term->size() != 1 )	// must have 1 arg - anything allowed
    {
	return YCPNull();
    }

    fakeUserInputQueue.push_back( term->value(0) );

    return YCPVoid();
}



/**
 * @builtin Glyph( symbol glyph ) -> string
 *
 * Return a special character (a 'glyph') according to the symbol specified.
 * <p>
 * Not all UIs may be capable of displaying every glyph; if a specific UI
 * doesn't support it, a textual representation (probably in plain ASCII) will
 * be returned.
 * <p>
 * This is also why there is only a limited number of predefined
 * glyphs: An ASCII equivalent is required which is sometimes hard to find for
 * some characters defined in Unicode / UTF-8.
 * <p>
 * Please note the value returned may consist of more than one character; for
 * example, Glyph(`ArrowRight) may return something like "-&gt;".
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
YCPValue YUIInterpreter::evaluateGlyph(const YCPTerm& term)
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

	if      ( sym == YUIGlyph_ArrowLeft		)	glyphText = YCPString( "<-"  );
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
 *		<td>	The width of a `opt(`defaultsize) dialog.</td>
 *	</tr>
 *	<tr>
 *		<td>	DefaultHeight		</td>	<td>integer</td>
 *		<td>	The height of a `opt(`defaultsize) dialog.</td>
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
 *			having to use <tt>SCR::Read(.target.byte, ...)</tt>.
 *		</td>
 *	</tr>
 *	<tr>
 *		<td>	HasAnimationSupport	</td>	<td>boolean</td>
 *		<td>	<i>true</i> if animations can be displayed,
 *			i.e. if the Image widget supports <tt>`opt(`animated)</tt>.
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
 * mode</>. If you think there is no proper layout etc. solution for NCurses,
 * it might be time to reconsider the complexity or even the concept of your
 * dialog.
 *
 */
YCPValue YUIInterpreter::evaluateGetDisplayInfo(const YCPTerm& term)
{
    if (term->size() != 0 )
	return YCPNull();

    YCPMap info_map;

    info_map->add( YCPString( YUICap_Width	        	), YCPInteger( getDisplayWidth()	));
    info_map->add( YCPString( YUICap_Height	        	), YCPInteger( getDisplayHeight()   	));
    info_map->add( YCPString( YUICap_Depth	        	), YCPInteger( getDisplayDepth()   	));
    info_map->add( YCPString( YUICap_Colors	        	), YCPInteger( getDisplayColors()   	));
    info_map->add( YCPString( YUICap_DefaultWidth       	), YCPInteger( getDefaultWidth()   	));
    info_map->add( YCPString( YUICap_DefaultHeight      	), YCPInteger( getDefaultHeight()   	));
    info_map->add( YCPString( YUICap_TextMode	        	), YCPBoolean( textMode()	   	));
    info_map->add( YCPString( YUICap_HasImageSupport    	), YCPBoolean( hasImageSupport()   	));
    info_map->add( YCPString( YUICap_HasLocalImageSupport    	), YCPBoolean( hasLocalImageSupport()  	));
    info_map->add( YCPString( YUICap_HasAnimationSupport	), YCPBoolean( hasAnimationSupport()   	));
    info_map->add( YCPString( YUICap_HasIconSupport     	), YCPBoolean( hasIconSupport()	    	));
    info_map->add( YCPString( YUICap_HasFullUtf8Support 	), YCPBoolean( hasFullUtf8Support() 	));

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
 * like the a Label widget's value. Call this once (!) after changing all such
 * widget properties.
 */
YCPValue YUIInterpreter::evaluateRecalcLayout(const YCPTerm& term)
{
    if (term->size() != 0 )
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
 * @builtin WidgetExists(`id(any widgetId)) -> boolean
 *
 * Check whether or not a widget with the given ID currently exists in the
 * current dialog. Use this to avoid errors in the log file before changing the
 * properties of widgets that might or might not be there.
 */
YCPValue YUIInterpreter::evaluateWidgetExists(const YCPTerm& term)
{
    if (term->size() != 1
	|| ! checkId( term->value(0) ) ) return YCPNull();

    YCPValue id = getId(term->value(0));
    YWidget *widget = widgetWithId(id, false ); // reports error
    return widget ? YCPBoolean( true ) : YCPBoolean( false );
}


/**
 * @builtin WFM/SCR ( expression ) -> any
 *
 * This is used for a callback mechanism. The expression will
 * be sent to the WFM interpreter and evaluated there.
 * USE WITH CAUTION.
 */

YCPValue YUIInterpreter::evaluateCallback (const YCPTerm& term, bool to_wfm)
{
    y2debug ("(%s), callback @ %p", term->toString().c_str(), callbackComponent);
    if ( term->size() != 1 )	// must have 1 arg - anything allowed
    {
	return YCPNull();
    }

    if (callbackComponent)
    {
	YCPValue v = YCPNull();
	if (to_wfm)		// if it goes to WFM, just send the value
	{
	    v = callbackComponent->evaluate (term->value(0));
	}
	else		// going to SCR, send the complete term
	{
	    v = callbackComponent->evaluate (term);
	}
	y2debug ("callback returns (%s)", v->toString().c_str());
	return v;
    }
    return YCPVoid();
}


static iconv_t fromutf8_cd   = (iconv_t)(-1);
static string  fromutf8_name = "";

static iconv_t toutf8_cd     = (iconv_t)(-1);
static string  toutf8_name   = "";

static iconv_t fromto_cd     = (iconv_t)(-1);
static string  from_name     = "";
static string  to_name       = "";

static const unsigned recode_buf_size = 1024;
static char           recode_buf[recode_buf_size];

int YUIInterpreter::Recode( const string & instr, const string & from,
			    const string & to, string & outstr )
{
    if (from == to
	|| instr.empty())
    {
	outstr = instr;
	return 0;
    }

    iconv_t cd = (iconv_t)(-1);

    if ( from == "UTF-8" )
    {
	if (fromutf8_cd == (iconv_t)(-1)
	    || fromutf8_name != to)
	{
	    if (fromutf8_cd != (iconv_t)(-1))
	    {
		iconv_close (fromutf8_cd);
	    }
	    fromutf8_cd = iconv_open (to.c_str(), from.c_str());
	    fromutf8_name = to;
	}
	cd = fromutf8_cd;
    }
    else if ( to == "UTF-8" )
    {
	if (toutf8_cd == (iconv_t)(-1)
	    || toutf8_name != from)
	{
	    if (toutf8_cd != (iconv_t)(-1))
	    {
		iconv_close (toutf8_cd);
	    }
	    toutf8_cd = iconv_open (to.c_str(), from.c_str());
	    toutf8_name = from;
	}
	cd = toutf8_cd;
    }
    else
    {
	if (fromto_cd == (iconv_t)(-1)
	    || from_name != from
	    || to_name != to)
	{
	    if (fromto_cd != (iconv_t)(-1))
	    {
		iconv_close (fromto_cd);
	    }
	    fromto_cd = iconv_open (to.c_str(), from.c_str());
	    from_name = from;
	    to_name   = to;
	}
	cd = fromto_cd;
    }

    if (cd == (iconv_t)(-1))
    {
	static bool complained = false;
	if (!complained)
	{
	    // glibc-locate is not necessarily installed so only complain once
	    y2error ("Recode: (errno %d) failed conversion '%s' to '%s'", errno, from.c_str(), to.c_str());
	    complained = true;
	}
	outstr = instr;
	return 1;
    }

    size_t inbuf_len  = instr.length();
    size_t outbuf_len = inbuf_len * 6 + 1; // worst case

    char * outbuf = recode_buf;
    if (outbuf_len > recode_buf_size)
    {
	outbuf = new char[outbuf_len];
    }

    char * inptr  = (char *) instr.c_str();
    char * outptr = outbuf;
    char * l      = NULL;

    size_t iconv_ret = (size_t)(-1);

    do
    {

#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)
	iconv_ret = iconv (cd, (&inptr), &inbuf_len, &outptr, &outbuf_len);
#else
	iconv_ret = iconv (cd, (const char **) (&inptr), &inbuf_len, &outptr, &outbuf_len);
#endif

	if (iconv_ret == (size_t)(-1))
	{

	    if (errno == EILSEQ)
	    {
		if (l != outptr)
		{
		    *outptr++ = '?';
		    outbuf_len--;
		    l = outptr;
		}
		inptr++;
		continue;
	    }
	    else if (errno == EINVAL)
	    {
		inptr++;
		continue;
	    }
	    else if (errno == E2BIG)
	    {
		if (!outbuf_len)
		{
		    y2internal ( "Recode: unexpected small output buffer" );
		    break;
		}
	    }
	}

    } while (inbuf_len != (size_t)(0));

    *outptr = '\0';
    outstr = outbuf;

    if (outbuf != recode_buf)
    {
	delete [] outbuf;
    }
    return 0;
}


/**
 * @builtin Recode (string from, string to, string text) -> any
 *
 * Recode encoding of string from or to "UTF-8" encoding.
 * One of from/to must be "UTF-8", the other should be an
 * iso encoding specifier (i.e. "ISO-8859-1" for western languages,
 * "ISO-8859-2" for eastern languages, etc.)
 */

YCPValue YUIInterpreter::evaluateRecode (const YCPTerm& term)
{
    if ((term->size() != 3)
	|| ! (term->value(0)->isString())
	|| ! (term->value(1)->isString())
	|| ! (term->value(2)->isString()))
    {
	y2error("Wrong number or type of arguments for Recode (string from, string to, string text)");
	return YCPVoid();
    }

    string outstr;
    if (Recode (term->value(2)->asString()->value(),
		term->value(0)->asString()->value(),
		term->value(1)->asString()->value(),
		outstr) != 0)
    {
	y2error("Bad arguments for Recode");
	return YCPVoid();
    }
    return YCPString (outstr);
}



YRadioButtonGroup *YUIInterpreter::findRadioButtonGroup(YContainerWidget *root, YWidget *widget, bool *contains)
{
    YCPValue root_id = root->id();

#ifdef VERBOSE_FIND_RADIO_BUTTON_GROUP
    y2debug("findRadioButtonGroup(%s, %s)",
	    root_id.isNull() ? "__" : root_id->toString().c_str(),
	    widget->id()->toString().c_str());
#endif

    bool is_rbg = root->isRadioButtonGroup();
    if (widget == root) *contains = true;
    else
    {
	for (int i=0; i<root->numChildren(); i++)
	{
	    if (root->child(i)->isContainer())
	    {
		YRadioButtonGroup *rbg =
		    findRadioButtonGroup(dynamic_cast <YContainerWidget *> ( root->child(i) ), widget, contains);
		if (rbg) return rbg; // Some other lower rbg is it.
	    }
	    else if (root->child(i) == widget) *contains = true;
	}
    }
    if (is_rbg && *contains) return dynamic_cast <YRadioButtonGroup *> (root);
    else return 0;
}



YCPValue YUIInterpreter::returnEvent(const YWidget *widget, EventType et)
{
    return YCPVoid();
}


YWidget *YUIInterpreter::widgetWithId(const YCPValue &id, bool log_error)
{
    if (currentDialog())
    {
	YWidget *widget = currentDialog()->findWidget(id);
	if (widget) return widget;
	if (log_error)
	    y2error("No widget with `" YUISymbol_id "(%s)", id->toString().c_str() );
    }
    else if (log_error)
	y2error("No dialog existing, therefore no widget with `" YUISymbol_id "(%s)",
		id->toString().c_str());

    return 0;
}


void YUIInterpreter::registerDialog(YDialog *dialog)
{
    dialogstack.push_back(dialog);
}


void YUIInterpreter::removeDialog()
{
    delete currentDialog();
    dialogstack.pop_back();
}


YDialog *YUIInterpreter::currentDialog() const
{
    if (dialogstack.size() >= 1) return dialogstack.back();
    else return 0;
}


bool YUIInterpreter::checkId(const YCPValue& v, bool complain) const
{
    if (v->isTerm()
	&& v->asTerm()->size() == 1
	&& v->asTerm()->symbol()->symbol() == YUISymbol_id) return true;
    else
    {
	if ( complain )
	{
	    y2error("Expected `" YUISymbol_id "(any v), you gave me %s", v->toString().c_str());
	}
	return false;
    }
}


YCPValue YUIInterpreter::getId(const YCPValue& v) const
{
    return v->asTerm()->value(0);
}


bool YUIInterpreter::parseRgb(const YCPValue & val, YColor *color, bool complain)
{
    if ( ! color )
    {
	y2error( "Null pointer for color" );
	return false;
    }

    bool ok = val->isTerm();

    if ( ok )
    {
	YCPTerm term = val->asTerm();

	ok = term->size() == 3 && term->symbol()->symbol() == YUISymbol_rgb;

	if ( ok )
	{
	    ok =   term->value(0)->isInteger()
		&& term->value(1)->isInteger()
		&& term->value(2)->isInteger();
	}

	if ( ok )
	{
	    color->red   = term->value(0)->asInteger()->value();
	    color->green = term->value(1)->asInteger()->value();
	    color->blue  = term->value(2)->asInteger()->value();
	}

	if ( ok )
	{
	    if ( color->red      < 0 || color->red   > 255
		 || color->green < 0 || color->green > 255
		 || color->blue  < 0 || color->blue  > 255 )
	    {
		y2error( "RGB value out of range! (0..255)" );
		return false;
	    }
	}
    }


    if ( ! ok && complain )
    {
	y2error("Expected `" YUISymbol_rgb "(integer red, blue, green), you gave me %s",
		val->toString().c_str());
    }

    return ok;
}


YCPValue YUIInterpreter::getWidgetId(const YCPTerm &term, int *argnr)
{
    if (term->size() > 0
	&& term->value(0)->isTerm()
	&& term->value(0)->asTerm()->symbol()->symbol() == YUISymbol_id)
    {
	YCPTerm idterm = term->value(0)->asTerm();
	if (idterm->size() != 1)
	{
	    y2error("Widget id `" YUISymbol_id "() must have exactly one argument. You gave %s",
		    idterm->toString().c_str());
	    return YCPNull();
	}

	YCPValue id = idterm->value(0);
	// unique?
	if (widgetWithId(id))
	{
	    y2error("Widget id %s is not unique", id->toString().c_str());
	    return YCPNull();
	}

	*argnr = 1;
	return id;
    }
    else
    {
	*argnr = 0;
	return YCPVoid();	// no `id() specified -> use "nil"
    }
}


YCPList YUIInterpreter::getWidgetOptions(const YCPTerm &term, int *argnr)
{
    if (term->size() > *argnr
	&& term->value(*argnr)->isTerm()
	&& term->value(*argnr)->asTerm()->symbol()->symbol() == YUISymbol_opt)
    {
	YCPTerm optterm = term->value(*argnr)->asTerm();
	*argnr = *argnr + 1;
	return optterm->args();
    }
    else return YCPList();
}


void YUIInterpreter::logUnknownOption(const YCPTerm &term, const YCPValue &option)
{
    y2warning("Unknown option %s in %s widget",
	      option->toString().c_str(), term->symbol()->symbol().c_str());
}


void YUIInterpreter::rejectAllOptions(const YCPTerm &term, const YCPList &optList)
{
    for (int o=0; o < optList->size(); o++)
    {
	logUnknownOption(term, optList->value(o));
    }
}


void YUIInterpreter::showDialog(YDialog *)
{
    // dummy default implementation
}


void YUIInterpreter::closeDialog(YDialog *)
{
    // dummy default implementation
}


// =============================================================================


/**
 * The true core of the UI interpreter: Recursively create a widget tree from
 * the contents of "term". Most create???() functions that create container
 * widgets will recurse to this function.
 *
 * Note: There is also an overloaded version without YWidgetOpt - see below.
 */

YWidget *YUIInterpreter::createWidgetTree( YWidget *		p,
					   YWidgetOpt &		opt,
					   YRadioButtonGroup *	rbg,
					   const YCPTerm &	term )
{
    // Extract optional widget ID, if present
    int n;
    YCPValue id = getWidgetId(term, &n);


    // Extract optional widget options `opt(`xyz)

    YCPList rawopt = getWidgetOptions(term, &n);

    // Handle generic options

    /**
     * @widget	AAA_All-Widgets
     * @usage	---
     * @examples	none
     * @short	Generic options for all widgets
     * @class	YWidget
     *
     * @option	notify		Make UserInput() return on any action in this widget.
     *				Normally UserInput() returns only when a button is clicked;
     *				with this option on you can make it return for other events, too,
     *				e.g. when the user selects an item in a SelectionBox
     *				(if `opt(`notify) is set for that SelectionBox).
     *				Only widgets with this option set are affected.
     *
     * @option	disabled	Set this widget insensitive, i.e. disable any user interaction.
     *				The widget will show this state by being greyed out
     *				(depending on the specific UI).
     *
     * @option	hstretch	Make this widget stretchable in the horizontal dimension.
     *				<br>See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
     *
     * @option	vstretch	Make this widget stretchable in the vertical   dimension.
     *				<br>See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
     *
     * @option	hstretch	Make this widget stretchable in both dimensions.
     *				<br>See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
     *
     * @option  autoShortcut	Automatically choose a keyboard shortcut for this widget and don't complain
     *				in the log file about the missing shortcut.
     *				<br>Don't use this regularly for all widgets - manually chosen keyboard shortcuts
     *				are almost always better than those automatically assigned. Refer to the style guide
     *				for details.
     *				<br>This option is intended used for automatically generated data, e.g., RadioButtons
     *				for software selections that come from file or from some other data base.
     *
     * @description
     *
     * This is not a widget for general usage, this is just a placeholder for
     * descriptions of options that all widgets have in common.
     * <p>
     * Use them for any widget whenever it makes sense.
     *
     * @example AutoShortcut1.ycp AutoShortcut2.ycp
     */

    YCPList ol;

    for (int o=0; o<rawopt->size(); o++)
    {
	if (rawopt->value(o)->isSymbol())
	{
	    string s = rawopt->value(o)->asSymbol()->symbol();
	    if	    (s == YUIOpt_notify)	opt.notifyMode.setValue(true);
	    else if (s == YUIOpt_disabled)	opt.isDisabled.setValue(true);
	    else if (s == YUIOpt_hstretch)	opt.isHStretchable.setValue(true);
	    else if (s == YUIOpt_vstretch)	opt.isVStretchable.setValue(true);
	    else if (s == YUIOpt_hvstretch)     { opt.isHStretchable.setValue(true); opt.isVStretchable.setValue(true); }
	    else if (s == YUIOpt_autoShortcut)	opt.autoShortcut.setValue(true);
	    else if (s == YUIOpt_easterEgg)	opt.easterEgg.setValue(true);
	    else ol->add(rawopt->value(o));
	}
	else if (!rawopt->value(o)->isTerm())
	{
	    y2warning("Invalid widget option %s. Options must be symbols or terms",
		      rawopt->value(o)->toString().c_str());
	}
	else ol->add(rawopt->value(o));
    }


    //
    // Extract the widget class
    //

    YWidget *w	= 0;
    string   s	= term->symbol()->symbol();

    // Container widgets

    if      (s == YUIWidget_Bottom	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignUnchanged, YAlignEnd	    );
    else if (s == YUIWidget_Frame	)	w = createFrame			(p, opt, term, ol, n, rbg);
    else if (s == YUIWidget_HBox	)	w = createLBox			(p, opt, term, ol, n, rbg, YD_HORIZ);
    else if (s == YUIWidget_HCenter	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignCenter,    YAlignUnchanged );
    else if (s == YUIWidget_HSquash	)	w = createSquash		(p, opt, term, ol, n, rbg, true,  false);
    else if (s == YUIWidget_HVCenter	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignCenter,    YAlignCenter    );
    else if (s == YUIWidget_HVSquash	)	w = createSquash		(p, opt, term, ol, n, rbg, true,  true);
    else if (s == YUIWidget_HWeight	)	w = createWeight		(p, opt, term, ol, n, rbg, YD_HORIZ);
    else if (s == YUIWidget_Left	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignBegin,	    YAlignUnchanged );
    else if (s == YUIWidget_RadioButtonGroup)	w = createRadioButtonGroup	(p, opt, term, ol, n, rbg);
    else if (s == YUIWidget_Right	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignEnd,	    YAlignUnchanged );
    else if (s == YUIWidget_Top		)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignUnchanged, YAlignBegin	    );
    else if (s == YUIWidget_VBox	)	w = createLBox			(p, opt, term, ol, n, rbg, YD_VERT);
    else if (s == YUIWidget_VCenter	)	w = createAlignment		(p, opt, term, ol, n, rbg, YAlignUnchanged, YAlignCenter    );
    else if (s == YUIWidget_VSquash	)	w = createSquash		(p, opt, term, ol, n, rbg, false, true);
    else if (s == YUIWidget_VWeight	)	w = createWeight		(p, opt, term, ol, n, rbg, YD_VERT);
    else if (s == YUIWidget_ReplacePoint)	w = createReplacePoint		(p, opt, term, ol, n, rbg);

    // Leaf widgets

    else if (s == YUIWidget_CheckBox	)	w = createCheckBox		(p, opt, term, ol, n);
    else if (s == YUIWidget_ComboBox	)	w = createComboBox		(p, opt, term, ol, n);
    else if (s == YUIWidget_Empty	)	w = createEmpty			(p, opt, term, ol, n, false, false);
    else if (s == YUIWidget_HSpacing	)	w = createSpacing		(p, opt, term, ol, n, true,  false);
    else if (s == YUIWidget_HStretch	)	w = createEmpty			(p, opt, term, ol, n, true,  false);
    else if (s == YUIWidget_HVStretch	)	w = createEmpty			(p, opt, term, ol, n, true,  true);
    else if (s == YUIWidget_Heading	)	w = createLabel			(p, opt, term, ol, n, true);
    else if (s == YUIWidget_Image	)	w = createImage			(p, opt, term, ol, n);
    else if (s == YUIWidget_IntField	)	w = createIntField		(p, opt, term, ol, n);
    else if (s == YUIWidget_Label	)	w = createLabel			(p, opt, term, ol, n, false);
    else if (s == YUIWidget_LogView	)	w = createLogView		(p, opt, term, ol, n);
    else if (s == YUIWidget_MultiLineEdit)	w = createMultiLineEdit		(p, opt, term, ol, n);
    else if (s == YUIWidget_MultiSelectionBox)	w = createMultiSelectionBox	(p, opt, term, ol, n);
    else if (s == YUIWidget_Password	)	w = createTextEntry		(p, opt, term, ol, n, true);
    else if (s == YUIWidget_ProgressBar	)	w = createProgressBar		(p, opt, term, ol, n);
    else if (s == YUIWidget_PushButton	)	w = createPushButton		(p, opt, term, ol, n);
    else if (s == YUIWidget_MenuButton	)	w = createMenuButton		(p, opt, term, ol, n);
    else if (s == YUIWidget_RadioButton	)	w = createRadioButton		(p, opt, term, ol, n, rbg);
    else if (s == YUIWidget_RichText	)	w = createRichText		(p, opt, term, ol, n);
    else if (s == YUIWidget_SelectionBox)	w = createSelectionBox		(p, opt, term, ol, n);
    else if (s == YUIWidget_Table	)	w = createTable			(p, opt, term, ol, n);
    else if (s == YUIWidget_TextEntry	)	w = createTextEntry		(p, opt, term, ol, n, false);
    else if (s == YUIWidget_Tree	)	w = createTree			(p, opt, term, ol, n);
    else if (s == YUIWidget_VSpacing	)	w = createSpacing		(p, opt, term, ol, n, false, true);
    else if (s == YUIWidget_VStretch	)	w = createEmpty			(p, opt, term, ol, n, false, true);

    // Special widgets - may or may not be supported by the specific UI.
    // The YCP application should ask for presence of such a widget with Has???Widget() prior to creating one.

    else if (s == YUISpecialWidget_DummySpecialWidget)	w = createDummySpecialWidget	(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_DownloadProgress  )	w = createDownloadProgress	(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_BarGraph	)	w = createBarGraph		(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_ColoredLabel	)	w = createColoredLabel		(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_Slider	)	w = createSlider		(p, opt, term, ol, n);
    else if (s == YUISpecialWidget_PartitionSplitter)	w = createPartitionSplitter	(p, opt, term, ol, n);
    else
    {
	y2error("Unknown widget type %s", s.c_str());
	return 0;
    }


    // Post-process the newly created widget

    if (w)
    {
	if ( ! id.isNull()  &&	// ID specified for this widget
	     ! id->isVoid() &&
	     ! w->hasId() )	// widget doesn't have an ID yet
	{
	    w->setId(id);

	    /*
	     * Note: Don't set the ID if it is already set!
	     * This is important for createXy() functions that don't really create
	     * anything immediately but recursively call createWidgetTree()
	     * internally - e.g. createWeight(). In this case, the widget might
	     * already have an ID, so leave it alone.
	     */
	}

	if ( opt.isDisabled.value() )
	{
	    w->setEnabling(false);
	}

	w->setParent(p);
    }

    return w;
}


/**
 * Overloaded version - just for convenience.
 * Most callers don't need to set up the widget options before calling, so this
 * version will pass through an empty set of widget options.
 */

YWidget *YUIInterpreter::createWidgetTree(YWidget *parent, YRadioButtonGroup *rbg, const YCPTerm &term)
{
    YWidgetOpt opt;

    return createWidgetTree(parent, opt, rbg, term);
}



//
// =============================================================================
// High level (abstract libyui layer) widget creation functions.
// Most call a corresponding low level (specific UI) widget creation function.
// =============================================================================
//


/**
 * @widget	ReplacePoint
 * @short	Pseudo widget to replace parts of a dialog
 * @class	YReplacePoint
 * @arg		term child the child widget
 * @usage	`ReplacePoint(`id(`rp), `Empty())
 * @example	ReplacePoint1.ycp
 *
 * @description
 *
 * A ReplacePoint can be used to dynamically change parts of a dialog.
 * It contains one widget. This widget can be replaced by another widget
 * by calling <tt>ReplaceWidget(`id(id), newchild)</tt>, where <tt>id</tt> is the
 * the id of the new child widget of the replace point. The ReplacePoint widget
 * itself has no further effect and no optical representation.
 */

YWidget *YUIInterpreter::createReplacePoint(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					    int argnr, YRadioButtonGroup *rbg)
{
    if ( term->size() != argnr+1 ||
	 term->value( argnr ).isNull() ||
	 term->value( argnr )->isVoid()  )
    {
	y2error("Invalid arguments for the ReplacePoint widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YContainerWidget *replacePoint = createReplacePoint(parent, opt);

    if (replacePoint)
    {
	replacePoint->setParent( parent );
	YWidget *child = createWidgetTree(replacePoint, rbg, term->value(argnr)->asTerm());
	if (child) replacePoint->addChild(child);
	else
	{
	    delete replacePoint;
	    return 0;
	}
    }
    return replacePoint;
}


/**
 * @widgets	Empty HStretch VStretch HVStretch
 * @short	Stretchable space for layout
 * @class	YEmpty
 * @usage	`HStretch()
 * @example	HStretch1.ycp Layout-Buttons-Equal-Even-Spaced1.ycp
 *
 * @description
 *
 * These four widgets denote an empty place in the dialog. They differ
 * in whether they are stretchable or not. <tt>Empty</tt> is not stretchable
 * in either direction. It can be used in a <tt>`ReplacePoint</tt>, when
 * currently no real widget should be displayed. <tt>HStretch</tt> and <tt>VStretch</tt>
 * are stretchable horizontally or vertically resp., <tt>HVStretch</tt> is
 * stretchable in both directions. You can use them to control
 * the layout.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createEmpty(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				     int argnr, bool hstretchable, bool vstretchable)
{
    if (term->size() != argnr)
    {
	y2error("Invalid arguments for the %s widget: %s",
		term->symbol()->symbol().c_str(),term->toString().c_str());
	return 0;
    }
    rejectAllOptions(term,optList);

    if ( hstretchable ) opt.isHStretchable.setValue(true);
    if ( vstretchable ) opt.isVStretchable.setValue(true);

    return createEmpty(parent, opt);
}



/**
 * @widgets	HSpacing VSpacing
 * @short	Fixed size empty space for layout
 * @class	YSpacing
 * @optarg	integer|float size
 * @usage	`HSpacing(0.3)
 * @example	Spacing1.ycp Layout-Buttons-Equal-Even-Spaced2.ycp
 *		Table2.ycp Table3.ycp
 *
 * @description
 *
 * These widgets can be used to create empty space within a dialog to avoid
 * widgets being cramped together - purely for aesthetical reasons. There is no
 * functionality attached.
 * <p>
 * <em>Do not try to use spacings with excessive sizes to create layouts!</em>
 * This is very likely to work for just one UI.	 Use spacings only to separate
 * widgets from each other or from dialog borders. For other purposes, use
 * <tt>`HWeight</tt> and <tt>`VWeight</tt> and describe the dialog logically
 * rather than physically.
 * <p>
 * The <tt>size</tt> given is measured in units roughly equivalent to the size
 * of a character in the respective UI. Fractional numbers can be used here,
 * but text based UIs may choose to round the number as appropriate - even if
 * this means simply ignoring a spacing when its size becomes zero.
 *
 * <p>If <tt>size</tt> is omitted, it defaults to 1.
 * <p><tt>HSpacing</tt> will create a horizontal spacing with default width and zero height.
 * <p><tt>VSpacing</tt> will create a vertical spacing with default height and zero width.
 * <br><p>
 * With options <tt>hstretch</tt> or <tt>vstretch</tt>, the spacing
 * will at least take the amount of space specified with
 * <tt>size</tt>, but it will be stretchable in the respective
 * dimension. Thus,
 * <p><tt>`HSpacing(`opt(`hstretch)</tt>
 * <p>is equivalent to
 * <p>`HBox( `HSpacing(0.5), `HSpacing(0.5) )</tt>
 * <br>
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createSpacing ( YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					 int argnr, bool horizontal, bool vertical)
{
    // Process parameters

    float size	  = 1.0;
    bool	 param_ok = false;

    if ( term->size() == argnr )			// no parameter
    {
	param_ok = true;
    }
    else if ( term->size() == argnr + 1 )	// one parameter
    {
	if ( term->value(argnr)->isInteger() )
	{
	    size	= (float) term->value(argnr)->asInteger()->value();
	    param_ok	= true;
	}
	else if ( term->value(argnr)->isFloat() )
	{
	    size	= term->value(argnr)->asFloat()->value();
	    param_ok	= true;
	}
    }

    if ( ! param_ok )
    {
	y2error("Invalid arguments for the %s widget: %s",
		term->symbol()->symbol().c_str(),term->toString().c_str());
	return 0;
    }


    rejectAllOptions(term,optList);
    return createSpacing(parent, opt, size, horizontal, vertical);
}



/**
 * @widgets	Left Right Top Bottom HCenter VCenter HVCenter
 * @short	Layout alignment
 * @class	YAlignment
 * @arg		term child The contained child widget
 * @optarg	boolean enabled true if ...
 * @usage	`Left(`CheckBox("Crash every five minutes"))
 * @example	HCenter1.ycp HCenter2.ycp HCenter3.ycp Alignment1.ycp
 *
 * @description
 *
 * The Alignment widgets are used to control the layout of a dialog. They are
 * useful in situations, where to a widget is assigned more space than it can
 * use. For example if you have a VBox containing four CheckBoxes, the width of
 * the VBox is determined by the CheckBox with the longest label. The other
 * CheckBoxes are centered per default. With <tt>`Left(widget)</tt> you tell
 * widget that it should be layouted leftmost of the space that is available to
 * it. <tt>Right, Top</tt> and <tt>Bottom</tt> are working accordingly.	 The
 * other three widgets center their child widget horizontally, vertically or in
 * both directions. The important fact for all alignment widgets is, that they
 * make their child widget <b>stretchable</b> in the dimension it is aligned.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createAlignment(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					 int argnr, YRadioButtonGroup *rbg,
					 YAlignmentType halign, YAlignmentType valign)
{
    if (term->size() != argnr+1)
    {
	y2error("%s: The alignment widgets take one widget as argument",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YAlignment *alignment = dynamic_cast <YAlignment *> ( createAlignment(parent, opt, halign, valign) );
    assert(alignment);

    if (alignment)
    {
	alignment->setParent( parent );
	YWidget *child = createWidgetTree(alignment, rbg, term->value(argnr)->asTerm());
	if (child) alignment->addChild(child);
	else
	{
	    delete alignment;
	    return 0;
	}
    }
    return alignment;
}



/**
 * @widgets	Frame
 * @short	Frame with label
 * @class	YFrame
 * @arg		string label title to be displayed on the top left edge
 * @arg		term child the contained child widget
 * @usage	`Frame(`RadioButtonGroup(`id(rb), `VBox(...)));
 * @examples	Frame1.ycp Frame2.ycp TextEntry5.ycp
 *
 * @description
 *
 * This widget draws a frame around its child and displays a title label within
 * the top left edge of that frame. It is used to visually group widgets
 * together. It is very common to use a frame like this around radio button
 * groups.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createFrame ( YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				       const YCPList &optList, int argnr, YRadioButtonGroup *rbg )
{

    int s = term->size() - argnr;
    if (s != 2
	|| ! term->value( argnr )->isString() )
    {
	y2error("Invalid arguments for the Frame widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YCPString label = term->value( argnr++ )->asString();
    YContainerWidget *frame = createFrame( parent, opt, label );

    if ( frame )
    {
	frame->setParent( parent );
	YWidget *child = createWidgetTree( frame, rbg, term->value(argnr)->asTerm() );

	if ( child )
	{
	    frame->addChild( child );
	}
	else
	{
	    delete frame;
	    return 0;
	}
    }

    return frame;
}



/**
 * @widgets	HSquash VSquash HVSquash
 * @short	Layout aid: Minimize widget to its nice size
 * @class	YSquash
 * @arg		term child the child widget
 * @usage	HSquash(`TextEntry("Name:"))
 * @example	HSquash1.ycp
 *
 * @description
 *
 * The Squash widgets are used to control the layout. A <tt>HSquash</tt> widget
 * makes its child widget <b>nonstretchable</b> in the horizontal dimension.
 * A <tt>VSquash</tt> operates vertically, a <tt>HVSquash</tt> in both
 * dimensions.	You can used this for example to reverse the effect of
 * <tt>`Left</tt> making a widget stretchable. If you want to make a VBox
 * containing for left aligned CheckBoxes, but want the VBox itself to be
 * nonstretchable and centered, than you enclose each CheckBox with a
 * <tt>`Left(..)</tt> and the whole VBox with a <tt>HSquash(...)</tt>.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createSquash(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				      int argnr, YRadioButtonGroup *rbg,
				      bool hsquash, bool vsquash)
{
    if (term->size() != argnr+1)
    {
	y2error("%s: The squash widgets take one widget as argument",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YContainerWidget *squash = createSquash(parent, opt, hsquash, vsquash);

    if (squash)
    {
	squash->setParent( parent );
	YWidget *child = createWidgetTree(squash, rbg, term->value(argnr)->asTerm());
	if (child) squash->addChild(child);
	else
	{
	    delete squash;
	    return 0;
	}
    }
    return squash;
}



/**
 * @widgets	HWeight VWeight
 * @short	Control relative size of layouts
 * @class	(YWeight)
 * @arg		integer weight the new weight of the child widget
 * @arg		term child the child widget
 * @usage	`HWeight(2, `SelectionBox("Language"))
 * @examples	Weight1.ycp
 *		Layout-Buttons-Equal-Even-Spaced1.ycp
 *		Layout-Buttons-Equal-Even-Spaced2.ycp
 *		Layout-Buttons-Equal-Growing.ycp
 *		Layout-Mixed.ycp
 *		Layout-Weights1.ycp
 *		Layout-Weights2.ycp
 *
 * @description
 *
 * This widget is used to control the layout. When a <tt>HBox</tt> or
 * <tt>VBox</tt> widget decides how to devide remaining space amount two
 * <b>stretchable</b> widgets, their weights are taken into account. This
 * widget is used to change the weight of the child widget.  Each widget has a
 * vertical and a horizontal weight. You can change on or both of them.	 If you
 * use <tt>HVWeight</tt>, the weight in both dimensions is set to the same
 * value.
 * <p>
 * <b>Note:</b> No real widget is created (any more), just the weight value is
 * passed to the child widget.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createWeight(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				      int argnr, YRadioButtonGroup *rbg, YUIDimension dim)
{
    if (term->size() != argnr + 2
	|| !term->value(argnr)->isInteger()
	|| !term->value(argnr+1)->isTerm())
    {
	y2error("Invalid arguments for the Weight widget: %s",
		term->toString().c_str());
	return 0;
    }
    rejectAllOptions(term,optList);
    long weightValue = (long)(term->value(argnr)->asInteger()->value());

    /**
     * This is an exception from the general rule: No YWeight widget is created,
     * just the weight is passed as widget options to a newly created child widget.
     * The YWeight widget is plain superfluos - YWidget can handle everything itself.
     */

    YWidgetOpt childOpt;

    if (dim == YD_HORIZ)	childOpt.hWeight.setValue(weightValue);
    else			childOpt.vWeight.setValue(weightValue);

    return createWidgetTree(parent, childOpt,rbg, term->value(argnr+1)->asTerm());
}



/**
 * @widgets	HBox VBox
 * @short	Generic layout: Arrange widgets horizontally or vertically
 * @class	(Box)
 * @optarg	term child1 the first child widget
 * @optarg	term child2 the second child widget
 * @optarg	term child3 the third child widget
 * @optarg	term child4 the fourth child widget (and so on...)
 * @option	debugLayout verbose logging
 * @usage	HBox(`PushButton(`id(`ok), "OK"), `PushButton(`id(`cancel), "Cancel"))
 *
 * @examples	VBox1.ycp HBox1.ycp
 *		Layout-Buttons-Equal-Growing.ycp
 *		Layout-Fixed.ycp
 *		Layout-Mixed.ycp
 *
 * @description
 *
 * The layout boxes are used to split up the dialog and layout a number of
 * widgets horizontally (<tt>HBox</tt>) or vertically (<tt>VBox</tt>).
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createLBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
				    int argnr, YRadioButtonGroup *rbg, YUIDimension dim)
{
    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if   (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_debugLayout)	opt.debugLayoutMode.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YContainerWidget *split = createSplit(parent, opt, dim);

    if (split)
    {
	split->setParent(parent);

	for (int w=argnr; w<term->size(); w++)
	{
	    // Create and add the next child widget.

	    if (!term->value(w)->isTerm())
	    {
		y2error("%s: Should be a widget specification",
			term->value(w)->toString().c_str());
		delete split;
		return 0;
	    }

	    YWidget *child = createWidgetTree(split, rbg, term->value(w)->asTerm());

	    if (!child )
	    {
		delete split;
		return 0;
	    }

	    split->addChild(child);
	}
    }
    return split;
}



/**
 * @widgets	Label Heading
 * @short	Simple static text
 * @class	YLabel
 * @arg		string label
 * @option	outputField make the label look like an input field in read-only mode
 * @usage	`Label("Here goes some text\nsecond line")
 *
 * @examples	Label1.ycp Label2.ycp Label3.ycp
 *		Heading1.ycp Heading2.ycp Heading3.ycp
 *
 * @description
 *
 * A <tt>Label</tt> is some text displayed in the dialog. A <tt>Heading</tt> is
 * a text with a font marking it as heading. The text can have more than one
 * line, in which case line feed must be entered.
 */

YWidget *YUIInterpreter::createLabel(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				     const YCPList &optList, int argnr, bool heading)
{
    if (term->size() - argnr != 1
	|| !term->value(argnr)->isString())
    {
	y2error("Invalid arguments for the Label widget: %s",
		term->toString().c_str());
	return 0;
    }


    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_outputField) opt.isOutputField.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    if(heading) opt.isHeading.setValue(true);

    return createLabel(parent, opt, term->value(argnr)->asString());
}



/**
 * @widget	RichText
 * @short	Static text with HTML-like formatting
 * @class	YRichText
 * @arg		string text
 * @option	plainText don't interpret text as HTML
 * @option	shrinkable make the widget very small
 * @usage	`RichText("This is a <b>bold</b> text")
 * @example	RichText1.ycp
 *
 * @description
 *
 * A <tt>RichText</tt> is a text area with two major differences to a
 * <tt>Label</tt>: The amount of data it can contain is not restricted by the
 * layout and a number of control sequences are allowed, which control the
 * layout of the text.
 * <p>
 * Refer to the <a href="../YCP-UI-richtext.html">YaST2 RichText specification</a> for details.
 */

YWidget *YUIInterpreter::createRichText(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if (term->size() - argnr != 1
	|| !term->value(argnr)->isString())
    {
	y2error("Invalid arguments for the Label RichText: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if      (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_plainText ) opt.plainTextMode.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable) opt.isShrinkable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    return createRichText(parent, opt, term->value(argnr)->asString());
}



/**
 * @widget	LogView
 * @short	scrollable log lines like "tail -f"
 * @class	YLogView
 * @arg		string label (above the log lines)
 * @arg		integer visibleLines number of visible lines (without scrolling)
 * @arg		integer maxLines number of log lines to store (use 0 for "all")
 * @usage	`LogView("Log file", 4, 200);
 * @example	LogView1.ycp
 *
 * @description
 *
 * A scrolled output-only text window where ASCII output of any kind can be
 * redirected - very much like a shell window with "tail -f".
 * <p>
 * The LogView will keep up to "maxLines" of output, discarding the oldest
 * lines if there are more. If "maxLines" is set to 0, all lines will be kept.
 * <p>
 * "visibleLines" lines will be visible by default (without scrolling) unless
 * you stretch the widget in the layout.
 * <p>
 * Use <tt>ChangeWidget(`id(`log), `LastLine, "bla blurb...\n")</tt> to append
 * one or several line(s) to the output. Notice the newline at the end of each line!
 * <p>
 * Use <tt>ChangeWidget(`id(`log), `Value, "bla blurb...\n")</tt> to replace
 * the entire contents of the LogView.
 * <p>
 * Use <tt>ChangeWidget(`id(`log), `Value, "")</tt> to clear the contents.
 */

YWidget *YUIInterpreter::createLogView(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if (term->size() - argnr != 3
	|| ! term->value(argnr  )->isString()
	|| ! term->value(argnr+1)->isInteger()
	|| ! term->value(argnr+2)->isInteger())
    {
	y2error("Invalid arguments for the LogView widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);

    YCPString label	= term->value(argnr)->asString();
    int visibleLines	= term->value(argnr+1)->asInteger()->value();
    int maxLines	= term->value(argnr+2)->asInteger()->value();

    return createLogView(parent, opt, label, visibleLines, maxLines);
}



/**
 * @widget	PushButton
 * @short	Perform action on click
 * @class	YPushButton
 * @arg		string label
 * @option	default makes this button the dialogs default button
 * @usage	`PushButton(`id(`click), `opt(`default, `hstretch), "Click me")
 * @examples	PushButton1.ycp PushButton2.ycp
 *
 * @description
 *
 * A <tt>PushButton</tt> is a simple button with a text label the user can
 * press in order to activate some action. If you call <tt>UserInput()</tt> and
 * the user presses the button, <tt>UserInput()</tt> returns with the id of the
 * pressed button.
 * <p>
 * You can (and should) provide keybord shortcuts along with the button
 * label. For example "&Apply" as a button label will allow the user to
 * activate the button with Alt-A, even if it currently doesn't have keyboard
 * focus. This is important for UIs that don't support using a mouse.
 */

YWidget *YUIInterpreter::createPushButton(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if (term->size() - argnr != 1
	|| !term->value(argnr)->isString())
    {
	y2error("Invalid arguments for the PushButton widget: %s",
		term->toString().c_str());
	return 0;
    }

    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_default) opt.isDefaultButton.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }
    return createPushButton(parent, opt, term->value(argnr)->asString());
}


/**
 * @widget	MenuButton
 * @short	Button with popup menu
 * @class	YMenuButton
 * @arg		string		label
 * @arg		itemList	menu items
 * @usage	`MenuButton( "button label", [ `item(`id(`doit), "&Do it"), `item(`id(`something), "&Something") ] );
 * @examples	MenuButton1.ycp MenuButton2.ycp
 *
 * @description
 *
 * This is a widget that looks very much like a <tt>PushButton</tt>, but unlike
 * a <tt>PushButton</tt> it doesn't immediately start some action but opens a
 * popup menu from where the user can select an item that starts an action. Any
 * item may in turn open a submenu etc.
 * <p>
 * <tt>UserInput()</tt> returns the ID of a menu item if one was activated. It
 * will never return the ID of the <tt>MenuButton</tt> itself.
 * <p>
 * <b>Style guide hint:</b> Don't overuse this widget. Use it for dialogs that
 * provide lots of actions. Make the most frequently used actions accessible
 * via normal <tt>PushButtons</tt>. Move less frequently used actions
 * (e.g. "expert" actions) into one or more <tt>MenuButtons</tt>. Don't nest
 * the popup menus too deep - the deeper the nesting, the more awkward the user
 * interface will be.
 * <p>
 * You can (and should) provide keybord shortcuts along with the button
 * label as well as for any menu item.
 */

YWidget *YUIInterpreter::createMenuButton(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if ( term->size() - argnr != 2
	 || ! term->value(argnr  )->isString()
	 || ! term->value(argnr+1)->isList()  )
    {
	y2error("Invalid arguments for the MenuButton widget: "
		"expected \"label\", [ `item(), `item(), ...], not %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);

    YMenuButton *menu_button = dynamic_cast <YMenuButton *>
	( createMenuButton( parent, opt, term->value(argnr)->asString() ) );

    if ( ! menu_button ||
	 parseMenuItemList( term->value( argnr+1 )->asList(), menu_button ) == -1 )
    {
	return 0;
    }

    menu_button->createMenu();	// actually create the specific UI's menu hierarchy

    return menu_button;
}



int
YUIInterpreter::parseMenuItemList( const YCPList &	itemList,
				   YMenuButton *	menu_button,
				   YMenu *		parent_menu )	// 0 if top level
{
    for ( int i=0; i < itemList->size(); i++ )
    {
	YCPValue item = itemList->value(i);

	if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_item )
	{
	    // found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() != 2 ||
		 ! iterm->value( 0 )->isTerm()	||	// `id(...)
		 ! iterm->value( 1 )->isString()  )	// "menu item label"
	    {
		y2error("MenuButton: Invalid menu item - expected `item(`id(...), \"label\"), not %s",
			iterm->toString().c_str() );

		return -1;
	    }


	    // check for item `id() (mandatory)

	    YCPValue item_id = YCPNull();

	    if ( checkId ( iterm->value( 0 ), true ) )
	    {
		item_id = getId ( iterm->value( 0 ) );
	    }
	    else	// no `id()
	    {
		y2error( "MenuButton: Invalid menu item - no `id() specified: %s",
			 item->toString().c_str() );

		return -1;
	    }


	    // extract item label (mandatory) and create the item

	    YCPString item_label = iterm->value( 1 )->asString();
	    menu_button->addMenuItem( item_label, item_id, parent_menu );
	    // y2debug( "Inserted menu entry '%s'", item_label->value().c_str() );
	}
	else if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_menu )
	{
	    // found `menu()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() != 2 ||
		 ! iterm->value( 0 )->isString() &&	// "submenu label"
		 ! iterm->value( 1 )->isList()     )	// [ `item(...), `item(...) ] )
	    {
		y2error( "MenuButton: Invalid submenu specification: "
			 "expected `menu( \"submenu label\", [ `item(), `item(), ...] ), not %s",
			 item->toString().c_str() );

		return -1;
	    }

	    YCPString	sub_menu_label	= iterm->value( 0 )->asString();
	    YMenu *	sub_menu	= menu_button->addSubMenu( sub_menu_label, parent_menu );
	    // y2debug( "Inserted sub menu '%s'", sub_menu_label->value().c_str() );

	    if ( parseMenuItemList( iterm->value( 1 )->asList(), menu_button, sub_menu ) == -1 )
	    {
		return -1;
	    }
	}
	else
	{
	    y2error("MenuButton: Invalid menu item - use either `item() or `menu(), not %s",
		    item->toString().c_str() );
	    return -1;
	}
    }

    return 0;
}



/**
 * @widget	CheckBox
 * @short	Clickable on/off toggle button
 * @class	YCheckBox
 * @arg		string label the text describing the check box
 * @optarg	boolean|nil checked whether the check box should start checked -
 *		nil means tristate condition, i.e. neither on nor off
 * @usage	`CheckBox(`id(`cheese), "&Extra cheese")
 * @examples	CheckBox1.ycp CheckBox2.ycp CheckBox3.ycp
 *
 * @description
 *
 * A checkbox widget has two states: Checked and not checked. It returns no
 * user input but you can query and change its state via the <tt>Value</tt>
 * property.
 */

YWidget *YUIInterpreter::createCheckBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int s = term->size() - argnr;
    if (s < 1 || s > 2
	|| !term->value(argnr)->isString()
	|| (s == 2 && !term->value(argnr+1)->isBoolean()))
    {
	y2error("Invalid arguments for the CheckBox widget: %s",
		term->toString().c_str());
	return 0;
    }
    rejectAllOptions(term,optList);
    YCPBoolean checked(false);
    if (s == 2) checked = term->value(argnr+1)->asBoolean();
    return createCheckBox(parent, opt, term->value(argnr)->asString(), checked->value());
}



/**
 * @widget	RadioButton
 * @short	Clickable on/off toggle button for radio boxes
 * @class	YRadioButton
 * @arg		string label
 * @optarg	boolean selected
 * @usage	`RadioButton(`id(`now), "Crash now", true)
 * @examples	RadioButton1.ycp RadioButton2.ycp Frame2.ycp
 *
 * @description
 *
 * A radio button is not usefull alone. Radio buttons are group such that the
 * user can select one radio button of a group. It is much like a selection
 * box, but radio buttons can be dispersed over the dialog.  Radio buttons must
 * be contained in a <tt>RadioButtonGroup</tt>.
 */

YWidget *YUIInterpreter::createRadioButton(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
					   int argnr, YRadioButtonGroup *rbg)
{

    int s = term->size() - argnr;
    if (s < 1 || s > 2
	|| !term->value(argnr)->isString()
	|| (s == 2 && !term->value(argnr+1)->isBoolean()))
    {
	y2error("Invalid arguments for the RadioButton widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YCPBoolean checked(false);
    if (s == 2) checked = term->value(argnr+1)->asBoolean();

    if (!rbg)
    {
	y2error("%s: must be inside a RadioButtonGroup",
		term->toString().c_str());
	return 0;
    }

    YRadioButton *radioButton = dynamic_cast <YRadioButton *> ( createRadioButton( parent, opt, rbg, term->value(argnr)->asString(), checked->value() ) );
    assert(radioButton);

    // Add to radiobutton group. This can _not_ be done in the
    // constructor of YRadioButton, since the ui specific widget is not yet
    // constructed yet at this stage.

    rbg->addRadioButton(radioButton);

    return radioButton;
}



/**
 * @widget	RadioButtonGroup
 * @short	Radio box - select one of many radio buttons
 * @class	YRadioButtonGroup
 * @arg		term child the child widget
 * @usage	`RadioButtonGroup(`id(rb), `VBox(...))
 * @examples	RadioButton1.ycp Frame2.ycp
 *
 * @description
 *
 * A <tt>RadioButtonGroup</tt> is a container widget that has neither impact on
 * the layout nor has it a graphical representation. It is just used to
 * logically group RadioButtons together so the one-out-of-many selection
 * strategy can be ensured.
 * <p>
 * Radio button groups may be nested.  Looking bottom up we can say that a
 * radio button belongs to the radio button group that is nearest to it. If you
 * give the <tt>RadioButtonGroup</tt> widget an id, you can use it to query and
 * set which radio button is currently selected.
 * <p>
 * See the <a href="../YCP-UI-layout.html">Layout HOWTO</a> for details.
 */

YWidget *YUIInterpreter::createRadioButtonGroup(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList,
						int argnr, YRadioButtonGroup *)
{
    if (term->size() != argnr+1
	|| !term->value(argnr)->isTerm())
    {
	y2error("Invalid arguments for the RadioButtonGroup widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YRadioButtonGroup *rbg = dynamic_cast <YRadioButtonGroup *> ( createRadioButtonGroup(parent, opt) );
    assert(rbg);

    rbg->setParent( parent );
    YWidget *child = createWidgetTree(rbg, rbg, term->value(argnr)->asTerm());

    if (child) rbg->addChild(child);
    else
    {
	delete rbg;
	return 0;
    }

    return rbg;
}



/**
 * @widgets	TextEntry Password
 * @short	Input field
 * @class	YTextEntry
 * @arg		string label the label describing the meaning of the entry
 * @optarg	string defaulttext The text contained in the text entry
 * @option	shrinkable make the input field very small
 * @usage	`TextEntry(`id(`name), "Enter your name:", "Kilroy")
 *
 * @examples	TextEntry1.ycp TextEntry2.ycp TextEntry3.ycp TextEntry4.ycp
 *		TextEntry5.ycp TextEntry6.ycp
 *		Password1.ycp Password2.ycp
 *
 * @description
 *
 * This widget is a one line text entry field with a label above it. An initial
 * text can be provided.
 * <p>
 * <b>Notice</b>: You can and should set a keyboard shortcut within the
 * label. When the user presses the hotkey, the corresponding text entry widget
 * will get the keyboard focus.
 */

YWidget *YUIInterpreter::createTextEntry(YWidget *parent, YWidgetOpt &opt,
					 const YCPTerm &term, const YCPList &optList, int argnr,
					 bool passwordMode)
{

    if (term->size() - argnr < 1 || term->size() - argnr > 2
	|| !term->value(argnr)->isString()
	|| (term->size() == argnr+2 && !term->value(argnr+1)->isString()))
    {
	y2error("Invalid arguments for the %s widget: %s",
		passwordMode ? "Password" : "TextEntry",
		term->toString().c_str());
	return 0;
    }
    YCPString initial_text("");
    if (term->size() >= argnr + 2) initial_text = term->value(argnr+1)->asString();

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable) opt.isShrinkable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    if ( passwordMode ) opt.passwordMode.setValue(true);

    return createTextEntry(parent, opt, term->value(argnr)->asString(), initial_text);
}


/**
 * @widgets	MultiLineEdit
 * @short	multiple line text edit field
 * @class	YMultiLineEdit
 * @arg		string label label above the field
 * @optarg	string initialText the initial contents of the field
 * @usage	`MultiLineEdit(`id(`descr), "Enter problem &description:", "No problem here.")
 *
 * @examples	MultiLineEdit1.ycp MultiLineEdit2.ycp MultiLineEdit3.ycp
 *
 * @description
 *
 * This widget is a multiple line text entry field with a label above it.
 * An initial text can be provided.
 * <p>
 * <b>Notice</b>: You can and should set a keyboard shortcut within the
 * label. When the user presses the hotkey, the corresponding MultiLineEdit
 * widget will get the keyboard focus.
 */

YWidget *YUIInterpreter::createMultiLineEdit(YWidget *parent, YWidgetOpt &opt,
					     const YCPTerm &term, const YCPList &optList, int argnr)
{

    if (term->size() - argnr < 1 || term->size() - argnr > 2
	|| !term->value(argnr)->isString()
	|| (term->size() == argnr+2 && !term->value(argnr+1)->isString()))
    {
	y2error("Invalid arguments for the MultiLineEdit widget: %s",
		term->toString().c_str() );
	return 0;
    }

    YCPString initial_text("");
    if (term->size() >= argnr + 2) initial_text = term->value(argnr+1)->asString();

    rejectAllOptions(term,optList);
    return createMultiLineEdit(parent, opt, term->value(argnr)->asString(), initial_text);
}



/**
 * @widget	SelectionBox
 * @short	Scrollable list selection
 * @class	YSelectionBox
 * @arg		string label
 * @optarg	list items the items contained in the selection box
 * @option	shrinkable make the widget very small
 * @usage	`SelectionBox(`id(`pizza), "select your Pizza:", [ "Margarita", `item(`id(`na), "Napoli") ])
 * @examples	SelectionBox1.ycp SelectionBox2.ycp SelectionBox3.ycp SelectionBox4.ycp
 *
 * @description
 *
 * A selection box offers the user to select an item out of a list. Each item
 * has a label and an optional id. When constructing the list of items, you
 * have two way of specifying an item. Either you give a plain string, in which
 * case the string is used both for the id and the label of the item. Or you
 * specify a term <tt>`item(term id, string label)</tt> or <tt>`item(term id,
 * string label, boolean selected)</tt>, where you give an id of the form
 * <tt>`id(any v)</tt> where you can store an aribtrary value as id. The third
 * argument controls whether the item is the selected item.
 */

YWidget *YUIInterpreter::createSelectionBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int numargs = term->size() - argnr;
    if (numargs < 1 || numargs > 2
	|| !term->value(argnr)->isString()
	|| (numargs == 2 && !term->value(argnr+1)->isList()))
    {
	y2error("Invalid arguments for the SelectionBox widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable) opt.isShrinkable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YSelectionBox *selbox = dynamic_cast <YSelectionBox *> ( createSelectionBox(parent, opt, term->value(argnr)->asString()) );
    assert(selbox);
    if ( numargs == 2)
    {
	YCPList itemlist = term->value(argnr+1)->asList();
	for (int i=0; i<itemlist->size(); i++)
	{
	    YCPValue item = itemlist->value(i);
	    if (item->isString())
	    {
		selbox->addItem(YCPNull(), item->asString(), false);
	    }
	    else if (item->isTerm()
		     && item->asTerm()->symbol()->symbol() == YUISymbol_item)
	    {
		YCPTerm iterm = item->asTerm();
		if (iterm->size() < 1 || iterm->size() > 3)
		{
		    y2error("SelectionBox: Invalid argument number in %s",
			    iterm->toString().c_str());
		}
		else
		{
		    int argnr = checkId(iterm->value(0)) ? 1 : 0;
		    if (iterm->size() <= argnr || !iterm->value(argnr)->isString())
			y2error("SelectionBox: Invalid item arguments in %s",
				iterm->toString().c_str());
		    else
		    {
			YCPValue item_id = YCPNull();
			if (argnr == 1) item_id = getId(iterm->value(0));
			YCPString item_label = iterm->value(argnr)->asString();
			bool item_selected = false;
			if (iterm->size() >= argnr + 2)
			{
			    if (iterm->value(argnr+1)->isBoolean())
				item_selected = iterm->value(argnr+1)->asBoolean()->value();
			    else
			    {
				y2error("SelectionBox: Invalid item arguments in %s",
					iterm->toString().c_str());
			    }
			}

			// UFF! It's made. All arguments checked and gathered :-)
			selbox->addItem(item_id, item_label, item_selected);
		    }
		}
	    }
	    else
	    {
		y2error("Invalid item %s: SelectionBox items must be strings or specified with `"
			YUISymbol_item "()", item->toString().c_str());
	    }
	    // `item
	} // loop over items
    }
    return selbox;
}


/**
 * @widget	MultiSelectionBox
 * @short	Selection box that allows selecton of multiple items
 * @class	YMultiSelectionBox
 * @arg		string	label
 * @arg		list	items	the items initially contained in the selection box
 * @option	shrinkable make the widget very small
 * @usage	`MultiSelectionBox(`id(`topping), "select pizza toppings:", [ "Salami", `item(`id(`cheese), "Cheese", true) ])
 * @examples	MultiSelectionBox1.ycp MultiSelectionBox2.ycp MultiSelectionBox3.ycp
 *
 * @description
 *
 * The MultiSelectionBox displays a (scrollable) list of items from which any
 * number (even nothing!) can be selected. Use the MultiSelectionBox's
 * <tt>SelectedItems</tt> property to find out which.
 * <p>
 * Each item can be specified either as a simple string or as
 * <tt>`item(...)</tt> which includes an (optional) ID and an (optional)
 * 'selected' flag that specifies the initial selected state ('not selected',
 * i.e. 'false', is default).
 */

YWidget *YUIInterpreter::createMultiSelectionBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int term_size = term->size() - argnr;

    if ( term_size != 2
	 || ! term->value( argnr   )->isString()
	 || ! term->value( argnr+1 )->isList() )
    {
	y2error("Invalid arguments for the MultiSelectionBox widget: %s",
		term->toString().c_str());

	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_shrinkable) opt.isShrinkable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YMultiSelectionBox *multi_sel_box = dynamic_cast <YMultiSelectionBox *>
	( createMultiSelectionBox( parent, opt, term->value(argnr)->asString() ) );
    assert( multi_sel_box );


    if ( ! multi_sel_box ||
	 parseMultiSelectionBoxItemList( term->value ( argnr+1 )->asList(), multi_sel_box ) < 0 )
    {
	return 0;
    }

    return multi_sel_box;
}


int
YUIInterpreter::parseMultiSelectionBoxItemList( const YCPList &		item_list,
						YMultiSelectionBox *	multi_sel_box )
{
    multi_sel_box->deleteAllItems();

    for ( int i=0; i < item_list->size(); i++ )
    {
	YCPValue item = item_list->value(i);

	if ( item->isString() )
	{
	    // Simple case: string

	    multi_sel_box->addItem( item->asString() );
	}
	else if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_item )
	{
	    // Found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() < 1 || iterm->size() > 3 )	// `item( `id(...), "label", true )
	    {
		y2error( "MultiSelectionBox: Invalid argument number in %s",
			 iterm->toString().c_str() );

		return -1;
	    }

	    int argnr = 0;


	    // check for item `id() (optional)

	    YCPValue item_id = YCPNull();

	    if ( checkId ( iterm->value( argnr ), false ) )
	    {
		item_id = getId ( iterm->value ( argnr++ ) );
	    }


	    // extract item label (mandatory)

	    if ( iterm->size() <= argnr || ! iterm->value(argnr)->isString() )
	    {
		y2error( "MultiSelectionBox: Expected item label string, not %s",
			 iterm->toString().c_str() );

		return -1;
	    }

	    YCPString item_label = iterm->value( argnr++ )->asString();


	    bool item_selected = false;

	    if ( argnr < iterm->size() )
	    {
		// check for 'selected' flag (true/false) (optional)

		if ( iterm->value( argnr )->isBoolean() )
		{
		    item_selected = iterm->value( argnr++ )->asBoolean()->value();
		}
	    }


	    // Anything left over must be an error.

	    if ( argnr != iterm->size() )
	    {
		y2error("MultiSelectinBox: Wrong number of arguments in %s",
			item->toString().c_str() );

		return -1;
	    }

	    multi_sel_box->addItem( item_label, item_id, item_selected );
	}
	else
	{
	    y2error("MultiSelectionBox: Invalid item - use either a "
		    "simple string or `item(`opt(...), \"label\", true/false), not %s",
		    item->toString().c_str() );
	    return -1;
	}
    }

    return 0;
}


/**
 * @widget	ComboBox
 * @short	drop-down list selection (optionally editable)
 * @class	YComboBox
 * @arg		string label
 * @optarg	list items the items contained in the combo box
 * @option	editable the user can enter any value.
 * @usage	`ComboBox(`id(`pizza), "select your Pizza:", [ "Margarita", `item(`id(`na), "Napoli") ])
 * @examples	ComboBox1.ycp ComboBox2.ycp ComboBox3.ycp ComboBox4.ycp
 *
 * @description
 *
 * A combo box is a combination of a selection box and an input field. It gives
 * the user a one-out-of-many choice from a list of items.  Each item has a
 * (mandatory) label and an (optional) id.  When the 'editable' option is set,
 * the user can also freely enter any value. By default, the user can only
 * select an item already present in the list.
 * <p>
 * The items are very much like SelectionBox items: They can have an (optional)
 * ID, they have a mandatory text to be displayed and an optional boolean
 * parameter indicating the selected state. Only one of the items may have this
 * parameter set to "true"; this will be the default selection on startup.
 * <p>
 * <b>Notice</b>: You can and should set a keyboard shortcut within the
 * label. When the user presses the hotkey, the combo box will get the keyboard
 * focus.
 */

YWidget *YUIInterpreter::createComboBox(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int numargs = term->size() - argnr;
    if (numargs < 1 || numargs > 2
	|| !term->value(argnr)->isString()
	|| (numargs == 2 && !term->value(argnr+1)->isList()))
    {
	y2error("Invalid arguments for the ComboBox widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_editable) opt.isEditable.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YComboBox *cbox = dynamic_cast <YComboBox *> ( createComboBox(parent, opt, term->value(argnr)->asString()) );
    assert(cbox);
    if ( numargs == 2)
    {
	YCPList itemlist = term->value(argnr+1)->asList();
	for (int i=0; i<itemlist->size(); i++)
	{
	    YCPValue item = itemlist->value(i);
	    if (item->isString())
	    {
		cbox->addItem(YCPNull(), item->asString(), false);
	    }
	    else if (item->isTerm()
		     && item->asTerm()->symbol()->symbol() == YUISymbol_item)
	    {
		YCPTerm iterm = item->asTerm();
		if (iterm->size() < 1 || iterm->size() > 3)
		{
		    y2error("ComboBox: Invalid argument number in %s",
			    iterm->toString().c_str());
		}
		else
		{
		    int argnr = checkId(iterm->value(0)) ? 1 : 0;
		    if (iterm->size() <= argnr || !iterm->value(argnr)->isString())
			y2error("ComboBox: Invalid item arguments in %s",
				iterm->toString().c_str());
		    else
		    {
			YCPValue item_id = YCPNull();
			if (argnr == 1) item_id = getId(iterm->value(0));
			YCPString item_label = iterm->value(argnr)->asString();
			bool item_selected = false;
			if (iterm->size() >= argnr + 2)
			{
			    if (iterm->value(argnr+1)->isBoolean())
				item_selected = iterm->value(argnr+1)->asBoolean()->value();
			    else
			    {
				y2error("ComboBox: Invalid item arguments in %s",
					iterm->toString().c_str());
			    }
			}

			// We're all set - all arguments checked and gathered.
			cbox->addItem(item_id, item_label, item_selected);
		    }
		}
	    }
	    else
	    {
		y2error("Invalid item %s: ComboBox items must be strings or specified with `"
			YUISymbol_item "()", item->toString().c_str());
	    }
	    // `item
	} // loop over items
    }
    return cbox;
}



/**
 * @widget	Tree
 * @short	Scrollable tree selection
 * @class	YTree
 * @arg		string		label
 * @optarg	itemList	items	the items contained in the tree <br>
 *		<br>
 *		itemList ::= <br>
 *		<blockquote>
 *			<tt><b>[</b></tt>		<br>
 *			<blockquote>
 *				item			<br>
 *				[ , item ]		<br>
 *				[ , item ]		<br>
 *				...			<br>
 *			</blockquote>
 *			<tt><b>]</b></tt> <br>
 *		</blockquote>
 *		<br>
 *		item ::= <br>
 *		<blockquote>
 *			string |		<br>
 *			<tt><b>`item(</b></tt>	<br>
 *			<blockquote>
 *				[ <tt><b>`id(</b></tt> string <tt><b>),</b></tt> ] <br>
 *				string			<br>
 *				[ , true | false ]	<br>
 *				[ , itemList ]		<br>
 *			</blockquote>
 *			<tt><b>)</b></tt>		<br>
 *		</blockquote>
 *		<br>
 *		The boolean parameter inside `item() indicates whether or not
 *		the respective tree item should be opened by default - if it
 *		has any subitems and if the respective UI is capable of closing
 *		and opening subtrees. If the UI cannot handle this, all
 *		subtrees will always be open.
 *
 * @usage	`Tree(`id(`treeID), "treeLabel", [ "top1", "top2", "top3" ] );
 * @examples	Tree1.ycp Tree2.ycp
 *
 * @description
 *
 * A tree widget provides a selection from a hierarchical tree structure. The
 * semantics are very much like those of a SelectionBox. Unlike the
 * SelectionBox, however, tree items may have subitems that in turn may have
 * subitems etc.
 * <p>
 * Each item has a label string, optionally preceded by an ID. If the item has
 * subitems, they are specified as a list of items after the string.
 * <p>
 * The tree widget will not perform any sorting on its own: The items are
 * always sorted by insertion order. The application needs to handle sorting
 * itself, if desired.
 */

YWidget *YUIInterpreter::createTree (YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int termSize = term->size() - argnr;

    if ( termSize < 1 || termSize > 2
	 || ! term->value( argnr )->isString() )
    {
	y2error("Invalid arguments for the Tree widget: %s",
		term->toString().c_str() );
	return 0;
    }

    rejectAllOptions(term,optList);
    YTree *tree = dynamic_cast <YTree *> ( createTree ( parent, opt, term->value ( argnr )->asString() ) );
    assert(tree);

    if ( tree && termSize > 1 )
    {
	if ( ! term->value( argnr+1 )->isList() )
	{
	    y2error("Expecting tree item list instead of: %s",
		    term->value( argnr+1 )->toString().c_str() );

	    return 0;
	}

	if ( parseTreeItemList ( term->value ( argnr+1 )->asList(), tree ) == -1 )
	    return 0;
    }

    if ( tree )
    {
	tree->rebuildTree();
    }

    return tree;
}


int
YUIInterpreter::parseTreeItemList ( const YCPList &	itemList,
				    YTree *		tree,
				    YTreeItem *		parentItem )
{
    for ( int i=0; i < itemList->size(); i++ )
    {
	YCPValue item = itemList->value(i);

	if ( item->isString() )
	{
	    // The simplest case: just a string, nothing else

	    (void) tree->addItem ( parentItem, YCPNull(), item->asString(), false );
	}
	else if ( item->isTerm() && item->asTerm()->symbol()->symbol() == YUISymbol_item )
	{
	    // found `item()

	    YCPTerm iterm = item->asTerm();

	    if ( iterm->size() < 1 || iterm->size() > 4 )
	    {
		y2error("Tree: Invalid argument number in %s",
			iterm->toString().c_str() );

		return -1;
	    }

	    int argnr = 0;


	    // check for item `id() (optional)

	    YCPValue item_id = YCPNull();

	    if ( checkId ( iterm->value( argnr ), false ) )
	    {
		item_id = getId ( iterm->value ( argnr++ ) );
	    }


	    // extract item label (mandatory)

	    if ( iterm->size() <= argnr || ! iterm->value(argnr)->isString() )
	    {
		y2error("Tree: Invalid item arguments in %s",
			iterm->toString().c_str() );

		return -1;
	    }

	    YCPString item_label = iterm->value( argnr++ )->asString();


	    bool item_open = false;

	    if ( argnr < iterm->size() )
	    {
		// check for 'open' flag (true/false) (optional)

		if ( iterm->value( argnr )->isBoolean() )
		{
		    item_open = iterm->value( argnr++ )->asBoolean()->value();
		}
	    }

	    YTreeItem * treeItem = tree->addItem ( parentItem, item_id, item_label, item_open );

	    if ( argnr < iterm->size() )
	    {
		// check for sub-item list (optional)

		if ( ! iterm->value( argnr )->isList() )
		{
		    y2error("Expecting tree item list instead of: %s",
			    iterm->value( argnr )->toString().c_str() );

		    return -1;
		}

		if ( parseTreeItemList ( iterm->value ( argnr++ )->asList(), tree, treeItem ) == -1 )
		{
		    return -1;
		}
	    }


	    // Anything left over must be an error.

	    if ( argnr != iterm->size() )
	    {
		y2error("Tree: Wrong number of arguments in %s", item->toString().c_str() );
	    }
	}
	else
	{
	    y2error("Invalid item %s: Tree items must be strings or specified with `"
		    YUISymbol_item "()", item->toString().c_str() );
	}
    }

    return 0;
}



/**
 * @widget	Table
 * @short	Multicolumn table widget
 * @class	YTable
 * @arg		term header the headers of the columns
 * @optarg	list items the items contained in the selection box
 * @option	immediate make `notify trigger immediately when the selected item changes
 * @option	keepSorting keep the insertion order - don't let the user sort manually by clicking
 * @usage	`Table(`header("Game", "Highscore"), [ `item(`id(1), "xkobo", "1708") ])
 * @examples	Table1.ycp Table2.ycp Table3.ycp Table4.ycp Table5.ycp
 *
 * @description
 *
 * A Table widget is a generalization of the SelectionBox. Information is
 * displayed in a number of columns. Each column has a header.	The number of
 * columns and their titles are described by the first argument, which is a
 * term with the symbol <tt>header</tt>. For each column you add a string
 * specifying its title. For example <tt>`header("Name", "Price")</tt> creates
 * the two columns "Name" and "Price".
 * <p>
 * The second argument is an optional list of items (rows) that are inserted in
 * the table. Each item has the form <tt>`item(`id(</tt> id <tt>), first
 * column, second column, ...)</tt>. For each column one argument has to be
 * specified, which must be of type void, string or integer. Strings are being
 * left justified, integer right and a nil denote an empty cell, just as the
 * empty string.
 */

YWidget *YUIInterpreter::createTable(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int numargs = term->size() - argnr;
    if (numargs < 1 || numargs > 2
	|| !term->value(argnr)->isTerm()
	|| term->value(argnr)->asTerm()->symbol()->symbol() != YUISymbol_header
	|| (numargs == 2 && !term->value(argnr+1)->isList()))
    {
	y2error("Invalid arguments for the Table widget: %s",
		term->toString().c_str());
	return 0;
    }

    // Parse options

    for (int o=0; o < optList->size(); o++)
    {
	if      (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_immediate  ) opt.immediateMode.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_keepSorting) opt.keepSorting.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    // Build header. The header is a vector of strings, each defining
    // one column. The first string is one of the characters L, R or C
    // denoting left, right or center justification, resp.

    vector<string> header;
    YCPTerm headerterm = term->value(argnr)->asTerm();
    for (int i=0; i< headerterm->size(); i++)
    {
	YCPValue v = headerterm->value(i);
	string this_column;

	if (v->isString())
	    this_column = "L" + v->asString()->value(); // left justified per default

	else if (v->isTerm())
	{
	    YCPTerm t=v->asTerm();
	    if (t->size() != 1 ||
		!t->value(0)->isString())
	    {
		y2error("Invalid Table column specification %s",
			t->toString().c_str());
		return 0;
	    }

	    string s = t->symbol()->symbol();
	    string just;
	    if	 (s == "Left")	 just = "L";
	    else if (s == "Right")	 just = "R";
	    else if (s == "Center") just = "C";
	    else
	    {
		y2error("Invalid Table column specification %s",
			t->toString().c_str());
		return 0;
	    }
	    this_column = just + t->value(0)->asString()->value();
	}
	else
	{
	    y2error("Invalid header declaration in Table widget: %s",
		    headerterm->toString().c_str());
	    return 0;
	}
	header.push_back(this_column);
    }


    // Empty header not allowed!
    if (header.size() == 0)
    {
	y2error("empty header in Table widget not allowed");
	return 0;
    }


    YTable *table = dynamic_cast <YTable *> ( createTable(parent, opt, header) );
    assert(table);

    if (table && numargs == 2) // Fill table with items, if item list is specified
    {
	YCPList itemlist = term->value(argnr+1)->asList();
	table->addItems(itemlist);
    }
    return table;
}



/**
 * @widget	ProgressBar
 * @short	Graphical progress indicator
 * @class	YProgressBar
 * @arg		string label the label describing the bar
 * @optarg	integer maxvalue the maximum value of the bar
 * @optarg	integer progress the current progress value of the bar
 * @usage	`ProgressBar(`id(`pb), "17 of 42 Packages installed", 42, 17)
 * @examples	ProgressBar1.ycp ProgressBar2.ycp
 *
 * @description
 *
 * A progress bar is a horizontal bar with a label that shows a progress
 * value. If you omit the optional parameter <tt>maxvalue</tt>, the maximum
 * value will be 100. If you omit the optional parameter <tt>progress</tt>, the
 * progress bar will set to 0 initially.
 */

YWidget *YUIInterpreter::createProgressBar(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    int s = term->size() - argnr;
    if (s < 1
	|| s > 3
	|| (s >= 1 && !term->value(argnr)->isString())
	|| (s >= 2 && !term->value(argnr+1)->isInteger())
	|| (s >= 3 && !term->value(argnr+2)->isInteger()))
    {
	y2error("Invalid arguments for the ProgressBar widget: %s",
		term->toString().c_str());
	return 0;
    }

    YCPString  label	  = term->value(argnr)->asString();
    YCPInteger maxprogress(100);
    if (s >= 2) maxprogress = term->value(argnr+1)->asInteger();
    YCPInteger progress(0LL);
    if (s >= 3) progress = term->value(argnr+2)->asInteger();

    if (maxprogress->value() < 0
	|| progress->value() < 0
	|| progress->value() > maxprogress->value())
    {
	y2error("Invalid maxprogress/progress value for the ProgressBar widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    return createProgressBar(parent, opt, label, maxprogress, progress);
}



/**
 * @widget	Image
 * @short	Pixmap image
 * @class	YImage
 * @arg		symbol|byteblock|string image specification which image to display
 * @arg		string label label or default text of the image
 * @option	tiled tile pixmap: repeat it as often as needed to fill all available space
 * @option	scaleToFit scale the pixmap so it fits the available space: zoom in or out as needed
 * @option	zeroWidth make widget report a nice width of 0
 * @option	zeroHeight make widget report a nice height of 0
 * @option	animated image data contain an animated image (e.g. MNG)
 * @usage	`Image(`suseheader, "SuSE Linux 7.0")
 * @example	Image1.ycp Image-animated.ycp Image-local.ycp Image-scaled.ycp Image-tiled.ycp
 *
 * @description
 *
 * Displays an image - if the respective UI is capable of that. If not, it is
 * up to the UI to decide whether or not to display the specified default text
 * instead (e.g. with the NCurses UI).
 * <p>
 * The image is specified as any of:
 * <ul>
 * <li>symbol - load a predefined static image. Valid values are:
 *	<ul>
 *	<li><tt>`suseheader</tt> the SuSE standard header image
 *	<li><tt>`yast2</tt> the YaST2 logo
 *	</ul>
 * <li>byteblock - something you read with
 *     <tt>SCR::Read( .target.byte, "image1.png" )</tt>.
 *     This works on any configuration, even remote.
 * <li>string - a complete path name to an image in a supported format.
 *     This is the most convenient method (since you don't need that
 *     <tt>SCR::</tt> call mentioned above, but it has its limitations:
 *     It only works if the UI runs locally, i.e. has access to the local file
 *     system. This can <b>not</b> always be safely assumed.
 * </ul>
 * <p>
 * Use `opt(`zeroWidth) and / or `opt(`zeroHeight) if the real size of the
 * image widget is determined by outside factors, e.g. by the size of
 * neighboring widgets. With those options you can override the default "nice
 * size" of the image widget and make it show just a part of the image.
 * This is used for example in the YaST2 title graphics that are 2000 pixels
 * wide even when only 640 pixels are shown normally. If more screen space is
 * available, more of the image is shown, if not, the layout engine doesn't
 * complain about the image widget not getting its nice size.
 * <p>
 * `opt(`tiled) will make the image repeat endlessly in both dimensions to fill
 * up any available space. You might want to add `opt(`zeroWidth) or
 * `opt(`zeroHeight) (or both), too to make use of this feature.
 * <p>
 * `opt(`scaleToFit) scales the image to fit into the available space, i.e. the
 * image will be zoomed in or out as needed.
 * <p>
 * This option implicitly sets `opt(`zeroWidth) and `opt(zeroHeight),
 * too since there is no useful default size for such an image.
 * <p>
 * Please note that setting both `opt(`tiled) and `opt(`scaleToFit) at once
 * doesn't make any sense.
 */

YWidget *YUIInterpreter::createImage(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term, const YCPList &optList, int argnr)
{
    if ( term->size() - argnr != 2
	|| ( ! term->value( argnr )->isSymbol() &&
	     ! term->value( argnr )->isString() &&
	     ! term->value( argnr )->isByteblock() )
	|| ! term->value( argnr+1 )->isString())
    {
	y2error("Invalid arguments for the Image widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if      (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_zeroWidth	)  opt.zeroWidth.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_zeroHeight	)  opt.zeroHeight.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_animated	)  opt.animated.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_tiled	)  opt.tiled.setValue(true);
	else if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_scaleToFit	)  opt.scaleToFit.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    if ( opt.scaleToFit.value() )
    {
	opt.zeroWidth.setValue( true );
	opt.zeroHeight.setValue( true );
    }

    if ( term->value(argnr)->isByteblock() )
	return createImage( parent, opt, term->value( argnr )->asByteblock(), term->value(argnr+1)->asString() );

    if ( term->value(argnr)->isString() )
	return createImage( parent, opt, term->value( argnr )->asString(), term->value(argnr+1)->asString() );

    ImageType img;
    string symbol = term->value(argnr)->asSymbol()->symbol();
    if	   (symbol == "suseheader") img = IT_SUSEHEADER;
    else if (symbol == "yast2")	    img = IT_YAST2;
    else
    {
	y2error("Unknown predefined image %s", symbol.c_str());
	return 0;
    }

    return createImage(parent, opt, img, term->value(argnr+1)->asString());
}



/*
 * @widget	IntField
 * @short	Numeric limited range input field
 * @class	YIntField
 * @arg		string	label		Explanatory label above the input field
 * @arg		integer minValue	minimum value
 * @arg		integer maxValue	maximum value
 * @arg		integer initialValue	initial value
 * @usage	`IntField("Percentage", 1, 100, 50)
 *
 * @examples	IntField1.ycp IntField2.ycp
 *
 * @description
 *
 * A numeric input field for integer numbers within a limited range.
 * This can be considered a lightweight version of the
 * <a href="YSlider-widget.html">Slider</a> widget, even as a replacement for
 * this when the specific UI doesn't support the Slider.
 * Remember it always makes sense to specify limits for numeric input, even if
 * those limits are very large (e.g. +/- MAXINT).
 * <p>
 * Fractional numbers are currently not supported.
 */

YWidget *YUIInterpreter::createIntField(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 4
	 || ! term->value(argnr)->isString()
	 || ! term->value(argnr+1)->isInteger()
	 || ! term->value(argnr+2)->isInteger()
	 || ! term->value(argnr+3)->isInteger()
	 )
    {
	y2error( "Invalid arguments for the IntField widget: %s",
		 term->toString().c_str() );
	return 0;
    }

    rejectAllOptions(term,optList);

    YCPString label	= term->value(argnr)->asString();
    int minValue		= term->value(argnr+1)->asInteger()->value();
    int maxValue		= term->value(argnr+2)->asInteger()->value();
    int initialValue	= term->value(argnr+3)->asInteger()->value();

    return createIntField(parent, opt, label, minValue, maxValue, initialValue);
}



//
// =============================================================================
// Special widgets that may or may not be supported by a specific UI.
// Remember to overwrite the has...() method as well as the create...() method!
// =============================================================================
//


YWidget *YUIInterpreter::createDummySpecialWidget(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
						  const YCPList &optList, int argnr)
{
    if (term->size() - argnr > 0)
    {
	y2error("Invalid arguments for the DummySpecialWidget widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);

    if ( hasDummySpecialWidget() )
    {
	return createDummySpecialWidget(parent, opt);
    }
    else
    {
	y2error("This UI does not support the DummySpecialWidget.");
	return 0;
    }
}


bool YUIInterpreter::hasDummySpecialWidget()
{
    return true;
}


/**
 * Low level specific UI implementation of DummySpecialWidget,
 * just for demonstration purposes: Creates a heading with a fixed text.
 * Normally, the implementation within the libyui returns 0.
 */

YWidget *YUIInterpreter::createDummySpecialWidget(YWidget *parent, YWidgetOpt &opt)
{
    opt.isHeading.setValue(true);
    opt.isOutputField.setValue(true);
    return createLabel(parent, opt, YCPString("DummySpecialWidget"));
}


// ----------------------------------------------------------------------

/*
 * @widget	BarGraph
 * @short	Horizontal bar graph (optional widget)
 * @class	YBarGraph
 * @arg		list values the initial values (integer numbers)
 * @optarg	list labels the labels for each part; use "%1" to include the
 *		current numeric value. May include newlines.
 * @usage	if ( HasSpecialWidget(`BarGraph) {...
 *		`BarGraph( [450, 100, 700],
 *		[ "Windows used\n%1 MB", "Windows free\n%1 MB", "Linux\n%1 MB" ] )
 *
 * @examples	BarGraph1.ycp BarGraph2.ycp BarGraph3.ycp
 *
 * @description
 *
 * A horizontal bar graph for graphical display of proportions of integer
 * values.  Labels can be passed for each portion; they can include a "%1"
 * placeholder where the current value will be inserted (sformat() -style) and
 * newlines. If no labels are specified, only the values will be
 * displayed. Specify empty labels to suppress this.
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget(`BarGraph)</tt> before using it.
 *
 */

YWidget *YUIInterpreter::createBarGraph(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs < 1 || numArgs > 2
	 || ! term->value(argnr)->isList()
	 || ( numArgs > 1 && ! term->value(argnr+1)->isList() )
	 )
    {
	y2error("Invalid arguments for the BarGraph widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YBarGraph *barGraph;

    if ( hasBarGraph() )
    {
	barGraph = dynamic_cast <YBarGraph *> ( createBarGraph(parent, opt) );
	assert(barGraph);

	barGraph->parseValuesList( term->value( argnr )->asList() );

	if ( numArgs > 1 )
	{
	    barGraph->parseLabelsList( term->value( argnr+1 )->asList() );
	}

	barGraph->doUpdate();
    }
    else
    {
	y2error("This UI does not support the BarGraph widget.");
	return 0;
    }

    return barGraph;
}

// ----------------------------------------------------------------------


/**
 * @widgets	ColoredLabel
 * @short	Simple static text with specified background and foreground color
 * @class	YColoredLabel
 * @arg		string label
 * @arg		color foreground color
 * @arg		color background color
 * @arg		integer margin around the widget in pixels
 * @usage	`ColoredLabel("Hello, World!", `rgb(255, 0, 255), `rgb(0, 128, 0), 20 )
 *
 * @examples	ColoredLabel1.ycp ColoredLabel2.ycp ColoredLabel3.ycp ColoredLabel4.ycp
 *
 * @description
 *
 * Very much the same as a `Label except you specify foreground and background colors and margins.
 * This widget is only available on graphical UIs with at least 15 bit color depth (32767 colors).
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget(`ColoredLabel)</tt> before using it.
 */

YWidget *YUIInterpreter::createColoredLabel(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
					    const YCPList &optList, int argnr)
{
    if (term->size() - argnr != 4
	|| ! term->value( argnr   )->isString()		// label
	|| ! term->value( argnr+1 )->isTerm()		// foreground color
	|| ! term->value( argnr+2 )->isTerm()		// background color
	|| ! term->value( argnr+3 )->isInteger() )	// margin
    {
	y2error( "Invalid arguments for the ColoredLabel widget: %s", term->toString().c_str() );
	return 0;
    }

    rejectAllOptions(term,optList);

    YColor fg;
    YColor bg;
    YCPString label = term->value( argnr )->asString();
    parseRgb( term->value( argnr+1 ), &fg, true );
    parseRgb( term->value( argnr+2 ), &bg, true );
    int margin  = term->value( argnr+3 )->asInteger()->value();

    if ( ! hasColoredLabel() )
    {
	y2error("This UI does not support the ColoredLabel widget.");
	return 0;
    }

    return createColoredLabel( parent, opt, label, fg, bg, margin );
}


// ----------------------------------------------------------------------

/*
 * @widget	DownloadProgress
 * @short	Self-polling file growth progress indicator (optional widget)
 * @class	YDownloadProgress
 * @arg		string label label above the indicator
 * @arg		string filename file name with full path of the file to poll
 * @arg		integer expectedSize expected final size of the file in bytes
 * @usage	if ( HasSpecialWidget(`DownloadProgress) {...
 *		`DownloadProgress("Base system (230k)", "/tmp/aaa_base.rpm", 230*1024);
 *
 * @examples	DownloadProgress1.ycp
 *
 * @description
 *
 * This widget automatically displays the progress of a lengthy download
 * operation. The widget itself (i.e. the UI) polls the specified file and
 * automatically updates the display as required even if the download is taking
 * place in the foreground.
 * <p>
 * Please notice that this will work only if the UI runs on the same machine as
 * the file to download which may not taken for granted (but which is so for
 * most users).
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget(`DownloadProgress)</tt> before using it.
 *
 */

YWidget *YUIInterpreter::createDownloadProgress(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
						const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 3
	 || ! term->value(argnr  )->isString()
	 || ! term->value(argnr+1)->isString()
	 || ! term->value(argnr+2)->isInteger()
	 )
    {
	y2error("Invalid arguments for the DownloadProgress widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YWidget *downloadProgress;

    if ( hasDownloadProgress() )
    {
	YCPString label	 = term->value(argnr)->asString();
	YCPString filename = term->value(argnr+1)->asString();
	int expectedSize	 = term->value(argnr+2)->asInteger()->value();

	downloadProgress = createDownloadProgress(parent, opt, label, filename, expectedSize);
    }
    else
    {
	y2error("This UI does not support the DownloadProgress widget.");
	return 0;
    }

    return downloadProgress;
}



/*
 * @widget	Slider
 * @short	Numeric limited range input (optional widget)
 * @class	YSlider
 * @arg		string	label		Explanatory label above the slider
 * @arg		integer minValue	minimum value
 * @arg		integer maxValue	maximum value
 * @arg		integer initialValue	initial value
 * @usage	if ( HasSpecialWidget(`Slider) {...
 *		`Slider("Percentage", 1, 100, 50)
 *
 * @examples	Slider1.ycp Slider2.ycp ColoredLabel3.ycp
 *
 * @description
 *
 * A horizontal slider with (numeric) input field that allows input of an
 * integer value in a given range. The user can either drag the slider or
 * simply enter a value in the input field.
 * <p>
 * Remember you can use <tt>`opt(`notify)</tt> in order to get instant response
 * when the user changes the value - if this is desired.
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget(`Slider)</tt> before using it.
 *
 */

YWidget *YUIInterpreter::createSlider(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				      const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 4
	 || ! term->value(argnr)->isString()
	 || ! term->value(argnr+1)->isInteger()
	 || ! term->value(argnr+2)->isInteger()
	 || ! term->value(argnr+3)->isInteger()
	 )
    {
	y2error("Invalid arguments for the Slider widget: %s",
		term->toString().c_str());
	return 0;
    }

    rejectAllOptions(term,optList);
    YWidget *slider;

    if ( hasSlider() )
    {
	YCPString label	= term->value(argnr)->asString();
	int minValue	= term->value(argnr+1)->asInteger()->value();
	int maxValue	= term->value(argnr+2)->asInteger()->value();
	int initialValue	= term->value(argnr+3)->asInteger()->value();
	slider = createSlider(parent, opt, label, minValue, maxValue, initialValue);
    }
    else
    {
	y2error("This UI does not support the Slider widget.");
	return 0;
    }

    return slider;
}


/*
 * @widget	PartitionSplitter
 * @short	Hard disk partition splitter tool (optional widget)
 * @class	YPartitionSplitter
 *
 * @arg integer	usedSize		size of the used part of the partition
 * @arg integer	totalFreeSize 		total size of the free part of the partition
 *					(before the split)
 * @arg integer newPartSize		suggested size of the new partition
 * @arg integer minNewPartSize		minimum size of the new partition
 * @arg integer minFreeSize		minimum free size of the old partition
 * @arg string	usedLabel 		BarGraph label for the used part of the old partition
 * @arg string	freeLabel 		BarGraph label for the free part of the old partition
 * @arg string	newPartLabel  		BarGraph label for the new partition
 * @arg string	freeFieldLabel		label for the remaining free space field
 * @arg string	newPartFieldLabel	label for the new size field
 * @usage	if ( HasSpecialWidget(`PartitionSplitter) {...
 *		`PartitionSplitter(600, 1200, 800, 300, 50,
 *                                 "Windows used\n%1 MB", "Windows used\n%1 MB", "Linux\n%1 MB", "Linux (MB)")
 *
 * @examples	PartitionSplitter1.ycp PartitionSplitter2.ycp
 *
 * @description
 *
 * A very specialized widget to allow a user to comfortably split an existing
 * hard disk partition in two parts. Shows a bar graph that displays the used
 * space of the partition, the remaining free space (before the split) of the
 * partition and the space of the new partition (as suggested).
 * Below the bar graph is a slider with an input fields to the left and right
 * where the user can either input the desired remaining free space or the
 * desired size of the new partition or drag the slider to do this.
 * <p>
 * The total size is <tt>usedSize+freeSize</tt>.
 * <p>
 * The user can resize the new partition between <tt>minNewPartSize</tt> and
 * <tt>totalFreeSize-minFreeSize</tt>.
 * <p>
 * <b>Note:</b>
 * This is a "special" widget, i.e. not all UIs necessarily support it.  Check
 * for availability with <tt>HasSpecialWidget(`PartitionSplitter)</tt> before using it.
 * */

YWidget *YUIInterpreter::createPartitionSplitter(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
						 const YCPList &optList, int argnr)
{
    int numArgs = term->size() - argnr;

    if ( numArgs != 10
	 || ! term->value(argnr  )->isInteger()	// usedSize
	 || ! term->value(argnr+1)->isInteger()	// freeSize
	 || ! term->value(argnr+2)->isInteger()	// newPartSize
	 || ! term->value(argnr+3)->isInteger()	// minNewPartSize
	 || ! term->value(argnr+4)->isInteger()	// minFreeSize
	 || ! term->value(argnr+5)->isString()	// usedLabel
	 || ! term->value(argnr+6)->isString()	// freeLabel
	 || ! term->value(argnr+7)->isString()	// newPartLabel
	 || ! term->value(argnr+8)->isString()	// freeFieldLabel
	 || ! term->value(argnr+9)->isString()	// newPartFieldLabel
	 )
    {
	y2error("Invalid arguments for the PartitionSplitter widget: %s",
		term->toString().c_str());
	return 0;
    }

    for (int o=0; o < optList->size(); o++)
    {
	if (optList->value(o)->isSymbol() && optList->value(o)->asSymbol()->symbol() == YUIOpt_countShowDelta) opt.countShowDelta.setValue(true);
	else logUnknownOption(term, optList->value(o));
    }

    YComboBox *cbox = dynamic_cast <YComboBox *> ( createComboBox(parent, opt, term->value(argnr)->asString()) );
    YWidget *splitter;

    if ( hasPartitionSplitter() )
    {
	int 	usedSize		= term->value(argnr  )->asInteger()->value();
	int 	totalFreeSize		= term->value(argnr+1)->asInteger()->value();
	int 	newPartSize		= term->value(argnr+2)->asInteger()->value();
	int 	minNewPartSize		= term->value(argnr+3)->asInteger()->value();
	int 	minFreeSize		= term->value(argnr+4)->asInteger()->value();
	YCPString usedLabel		= term->value(argnr+5)->asString();
	YCPString freeLabel		= term->value(argnr+6)->asString();
	YCPString newPartLabel		= term->value(argnr+7)->asString();
	YCPString freeFieldLabel		= term->value(argnr+8)->asString();
	YCPString newPartFieldLabel	= term->value(argnr+9)->asString();

	splitter = createPartitionSplitter( parent, opt,
					    usedSize, totalFreeSize,
					    newPartSize, minNewPartSize, minFreeSize,
					    usedLabel, freeLabel, newPartLabel,
					    freeFieldLabel, newPartFieldLabel );

    }
    else
    {
	y2error("This UI does not support the PartitionSplitter widget.");
	return 0;
    }

    return splitter;
}


/**
 * Special widget availability check methods.
 *
 * Overwrite if the specific UI provides the corresponding widget.
 */

bool YUIInterpreter::hasDownloadProgress()
{
    return false;
}

bool YUIInterpreter::hasBarGraph()
{
    return false;
}

bool YUIInterpreter::hasColoredLabel()
{
    return false;
}

bool YUIInterpreter::hasSlider()
{
    return false;
}

bool YUIInterpreter::hasPartitionSplitter()
{
    return false;
}


/**
 * Default low level specific UI implementations for optional widgets.
 *
 * UIs that overwrite any of those should overwrite the corresponding
 * has...() method as well!
 */

YWidget *YUIInterpreter::createDownloadProgress(YWidget *parent, YWidgetOpt &opt,
						const YCPString &label,
						const YCPString &filename,
						int expectedSize)
{
    y2error("Default createDownloadProgress() method called - "
	    "forgot to call HasSpecialWidget()?" );

    return 0;
}

YWidget *YUIInterpreter::createBarGraph(YWidget *parent, YWidgetOpt &opt)
{
    y2error("Default createBarGraph() method called - "
	    "forgot to call HasSpecialWidget()?" );

    return 0;
}

YWidget *YUIInterpreter::createColoredLabel(YWidget *parent, YWidgetOpt &opt,
					    YCPString label,
					    YColor fg, YColor bg, int margin )
{
    y2error("Default createColoredLabel() method called - "
	    "forgot to call HasSpecialWidget()?" );

    return 0;
}

YWidget *YUIInterpreter::createSlider(YWidget *parent, YWidgetOpt &opt,
				      const YCPString &label,
				      int minValue, int maxValue, int initialValue)
{
    y2error("Default createSlider() method called - "
	    "forgot to call HasSpecialWidget()?" );

    return 0;
}


YWidget *YUIInterpreter::createPartitionSplitter(YWidget *		parent,
						 YWidgetOpt &		opt,
						 int 			usedSize,
						 int 			totalFreeSize,
						 int 			newPartSize,
						 int 			minNewPartSize,
						 int 			minFreeSize,
						 const YCPString &	usedLabel,
						 const YCPString &	freeLabel,
						 const YCPString &	newPartLabel,
						 const YCPString &	freeFieldLabel,
						 const YCPString &	newPartFieldLabel )
{
    y2error("Default createPartitionSplitter() method called - "
	    "forgot to call HasSpecialWidget()?" );

    return 0;
}




/**
 * @widget EOF marker for make_widget_doc
 * (this might be called a bug of that script, but it is much easier to fix here!)
 */


// EOF
