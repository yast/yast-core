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

  File:	      YRadioButtonGroup.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YRadioButtonGroup_h
#define YRadioButtonGroup_h

#include "YContainerWidget.h"

class YRadioButton;

/**
 * @short Implementation of the RadioButtonGroup widget
 */
class YRadioButtonGroup : public YContainerWidget
{
public:
    /**
     * Creates a new and empty radio button group.
     */
    YRadioButtonGroup( YWidgetOpt & opt );

    /**
     * Cleans up
     */
    virtual ~YRadioButtonGroup();

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YRadioButtonGroup"; }

    /**
     * Returns true, since this is a radio button group
     */
    bool isRadioButtonGroup() const;

    /**
     * Adds a radio button to the button group. If you overload
     * this function ui specific, call YRadioButtonGroup::addRadioButton()
     * as well!
     */
    virtual void addRadioButton( YRadioButton *button );

    /**
     * Removes a radio button from the button group. If you overload
     * this function ui specific, call YRadioButtonGroup::removeRadioButton()
     * as well! Don't delete the removed radio button in this function!
     * It's not ours. We just have a pointer to it. It's parent widget
     * will deletes it when neccessary.
     */
    virtual void removeRadioButton( YRadioButton *button );

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Unchecks all radio buttons except one. This method
     * can be used by an actual ui ( for example ncurses )
     * in the implementation of setValue() of the Radiobutton
     */
    void uncheckOtherButtons( const YRadioButton *radiobutton );


protected:
    /**
     * Find the currently selected button
     */
    YRadioButton *YRadioButtonGroup::currentButton() const;

    /**
     * Make another of the buttons currently selected
     */
    bool setCurrentButton( const YCPValue & id );


    typedef vector <YRadioButton *> buttonlist_type;

    /**
     * List of all RadioButtons contained in this group. The buttons
     * do not _live_ here, so we don't delete them after use.
     */
    buttonlist_type buttonlist;

};


#endif // YRadioButtonGroup_h
