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

  File:	      YAlignment.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YAlignment_h
#define YAlignment_h

#include "YContainerWidget.h"

enum YAlignmentType
{
    YAlignUnchanged,
    YAlignBegin,
    YAlignEnd,
    YAlignCenter
};


/**
 * @short Implemenation of the Left, Right, Bottom, Top, HCenter, VCenter and HVCenter widgets
 */
class YAlignment : public YContainerWidget
{
public:
    /**
     * Symbols for the different kinds of alignments. AT_UNCHANGED
     * means that the widget should not be made stretchable and
     * have the default alignment ( centered ).
     */

    /**
     * Constructor
     * @param halign How the child widget is aligned horizontally
     * @param valign How the child widget is aligned vertically
     * @param opt the widget options
     */
    YAlignment( YWidgetOpt & opt, YAlignmentType halign, YAlignmentType valign );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YAlignment"; }

    /**
     * Moves a child widget to a new position.
     */
    virtual void moveChild( YWidget *child, long newx, long newy ) = 0;

    /**
     * In an aligned dimension the widget is always stretchable.
     * In an unchanged dimension the widget is stretchable if the
     * child is stretchable.
     */
    bool stretchable( YUIDimension dim ) const;

    /**
     * Sets the size and move the child widget according to its
     * alignment. The UI specific widget subclass overrides this
     * function in order to resize the ui specific widget, but
     * calls this method at the end.
     */
    void setSize( long newwidth, long newheight );

protected:
    /**
     * Alignment of the contained widget in each dimension
     */
    YAlignmentType align[ YUIAllDimensions ];
};


#endif // YAlignment_h

