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
     * @param size the relative size of this widget in logical layout units (normalized to 80x25)
     * @param horizontal flag: use "size" for the horizontal dimension?
     * @param vertical flag: use "size" for the vertical dimension?
     */
    YSpacing( const YWidgetOpt & opt, float layoutUnits, bool horizontal, bool vertical );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YSpacing"; }

    /**
     * Minimum size the widget should have to make it look and feel
     * nice. This is 0 for the empty widget.
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    long nicesize( YUIDimension dim );

    /**
     * Returns true if this is a layout stretch space in dimension "dim".
     * Such widgets will receive special treatment in layout calculations.
     * Inherited from YWidget.
     */
    bool isLayoutStretch( YUIDimension dim ) const;

    /**
     * Width in device dependent units (pixels or character cells)
     **/
    long width() const { return _size[ YD_HORIZ ]; }
    
    /**
     * Height in device dependent units (pixels or character cells)
     **/
    long height() const { return _size[ YD_VERT ]; }

    
private:
    
    long _size[ YUIAllDimensions ];
};


#endif // YSpacing_h
