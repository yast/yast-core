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

  File:	      YSpacing.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YSpacing_h
#define YSpacing_h

#include "YWidget.h"

/**
 *  @short Implementation of the HSpacing and VSpacing and widgets.
 */
class YSpacing : public YWidget
{
public:

    /**
     * Constructor
     * @param size the relative size of this widget
     * @param horizontal flag: use "size" for the horizontal dimension?
     * @param vertical flag: use "size" for the vertical dimension?
     */
    YSpacing( YWidgetOpt & opt, float size, bool horizontal, bool vertical);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YSpacing"; }

    /**
     * Minimum size the widget should have to make it look and feel
     * nice. This is 0 for the empty widget.
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    long nicesize( YUIDimension dim);

    /**
     * Returns true if this is a layout stretch space in dimension "dim".
     * Such widgets will receive special treatment in layout calculations.
     * Inherited from YWidget.
     */
    bool isLayoutStretch( YUIDimension dim ) const;

    /**
     * Convert a relative size in the given dimension in units actually
     * used by the respective UI ( pixels or characters).
     * Overwrite this method to round sizes < 1.0 to zero or to make
     * sure a widget gets at least one unit ( e.g. pixel) in any
     * direction if the UI cannot handle zero sizes.
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     * @param relativeSize size as passed as widget parameter
     */
    virtual long absoluteSize( YUIDimension dim, float relativeSize);


protected:
    /**
     * Relative size in both dimensions. The virtual absoluteSize( )
     * method is used in order to determine the real size in units used
     * by the UI.
     */
    float relativeSize[YUIAllDimensions];
};


#endif // YSpacing_h
