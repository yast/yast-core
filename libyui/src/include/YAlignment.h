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
 * @short Implemenation of the Left, Right, Bottom, Top, HCenter, VCenter and
 * HVCenter widgets 
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
    YAlignment( const YWidgetOpt & opt, YAlignmentType halign, YAlignmentType valign );

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
    
    /**
     * Returns the preferred size of this widget, taking margins into account.
     *
     * Reimplemented from YContainerWidget.
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    long nicesize( YUIDimension dim );

    /**
     * Returns the left margin in pixels, the distance between the left edge of
     * this alignment and the left edge of the child widget. 
     **/
    long leftMargin()	const	{ return _leftMargin;	}

    /**
     * Returns the right margin in pixels, the distance between the right edge
     * of this alignment and the right edge of the child widget. 
     **/
    long rightMargin()	const	{ return _rightMargin;	}

    /**
     * Returns the top margin in pixels, the distance between the top edge of
     * this alignment and the top edge of the child widget. 
     **/
    long topMargin()	const	{ return _topMargin;	}

    /**
     * Returns the bottom margin in pixels, the distance between the bottom
     * edge of this alignment and the bottom edge of the child widget. 
     **/
    long bottomMargin() const	{ return _bottomMargin; }

    /**
     * Returns the sum of all margins in the specified dimension.
     **/
    long totalMargins( YUIDimension dim ) const;

    /**
     * Set the left margin in pixels.
     **/ 
    void setLeftMargin( long margin ) { _leftMargin = margin; }
    
    /**
     * Set the right margin in pixels.
     **/ 
    void setRightMargin( long margin ) { _rightMargin = margin; }
    
    /**
     * Set the top margin in pixels.
     **/ 
    void setTopMargin( long margin ) { _topMargin = margin; }
    
    /**
     * Set the bottom margin in pixels.
     **/ 
    void setBottomMargin( long margin ) { _bottomMargin = margin; }

    /**
     * Returns the minimum width of this alignment or 0 if none is set.
     * nicesize() will never return less than this value.
     **/
    long minWidth() const { return _minWidth; }

    /**
     * Returns the minimum height of this alignment or 0 if none is set.
     * nicesize() will never return less than this value.
     **/
    long minHeight() const { return _minHeight; }

    /**
     * Set the minimum width to return for nicesize().
     **/
    void setMinWidth( long width ) { _minWidth = width; }
    
    /**
     * Set the minimum height to return for nicesize().
     **/
    void setMinHeight( long height ) { _minHeight = height; }


protected:

    
    // Data members
    
    YAlignmentType align[ YUIAllDimensions ];

    long _leftMargin;
    long _rightMargin;
    long _topMargin;
    long _bottomMargin;

    long _minWidth;
    long _minHeight;
};


#endif // YAlignment_h

