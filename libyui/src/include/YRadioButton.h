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

  File:	      YRadioButton.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YRadioButton_h
#define YRadioButton_h

#include "YWidget.h"
#include <ycp/YCPString.h>

class YRadioButtonGroup;
class YMacroRecorder;

/**
 * @short Implementation of the RadioButton widget
 */
class YRadioButton : public YWidget
{
public:
    /**
     * Creates a new text entry with a label and an initial text. Enters it into
     * the radio button group rbg.
     */
    YRadioButton( YWidgetOpt & opt, const YCPString & label, YRadioButtonGroup *rbg);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YRadioButton"; }

    /**
     * Cleans up. Removes the button from the radio button group
     */
    virtual ~YRadioButton( );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue);

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property);

    /**
     * Set the text in the entry to a new value
     */
    virtual void setValue( const YCPBoolean & checked) = 0;

    /**
     * get the text currently entered in the text entry
     */
    virtual YCPBoolean getValue( ) = 0;

    /**
     * change the label of the text entry. Overload this, but call
     * YRadioButton::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label);

    /**
     * Get the current label of the text entry. This method cannot be overidden.
     * The value of the label cannot be changed other than by calling setLabel,
     * i.e. not by the ui. Therefore setLabel stores the current label in #label.
     */
    YCPString getLabel( );

    /**
     * This function is called from @ref YRadioButtonGroup#~YRadioButtonGroup and
     * tells that the pointer to the radiobuttongroup is not longer valid.
     */
    void buttonGroupIsDead( );

    /**
     * Get a pointer to the radio button group this button belongs to.
     */
    YRadioButtonGroup *buttonGroup( );

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty( ) { return YUIProperty_Label; }
    

protected:
    /**
     * The CheckBox label
     */
    YCPString label;

    /**
     * The radio button group this button belongs to
     */
    YRadioButtonGroup *radiobuttongroup;

private:
    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );
};


#endif // YRadioButton_h
