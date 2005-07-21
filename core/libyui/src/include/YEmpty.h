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

  File:	      YEmpty.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

// -*- c++ -*-

#ifndef YEmpty_h
#define YEmpty_h

#include "YWidget.h"

/**
 *  @short Implementation of the Empty, HStretch, VStretch, and HVStretch widgets
 */
class YEmpty : public YWidget
{
public:
    /**
     * Constructor
     */
    YEmpty( const YWidgetOpt & opt );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YEmpty"; }

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

};


#endif // YEmpty_h
