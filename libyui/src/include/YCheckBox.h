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

  File:	      YCheckBox.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YCheckBox_h
#define YCheckBox_h

#include "YWidget.h"
#include <ycp/YCPString.h>


class YMacroRecorder;

/**
 * @short Implementation of the CheckBox widget
 */
class YCheckBox : public YWidget
{
public:
    /**
     * Constructor
     */
    YCheckBox(YWidgetOpt &opt, const YCPString & label);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YCheckBox"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget(const YCPSymbol & property, const YCPValue & newvalue);

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget(const YCPSymbol & property);

    /**
     * Set the text in the entry to a new value
     */
    virtual void setValue(const YCPValue & checked) = 0;

    /**
     * Returns whether the checkbox is checked.
     * This may return 'true' or 'false' or 'nil' for a tristate check box.
     */
    virtual YCPValue getValue() = 0;

    /**
     * Change the check box label. Overload this, but call
     * YCheckBox::setLabel at the end of your own function.
     */
    virtual void setLabel(const YCPString & label);

    /**
     * Get the current check box label. This method cannot be overidden.
     * The value of the label cannot be changed other than by calling setLabel,
     * i.e. not by the ui. Therefore setLabel stores the current label in #label.
     */
    YCPString getLabel();

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }
    
protected:
    /**
     * The CheckBox label
     */
    YCPString label;


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );
};


#endif // YCheckBox_h
