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

  File:	      YComboBox.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YComboBox_h
#define YComboBox_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>

class YMacroRecorder;

/**
 * @short Implementation of the ComboBox widget
 */
class YComboBox : public YWidget
{
public:
    /**
     * Constructor
     */
    YComboBox( const YWidgetOpt & opt, YCPString label );

    /**
     * Returns whether or not any value ( not only from the list ) can be
     * entered.
     */
    bool editable() const { return _editable; }

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YComboBox"; }

    /**
     * Adds an item to the selection box.
     */
    void addItem( const YCPValue & id, const YCPString & text, bool selected );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the label text. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label text. This method cannot be overidden.
     * The value of the label cannot be changed other than by calling setLabel,
     * i.e. not by the ui. Therefore setLabel stores the current label in #label.
     */
    YCPString getLabel();

    /**
     * Change the valid input characters.
     *
     * Overload this, but call YComboBox::setValidChars at the end of your own
     * method.
     */
    virtual void setValidChars( const YCPString & validChars );

    /**
     * Get the valid input characters.
     */
    YCPString getValidChars();

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }
    

protected:
    /**
     * Is called, when an item has been added. Overload this to
     * fill the ui specific widget with items.
     * @param string text of the new item
     * @param index index of the new item.
     * @param selected true if the item should be selected.
     */
    virtual void itemAdded( const YCPString & string, int index, bool selected );

    /**
     * Returns the ComboBox value.
     */
    virtual YCPString getValue() const = 0;

    /**
     * Sets the ComboBox value to a random value that is not already in
     * the item list. Will be called for editable ComboBox widgets only.
     */
    virtual void setValue( const YCPString & new_value ) = 0;

    /**
     * Selects an item from the list. Notice there intentionally is no
     * corresponding getCurrentItem() method - use getValue() instead.
     */
    virtual void setCurrentItem( int index ) = 0;


private:
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
    int itemWithId( const YCPValue & id, bool report_error );

    /**
     * Looks up the ID of a given list item's value.
     */
    YCPValue IdForValue( const YCPValue & val );

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );

    /**
     * The current label of the selectionbox
     */
    YCPString label;

    /**
     * Valid input characters
     */
    YCPString validChars;

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
     * Any input ( not only from the list ) permitted?
     */
    bool _editable;
};


#endif // YComboBox_h
