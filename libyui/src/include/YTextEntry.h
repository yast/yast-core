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

  File:	      YTextEntry.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YTextEntry_h
#define YTextEntry_h

#include "YWidget.h"
#include <ycp/YCPString.h>

class YMacroRecorder;

/**
 * @short Implementation of the TextEntry and Password widgets
 * Notice: Derived classes need to check opt.passwordMode!
 */
class YTextEntry : public YWidget
{
public:
    /**
     * Creates a new text entry with a label and an initial text.
     */
    YTextEntry( YWidgetOpt & opt, const YCPString & label);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YTextEntry"; }


    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue);

    /**
     * Implements the ui command changeWidget for the widget specific properties.
     */
    YCPValue queryWidget( const YCPSymbol & property);

    /**
     * Set the text in the entry to a new value
     */
    virtual void setText( const YCPString & text) = 0;

    /**
     * get the text currently entered in the text entry
     */
    virtual YCPString getText( ) = 0;

    /**
     * change the label of the text entry. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label);

    /**
     * Get the current label of the text entry. This method cannot be
     * overidden.  The value of the label cannot be changed other than by
     * calling setLabel, i.e. not by the ui. Therefore setLabel stores the
     * current label in #label.
     */
    YCPString getLabel( );

    /**
     * Change the valid input characters.
     *
     * Overload this, but call YTextEntry::setValidChars at the end of your own
     * method.
     */
    virtual void setValidChars( const YCPString & validChars);

    /**
     * Get the valid input characters.
     */
    YCPString getValidChars( );

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty( ) { return YUIProperty_Label; }
    
    
protected:

    /**
     * The text entry label
     */
    YCPString label;

    /**
     * Valid input characters
     */
    YCPString validChars;


private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );
};


#endif // YTextEntry_h
