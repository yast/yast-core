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

  File:	      YMultiLineEdit.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMultiLineEdit_h
#define YMultiLineEdit_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <deque>


class YMacroRecorder;

/**
 * @short Implementation of the MultiLineEdit widget
 */
class YMultiLineEdit : public YWidget
{
public:
    /**
     * Constructor.
     */
    YMultiLineEdit( const YWidgetOpt &	opt,
		    const YCPString &	label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YMultiLineEdit"; }


    /**
     * Implements the ui command changeWidget for the widget specific
     * properties.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command changeWidget for the widget specific properties.
     */
    YCPValue queryWidget( const YCPSymbol & property );


    /**
     * Set the label above the log lines. Overload this, but call
     * YMultiLineEdit::setLabel at the end of your own function.
     */
    virtual void	setLabel( const YCPString & newLabel );

    /**
     * Set the edited text.
     */
    virtual void setText( const YCPString & text ) = 0;

    /**
     * Get the edited text.
     */
    virtual YCPString text() = 0;

    /**
     * Get the label above the log lines.
     */
    YCPString label() const { return _label; }

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }
    

private:

    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );


    // Data members

    YCPString _label;
};


#endif // YMultiLineEdit_h
