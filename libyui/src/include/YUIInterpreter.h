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

  File:		YUIInterpreter.h

  Author:	Mathias Kettner <kettner@suse.de>
  Maintainer:	Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YUIInterpreter_h
#define YUIInterpreter_h

#include <pthread.h>
#include <deque>
#include <ycp/YCPInterpreter.h>
#include <y2/Y2Component.h>
#include "YWidget.h"
#include "YAlignment.h"

using std::deque;

class YContainerWidget;
class YDialog;
class YMacroPlayer;
class YMacroRecorder;
class YMenu;
class YMenuButton;
class YMultiSelectionBox;
class YRadioButtonGroup;
class YTree;
class YTreeItem;


typedef struct
{
    int red;
    int green;
    int blue;
} YColor;


/**
 * @short base class of a YaST2 user interface
 * The implementation of a YaST2 user interface such as qt and ncurses
 * constists in subclassing YUI.
 *
 * We have to handle to cases slightly different: The case with and without
 * a seperate UI thread.
 *
 * You have two alternatives how to implement event handling in your UI.
 * Either override @ref #idleLoop, @ref #userInput and @ref #pollInput
 * or override @ref #pollInput and @ref #waitForEvent, whichever is
 * easier for you.
 */
class YUIInterpreter : public YCPInterpreter
{
public:
    /**
     * Looks up the topmost dialog
     */
    YDialog *currentDialog() const;

    /**
     * Creates a YUIInterpreter.
     * @param with_threads Set this to true if you want a seperate ui thread
     */
    YUIInterpreter(bool with_threads, Y2Component *callback);

    /**
     * Cleans up, terminates the ui thread.
     */
    virtual ~YUIInterpreter();

    /**
     * switch callback pointer
     * used for CallModule() implementation which creates a new
     * workflow manager (the target of the callback pointer), to
     * which the callback pointer must be adjusted.
     * this function is used to pass the getCallback/setCallback
     * functions from the underlying Y2{Qt,Ncurses,whatever}Component
     * up to the interpreter.
     */
    Y2Component *getCallback (void);
    void setCallback (Y2Component *callback);

    /**
     * Must be called after the constructor of the Qt/NCurses ui
     * is ready. Starts the ui thread.
     */
    void topmostConstructorHasFinished();

    /**
     * Constants for different user input events.
     * The ET_CHANGED event is currently not implemented. It should
     * render it possible for the module the react to a event like
     * the changement of the selected item in a list box. Widgets must be
     * tagged in a special way in order to return ET_CHANGED events.
     */
    enum EventType
    {
	ET_NONE,
	ET_WIDGET,
	ET_MENU,
	ET_CANCEL,
	ET_DEBUG
    };

    /**
     * Constants for different predifined images
     */
    enum ImageType
    {
	IT_SUSEHEADER,
	IT_YAST2
    };

    /**
     * Name of interpreter, returns "ui".
     */
    string interpreter_name () const;

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
     * Retrieve the last menu selection.
     * Only valid if userInput() or pollInput() returned ET_MENU.
     */
    const YCPValue & getMenuSelection() const		{ return menuSelection; }

    /**
     * Set the last menu selection.
     *
     * Derived UIs should call this in their menu handlers prior to returning
     * from pollInput() or userInput() with ET_MENU.
     */
    void setMenuSelection( const YCPValue & sel )	{ menuSelection = sel; }


    /**
     * Might be handy, if you have to recode strings from/to utf-8
     */
    static int Recode( const string & str,
		       const string & from,
		       const string & to,
		       string & outstr );

    /**
     * Parse a menu list (for menu buttons)
     */
    int parseMenuItemList ( const YCPList &itemList, YMenuButton *menu_button, YMenu *parentMenu = 0 );


    /**
     * Parse a tree item list
     */
    int parseTreeItemList ( const YCPList &itemList, YTree *tree, YTreeItem *parentItem = 0 );


    /**
     * Parse an item list for a MultiSelectionBox
     */
    int parseMultiSelectionBoxItemList( const YCPList &	item_list, YMultiSelectionBox *	multi_sel_box );

    /**
     * Parse an `rgb() value
     **/
    bool YUIInterpreter::parseRgb(const YCPValue & val, YColor *color, bool complain);

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
    YWidget *createWidgetTree(YWidget *parent, YWidgetOpt &opt, YRadioButtonGroup *rbg, const YCPTerm& term);

    /**
     * Overloaded version for convenience. Supplies empty widget options.
     */
    YWidget *createWidgetTree(YWidget *parent, YRadioButtonGroup *rbg, const YCPTerm& term);

    /**
     * Looks up a widget with a certain id. Returns 0 if no
     * such exists. Only the topmost dialog ist searched for the id.
     * @param id id without `id(..)
     * @param log_error set to true if you want me to log an error if
     * the id is not existing.
     */

    YWidget *widgetWithId(const YCPValue &id, bool log_error=false);

    /**
     * Overloaded version of widgetWithId. Does not search for the widget id in the
     * topmost dialog but in given widget tree.
     * @param widgetRoot root of the widget tree
     * @param id id without `id(..)
     * @param log_error set to true if you want me to log an error if
     * the id is not existing.
     */
    YWidget *widgetWithId(YContainerWidget *widgetRoot, const YCPValue &id, bool log_error=false);

    /**
     * Implements the UI command ReplaceWidget.
     */
    YCPValue evaluateReplaceWidget(const YCPTerm& term);


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
     * The default implementation calls @ref #waitForEvent
     *
     * @param fd_ycp filedescriptor that should be used to determine when
     * to leave the idle loop. As soon as it is readable, the loop must
     * be left. In order to avoid polling you can combine it with other
     * ui-specific fds and do common <tt>select</tt> call.
     */
    virtual void idleLoop(int fd_ycp);

    /**
     * This virtual method is called, when the YCPUIInterpreter evaluates
     * the YCP command <tt>UserInput()</tt>. You should go into your
     * event loop and return as soon as the user has pressed some widget that
     * passes control back to the module. Currently this are push buttons and
     * the window-close button.
     *
     * The default implementation calls @ref #pollInput and @ref #waitForEvent
     * until an event has happened.
     *
     * @param dialog the dialog that should receive user input. Only one dialog
     * can be active at a time.
     *
     * @param event Return parameter. Put here which kind of event has happened.
     *
     * @return the pressed or activated widget. Return 0 if the event is not
     * related to any widget (such as close window).
     */
    virtual YWidget *userInput(YDialog *dialog, EventType *event);

    /**
     * This virtual method is called, when the YCPUIInterpreter evaluates
     * the YCP command <tt>PollInput()</tt>. You should <i>not</i> go into
     * your event loop but just look whether there is pending user input.
     *
     * The default implementation always returns immediatley with event == ET_NONE.
     *
     * @param dialog the dialog that should receive user input. Only one dialog
     * can be active at a time.
     * @param event Return parameter. Put here which kind of event has happened.
     * Put ET_NONE if no event has happened.
     * @return the pressed or activated widget. Return 0 if the event is not
     * related to any widget (such as close window).
     */
    virtual YWidget *pollInput(YDialog *dialog, EventType *event);

    /**
     * Override this virtual method the specify how it can be determined whether either
     * a user input event has happened or fd_ycp is readable. Each UI has it's own
     * mechanism how events are received. If your UI gets events via one or more filedestriptors
     * you can combine them together with fd_ycp into a </tt>select()</tt> call. This avoids
     * polling.
     *
     * The default implementation makes a select() call on fd_ycp and returns false.
     *
     * @param fd_ycp filedescriptor you have to look at in addition to your own
     * input. If it is -1 then ignore it.
     * @return true if fd_ycp is readable. false if an event of your UI system is pending.
     * Don't return until one of both has happened.
     */
    virtual bool waitForEvent(int fd_ycp);

    /**
     * Shows and activates a previously created dialog. The default implementation does nothing.
     * @param dialog dialog to show.
     */
    virtual void showDialog(YDialog *dialog);

    /**
     * Decativates and closes a previously created dialog. The default implementation does nothing.
     * Don't delete the dialog. This will be done at some other place.
     * @param dialog dialog to close.
     */
    virtual void closeDialog(YDialog *dialog);


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
    virtual YDialog *createDialog(YWidgetOpt &opt) = 0;

    /**
     * Creates a split
     */
    virtual YContainerWidget *createSplit(YWidget *parent, YWidgetOpt &opt, YUIDimension dimension) = 0;

    /**
     * Creates a replace point.
     */
    virtual YContainerWidget *createReplacePoint(YWidget *parent, YWidgetOpt &opt) = 0;

    /**
     * Creates an alignment widget
     */
    virtual YContainerWidget *createAlignment(YWidget *parent, YWidgetOpt &opt,
					      YAlignmentType halign,
					      YAlignmentType valign) = 0;

    /**
     * Creates a squash widget
     */
    virtual YContainerWidget *createSquash(YWidget *parent, YWidgetOpt &opt, bool hsquash, bool vsquash) = 0;


    /**
     * Creates a radio button group.
     */
    virtual YContainerWidget *createRadioButtonGroup(YWidget *parent, YWidgetOpt &opt) = 0;

    /**
     * Creates a frame widget
     */
    virtual YContainerWidget *createFrame(YWidget *parent, YWidgetOpt &opt, const YCPString& label) = 0;



    //
    // Widget creation functions - leaf widgets
    //

    /**
     * Creates an empty widget
     */
    virtual YWidget *createEmpty(YWidget *parent, YWidgetOpt &opt) = 0;

    /**
     * Creates a spacing widget
     */
    virtual YWidget *createSpacing(YWidget *parent, YWidgetOpt &opt, float size, bool horizontal, bool vertical) = 0;

    /**
     * Creates a label.
     * @param text Initial text of the label
     * @param heading true if the label is a Heading()
     * @param output_field true if the label should look like an output field (3D look)
     */
    virtual YWidget *createLabel(YWidget *parent, YWidgetOpt &opt, const YCPString& text) = 0;

    /**
     * Creates a rich text widget
     * @param text Initial text of the label
     */
    virtual YWidget *createRichText(YWidget *parent, YWidgetOpt &opt, const YCPString& text) = 0;


    /**
     * Creates a log view widget
     * @param label label above the log view
     * @param visibleLines default number of vislible lines
     * @param maxLines number of lines to store (use 0 for "all")
     */
    virtual YWidget *createLogView(YWidget *parent, YWidgetOpt &opt,
				   const YCPString& label, int visibleLines, int maxLines ) = 0;

    /**
     * Creates a push button.
     * @param label Label of the button
     */
    virtual YWidget *createPushButton(YWidget *parent, YWidgetOpt &opt, const YCPString& label) = 0;

    /**
     * Creates a menu button.
     * @param label Label of the button
     */
    virtual YWidget *createMenuButton(YWidget *parent, YWidgetOpt &opt, const YCPString& label) = 0;

    /**
     * Creates a radio button and inserts it into a radio button group
     * @param label Label of the radio button
     * @param rbg the radio button group the new button will belong to
     */
    virtual YWidget *createRadioButton(YWidget *parent, YWidgetOpt &opt, YRadioButtonGroup *rbg,
				       const YCPString& label, bool checked) = 0;

    /**
     * Creates a check box
     * @param label Label of the checkbox
     * @param true if it is checked
     */
    virtual YWidget *createCheckBox(YWidget *parent, YWidgetOpt &opt, const YCPString& label, bool checked) = 0;

    /**
     * Creates a text entry or password entry field.
     */
    virtual YWidget *createTextEntry(YWidget *parent, YWidgetOpt &opt, const YCPString &label, const YCPString& text) = 0;

    /**
     * Creates a MultiLineEdit widget.
     */
    virtual YWidget *createMultiLineEdit(YWidget *parent, YWidgetOpt &opt, const YCPString &label, const YCPString& text) = 0;

    /**
     * Creates a selection box
     */
    virtual YWidget *createSelectionBox(YWidget *parent, YWidgetOpt &opt, const YCPString &label) = 0;

    /**
     * Creates a multi selection box
     */
    virtual YWidget *createMultiSelectionBox(YWidget *parent, YWidgetOpt &opt, const YCPString &label) = 0;

    /**
     * Creates a combo box
     */
    virtual YWidget *createComboBox(YWidget *parent, YWidgetOpt &opt, const YCPString &label) = 0;

    /**
     * Creates a tree
     */
    virtual YWidget *createTree(YWidget *parent, YWidgetOpt &opt, const YCPString &label) = 0;

    /**
     * Creates a table widget
     */
    virtual YWidget *createTable(YWidget *parent, YWidgetOpt &opt, vector<string> header) = 0;

    /**
     * Creates a progress bar
     */
    virtual YWidget *createProgressBar(YWidget *parent, YWidgetOpt &opt, const YCPString &label,
				       const YCPInteger& maxprogress, const YCPInteger& progress) = 0;

    /**
     * Creates an image widget from a YCP byteblock
     */
    virtual YWidget *createImage(YWidget *parent, YWidgetOpt &opt, YCPByteblock imagedata, YCPString defaulttext) = 0;

    /**
     * Creates an image widget from a file name
     */
    virtual YWidget *createImage(YWidget *parent, YWidgetOpt &opt, YCPString file_name, YCPString defaulttext) = 0;

    /**
     * Creates an image widget from a predefined set of images
     */
    virtual YWidget *createImage(YWidget *parent, YWidgetOpt &opt, ImageType img, YCPString defaulttext) = 0;

    /**
     * Creates an IntField widget.
     */
    virtual YWidget *createIntField(YWidget *parent, YWidgetOpt &opt,
				    const YCPString &label, int minValue, int maxValue, int initialValue) = 0;

    /**
     * Creates a PackageSelector widget.
     */
    virtual YWidget *createPackageSelector(YWidget *parent, YWidgetOpt &opt ) = 0;

    /**
     * Creates a PkgSpecial widget, i.e. a specific subwidget.
     */
    virtual YWidget *createPkgSpecial(YWidget *parent, YWidgetOpt &opt, const YCPString &subwidget ) = 0;

    
    /**
     * Creates a DummySpecialWidget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *createDummySpecialWidget(YWidget *parent, YWidgetOpt &opt);
    virtual bool     hasDummySpecialWidget();

    /**
     * Creates a DownloadProgress widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createDownloadProgress(YWidget *parent, YWidgetOpt &opt,
					       const YCPString &label,
					       const YCPString &filename,
					       int expectedSize);
    virtual bool 	hasDownloadProgress();

    /**
     * Creates a BarGraph widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createBarGraph(YWidget *parent, YWidgetOpt &opt);
    virtual bool 	hasBarGraph();

    /**
     * Creates a ColoredLabelwidget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createColoredLabel(YWidget *parent, YWidgetOpt &opt,
					   YCPString label,
					   YColor foreground, YColor background,
					   int margin );
    virtual bool 	hasColoredLabel();


    /**
     * Creates a Slider widget.
     *
     * This is a special widget that the UI may or may not support.
     * Overwrite this method at your own discretion.
     * If you do, remember to overwrite the has...() method as well!
     */
    virtual YWidget *	createSlider(YWidget *		parent,
				     YWidgetOpt &	opt,
				     const YCPString &	label,
				     int 		minValue,
				     int 		maxValue,
				     int 		initialValue );
    virtual bool 	hasSlider();

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
     * @param totalFreeSize 	total size of the free part of the partition (before the split)
     * @param newPartSize	suggested size of the new partition
     * @param minNewPartSize	minimum size of the new partition
     * @param minFreeSize 	minimum remaining free size of the old partition
     * @param usedLabel 		BarGraph label for the used part of the old partition
     * @param freeLabel 		BarGraph label for the free part of the old partition
     * @param newPartLabel  	BarGraph label for the new partition
     * @param freeFieldLabel	label for the remaining free space field
     * @param newPartFieldLabel	label for the new partition size field
     */

    virtual YWidget *createPartitionSplitter( YWidget *		parent,
					      YWidgetOpt &	opt,
					      int 		usedSize,
					      int 		totalFreeSize,
					      int 		newPartSize,
					      int 		minNewPartSize,
					      int 		minFreeSize,
					      const YCPString &	usedLabel,
					      const YCPString &	freeLabel,
					      const YCPString &	newPartLabel,
					      const YCPString &	freeFieldLabel,
					      const YCPString &	newPartFieldLabel );
    virtual bool 	hasPartitionSplitter();


    /**
     * UI-specific setLanguage() function.
     * Returns YCPVoid() if OK and YCPNull() on error.
     * This default implementation does nothing.
     */
    virtual YCPValue setLanguage(const YCPTerm &term);


    /**
     * UI-specific setConsoleFont() function.
     * Returns YCPVoid() if OK and YCPNull() on error.
     * This default implementation does nothing.
     */
    virtual YCPValue setConsoleFont ( const YCPString &console_magic,
				      const YCPString &font,
				      const YCPString &screen_map,
				      const YCPString &unicode_map,
				      const YCPString &encoding );


    /**
     * UI-specific getDisplayInfo() functions.
     * See UI builtin GetDisplayInfo() doc for details.
     **/
    virtual int  getDisplayWidth()		{ return -1; }
    virtual int  getDisplayHeight() 		{ return -1; }
    virtual int  getDisplayDepth() 		{ return -1; }
    virtual long getDisplayColors() 		{ return -1; }
    virtual int  getDefaultWidth()		{ return -1; }
    virtual int  getDefaultHeight()		{ return -1; }
    virtual bool textMode()	 		{ return true; }
    virtual bool hasImageSupport()		{ return false; }
    virtual bool hasLocalImageSupport()		{ return true;  }
    virtual bool hasAnimationSupport()		{ return false; }
    virtual bool hasIconSupport()		{ return false; }
    virtual bool hasFullUtf8Support()		{ return false; }

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
    virtual void makeScreenShot();


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
     * or 'nil' (YCPVoid()) if the user canceled the operation. 
     **/
    virtual YCPValue askForExistingDirectory ( const YCPString & startDir,
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
     * or 'nil' (YCPVoid()) if the user canceled the operation.
     **/
    virtual YCPValue askForExistingFile	( const YCPString & startWith,
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
     * or 'nil' (YCPVoid()) if the user canceled the operation.
     **/
    virtual YCPValue askForSaveFileName	( const YCPString & startWith,
					  const YCPString & filter,
					  const YCPString & headline ) = 0;



    /**
     * This function is inherited from YCPInterpreter. It checks
     * if the given term is a command the UI understands. If no,
     * it returns 0 and to interpreter will look at it's builtin
     * functions instead.
     *
     * If it is a valid UI command, executes, tt checks the parameters
     * and decodes the command. If it is invalid, it logs an error
     * to y2log and immediatly returns YCPVoid. If it is valid,
     * it executes it, which may involve calling a virtual method that
     * is overridden by some actual UI, such as @ref #userInput or
     * @ref #pollInput.
     */
    YCPValue evaluateInstantiatedTerm( const YCPTerm & term );
    
    YCPValue callback		( const YCPValue & value );
    YCPValue evaluateUI		( const YCPValue & value );
    YCPValue evaluateWFM	( const YCPValue & value );
    YCPValue evaluateSCR	( const YCPValue & value );
    YCPValue setTextdomain	( const string & textdomain) ;
    string getTextdomain (void);

    /**
     * Evaluates a locale. Evaluate _("string") to "string".
     */
    YCPValue evaluateLocale(const YCPLocale&);

    /**
     * Start macro recording to file "filename".
     * Any previous active macro recorder will be terminated (regularly) prior
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
    friend void *start_ui_thread(void *ui_int);

    /**
     * This method implements the UI thread in case it is existing.
     * The loop consists of calling @ref #idleLoop, getting the next
     * command from the @ref YCPUIInterpreter, evaluating it, which
     * possibly invovles calling @ref #userInput() or @ref #pollInput()
     * and writes the answer back to the other thread where the request
     * came from.
     */
    void uiThreadMainLoop();

    /**
     * Signals the ui thread by sending one byte through the pipe
     * to it.
     */
    void signalUIThread();

    /**
     * Waits for the ui thread to send one byte through the pipe
     * to the ycp thread and reads this byte from the pipe.
     */
    bool waitForUIThread ();

    /**
     * Signals the ycp thread by sending one byte through the pipe
     * to it.
     */
    void signalYCPThread();

    /**
     * Waits for the ycp thread to send one byte through the pipe
     * to the ycp thread and reads this byte from the pipe.
     */
    bool waitForYCPThread ();

    /**
     * Filter out invalid events.
     **/
    YWidget * filterInvalidEvents( YWidget *event_widget );

    /**
     * actually executes an ui command term. Never returns YCPNull.
     */
    YCPValue executeUICommand			( const YCPTerm & term );


    /**
     * Implementations for most UI builtins.
     * Each method corresponds directly to one UI builtin.
     **/
    YCPValue evaluateAskForExistingDirectory	( const YCPTerm & term );
    YCPValue evaluateAskForExistingFile		( const YCPTerm & term );
    YCPValue evaluateAskForSaveFileName		( const YCPTerm & term );
    YCPValue evaluateBusyCursor			( const YCPTerm & term );
    YCPValue evaluateChangeWidget		( const YCPTerm & term );
    YCPValue evaluateCloseDialog		( const YCPTerm & term );
    YCPValue evaluateDumpWidgetTree		( const YCPTerm & term );
    YCPValue evaluateFakeUserInput 		( const YCPTerm & term );
    YCPValue evaluateGetDisplayInfo		( const YCPTerm & term );
    YCPValue evaluateGetLanguage		( const YCPTerm & term );
    YCPValue evaluateGetModulename		( const YCPTerm & term );
    YCPValue evaluateGlyph 			( const YCPTerm & term );
    YCPValue evaluateHasSpecialWidget		( const YCPTerm & term );
    YCPValue evaluateMakeScreenShot		( const YCPTerm & term );
    YCPValue evaluateNormalCursor		( const YCPTerm & term );
    YCPValue evaluateOpenDialog			( const YCPTerm & term );
    YCPValue evaluatePlayMacro 			( const YCPTerm & term );
    YCPValue evaluateQueryWidget		( const YCPTerm & term );
    YCPValue evaluateRecalcLayout		( const YCPTerm & term );
    YCPValue evaluateRecode 			( const YCPTerm & term );
    YCPValue evaluateRecordMacro 		( const YCPTerm & term );
    YCPValue evaluateRedrawScreen		( const YCPTerm & term );
    YCPValue evaluateRunPkgSelection		( const YCPTerm & term );
    YCPValue evaluateSetConsoleFont		( const YCPTerm & term );
    YCPValue evaluateSetFocus			( const YCPTerm & term );
    YCPValue evaluateSetLanguage		( const YCPTerm & term );
    YCPValue evaluateSetModulename		( const YCPTerm & term );
    YCPValue evaluateStopRecordMacro 		( const YCPTerm & term );
    YCPValue evaluateWidgetExists		( const YCPTerm & term );

    /**
     * Implements the UserInput and PollInput() UI commands:
     * @param poll set this to true if you want PollInput(), false for UserInput()
     */
    YCPValue evaluateUserInput	(const YCPTerm & term, bool poll);

    /**
     * Implements the WFM or SCR callback command.
     */
    YCPValue evaluateCallback (const YCPTerm & term, bool to_wfm);


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
    YRadioButtonGroup *findRadioButtonGroup(YContainerWidget *root, YWidget *w, bool *contains);

    /**
     * Creates a ycp return value for the client component. If the event
     * has been triggered by a widget, the return value is the id of that
     * widget.
     *
     * @param widget that triggered the event or 0 if it was
     * not triggered by a special widget
     *
     * @param et type of the event
     */
    YCPValue returnEvent(const YWidget *widget, EventType et);




    /**
     * Helper function of createWidgetTree. Creates a replace point
     * @param parent the widget or dialog this widget is contained in
     * @param term The term specifying the widget, e.g. `ReplacePoint(`PushButton("OK"))
     * @param optList The list of widget options (as specified with `opt(...))
     * @param argnr the index of the first non-id and non-opt argument (0, 1 or 2)
     * @param rbg Pointer to the current radio button group
     */
    YWidget *createReplacePoint(YWidget *parent, YWidgetOpt &opt,
				const YCPTerm &term, const YCPList &optList, int argnr,
				YRadioButtonGroup *rbg);


    /**
     * Helper function of createWidgetTree.
     * Creates one of Empty, HStretch, VStretch, Stretch
     */
    YWidget *createEmpty(YWidget *parent, YWidgetOpt &opt,
			 const YCPTerm &term, const YCPList &optList, int argnr,
			 bool hstretchable, bool vstretchable);

    /**
     * Helper function of createWidgetTree.
     * Creates one of HSpacing, VSpacing.
     * @param horizontal true if this is a HSpacing
     * @param vertical true if this is a VSpacing
     */
    YWidget *createSpacing(YWidget *parent, YWidgetOpt &opt,
			   const YCPTerm &term, const YCPList &optList, int argnr,
			   bool horizontal, bool vertical);

    /**
     * Helper function of createWidgetTree.
     * Creates a frame widget.
     */
    YWidget *createFrame(YWidget *parent, YWidgetOpt &opt,
			 const YCPTerm &term, const YCPList &optList, int argnr,
			 YRadioButtonGroup *rbg);

    /**
     * Helper function of createWidgetTree.
     * Creates a weight widget.
     * @param dim dimension of the weight, either YD_HORIZ or YD_VERT
     */
    YWidget *createWeight(YWidget *parent, YWidgetOpt &opt,
			  const YCPTerm &term, const YCPList &optList, int argnr,
			  YRadioButtonGroup *rbg, YUIDimension dim);

    /**
     * Helper function of createWidgetTree.
     * Creates an alignment (`Left, `Right, `Top, `Bottom) based on the alignment parameters.
     *
     * @param halign the horizontal alignment
     * @param valign the vertical alignment
     */
    YWidget *createAlignment(YWidget *parent, YWidgetOpt &opt,
			     const YCPTerm &term, const YCPList &optList, int argnr,
			     YRadioButtonGroup *rbg,
			     YAlignmentType halign, YAlignmentType valign);


    /**
     * Helper function of createWidgetTree.
     * Creates one of HSquash, VSquash, HVSquash.
     *
     * @param hsquash whether the child is being squashed horizontally
     * @param vsquash whether the child is being squashed vertically
     */
    YWidget *createSquash(YWidget *parent, YWidgetOpt &opt,
			  const YCPTerm &term, const YCPList &optList, int argnr,
			  YRadioButtonGroup *rbg, bool hsquash, bool vsquash);

    /**
     * Helper function of createWidgetTree.
     * Creates one of HBox, VBox
     *
     * @param dim Dimension of the layoutbox
     */
    YWidget *createLBox(YWidget *parent, YWidgetOpt &opt,
			const YCPTerm &term, const YCPList &optList, int argnr,
			YRadioButtonGroup *rbg, YUIDimension dim);

    /**
     * Helper function of createWidgetTree.
     * Creates a label.
     *
     * @param heading true if the label is a Heading()
     */
    YWidget *createLabel(YWidget *parent, YWidgetOpt &opt,
			 const YCPTerm &term, const YCPList &optList, int argnr,
			 bool heading);

    /**
     * Helper function of createWidgetTree.
     * Creates a RichText.
     */
    YWidget *createRichText(YWidget *parent, YWidgetOpt &opt,
			    const YCPTerm &term, const YCPList &optList, int argnr);


    /**
     * Helper function of createWidgetTree.
     * Creates a LogView.
     */
    YWidget *createLogView(YWidget *parent, YWidgetOpt &opt,
			   const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a PushButton or an IconButton.
     */
    YWidget *createPushButton(YWidget *parent, YWidgetOpt &opt,
			      const YCPTerm &term, const YCPList &optList, int argnr,
			      bool isIconButton );

    /**
     * Helper function for createWidgetTreeTree.
     * Creates a menu button.
     */
    YWidget *createMenuButton( YWidget *parent, YWidgetOpt &opt,
			       const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a CheckBox.
     */
    YWidget *createCheckBox(YWidget *parent, YWidgetOpt &opt,
			    const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a RadioButton.
     */
    YWidget *createRadioButton(YWidget *parent, YWidgetOpt &opt,
			       const YCPTerm &term, const YCPList &optList, int argnr,
			       YRadioButtonGroup *rbg);

    /**
     * Helper function of createWidgetTree.
     * Creates a RadioButtonGroup.
     */
    YWidget *createRadioButtonGroup(YWidget *parent, YWidgetOpt &opt,
				    const YCPTerm &term, const YCPList &optList, int argnr,
				    YRadioButtonGroup *rbg);

    /**
     * Helper function of createWidgetTree.
     * Creates one of TextEntry, Password
     *
     * @param password true if this should be password entry field
     */
    YWidget *createTextEntry(YWidget *parent, YWidgetOpt &opt,
			     const YCPTerm &term, const YCPList &optList, int argnr,
			     bool password);

    /**
     * Helper function of createWidgetTree.
     * Creates a MultiLineEdit.
     */
    YWidget *createMultiLineEdit(YWidget *parent, YWidgetOpt &opt,
				 const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a SelectionBox.
     */
    YWidget *createSelectionBox(YWidget *parent, YWidgetOpt &opt,
				const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function for createWidgetTreeTree.
     * Creates a MultiSelectionBox.
     */
    YWidget *createMultiSelectionBox(YWidget *parent, YWidgetOpt &opt,
				     const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a ComboBox.
     */
    YWidget *createComboBox(YWidget *parent, YWidgetOpt &opt,
			    const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a Tree.
     */
    YWidget *createTree(YWidget *parent, YWidgetOpt &opt,
			const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a Table.
     */
    YWidget *createTable(YWidget *parent, YWidgetOpt &opt,
			 const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a ProgressBar.
     */
    YWidget *createProgressBar(YWidget *parent, YWidgetOpt &opt,
			       const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates an Image.
     */
    YWidget *createImage(YWidget *parent, YWidgetOpt &opt,
			 const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates an IntField.
     */
    YWidget *createIntField(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
			    const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a PackageSelector.
     */
    YWidget *createPackageSelector(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				   const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a PkgSpecial subwidget.
     */
    YWidget *createPkgSpecial(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
			      const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a DummySpecialWidget.
     */
    YWidget *createDummySpecialWidget(YWidget *parent, YWidgetOpt &opt,
				      const YCPTerm &term, const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a DownloadProgress.
     */
    YWidget *createDownloadProgress(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				    const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a BarGraph.
     */
    YWidget *createBarGraph(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
			    const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a ColoredLabel.
     */
    YWidget *createColoredLabel(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a Slider.
     */
    YWidget *createSlider(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
			  const YCPList &optList, int argnr);

    /**
     * Helper function of createWidgetTree.
     * Creates a PartitionSplitter.
     */
    YWidget *createPartitionSplitter(YWidget *parent, YWidgetOpt &opt, const YCPTerm &term,
				     const YCPList &optList, int argnr);



    /**
     * Looks for a widget id in a widget term. If it finds one, returns
     * it and sets argnr to 1, otherwise it creates a new unique widget
     * id and sets argnr to 0. For example PushButton(`id(17), ....)
     * has with id 17.
     * @return The widget id on success or YCPNull on failure
     */
    YCPValue getWidgetId(const YCPTerm &term, int *argnr);

    /**
     * Looks for widget options in the term. Returns the list
     * of options if available, otherwise an empty list.
     * Increases argnr by 1 if options are found.
     * For example PushButton(`id(17), `opt(`hirn, `color(`red))
     * has the option list [`hirn, `color(`red)]
     * @param term the Widgetterm
     * @param argnr in/out: The number of the first non-id argument.
     * Returns the number of the first non-id and non-opt argument.
     * @return The option list, which may be empty, but never YCPNull
     */
    YCPList getWidgetOptions(const YCPTerm &term, int *argnr);

    /**
     * Logs a warning for an unknown widget option
     * @param term the widget term, e.g. PushButton(`opt(`unknown), ...)
     * @param option the unknown option itself
     */
    void logUnknownOption(const YCPTerm &term, const YCPValue &option);

    /**
     * Logs warning messages for all widget options other than the
     * standard ones - for widgets that don't handle any options.
     * @param term the widget term, e.g. PushButton(`opt(`unknown), ...)
     * @param optList the list of options not yet processed
     */
    void rejectAllOptions(const YCPTerm &term, const YCPList &optList);

    /**
     * Enters a dialog into the dialog map.
     */
    void registerDialog(YDialog *);

    /**
     * Removes the topmost dialog from the dialog stack and deletes it.
     */
    void removeDialog();

    /**
     * Checks if the given value is a term with the symbol 'id and
     * size one. Logs an error if this is not so and 'complain' is set.
     * @return true if this is so
     */
    bool checkId(const YCPValue& v, bool complain = true ) const;

    /**
     * Assumes that the value v is of the form `id(any i) and returns
     * the contained i.
     */
    YCPValue getId(const YCPValue& v) const;

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
     * Used for communication with the ui thread. It is a box
     * where one of the threads puts some YCP value for the other.
     */
    YCPValue box_in_the_middle;

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
     * callback handler evaluate function
     */
    Y2Component *callbackComponent;

    /**
     * The last menu selection.
     * Only valid if userInput() or pollInput() returned ET_MENU.
     */
    YCPValue menuSelection;
};

#endif // YUIInterpreter_h
