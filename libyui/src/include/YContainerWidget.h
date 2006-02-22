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

  File:	      YContainerWidget.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YContainerWidget_h
#define YContainerWidget_h

#include <ycp/YCPMap.h>
#include "YWidget.h"

/**
 * @short Base class of all widgets that have child widgets
 *
 * A Container is a widget that has child widgets. Examples are @ref YSplit,
 * @ref YReplacePoint, @ref YAlignment.  This class generically handles the
 * houskeeping of child widgets, looks for children with certain IDs and
 * provides default implementations for nicesize, stretchable and
 * weight for container widgets with exactly one child.
 */
class YContainerWidget : public YWidget
{
public:
    /**
     * Constructor
     */
    YContainerWidget( const YWidgetOpt & opt );

    /**
     * Cleans up: Deletes all child widgets.
     */
    virtual ~YContainerWidget();

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YContainerWidget"; }

    /**
     * Returns 'true' if there are any child widgets.
     **/
    bool hasChildren() const;

    /**
     * Returns the number of child widgets.
     */
    int numChildren() const;

    /**
     * Returns one of the child widgets.
     */
    YWidget *child( int i ) const;

    /**
     * Default implementation, assuming exactly one child.
     * Returns the nicesize of the child.
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    long nicesize( YUIDimension dim );

    /**
     * Default implementation, that assumes exactly one child.
     * Returns, whether the child is stretchable
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    bool stretchable( YUIDimension dim ) const;

    /**
     * Default implementation, that assumes exactly one child.
     * Sets the size of the child.
     */
    void setSize( long newwidth, long newheight );

    /**
     * Adds a new child widget. The YContainerwidget assumes ownership and
     * takes care of deleting the child.
     */
    virtual void addChild( YWidget *child );

    /**
     * Removes and deletes all child widgets. Calls childRemoved()
     * for each child just before it is being removed in order
     * to inform the derived ui specific widget.
     */
    void removeChildren();

    /**
     * Looks for a child widget with a certain ID. Searches
     * recursively.
     * @return A pointer to the found widget or 0 if non has been found
     */
    YWidget *findWidget( const YCPValue & id ) const;

    /**
     * Returns true, since this is a container widget.
     */
    bool isContainer() const;

    /**
     * Child deletion notification. See YWidget.h for details.
     */
    virtual void childDeleted( YWidget *child );


protected:
    /**
     * Call back function that reports to the ui specific
     * widget that a child has been added. The default implementation
     * does nothing.
     */
    virtual void childAdded( YWidget *child );

    /**
     * Call back function that reports to the ui specific
     * widget that a child has been removed. The default implementation
     * does nothing.
     */
    virtual void childRemoved( YWidget *child );


public:
    /**
     * Debugging function:
     * Dump the widget tree from here on to the log file.
     */
    void dumpWidgetTree( int indentationLevel = 0 );

    /**
     * Debugging function:
     * Dump the widget tree from this widget's dialog parent.
     * If there is no such dialog parent, dump the widget tree from
     * here on.
     */
    void dumpDialogWidgetTree();

    /**
     * Returns a (possibly translated) text describing this dialog for
     * debugging.
     **/
    virtual std::string debugLabel();

    /**
     * Recursively save the user input of all child widgets
     * to a macro recorder:
     *
     * All child widgets that could contain data entered by the user
     * are requested to send their contents to the macro recorder, e.g. input
     * fields, check boxes etc.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );

    /**
     * (Recursively) collect the content of all input fields or other
     * interactive widgets in this container and its children and add them to
     * 'fieldContents'.
     **/
    virtual void collectUserInput( YCPList & fieldContents );


protected:

    /**
     * Helper function for dumpWidgetTree():
     * Dump one widget to the log file.
     */
    void dumpWidget( YWidget *w, int indentationLevel );

    /**
     * Format a debug label.
     **/
    string formatDebugLabel( YWidget * widget, const string & debLabel );

    //
    // Data members
    //

    vector<YWidget *> 	children;
    YWidget *		_debugLabelWidget;
};


#endif // YContainerWidget_h

