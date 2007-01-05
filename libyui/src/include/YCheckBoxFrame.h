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

  File:	      YCheckBoxFrame.h

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YCheckBoxFrame_h
#define YCheckBoxFrame_h

#include <ycp/YCPString.h>
#include "YContainerWidget.h"

/**
 * @short Implementation of the Frame widget
 */
class YCheckBoxFrame : public YContainerWidget
{
public:
    /**
     * Constructor
     */
    YCheckBoxFrame( const YWidgetOpt & opt, const YCPString & label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YCheckBoxFrame"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the Frame label. Overload this, but call
     * YCheckBoxFrame::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

    /**
     * Get the current label.
     */
    YCPString getLabel();

    /**
     * Set the check box on or off (check or uncheck it).
     */
    virtual void setValue( bool checked ) = 0;

    /**
     * Returns whether the checkbox is checked.
     */
    virtual bool getValue() = 0;

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }

    /**
     * The name of the widget property that will return user input.
     * Inherited from YWidget.
     **/
    const char *userInputProperty() { return YUIProperty_Value; }

    /**
     * Returns 'true' if autoEnable is on, i.e. if the frame content (the child
     * widgets) should automatically be enabled if this FrameCheckBox is set to
     * "on" (is checked).
     **/
    bool autoEnable() const { return _autoEnable; }

    /**
     * Returns 'true' if the behaviour of autoEnable is inverted, i.e. if the
     * frame content (the child widgets) should automatically be disabled if
     * this FrameCheckBox is set to "on" (is checked) and vice versa.
     **/
    bool invertAutoEnable() const { return _invertAutoEnable; }


protected:

    /**
     * Handle enabling/disabling of child widgets based on 'isChecked' (the
     * current status of the check box) and autoEnable() and
     * invertAutoEnable().
     *
     * Derived classes should call this when the check box status changes
     * rather than try to handle it on their level.
     **/
    void handleChildrenEnablement( bool isChecked );


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );

    //
    // Data members
    //

    YCPString   _label;
    bool	_value;
    bool	_autoEnable;
    bool	_invertAutoEnable;
};


#endif // YCheckBoxFrame_h

