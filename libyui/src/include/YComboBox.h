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

#include "YSelectionWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>

class YMacroRecorder;

/**
 * @short Implementation of the ComboBox widget
 */
class YComboBox : public YSelectionWidget
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
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

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
     * Specify the amount of characters which can be inserted.
     *
     * Overload this to limit the input.
     */
    virtual void setInputMaxLength( const YCPInteger & numberOfChars );
    
    /**
     * The name of the widget property that will return user input.
     * Inherited from YWidget.
     **/
    const char *userInputProperty() { return YUIProperty_Value; }


protected:
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
     * Returns the index of the currently selected item (from 0 on)
     * or -1 if no item is selected.
     **/
    virtual int getCurrentItem() const = 0;

    /**
     * Selects an item from the list. Notice there intentionally is no
     * corresponding getCurrentItem() method - use getValue() instead.
     */
    virtual void setCurrentItem( int index ) = 0;

private:
    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );

    /**
     * Valid input characters
     */
    YCPString validChars;

    /**
     * Any input ( not only from the list ) permitted?
     */
    bool _editable;
};


#endif // YComboBox_h
