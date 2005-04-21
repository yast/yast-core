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

  File:	      YSelectionWidget.h

  Author:     Holger Macht <hmacht@suse.de>

/-*/

#ifndef YSelectionWidget_h
#define YSelectionWidget_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>


/**
 * @Base class for all selection widgets like SelectionBox, Combobox and
 * MultiselectionBox
 */
class YSelectionWidget : public YWidget
{
public:

    /**
     * Constructor
     * @param text the initial text of the SelectionWidget label
     * @param opt the widget options
     */
    YSelectionWidget( const YWidgetOpt & opt, YCPString label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YSelectionWidget"; }

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    virtual const char *shortcutProperty() { return YUIProperty_Label; }

    /**
     * parses a given itemlist and calls addItem(...) to insert entries
     */
    int parseItems( const YCPList & itemlist );

    
protected:
 
    /**
     * Called when an item has been added. Overload this to
     * fill the ui specific widget with items.
     * @param string text of the new item
     * @param index index of the new item.
     * @param selected true if the item should be selected.
     */
    virtual void itemAdded( const YCPString & string, int index, bool selected );

    /**
     * Searches for an item with a certain id or a certain label.
     * Returns the index of the found item or -1 if none was found
     * @param report_error set this to true, if you want me to
     * report an error if non item can be found.
     */
    int itemWithId( const YCPValue & id, bool report_error );

    /**
     * The current label of the selectionWidget
     */
    YCPString label;

    /**
     * The current list of item ids. We make destructive changes to
     * this variable, so make sure only one reference to it exists!
     */
    YCPList item_ids;

    /**
     * The current list of item labels. We make destructive changes to
     * this variable, so make sure only one reference to it exists!
     */
    YCPList item_labels;

    /**
     * Returns the current number of items
     */
    int numItems() const;

    /**
     * Change the label text. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label text. This method cannot be overidden.  The value
     * of the label cannot be changed other than by calling setLabel, i.e. not
     * by the ui. Therefore setLabel stores the current label in #label.
     */
    YCPString getLabel();

    /**
     * Adds an item to the box.
     */
    virtual void addItem( const YCPValue & id, const YCPString & text, bool selected );
  
    /**
     * Cleares the two lists item_ids and item_labels. This function is
     * calles out of the corresponding YQ classes.
     */
    virtual void deleteAllItems();
    
    
    /**
     * Changes the widgets label. Is used in every changeWidget(...)
     * function in derived classes.
     */
    virtual YCPValue changeLabel( const YCPValue & newValue );
    
    /**
     * Changes the widgets displayed items. Is used in every
     * changeWidget(...)  function in derived classes.
     */
    virtual YCPValue changeItems ( const YCPValue & newValue );

};


#endif // YSelectionWidget_h
