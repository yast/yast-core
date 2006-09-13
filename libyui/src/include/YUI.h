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

  File:		YUI.h

  Author:	Mathias Kettner <kettner@suse.de>
  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YUI_h
#define YUI_h

#include <pthread.h>
#include <deque>
#include <ycp/YCPString.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPByteblock.h>

#include "YWidget.h"
#include "YAlignment.h"


using std::deque;
using std::string;

class YContainerWidget;
class YEvent;
class YDialog;
class YMacroPlayer;
class YMacroRecorder;
class YMenu;
class YMenuButton;
class YMultiSelectionBox;
class YRadioButtonGroup;
class YTree;
class YTreeItem;
class Y2Component;


typedef struct
{
    int red;
    int green;
    int blue;
} YColor;

struct  YUIBuiltinCallData
{
    void *	function;
    int		argc;
    YCPValue *	argv;
    YCPValue	result;

    YUIBuiltinCallData()
	: result( YCPVoid() )
    {
	function	= 0;
	argc		= 0;
	argv		= 0;
    }
};


/**
 * @short abstract base class of a YaST2 user interface
 * The implementation of a YaST2 user interface such as qt and ncurses
 * constists in subclassing YUI.
 *
 * We have to handle two cases slightly different: The case with and without
 * a seperate UI thread.
 *
 * You have two alternatives how to implement event handling in your UI.
 * Either override @ref #idleLoop, @ref #userInput and @ref #pollInput
 * or override @ref #pollInput and @ref #waitForEvent, whichever is
 * easier for you.
 *
 * This class is an abstract base class that contains pure virtuals.
 * It is not intended for direct instantiation, only for inheritance.
 */
class YUI
{
protected:
    /**
     * Constructor.
     */
    YUI( bool with_threads );


public:

    /**
     * Destructor.
     */
    virtual ~YUI();


    /**
     * Access the global UI.
     **/
    static YUI * ui() { return _yui; }


    /**
     * Looks up the topmost dialog
     */
    YDialog *currentDialog() const;


    /**
     * Block (or unblock) events. If events are blocked, any event sent
     * should be ignored until events are unblocked again.
     *
     * This default implementation keeps track of a simple internal flag that
     * can be queried with eventsBlocked(), so if you reimplement
     * blockEvents(), be sure to reimplement eventsBlocked() as well.
     **/
    virtual void blockEvents( bool block = true ) { _events_blocked = block; }

    /**
     * Unblock events previously blocked. This is just an alias for
     * blockEvents( false) for better readability.
     *
     * Note: This method is intentionally not virtual.
     **/
    void unblockEvents() { blockEvents( false ); }

    /**
     * Returns 'true' if events are currently blocked.
     *
     * Reimplent this if you reimplement blockEvents().
     **/
    virtual bool eventsBlocked() const { return _events_blocked; }

    /**
     * Must be called after the constructor of the Qt/NCurses ui
     * is ready. Starts the ui thread.
     */
    void topmostConstructorHasFinished();

    /**
     * Issue an internal error. Derived UIs should overwrite this to display
     * the error message in a suitable manner, e.g. open a popup (and wait for
     * confirmation!).
     *
     * The default implementation writes the error message to stderr.
     * Notice: This function does _not_ abort the program.
     */
    virtual void internalError( const char *msg );

    /**
     * Might be handy if you have to recode strings from/to utf-8
     */
    static int Recode( const string & str,
		       const string & from,
		       const string & to,
		       string & outstr );

    /**
     * Parse a menu list (for menu buttons)
     */
    int parseMenuItemList( const YCPList & itemList, YMenuButton *menu_button, YMenu *parentMenu = 0 );

    /**
     * Parse an `rgb() value
     **/
    bool parseRgb( const YCPValue & val, YColor *color, bool complain );

    /**
     * Creates a new widget tree.
     *
     * @param parent the widget or dialog this widget is contained in
     * @param term YCPTerm describing the widget
     * @param rbg Pointer to the current radio button group
     *
     * @return pointer to the new widget or 0 if it was not successful.
     * And error has been logged in this case
     */
    YWidget *createWidgetTree( YWidget *parent, YWidgetOpt & opt, YRadioButtonGroup *rbg, const YCPTerm & term );

    /**
     * Overloaded version for convenience. Supplies empty widget options.
     */
    YWidget *createWidgetTree( YWidget *parent, YRadioButtonGroup *rbg, const YCPTerm & term );

    /**
     * Looks up a widget with a certain id. Returns 0 if no
     * such exists. Only the topmost dialog ist searched for the id.
     * @param id id without `id( .. )
     * @param log_error set to true if you want me to log an error if
     * the id is not existing.
     */

    YWidget *widgetWithId( const YCPValue & id, bool log_error=false );

    /**
     * Overloaded version of widgetWithId. Does not search for the widget id in the
     * topmost dialog but in given widget tree.
     * @param widgetRoot root of the widget tree
     * @param id id without `id( .. )
     * @param log_error set to true if you want me to log an error if
     * the id is not existing.
     */
    YWidget *widgetWithId( YContainerWidget *widgetRoot, const YCPValue & id, bool log_error=false );

    /**
     * Returns the default function key number for a widget with the specified
     * label or 0 if there is none. Any keyboard shortcuts that may be
     * contained in 'label' are stripped away before any comparison.
     **/
    int defaultFunctionKey( YCPString label );

    /**
     * Returns the current product name
     * ("SuSE Linux", "SuSE Linux Enterprise Server", "United Linux", etc.).
     *
     * This can be set with the UI::SetProductName() builtin.
     * UI::GetProductName is the YCP equivalent to this function.
     **/
    string productName() const { return _productName; }

    /**
     * Convert logical layout spacing units into device dependent units.
     * A default size dialog is assumed to be 80x25 layout spacing units.
     *
     * Derived UI may want to reimplement this method.
     **/
    virtual long deviceUnits( YUIDimension dim, float layout_units );

    /**
     * Convert device dependent units into logical layout spacing units.
     * A default size dialog is assumed to be 80x25 layout spacing units.
     *
     * Derived UI may want to reimplement this method.
     **/
    virtual float layoutUnits( YUIDimension dim, long device_units );

    /**
     * Returns 'true' if widget geometry should be reversed for languages that
     * have right-to-left writing direction (Arabic, Hebrew).
     **/
    static bool reverseLayout() { return _reverseLayout; }

    /**
     * Set reverse layout for Arabic / Hebrew support
     **/
    static void setReverseLayout( bool rev ) { _reverseLayout = rev; }

    /**
     * Call a UI builtin function in the correct thread (the UI thread).
     * This is called from libycp/YExpression via the UI builtin declarations
     * that call UICallHandler.
     **/
    YCPValue callBuiltin( void * function, int argc, YCPValue argv[] );

    /**
     * Call 'function' with 'argc' YCPValue parameters and return the result of
     * 'function'.
     **/
    static YCPValue callFunction( void * function, int argc, YCPValue argv[] );

    /**
     * Set a callback component.
     **/
    void setCallback( Y2Component * callback ) { _callback = callback; }

    /**
     * Returns the callback previously set with setCallback().
     **/
    Y2Component * getCallback() const { return _callback; }

    /**
     * Implementations for most UI builtins.
     * Each method corresponds directly to one UI builtin.
     **/
    YCPValue evaluateAskForExistingDirectory		( const YCPString& startDir, const YCPString& headline );
    YCPValue evaluateAskForExistingFile			( const YCPString& startDir, const YCPString& filter, const YCPString& headline );
    YCPValue evaluateAskForSaveFileName			( const YCPString& startDir, const YCPString& filter, const YCPString& headline );
    void evaluateBusyCursor				();
    void evaluateBeep     				();
    YCPValue evaluateChangeWidget			( const YCPValue & value_id, const YCPValue & property, const YCPValue & new_value );
    void evaluateCheckShortcuts				();
    YCPValue evaluateCollectUserInput			();
    YCPValue evaluateCollectUserInput			( const YCPTerm & widgetId );
    YCPValue evaluateCloseDialog			();
    void evaluateDumpWidgetTree				();
    void evaluateFakeUserInput				( const YCPValue & next_input );
    YCPMap evaluateGetDisplayInfo			();
    YCPString evaluateGetLanguage			( const YCPBoolean & strip_encoding );
    YCPValue evaluateGetModulename			( const YCPTerm & term );
    YCPString evaluateGetProductName			();
    YCPString evaluateGlyph				( const YCPSymbol & symbol );
    YCPValue evaluateHasSpecialWidget			( const YCPSymbol & widget );
    void evaluateMakeScreenShot				( const YCPString & filename );
    void evaluateNormalCursor				();
    YCPBoolean evaluateOpenDialog			( const YCPTerm & term, const YCPTerm & opts = YCPNull() );
    void evaluatePlayMacro				( const YCPString & filename );
    void evaluatePostponeShortcutCheck			();
    YCPValue evaluateQueryWidget			( const YCPValue& value_id, const YCPValue& property );
    void evaluateRecalcLayout				();
    YCPValue evaluateRecode				( const YCPString & from, const YCPString & to, const YCPString & text );
    void evaluateRecordMacro				( const YCPString & filename );
    void evaluateRedrawScreen				();
    YCPBoolean evaluateReplaceWidget			( const YCPValue & value_id, const YCPTerm & term );
    YCPValue evaluateRunPkgSelection			( const YCPValue & value_id );
    void evaluateSetConsoleFont				( const YCPString& magic,
							  const YCPString& font,
							  const YCPString& screen_map,
							  const YCPString& unicode_map,
							  const YCPString& encoding );
    void evaluateSetKeyboard				();
    YCPBoolean evaluateSetFocus				( const YCPValue & value_id );
    void evaluateSetFunctionKeys			( const YCPMap & new_keys );
    void evaluateSetLanguage				( const YCPString& lang, const YCPString& encoding = YCPNull() );
    void evaluateSetModulename				( const YCPString & name );
    void evaluateSetProductName				( const YCPString & name );
    void evaluateStopRecordMacro			();
    YCPBoolean evaluateWidgetExists			( const YCPValue & value_id );

    YCPValue evaluateUserInput				();
    YCPValue evaluateTimeoutUserInput			( const YCPInteger & timeout );
    YCPValue evaluateWaitForEvent			( const YCPInteger & timeout = YCPNull() );
    YCPValue evaluateWizardCommand			( const YCPTerm & command );
    YCPValue evaluatePollInput				();

    /**
     * This method implements the UI thread in case it is existing.
     * The loop consists of calling @ref #idleLoop, getting the next
     * command from the @ref YCPUIComponent, evaluating it, which
     * possibly invovles calling @ref #userInput() or @ref #pollInput()
     * and writes the answer back to the other thread where the request
     * came from.
     */
    void uiThreadMainLoop();

    /**
     * Checks if the given value is a term with the symbol 'id and
     * size one. Logs an error if this is not so and 'complain' is set.
     *
     * @return 'true' if 'val' is a valid `id().
     */
    static bool checkId( const YCPValue & val, bool complain = true );

    /**
     * Assumes that the value v is of the form `id( any i ) and returns
     * the contained i.
     */
    static YCPValue getId( const YCPValue & v );


protected:




    const char *moduleName();

    // Event handling, execution flow

    /**
     * This virtual method is called when threads are activated in case
     * the execution control is currently on the side of the module.
     * This means that no UserInput() or PollInput() is pending. The module
     * just does some work. The UI <-> module protocol is in the state <i>
     * UI waits for the next command </i>. The UI can override this method
     * when it wants to react to user input or other external events such
     * as repaint requests from the X server.
     *
     * @param fd_ycp filedescriptor that should be used to determine when
     * to leave the idle loop. As soon as it is readable, the loop must
     * be left. In order to avoid polling you can combine it with other
     * ui-specific fds and do common <tt>select</tt> call.
     */
    virtual void idleLoop( int fd_ycp );

    /**
     * UI-specific UserInput() method.
     *
     * This is called upon evaluating the UI::UserInput() builtin command.
     * This method should remain in its own event loop until an event for the
     * topmost dialog (currentDialog() ) is  available or until the specified
     * timeout (in milliseconds, 0 for "wait forever") is expired (in which
     * case it should return the pointer to a YTimeoutEvent).
     *
     * This method is to return a pointer to an event created with the "new"
     * operator. The generic UI component assumes ownership of this newly
     * created object and destroys it when appropriate.
     *
     * The caller will gracefully handle if this method returns 0, albeit this
     * is always an error which may result in an error message in the log file.
     *
     * Derived UIs are required to implement this method.
     **/
    virtual YEvent * userInput( unsigned long timeout_millisec = 0 ) = 0;

    /**
     * UI-specific PollInput() method.
     *
     * This is called upon evaluating the UI::PollInput() builtin command.
     * This method should just check if there are any pending events for the
     * topmost dialog (currentDialog() ). This method never waits for any
     * events - if there is no pending event, it returns 0.
     *
     * If there is a pending event, a pointer to that event (newly created with
     * the "new" operator) is returned. The generic UI component assumes
     * ownership of this newly created object and destroys it when appropriate.
     *
     * Derived UIs are required to implement this method.
     **/
    virtual YEvent * pollInput() = 0;

    /**
     * Shows and activates a previously created dialog. The default implementation does nothing.
     * @param dialog dialog to show.
     */
    virtual void showDialog( YDialog *dialog );

    /**
     * Decativates and closes a previously created dialog. The default implementation does nothing.
     * Don't delete the dialog. This will be done at some other place.
     * @param dialog dialog to close.
     */
    virtual void closeDialog( YDialog *dialog );


    /**
     * Implement the 'Glyph()' builtin in the specific UI:
     *
     * Return a representation for the glyph symbol specified in UTF-8 encoding
     * or an empty string to get a default textual representation.
     *
     * Derived UIs may or may not choose to overwrite this.
     */
    virtual YCPString glyph( const YCPSymbol & glyphSymbol ) { return YCPString( "" ); }


    //
    // Widget creation functions - container widgets
    //

    /**
     * Creates a dialog.
     * @param widget Widget the dialog contains. Each dialog contains exactly one widget.
     */
    virtual YDialog *createDialog( YWidgetOpt & opt ) = 0;

    /**
     * Creates a split
     */
    virtual YContainerWidget *createSplit( YWidget *parent, YWidgetOpt & opt, YUIDimension dimension ) = 0;

    /**
     * Creates a replace point.
     */
    virtual YContainerWidget *createReplacePoint( YWidget *parent, YWidgetOpt & opt ) = 0;

    /**
     * Creates an alignment widget
     */
    virtual YContainerWidget *createAlignment( YWidget *parent, YWidgetOpt & opt,
					       YAlignmentType halign,
					       YAlignmentType valign ) = 0;

    /**
     * Creates a squash widget
     */
    virtual YContainerWidget *createSquash( YWidget *parent, YWidgetOpt & opt, bool hsquash, bool vsquash ) = 0;


    /**
     * Creates a radio button group.
     */
    virtual YContainerWidget *createRadioButtonGroup( YWidget *parent, YWidgetOpt & opt ) = 0;

    /**
     * Creates a frame widget
     */
    virtual YContainerWidget *createFrame( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;



    //
    // Widget creation functions - leaf widgets
    //

    /**
     * Creates an empty widget
     */
    virtual YWidget *createEmpty( YWidget *parent, YWidgetOpt & opt ) = 0;

    /**
     * Creates a spacing widget
     */
    virtual YWidget *createSpacing( YWidget *parent, YWidgetOpt & opt, float size, bool horizontal, bool vertical ) = 0;

    /**
     * Creates a label.
     * @param text Initial text of the label
     * @param heading true if the label is a Heading()
     * @param output_field true if the label should look like an output field( 3D look )
     */
    virtual YWidget *createLabel( YWidget *parent, YWidgetOpt & opt, const YCPString & text ) = 0;


    /**
     * Creates a rich text widget
     * @param text Initial text of the label
     */
    virtual YWidget *createRichText( YWidget *parent, YWidgetOpt & opt, const YCPString & text ) = 0;


    /**
     * Creates a log view widget
     * @param label label above the log view
     * @param visibleLines default number of vislible lines
     * @param maxLines number of lines to store (use 0 for "all")
     */
    virtual YWidget *createLogView( YWidget *parent, YWidgetOpt & opt,
				    const YCPString & label, int visibleLines, int maxLines ) = 0;

    /**
     * Creates a push button.
     * @param label Label of the button
     */
    virtual YWidget *createPushButton( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;

    /**
     * Creates a menu button.
     * @param label Label of the button
     */
    virtual YWidget *createMenuButton( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;

    /**
     * Creates a radio button and inserts it into a radio button group
     * @param label Label of the radio button
     * @param rbg the radio button group the new button will belong to
     */
    virtual YWidget *createRadioButton( YWidget *parent, YWidgetOpt & opt, YRadioButtonGroup *rbg,
					const YCPString & label, bool checked ) = 0;

    /**
     * Creates a check box
     * @param label Label of the checkbox
     * @param true if it is checked
     */
    virtual YWidget *createCheckBox( YWidget *parent, YWidgetOpt & opt, const YCPString & label, bool checked ) = 0;

    /**
     * Creates a text entry or password entry field.
     */
    virtual YWidget *createTextEntry( YWidget *parent, YWidgetOpt & opt, const YCPString & label, const YCPString & text ) = 0;

    /**
     * Creates a MultiLineEdit widget.
     */
    virtual YWidget *createMultiLineEdit( YWidget *parent, YWidgetOpt & opt, const YCPString & label, const YCPString & text ) = 0;

    /**
     * Creates a selection box
     */
    virtual YWidget *createSelectionBox( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;

    /**
     * Creates a multi selection box
     */
    virtual YWidget *createMultiSelectionBox( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;

    /**
     * Creates a combo box
     */
    virtual YWidget *createComboBox( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;

    /**
     * Creates a tree
     */
    virtual YWidget *createTree( YWidget *parent, YWidgetOpt & opt, const YCPString & label ) = 0;

    /**
     * Creates a table widget
     */
    virtual YWidget *createTable( YWidget *parent, YWidgetOpt & opt, vector<string> header ) = 0;

    /**
     * Creates a progress bar
     */
    virtual YWidget *createProgressBar( YWidget *parent, YWidgetOpt & opt, const YCPString & label,
					const YCPInteger & maxprogress, const YCPInteger & progress ) = 0;

    /**
     * Creates an image widget from a YCP byteblock
     */
    virtual YWidget *createImage( YWidget *parent, YWidgetOpt & opt, YCPByteblock imagedata, YCPString defaulttext ) = 0;

    /**
     * Creates an image widget from a file name
     */
    virtual YWidget *createImage( YWidget *parent, YWidgetOpt & opt, YCPString file_name, YCPString defaulttext ) = 0;

    /**
     * Creates an IntField widget.
     */
    virtual YWidget *createIntField( YWidget *parent, YWidgetOpt & opt,
				     const YCPString & label, int minValue, int maxValue, int initialValue ) = 0;

    /**
     * Creates a PackageSelector widget.
     *
     * "floppyDevice" may be an empty string if no such device was specified in the YCP code.
     * Usually it is something like "/dev/fd0".
     */
    virtual YWidget *createPackageSelector( YWidget *parent, YWidgetOpt & opt, const YCPString & floppyDevice ) = 0;

    /**
     * Creates a PkgSpecial widget, i.e. a specific subwidget.
     */
    virtual YWidget *createPkgSpecial( YWidget *parent, YWidgetOpt & opt, const YCPString & subwidget ) = 0;


    /**
     * Creates a DummySpecialWidget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *createDummySpecialWidget( YWidget *parent, YWidgetOpt & opt );
    virtual bool     hasDummySpecialWidget() { return true; }

    /**
     * Creates a DownloadProgress widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createDownloadProgress( YWidget *parent, YWidgetOpt & opt,
						const YCPString & label,
						const YCPString & filename,
						int expectedSize );

    virtual bool	hasDownloadProgress() { return false; }

    /**
     * Creates a BarGraph widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createBarGraph( YWidget *parent, YWidgetOpt & opt );
    virtual bool	hasBarGraph()  { return false; }

    /**
     * Creates a ColoredLabelwidget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createColoredLabel( YWidget *parent, YWidgetOpt & opt,
					    YCPString label,
					    YColor foreground, YColor background,
					    int margin );
    virtual bool	hasColoredLabel() { return false; }


    /**
     * Creates a Date input filed
     */
    virtual YWidget *	createDate( YWidget *parent,
				    YWidgetOpt & opt,
				    const YCPString & label,
				    const YCPString & date );
    virtual bool	hasDate() { return false; }


    /**
     * Creates a Time input filed
     */
    virtual YWidget *	createTime( YWidget *parent,
				    YWidgetOpt & opt,
				    const YCPString & label,
				    const YCPString & time );
    virtual bool	hasTime() { return false; }


    /**
     * Creates a DumbTab.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createDumbTab( YWidget *parent, YWidgetOpt & opt );
    virtual bool	hasDumbTab() { return false; }


    /**
     * Creates a (horizontal or vertical) MultiProgressMeter.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createMultiProgressMeter( YWidget *parent, YWidgetOpt & opt,
						  bool horizontal, const YCPList & maxValues );
    virtual bool	hasMultiProgressMeter() { return false; }


    /**
     * Creates a Slider widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createSlider( YWidget *		parent,
				      YWidgetOpt &	opt,
				      const YCPString &	label,
				      int		minValue,
				      int		maxValue,
				      int		initialValue );
    virtual bool	hasSlider() { return false; }

    /**
     * Creates a PartitionSplitter widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     *
     * @param parent		the parent widget
     * @param opt		the widget options
     * @param usedSize		size of the used part of the partition
     * @param totalFreeSize	total size of the free part of the partition ( before the split )
     * @param newPartSize	suggested size of the new partition
     * @param minNewPartSize	minimum size of the new partition
     * @param minFreeSize	minimum remaining free size of the old partition
     * @param usedLabel		BarGraph label for the used part of the old partition
     * @param freeLabel		BarGraph label for the free part of the old partition
     * @param newPartLabel	BarGraph label for the new partition
     * @param freeFieldLabel	label for the remaining free space field
     * @param newPartFieldLabel label for the new partition size field
     */

    virtual YWidget *createPartitionSplitter( YWidget *		parent,
					      YWidgetOpt &	opt,
					      int		usedSize,
					      int		totalFreeSize,
					      int		newPartSize,
					      int		minNewPartSize,
					      int		minFreeSize,
					      const YCPString & usedLabel,
					      const YCPString & freeLabel,
					      const YCPString & newPartLabel,
					      const YCPString & freeFieldLabel,
					      const YCPString & newPartFieldLabel );

    virtual bool	hasPartitionSplitter()  { return false; }

    /**
     * Creates a pattern selector.
     **/
    virtual YWidget *createPatternSelector( YWidget *parent, YWidgetOpt & opt );

    virtual bool	hasPatternSelector() { return false; }


    /**
     * Creates a Wizard frame.
     */
    virtual YWidget *createWizard( YWidget *parent, YWidgetOpt & opt,
				   const YCPValue & backButtonId,	const YCPString & backButtonLabel,
				   const YCPValue & abortButtonId,	const YCPString & abortButtonLabel,
				   const YCPValue & nextButtonId,	const YCPString & nextButtonLabel  );

    virtual bool	hasWizard() { return false; }


    /**
     * UI-specific setLanguage() function.
     * Returns YCPVoid() if OK and YCPNull() on error.
     * This default implementation does nothing.
     */
    virtual YCPValue setLanguage( const YCPTerm & term );


    /**
     * UI-specific setConsoleFont() function.
     * Returns YCPVoid() if OK and YCPNull() on error.
     * This default implementation does nothing.
     */
    virtual YCPValue setConsoleFont( const YCPString & console_magic,
				     const YCPString & font,
				     const YCPString & screen_map,
				     const YCPString & unicode_map,
				     const YCPString & encoding );

    virtual YCPValue setKeyboard();

    /**
     * UI-specific getDisplayInfo() functions.
     * See UI builtin GetDisplayInfo() doc for details.
     **/
    virtual int	 getDisplayWidth()		{ return -1; }
    virtual int	 getDisplayHeight()		{ return -1; }
    virtual int	 getDisplayDepth()		{ return -1; }
    virtual long getDisplayColors()		{ return -1; }
    virtual int	 getDefaultWidth()		{ return -1; }
    virtual int	 getDefaultHeight()		{ return -1; }
    virtual bool textMode()			{ return true; }
    virtual bool hasImageSupport()		{ return false; }
    virtual bool hasLocalImageSupport()		{ return true;	}
    virtual bool hasAnimationSupport()		{ return false; }
    virtual bool hasIconSupport()		{ return false; }
    virtual bool hasFullUtf8Support()		{ return false; }
    virtual bool richTextSupportsTable()	{ return false; }
    virtual bool leftHandedMouse()		{ return false; }

    /**
     * UI-specific busyCursor function.
     * This default implementation does nothing.
     */
    virtual void busyCursor();

    /**
     * UI-specific normalCursor function.
     * This default implementation does nothing.
     */
    virtual void normalCursor();

    /**
     * UI-specific redrawScreen method.
     * This default implementation does nothing.
     */
    virtual void redrawScreen();

    /**
     * UI-specific makeScreenShot function.
     * This default implementation does nothing.
     */
    virtual void makeScreenShot( string filename );

    /**
     * UI-specific beep method.
     *
     * Emit acoustic signal or something equivalent.
     * This default implementation does nothing.
     */
    virtual void beep();

    /**
     * UI-specific runPkgSelection method.
     * This default implementation does nothing.
     * Use this to post-initialize widget stuff that cannot be done in the
     * createPackageSelector() method.
     **/
    virtual YCPValue runPkgSelection( YWidget * packageSelector ) { return YCPVoid(); }


    /**
     * UI-specific implementation of the AskForExistingDirectory() builtin.
     *
     * Open a directory selection box and prompt the user for an existing directory.
     *
     * 'startDir' is the initial directory that is displayed.
     *
     * 'headline' is an explanatory text for the directory selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected directory name
     * or 'nil'( YCPVoid() ) if the user canceled the operation.
     **/
    virtual YCPValue askForExistingDirectory( const YCPString & startDir,
					      const YCPString & headline ) = 0;

    /**
     * UI-specific implementation of the AskForExistingFile() builtin.
     *
     * Open a file selection box and prompt the user for an existing file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g. "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or 'nil'( YCPVoid() ) if the user canceled the operation.
     **/
    virtual YCPValue askForExistingFile( const YCPString & startWith,
					 const YCPString & filter,
					 const YCPString & headline ) = 0;

    /**
     * UI-specific implementation of the AskForSaveFileName() builtin.
     *
     * Open a file selection box and prompt the user for a file to save data to.
     * Automatically asks for confirmation if the user selects an existing file.
     *
     * 'startWith' is the initial directory or file.
     *
     * 'filter' is one or more blank-separated file patterns, e.g. "*.png *.jpg"
     *
     * 'headline' is an explanatory text for the file selection box.
     * Graphical UIs may omit that if no window manager is running.
     *
     * Returns the selected file name
     * or 'nil'( YCPVoid() ) if the user canceled the operation.
     **/
    virtual YCPValue askForSaveFileName( const YCPString & startWith,
					 const YCPString & filter,
					 const YCPString & headline ) = 0;



    YCPValue callback		( const YCPValue & value );

    /**
     * Evaluates a locale. Evaluate _( "string" ) to "string".
     */
    YCPValue evaluateLocale( const YCPLocale & );

    /**
     * Start macro recording to file "filename".
     * Any previous active macro recorder will be terminated( regularly ) prior
     * to this.
     */
    void recordMacro( string filename );

    /**
     * Stop macro recording if this is in progress.
     * Nothing happens when no macro recording is in progress.
     */
    void stopRecordMacro();

    /**
     * Play macro in file "filename".
     * Any previous macro execution will be terminated prior to this.
     */
    void playMacro( string filename );

    /**
     * Return whether macro recording is in progress or not
     */
    bool recordingMacro()	{ return macroRecorder != 0;	}

    /**
     * Return whether macro playing is in progress or not
     */
    bool playingMacro()		{ return macroPlayer != 0;	}


protected:
    /**
     * Tells the ui thread that it should terminate and waits
     * until it does so.
     */
    void terminateUIThread();


    /**
     * Creates and launches the ui thread.
     */
    void createUIThread();
    friend void *start_ui_thread( void *ui_int );

    /**
     * Signals the ui thread by sending one byte through the pipe
     * to it.
     */
    void signalUIThread();

    /**
     * Waits for the ui thread to send one byte through the pipe
     * to the ycp thread and reads this byte from the pipe.
     */
    bool waitForUIThread();

    /**
     * Signals the ycp thread by sending one byte through the pipe
     * to it.
     */
    void signalYCPThread();

    /**
     * Waits for the ycp thread to send one byte through the pipe
     * to the ycp thread and reads this byte from the pipe.
     */
    bool waitForYCPThread();

    /**
     * Filter out invalid events.
     **/
    YEvent * filterInvalidEvents( YEvent * event );

    /**
     * Mid-level handler for the user input related UI commands:
     *	   UserInput()
     *	   TimeoutUserInput()
     *	   WaitForEvent()
     *	   PollInput()
     *
     * 'builtin_name' is the name of the specific UI builtin command (to use
     * the correct name in the log file).
     *
     * 'timeout_millisec' is the timeout in milliseconds to use (0 for "wait
     * forever").
     *
     * 'wait' specifies if this should wait until an event is available if
     * there is none yet.
     *
     * 'detailed' specifies if a full-fledged event map is desired as return
     * value (WaitForEvent()) or one simple YCPValue (an ID).
     */
    YCPValue doUserInput( const char *	builtin_name,
			  long 		timeout_millisec,
			  bool 		wait,
			  bool 		detailed );


    /**
     * Implements the WFM or SCR callback command.
     */
    YCPValue evaluateCallback( const YCPTerm & term, bool to_wfm );


    /**
     * Helper function for @ref #replaceWidget. Searches the current dialog for
     * the radio button group that applies to a certain widget w.  The searched
     * for widget must be a radio button group that is an ancestor of w but
     * there must be no other more immediate ancestor radio button to w.
     *
     * @param root root widget where to begin searching
     *
     * @param w the widget we search a button group for.
     *
     * @param contains output parameter - is set to true if the subtree root
     * contains the widget w. Otherwise it is kept unchanged.
     */
    YRadioButtonGroup *findRadioButtonGroup( YContainerWidget *root, YWidget *w, bool *contains );


    /**
     * Helper function of createWidgetTree. Creates a replace point
     * @param parent the widget or dialog this widget is contained in
     * @param term The term specifying the widget, e.g. `ReplacePoint( `PushButton( "OK" ) )
     * @param optList The list of widget options( as specified with `opt( ... ) )
     * @param argnr the index of the first non-id and non-opt argument( 0, 1 or 2 )
     * @param rbg Pointer to the current radio button group
     */
    YWidget *createReplacePoint( YWidget *parent, YWidgetOpt & opt,
				 const YCPTerm & term, const YCPList & optList, int argnr,
				 YRadioButtonGroup *rbg );


    /**
     * Helper function of createWidgetTree.
     * Creates one of Empty, HStretch, VStretch, Stretch
     */
    YWidget *createEmpty( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr,
			  bool hstretchable, bool vstretchable );

    /**
     * Helper function of createWidgetTree.
     * Creates one of HSpacing, VSpacing.
     * @param horizontal true if this is a HSpacing
     * @param vertical true if this is a VSpacing
     */
    YWidget *createSpacing( YWidget *parent, YWidgetOpt & opt,
			    const YCPTerm & term, const YCPList & optList, int argnr,
			    bool horizontal, bool vertical );

    /**
     * Helper function of createWidgetTree.
     * Creates a frame widget.
     */
    YWidget *createFrame( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr,
			  YRadioButtonGroup *rbg );

    /**
     * Helper function of createWidgetTree.
     * Creates a weight widget.
     * @param dim dimension of the weight, either YD_HORIZ or YD_VERT
     */
    YWidget *createWeight( YWidget *parent, YWidgetOpt & opt,
			   const YCPTerm & term, const YCPList & optList, int argnr,
			   YRadioButtonGroup *rbg, YUIDimension dim );

    /**
     * Helper function of createWidgetTree.
     * Creates an alignment (`Left, `Right, `Top, `Bottom  based on the alignment parameters.
     *
     * @param halign the horizontal alignment
     * @param valign the vertical alignment
     */
    YWidget *createAlignment( YWidget *parent, YWidgetOpt & opt,
			      const YCPTerm & term, const YCPList & optList, int argnr,
			      YRadioButtonGroup *rbg,
			      YAlignmentType halign, YAlignmentType valign );

    /**
     * Helper function of createWidgetTree.
     * Creates a MarginBox.
     */
    YWidget *createMarginBox( YWidget * parent, YWidgetOpt & opt,
			      const YCPTerm & term, const YCPList & optList,
			      int argnr, YRadioButtonGroup * rbg );

    /**
     * Helper function of createWidgetTree.
     * Creates a MinWidth, MinHeight, or MinSize.
     *
     * @param hor  use minimum width
     * @param vert use minimum height
     */
    YWidget *createMinSize( YWidget * parent, YWidgetOpt & opt,
			    const YCPTerm & term, const YCPList & optList, int argnr,
			    YRadioButtonGroup * rbg,
			    bool hor, bool vert );

    /**
     * Helper function of createWidgetTree.
     * Creates one of HSquash, VSquash, HVSquash.
     *
     * @param hsquash whether the child is being squashed horizontally
     * @param vsquash whether the child is being squashed vertically
     */
    YWidget *createSquash( YWidget *parent, YWidgetOpt & opt,
			   const YCPTerm & term, const YCPList & optList, int argnr,
			   YRadioButtonGroup *rbg, bool hsquash, bool vsquash );

    /**
     * Helper function of createWidgetTree.
     * Creates one of HBox, VBox
     *
     * @param dim Dimension of the layoutbox
     */
    YWidget *createLBox( YWidget *parent, YWidgetOpt & opt,
			 const YCPTerm & term, const YCPList & optList, int argnr,
			 YRadioButtonGroup *rbg, YUIDimension dim );

    /**
     * Helper function of createWidgetTree.
     * Creates a label.
     *
     * @param heading true if the label is a Heading()
     */
    YWidget *createLabel( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr,
			  bool heading );

    YWidget *createDate( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr);

    YWidget *createTime( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a RichText.
     */
    YWidget *createRichText( YWidget *parent, YWidgetOpt & opt,
			     const YCPTerm & term, const YCPList & optList, int argnr );


    /**
     * Helper function of createWidgetTree.
     * Creates a LogView.
     */
    YWidget *createLogView( YWidget *parent, YWidgetOpt & opt,
			    const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a PushButton or an IconButton.
     */
    YWidget *createPushButton( YWidget *parent, YWidgetOpt & opt,
			       const YCPTerm & term, const YCPList & optList, int argnr,
			       bool isIconButton );

    /**
     * Helper function for createWidgetTreeTree.
     * Creates a menu button.
     */
    YWidget *createMenuButton( YWidget *parent, YWidgetOpt & opt,
			       const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a CheckBox.
     */
    YWidget *createCheckBox( YWidget *parent, YWidgetOpt & opt,
			     const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a RadioButton.
     */
    YWidget *createRadioButton( YWidget *parent, YWidgetOpt & opt,
				const YCPTerm & term, const YCPList & optList, int argnr,
				YRadioButtonGroup *rbg );

    /**
     * Helper function of createWidgetTree.
     * Creates a RadioButtonGroup.
     */
    YWidget *createRadioButtonGroup( YWidget *parent, YWidgetOpt & opt,
				     const YCPTerm & term, const YCPList & optList, int argnr,
				     YRadioButtonGroup *rbg );

    /**
     * Helper function of createWidgetTree.
     * Creates one of TextEntry, Password
     *
     * @param password true if this should be password entry field
     */
    YWidget *createTextEntry( YWidget *parent, YWidgetOpt & opt,
			      const YCPTerm & term, const YCPList & optList, int argnr,
			      bool password );

    /**
     * Helper function of createWidgetTree.
     * Creates a MultiLineEdit.
     */
    YWidget *createMultiLineEdit( YWidget *parent, YWidgetOpt & opt,
				  const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a SelectionBox.
     */
    YWidget *createSelectionBox( YWidget *parent, YWidgetOpt & opt,
				 const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function for createWidgetTreeTree.
     * Creates a MultiSelectionBox.
     */
    YWidget *createMultiSelectionBox( YWidget *parent, YWidgetOpt & opt,
				      const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a ComboBox.
     */
    YWidget *createComboBox( YWidget *parent, YWidgetOpt & opt,
			     const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a Tree.
     */
    YWidget *createTree( YWidget *parent, YWidgetOpt & opt,
			 const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a Table.
     */
    YWidget *createTable( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a ProgressBar.
     */
    YWidget *createProgressBar( YWidget *parent, YWidgetOpt & opt,
				const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates an Image.
     */
    YWidget *createImage( YWidget *parent, YWidgetOpt & opt,
			  const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates an IntField.
     */
    YWidget *createIntField( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			     const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a PackageSelector.
     */
    YWidget *createPackageSelector( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				    const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a PkgSpecial subwidget.
     */
    YWidget *createPkgSpecial( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			       const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a Wizard widget.
     */
    YWidget *createWizard( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			   const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a DummySpecialWidget.
     */
    YWidget *createDummySpecialWidget( YWidget *parent, YWidgetOpt & opt,
				       const YCPTerm & term, const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a DownloadProgress.
     */
    YWidget *createDownloadProgress( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				     const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a BarGraph.
     */
    YWidget *createBarGraph( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			     const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a ColoredLabel.
     */
    YWidget *createColoredLabel( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				 const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a DumbTab.
     */
    YWidget *createDumbTab( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			    const YCPList & optList, int argnr, YRadioButtonGroup * rbg );

    /**
     * Helper function of createWidgetTree.
     * Creates a MultiProgressMeter.
     */
    YWidget * createMultiProgressMeter( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
					const YCPList & optList, int argnr, bool horizontal );
    /**
     * Helper function of createWidgetTree.
     * Creates a Slider.
     */
    YWidget *createSlider( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
			   const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a PartitionSplitter.
     */
    YWidget *createPartitionSplitter( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				      const YCPList & optList, int argnr );

    /**
     * Helper function of createWidgetTree.
     * Creates a PatternSelector.
     */
    YWidget *createPatternSelector( YWidget *parent, YWidgetOpt & opt, const YCPTerm & term,
				    const YCPList & optList, int argnr );


    /**
     * Looks for a widget id in a widget term. If it finds one, returns
     * it and sets argnr to 1, otherwise it creates a new unique widget
     * id and sets argnr to 0. For example PushButton( `id( 17 ), .... )
     * has with id 17.
     * @return The widget id on success or YCPNull on failure
     */
    YCPValue getWidgetId( const YCPTerm & term, int *argnr );

    /**
     * Looks for widget options in the term. Returns the list
     * of options if available, otherwise an empty list.
     * Increases argnr by 1 if options are found.
     * For example PushButton( `id( 17 ), `opt( `kilroy, `color( `red ) )
     * has the option list [ `kilroy, `color( `red ) ]
     * @param term the Widgetterm
     * @param argnr in/out: The number of the first non-id argument.
     * Returns the number of the first non-id and non-opt argument.
     * @return The option list, which may be empty, but never YCPNull
     */
    YCPList getWidgetOptions( const YCPTerm & term, int *argnr );

    /**
     * Logs a warning for an unknown widget option
     * @param term the widget term, e.g. PushButton( `opt( `unknown ), ... )
     * @param option the unknown option itself
     */
    void logUnknownOption( const YCPTerm & term, const YCPValue & option );

    /**
     * Logs warning messages for all widget options other than the
     * standard ones - for widgets that don't handle any options.
     * @param term the widget term, e.g. PushButton( `opt( `unknown ), ... )
     * @param optList the list of options not yet processed
     */
    void rejectAllOptions( const YCPTerm & term, const YCPList & optList );

    /**
     * Enters a dialog into the dialog map.
     */
    void registerDialog( YDialog * );

    /**
     * Removes the topmost dialog from the dialog stack and deletes it.
     */
    void removeDialog();

    /**
     * Checks if the given value is either a symbol or a term `id().
     *
     * @return 'true' if 'val' is a symbol or a valid `id().
     */
    bool isSymbolOrId( const YCPValue & val ) const;


    /**
     * Delete the internal macro recorder and set the pointer to 0.
     */
    void deleteMacroRecorder();

    /**
     * Delete the internal macro player and set the pointer to 0.
     */
    void deleteMacroPlayer();

    /**
     * Play the next block of an active macro player.
     */
    void playNextMacroBlock();

    /**
     * Define type for the dialog map
     */
    typedef vector<YDialog *> dialogstack_type;

    /**
     * Container for all dialogs. Each dialog has a unique id, which is
     * a YCPValue.
     */
    dialogstack_type dialogstack;

    /**
     * Counter for creating unique widget ids
     */
    long long id_counter;

    /**
     * true if a seperate UI thread is created
     */
    bool with_threads;

    /**
     * Handle to the ui thread.
     */
    pthread_t ui_thread;

    /**
     * Inter-thread communication between the YCP thread and the UI thread:
     * The YCP thread supplies data here and signals the UI thread,
     * the UI thread picks up the data, executes the function, puts
     * the result here and signals the YCP thread that waits until
     * the result is available.
     */
    YUIBuiltinCallData _builtinCallData;

    /**
     * Used to synchronize data transfer with the ui thread.
     * It stores a pair of file descriptors of a pipe. For each YCP value
     * we send to the ui thread, we write one aribrary byte here.
     */
    int pipe_to_ui[2];

    /**
     * Used to synchronize data transfer with the ui thread.
     * It stores a pair of file descriptors of a pipe. For each YCP value
     * we get from the ui thread, we read one aribrary byte from here.
     */
    int pipe_from_ui[2];

    /**
     * This is a flag that signals the ui thread that it should
     * terminate. This is done by setting the flag to true. The ui
     * thread replies by setting the flag back to false directly
     * after terminating itself.
     */
    bool terminate_ui_thread;

    /**
     * The current module name as set by the SetModuleName UI command.
     */
    string _moduleName;

    /**
     * The current product name ("SuSE Linux", "United Linux", ...).
     **/
    string _productName;

    /**
     * The current macro recorder.
     */
    YMacroRecorder *macroRecorder;

    /**
     * The current macro player.
     */
    YMacroPlayer *macroPlayer;

    /**
     * Queue for synthetic (faked) user input events.
     */
    deque<YCPValue> fakeUserInputQueue;

    /**
     * The current mapping of widget labels to default function keys.
     **/
    YCPMap default_fkeys;

    /**
     * Flag that keeps track of blocked events.
     * Never query this directly, use eventsBlocked() instead.
     **/
    bool _events_blocked;

    /**
     * Returns 'true' if widget geometry should be reversed for languages that
     * have right-to-left writing direction (Arabic, Hebrew).
     **/
    static bool _reverseLayout;

    /**
     * The callback component previously set with setCallback().
     **/
    Y2Component * _callback;


    /**
     * Global reference to the UI
     **/
    static YUI * _yui;

};

#endif // YUI_h
