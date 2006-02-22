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

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YMultiSelectionBox_h
#define YMultiSelectionBox_h

#include "YSelectionWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPVoid.h>

class YMacroRecorder;

/**
 * @short Implementation of the MultiSelectionBox widget.
 */
class YMultiSelectionBox : public YSelectionWidget
{
public:

    /**
     * Constructor
     * @param text the initial text of the MultiSelectionBox label
     * @param opt the widget options
     */
    YMultiSelectionBox( const YWidgetOpt & opt, YCPString label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YMultiSelectionBox"; }

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
    const char *userInputProperty() { return YUIProperty_SelectedItems; }
    
    
protected:
    /**
     * Check whether or not item #index is selected.
     *
     * Reimplement this in derived classes!
     */
    virtual bool itemIsSelected( int index ) = 0;

    /**
     * Select item #index.
     *
     * Reimplement this in derived classes!
     */
    virtual void selectItem( int index ) = 0;

    /**
     * Deselect all items.
     *
     * Reimplement this in derived classes!
     */
    virtual void deselectAllItems() = 0;

    /**
     * Returns the index of the item that currently has the keyboard focus.
     *
     * Reimplement this in derived classes!
     */
    virtual int getCurrentItem() = 0;

    /**
     * Set the keyboard focus to one item.
     *
     * Reimplement this in derived classes!
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


#endif // YMultiSelectionBox_h
