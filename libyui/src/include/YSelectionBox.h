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

  File:	      YSelectionBox.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YSelectionBox_h
#define YSelectionBox_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>

class YMacroRecorder;

/**
 * @short Implementation of the SelectionBox widget.
 */
class YSelectionBox : public YWidget
{
public:

    /**
     * Constructor
     * @param text the initial text of the SelectionBox label
     * @param opt the widget options
     */
    YSelectionBox(YWidgetOpt &opt, YCPString label);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YSelectionBox"; }

    /**
     * Adds an item to the selection box.
     */
    void addItem(const YCPValue &id, const YCPString & text, bool selected);

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget(const YCPSymbol & property, const YCPValue & newvalue);

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget(const YCPSymbol & property);

    /**
     * Change the label text. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel(const YCPString & label);

    /**
     * Get the current label text. This method cannot be overidden.  The value
     * of the label cannot be changed other than by calling setLabel, i.e. not
     * by the ui. Therefore setLabel stores the current label in #label.
     */
    YCPString getLabel();

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }
    

protected:
    /**
     * Called when an item has been added. Overload this to
     * fill the ui specific widget with items.
     * @param string text of the new item
     * @param index index of the new item.
     * @param selected true if the item should be selected.
     */
    virtual void itemAdded(const YCPString & string, int index, bool selected);

    /**
     * Returns the index of the currently
     * selected item or -1 if no item is selected.
     */
    virtual int getCurrentItem() = 0;

    /**
     * Selects an item from the list.
     */
    virtual void setCurrentItem(int index) = 0;

    /**
     * Returns the current number of items
     */
    int numItems() const;

    /**
     * Searches for an item with a certain id or a certain label.
     * Returns the index of the found item or -1 if none was found
     * @param report_error set this to true, if you want me to
     * report an error if non item can be found.
     */
    int itemWithId(const YCPValue &id, bool report_error);

    /**
     * The current label of the selectionbox
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

private:
    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );
};


#endif // YSelectionBox_h
