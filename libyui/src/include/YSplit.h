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

  File:		YSplit.h

  Author:	Stefan Hundhammer <sh@suse.de>
		( complete rewrite 09/2000)

/-*/

#ifndef YSplit_h
#define YSplit_h


#include "YContainerWidget.h"

/**
 * @short Implementation of the Split widget.
 */
class YSplit : public YContainerWidget
{
public:
    typedef vector<long>sizeVector;
    typedef vector<long>posVector;

    /**
     * Creates a new YSplit
     */
    YSplit( YWidgetOpt & opt, YUIDimension newPrimaryDimension );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YSplit"; }

    /**
     * Returns the dimensions.
     */
    YUIDimension dimension( ) const		{ return primary; }
    YUIDimension primaryDimension( ) const	{ return primary; }
    YUIDimension secondaryDimension( ) const	{ return secondary; }

    /**
     * Minimum size the widget should have to make it look and feel
     * nice, i.e. all of the widget's preferred size.
     *
     * For the "primary" dimension, this is the sum of the children's
     * nice sizes with respect to any specified weight ratios -
     * i.e. the weights will always be respected. Children may be
     * stretched as appropriate.
     *
     * For the "other" dimension, this is the maximum of the children's
     * nice sizes in that dimension.
     *
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    virtual long nicesize( YUIDimension dimension );

    /**
     * The split is stretchable if one of the children is stretchable in
     * that dimension.
     * @param dim Dimension, either YD_HORIZ or YD_VERT
     */
    virtual bool stretchable( YUIDimension dimension );

    /**
     * Sets the size of the Split. Here the layout policy
     * is implemented. The ui specific widget must call this
     * method at the end of it's own setSize method.
     */
    void setSize( long newwidth, long newheight);

    /**
     * Moves a child to a new position
     */
    virtual void moveChild( YWidget *child, long newx, long newy) = 0;


protected:

    /**
     * Add up all the children's weights.
     */
    long childrenTotalWeight( YUIDimension dimension );

    /**
     * Return the maximum nice size of all children in dimension d.
     */
    long childrenMaxNiceSize( YUIDimension dimension );

    /**
     * Add up all the non-weighted children's nice sizes in dimension d.
     */
    long totalNonWeightedChildrenNiceSize( YUIDimension dimension );

    /**
     * Count the number of non-weighted children.
     */
    int countNonWeightedChildren( YUIDimension dimension );

    /**
     * Count the number of stretchable ( non-weighted) children.
     * Note: Weighted children are _always_ considered stretchable.
     */
    int countStretchableChildren( YUIDimension dimension );

    /**
     * Count the number of "rubber bands", i.e. the number of
     * stretchable layout spacings ( e.g. {H|V}Weight,
     * {H|V}Spacing). Only those without a weight are counted.
     */
    int countLayoutStretchChildren( YUIDimension dimension );

    /**
     * Determine the number of the "boss child" - the child widget that
     * determines the overall size with respect to its weight.
     * Returns -1 if there is no boss, i.e. none of the children has a
     * weight specified.
     */
    int bossChild( );

    /**
     * Calculate the sizes and positions of all children in the primary
     * dimension and store them in "childSize" and "childPos".
     */
    void calcPrimaryGeometry	( long		newSize,
				  sizeVector &	childSize,
				  posVector  &	childPos );

    /**
     * Calculate the sizes and positions of all children in the secondary
     * dimension and store them in "childSize" and "childPos".
     */
    void calcSecondaryGeometry	( long		newSize,
				  sizeVector &	childSize,
				  posVector  &	childPos );

    /**
     * Actually perform resizing and moving the child widgets to the
     * appropriate position.
     *
     * The vectors passed are the sizes previously calculated by
     * calcPrimaryGeometry( ) and calcSecondaryGeometry( ).
     */
    void doResize( sizeVector & width,
		   sizeVector & height,
		   posVector  & x_pos,
		   posVector  & y_pos  );


protected:

    /**
     * Dimensions of the split. YD_HORIZ or YD_VERT
     */
    YUIDimension primary;
    YUIDimension secondary;
    bool debugLayout;
};


#endif // YSplit_h
