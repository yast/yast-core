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

  File:	      YWidget.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YWidget_h
#define YWidget_h

#include <string>
#include <list>
#include <ycp/YCPValue.h>
#include "YWidgetOpt.h"
#include "YUISymbols.h"

#define YWIDGET_MAGIC		42	// what else? ;- )

class YCPSymbol;
class YMacroRecorder;

#define YUIAllDimensions	2

enum YUIDimension { YD_HORIZ, YD_VERT };


class YWidget;
typedef std::list<YWidget *>			YWidgetList;
typedef std::list<YWidget *>::iterator		YWidgetListIterator;
typedef std::list<YWidget *>::const_iterator	YWidgetListConstIterator;


/**
 * @short Abstract base class of all ui widgets
 */
class YWidget
{
public:
    /**
     * Constructor
     */
    YWidget( YWidgetOpt & opt );

    /**
     * Destructor
     */
    virtual ~YWidget();

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char * widgetClass() { return "YWidget"; }

    /**
     * Returns a descriptive label of this widget instance.
     *
     * This default implementation returns this widget's "shortcut property"
     * (possibly trunctated to avoid over-long texts) - the property that
     * contains the keyboard shortcut used to activate this widget or to move
     * the keyboard focus to it. In most cases this is this widget's label.
     *
     * Note: This is usually translated to the user's target language.
     * This makes this useful for debugging only.
     **/
    virtual std::string debugLabel();

    /**
     * Checks whether or not this object is valid. This is to enable
     * dangling pointer error checking ( i.e. this object is already
     * deallocated, but a pointer to it is still in use ).
     */
    bool isValid() const 	{ return magic == YWIDGET_MAGIC; }

    /**
     * Return the widget serial number ( the internal widget ID ).
     */
    int internalId() const	{ return internal_widget_id; }

    /**
     * Sets the id of the widget
     */
    void setId( const YCPValue & id );

    /**
     * Checks whether or not the widget has an ID
     */
    bool hasId() const;

    /**
     * Gets the id of the widget
     */
    YCPValue id() const;

    /**
     * Set the parent YWidget of this widget.
     */
    void setParent( YWidget *parent );

    /**
     * Return the parent YWidget of this widget.
     */
    YWidget * YWidget::yParent() const;

    /**
     * Return the YDialog this widget belongs to.
     * Returns 0 if there is no dialog parent.
     */
    YWidget * yDialog();

    /**
     * Notify a widget that can have children that a child has been deleted.
     * This is called in the YWidget destructor for the parent widget.
     * All container widgets should overwrite this function and update
     * any internal pointers accordingly.
     *
     * Background: Qt widgets ( and objects inherited from them )
     * automatically delete any child widgets if they are deleted
     * themselves. This conflicts with destructors that delete child widgets.
     */
    virtual void childDeleted( YWidget *child ) {}

    /**
     * Returns true if this is a dialog widget.
     * The default implementation returns 'false'.
     */
    virtual bool isDialog() const;

    /**
     * Returns true if this is a container widget. The default implementation
     * returns false.
     */
    virtual bool isContainer() const;

    /**
     * Returns true if this is a replace point widget. The default implementation
     * returns false.
     */
    virtual bool isReplacePoint() const;

    /**
     * Returns true if this is a button group widget. The default implementation
     * return false.
     */
    virtual bool isRadioButtonGroup() const;

    /**
     * Returns true if this is a layout stretch space in dimension "dim".
     * Such widgets will receive special treatment in layout calculations.
     * The default implementation returns false.
     */
    virtual bool isLayoutStretch( YUIDimension dim );

    /**
     * Minimum size the widget should have to make it look and feel nice. For a
     * push button this would include some space around the text to make it look
     * nice.
     *
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    virtual long nicesize( YUIDimension dim ) = 0;

    /**
     * This is a boolean value that determines whether the widget is resizable
     * beyond its nice size in the specified dimension. A selection box is
     * stretchable in both dimensions, a push button is not stretchable by
     * default, a frame is stretchable if its contents are stretchable. Most
     * widgets accept a `hstretch or `vstretch option to become stretchable even
     * when they are not by default.
     *
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    virtual bool stretchable( YUIDimension dim );

    /**
     * Set the stretchable state to "newStretch" regardless of any `hstretch or
     * `vstretch options.
     */
    void setStretchable( YUIDimension dim, bool newStretch );

    /**
     * Set the stretchable state to "newStretch".
     * `hstretch or `vstretch options may override this.
     */
    void setDefaultStretchable( YUIDimension dim, bool newStretch );


    /**
     * The weight is used in situations where all widgets can get there nice
     * size and yet space is available. The remaining space will be devided
     * between all stretchable widgets according to their weights. A widget with
     * greater weight will get more space. The default weight for all widgets
     * is 0.
     *
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    virtual long weight( YUIDimension dim );

    /**
     * Return whether or not the widget has a weight.
     */
    virtual bool hasWeight( YUIDimension dim );

    /**
     * Sets the new size of the widget. If you overload this function make the
     * underlying ui specific widget have exactly that size. You should _not_
     * change the nicesize, of course. The unit is according to and
     * nicesize(). This function is called from the layout algorithm, which has
     * decides ( using the layout properties ) how large the widget now actually
     * should be.
     *
     * The default implementation does nothing.
     */
    virtual void setSize( long newwidth, long newheight );

    /**
     * Sets the enabled state of the widget. All new widgets are enabled per
     * definition. Only enabled widgets can take user input.
     */
    virtual void setEnabling( bool enabled );

    /**
     * Queries the enabled of a widget.
     */
    bool getEnabling() const;

    /**
     * Sets the Notify property
     */
    void setNotify( bool notify );

    /**
     * Returns whether the widget will notify, i.e. will case UserInput to
     * return.
     */
    bool getNotify() const;

    /**
     * Returns 'true' if this widget should send key events, i.e. if it has
     * `opt(`keyEvent) set. 
     **/
    bool sendKeyEvents() const { return _sendKeyEvents; }

    /**
     * Specify whether or not this widget should send key events.
     **/
    void setSendKeyEvents( bool doSend ) { _sendKeyEvents = doSend; }

    /**
     * Returns 'true' if a keyboard shortcut should automatically be assigned
     * to this widget - without complaints in the log file.
     **/
    bool autoShortcut() const { return _autoShortcut; }

    /**
     * Sets the 'autoShortcut' flag.
     **/
    void setAutoShortcut( bool _newAutoShortcut ) { _autoShortcut = _newAutoShortcut; }

    /**
     * Implements the ui command ChangeWidget. Implement this method in your
     * widget subclass and handle their all properties special to that widget.
     * If you encounter an unknown property, call YWidget::changeWidget
     */
    virtual YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command ChangeWidget in the incarnation, where the
     * property is given in form of a term rather than a symbol. Implement this
     * method in your widget subclass and handle their all properties special to
     * that widget.
     *
     * If you encounter an unknown property, call YWidget::changeWidget
     */
    virtual YCPValue changeWidget( const YCPTerm & property, const YCPValue & newvalue );

    /**
     * Implements the ui command QueryWidget. Implement this method in your
     * widget subclass and handle their all properties special to that widget.
     * If you encounter an unknown property, call YWidget::queryWidget
     */
    virtual YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Implements the ui command QueryWidget in the incarnation, where the property
     * is given in form of a term rather than a symbol. Implement this method in your
     * widget subclass and handle their all properties special to that widget.
     * If you encounter an unknown property, call YWidget::queryWidget
     */
    virtual YCPValue queryWidget( const YCPTerm & property );

    /**
     * Returns a pointer to the ui specific widget implementation.
     */
    void *widgetRep();

    /**
     * Sets the pointer to the ui specific widget implementation.
     */
    void setWidgetRep( void * );

    /**
     * Set the keyboard focus to this widget.
     * The default implementation just emits a warning message.
     * Overwrite this function for all widgets that can accept the
     * keyboard focus.
     *
     * This function returns true if the widget did accept the
     * keyboard focus, and false if not.
     */
    virtual bool setKeyboardFocus();


    /**
     * Save the user input of this widget to a macro recorder, e.g. input field
     * contents, check box state etc.
     *
     * This default implementation does nothing. Overwrite this method in any
     * derived class that has user input to save, e.g. TextEntry etc. and call
     * YMacroRecorder::recordWidgetProperty() in the overwritten method.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );


    /**
     * The name of the widget property that holds the keyboard shortcut, if any.
     * Overwrite this for widgets that can have keyboard shortcuts.
     */
    virtual const char *shortcutProperty() { return ( const char * ) 0; }

    
    // NCurses optimizations

    
    /**
     * In some UIs updating the screen content is an expensive operation. Use
     * startMultipleChanges() to tell the ui that you're going to perform multiple
     * chages to the widget.
     * The UI may delay any screen updates until doneMultipleChanges() is called.
     */
    virtual void startMultipleChanges() {}

    /**
     * In some UIs updating the screen content is an expensive operation. Use
     * startMultipleChanges() to tell the ui that you're going to perform multiple
     * chages to the widget.
     * The UI may delay any screen updates until doneMultipleChanges() is called.
     */
    virtual void doneMultipleChanges() {}


private:

    /**
     * Make this widget invalid. This operation cannot be reversed.
     */
    void invalidate() { magic=0; }

    /**
     * This object is only valid if this magic number is
     * YWIDGET_MAGIC. Use YWidget::isValid() to check this.
     */
    int magic;

protected:

    /**
     * The user provided widget id of this widget -
     * strictly for user purposes. May or may not be set.
     * May or may not be unique.
     */
    YCPValue user_widget_id;

    /**
     * The internal unique widget id of this widget.
     * Used to compare widget identity.
     */
    int internal_widget_id;

    /**
     * The next internal widget id to be used for the next object of
     * this class. Initialized in YWidget.cc.
     */
    static int next_internal_widget_id;

    /**
     * The parent YWidget.
     */
    YWidget *yparent;

    /**
     * Flag: Can this widget currently receive user input?
     */
    bool enabled;

    /**
     * Flag: Make UserInput() return on detailed events?
     **/
    bool notify;

    /**
     * Flag: Make UserInput() return on single key events?
     **/
    bool _sendKeyEvents;

    /**
     * Flag: Automatically assign a keyboard shortcut without complaints in the
     * log file? 
     **/
    bool _autoShortcut;

    /**
     * Pointer to the UI specific widget representation that belongs to this
     * widget. For Qt, this will be a pointer to a QWidget (subclass).
     **/
    void * rep;

    /**
     * Stretchability in both dimensions.
     */
    bool _stretch [ YUIAllDimensions ];

    /**
     * Weight in both dimensions.
     */
    long _weight [ YUIAllDimensions ];

    /**
     * window ID of the current widget
     */
    long windowID;


public:
    /**
     * Helper class that calls startMultipleChanges() in its constructor
     * and cares about the necessary call to doneMultipleChanges() when it goes
     * out of scope.
     */
    class OptimizeChanges
    {
    public:
	OptimizeChanges( YWidget & w ) : yw(w) { yw.startMultipleChanges(); }
	~OptimizeChanges()                       { yw.doneMultipleChanges(); }
    private:
	OptimizeChanges( const OptimizeChanges & ); // no copy
	void operator=( const OptimizeChanges & );  // no assign
	YWidget & yw;
    };

};


#endif // YWidget_h
