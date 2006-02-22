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

#include "YSelectionWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>

class YMacroRecorder;

/**
 * @short Implementation of the SelectionBox widget.
 */
class YSelectionBox : public YSelectionWidget
{
public:

    /**
     * Constructor
     * @param text the initial text of the SelectionBox label
     * @param opt the widget options
     */
    YSelectionBox( const YWidgetOpt & opt, YCPString label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YSelectionBox"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );
    
    /**
     * The name of the widget property that will return user input.
     * Inherited from YWidget.
     **/
    const char *userInputProperty() { return YUIProperty_CurrentItem; }
    

protected:
    /**
     * Returns the index of the currently
     * selected item or -1 if no item is selected.
     */
    virtual int getCurrentItem() = 0;

	/**
     * Selects an item from the list.
     */
    virtual void setCurrentItem( int index ) = 0;


private:
    /**
     * Save the widget's user input to a macro recorder.
     * Intentionally declared as "private" so all macro recording internals are
     * handled by the abstract libyui level, not by a specific UI.
     */
    virtual void saveUserInput( YMacroRecorder *macroRecorder );
};


#endif // YSelectionBox_h
