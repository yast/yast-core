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

  File:	      YPushButton.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YPushButton_h
#define YPushButton_h

#include "YWidget.h"
#include <ycp/YCPString.h>

/**
 * @short Implementation of the PushButton widget
 * Derived classes need to check opt.isDefaultButton!
 */
class YPushButton : public YWidget
{
public:
    /**
     * Creates a new YPushButton
     * @param label the button label
     * @param opt widget options
     */
    YPushButton( const YWidgetOpt & opt, YCPString label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YPushButton"; }

    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * change the label of the push button. Overload this, but call
     * YPushButton::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label of the text entry. This method cannot be overidden.
     * The value of the label cannot be changed other than by calling setLabel,
     * i.e. not by the ui. Therefore setLabel stores the current label in
     * #label.
     */
    YCPString getLabel();
    
    /**
     * Set this button's icon from an icon file in the UI's default icon directory.
     * Clear the icon if the name is empty.
     *
     * This default implementation does nothing.
     * UIs that can handle icons can choose to overwrite this method.
     **/
    virtual void setIcon( const YCPString & icon_name ) {}

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }


protected:
    /**
     * The button label
     */
    YCPString label;
};


#endif // YPushButton_h
