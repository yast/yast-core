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

  File:		YSplit.cc

  Author:	Stefan Hundhammer <sh@suse.de>
		( complete rewrite 09/2000 )

/-*/


#define y2log_component "ui-layout"
#include <ycp/y2log.h>

#include "YSplit.h"


#ifdef max
#undef max
#endif
#define max( a, b ) ( (a) >? (b) )


YSplit::YSplit( YWidgetOpt & opt, YUIDimension dimension )
    : YContainerWidget( opt )
    , primary( dimension )
    , secondary( dimension == YD_HORIZ ? YD_VERT : YD_HORIZ )
{
    debugLayout = opt.debugLayoutMode.value();

    if ( debugLayout )
    {
	y2debug( "YSplit: Layout debugging enabled" );
    }
}


long YSplit::nicesize( YUIDimension dimension )
{
    if ( dimension != primary )	// the easy case first: secondary dimension
    {
	return childrenMaxNiceSize( dimension );
    }
    else
    {
	/*
	 * In the primary dimension things are much more complicated: We want to
	 * honor any weights specified under all circumstances.  So we first
	 * need to determine the "boss widget" - the widget that determines the
	 * overall size with respect to its weight in that dimension. Once we
	 * know that, we need to stretch all other weighted children accordingly
	 * so the weight ratios are respected.
	 *
	 * As a final step, the nice sizes of all children that don't have
	 * a weight attached are summed up.
	 */

	long size = 0L;

	// Search for the boss
	int boss = bossChild();

	if ( boss > -1 )	// is there a boss child? ( 0 would be valid! )
	{
	    // Calculate size of all weighted widgets.
	    // The boss decides this size - that's why he is called the boss.

	    size = child( boss )->nicesize( primary )
		* childrenTotalWeight( primary )
		/ child( boss )->weight( primary );

	    // maintain this order of calculation in order to minimize integer
	    // rounding errors!
	}


	// Add up the size of all non-weighted children;
	// they will get their respective nice size.

	size += totalNonWeightedChildrenNiceSize( primary );

	return size;
    }
}


/*
 * Search for the "boss child" widget.
 *
 * This is the widget that determines the overall size of the
 * container with respect to all children's weights: It is the child
 * with the maximum ratio of nice size and weight. All other
 * weighted children need to be stretched accordingly so the weight
 * ratios can be maintained.
 *
 * If there is no boss ( i.e. there are only non-weighted children ),
 * -1 will be returned.
 */

int YSplit::bossChild()
{
    int	  boss		= -1;
    double bossRatio	= 0.0;
    double ratio;

    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( child(i)->weight( primary ) != 0 )	// avoid division by zero
	{
	    ratio = ( ( double ) child(i)->nicesize( primary ) )
		/ child(i)->weight( primary );

	    if ( ratio > bossRatio ) // we have a new boss
	    {
		boss = i;
		bossRatio = ratio;
	    }
	}
    }


    if ( debugLayout )
    {
	if ( boss >= 0 )
	{
	    y2debug( "YSplit::boss(): "
		     "Found boss child: #%d ( %s ) - nice size: %ld, weight: %ld",
		     boss,
		     child( boss )->widgetClass(),
		     child( boss )->nicesize( primary ),
		     child( boss )->weight( primary ) );
	}
	else
	{
	    y2debug( "YSplit::boss(): This layout doesn't have a boss." );
	}
    }

    return boss;
}


long YSplit::childrenMaxNiceSize( YUIDimension dimension )
{
    long maxNiceSize = 0L;

    for ( int i = 0; i < numChildren(); i++ )
    {
	maxNiceSize = max ( child(i)->nicesize( dimension ), maxNiceSize );
    }

    return maxNiceSize;
}


long YSplit::childrenTotalWeight( YUIDimension dimension )
{
    long totalWeight = 0L;

    for ( int i = 0; i < numChildren(); i++ )
    {
	totalWeight += child(i)->weight( dimension );
    }

    return totalWeight;
}


long YSplit::totalNonWeightedChildrenNiceSize( YUIDimension dimension )
{
    long size = 0L;

    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( ! child(i)->hasWeight( dimension ) ) // non-weighted children only
	    size += child(i)->nicesize( dimension );
    }

    return size;
}


int YSplit::countNonWeightedChildren( YUIDimension dimension )
{
    int count = 0;

    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( ! child(i)->hasWeight( dimension ) )
	    count++;
    }

    return count;
}


int YSplit::countStretchableChildren( YUIDimension dimension )
{
    int count = 0;

    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( ! child(i)->hasWeight( dimension ) &&
	     child(i)->stretchable( dimension ) )
	    count++;
    }

    return count;
}


int YSplit::countLayoutStretchChildren( YUIDimension dimension )
{
    int count = 0;

    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( ! child(i)->hasWeight( dimension ) &&
	     child(i)->isLayoutStretch( dimension ) )
	    count++;
    }

    return count;
}


bool YSplit::stretchable( YUIDimension dimension )
{
    for ( int i = 0; i < numChildren(); i++ )
    {
	if ( child(i)->stretchable( dimension ) ||
	     child(i)->hasWeight( dimension ) )
	    return true;
    }

    return false;
}


void YSplit::setSize( long newWidth, long newHeight )
{
    int childrenCount = numChildren();
    sizeVector	widths	( childrenCount );
    sizeVector	heights ( childrenCount );
    posVector	x_pos	( childrenCount );
    posVector	y_pos	( childrenCount );

    if ( primary == YD_HORIZ )
    {
	calcPrimaryGeometry  ( newWidth,	widths,	 x_pos );
	calcSecondaryGeometry( newHeight, heights, y_pos );
    }
    else
    {
	calcPrimaryGeometry  ( newHeight, heights, y_pos );
	calcSecondaryGeometry( newWidth,	widths,	 x_pos );
    }

    doResize( widths, heights, x_pos, y_pos );
}


void YSplit::calcPrimaryGeometry( long		newSize,
				  sizeVector &	childSize,
				  posVector  &	childPos )
{
    long pos = 0L;
    long distributableSize = newSize - totalNonWeightedChildrenNiceSize( primary );

    if ( distributableSize >= 0L )
    {
	// The ( hopefully ) normal case: There is enough space.
	// The non-weighted children will get their nice sizes,
	// the rest will be distributed among the weighted children
	// according to their respective weight ratios.

	long nonWeightedExtra 	= 0L;
	long totalWeight	= childrenTotalWeight( primary );
	int  rubberBands	= 0;
	long rubberBandExtra	= 0L;

	if ( totalWeight <= 0 )
	{
	    // If there are no weighted children, equally divide the
	    // extra space among the stretchable children ( if any ).
	    // This includes any layout stretch spaces.

	    int stretchableChildren = countStretchableChildren( primary );

	    if ( stretchableChildren > 0 )	// avoid division by zero
		nonWeightedExtra = distributableSize / stretchableChildren;
	}
	else
	{
	    // If there are weighted children and there are rubber band
	    // widgets, equally divide any surplus space ( i.e. space that
	    // exceeds the weighted children's nice sizes with respect to
	    // their weights ) between the rubber bands.
	    //
	    // This offers an easy way to make nice even spaced buttons
	    // of equal size: Give all buttons a weight of 1 and insert a
	    // stretch ( without weight! ) between each.

	    long surplusSize = newSize - nicesize( primary );

	    if ( surplusSize > 0L )
	    {
		rubberBands = countLayoutStretchChildren( primary );

		if ( rubberBands > 0 )
		{
		    rubberBandExtra 	  = surplusSize / rubberBands;
		    distributableSize -= rubberBandExtra * rubberBands;
		}
	    }
	}

	if ( debugLayout )
	{
	    y2debug( "Distributing extra space" );
	    y2debug( "new size: %ld", 			newSize			);
	    y2debug( "distributable size: %ld", 	distributableSize	);
	    y2debug( "rubber band extra: %ld",		rubberBandExtra		);
	    y2debug( "rubber bands: %d", 		rubberBands		);
	    y2debug( "total weight: %ld", 		totalWeight		);
	    y2debug( "non weighted extra: %ld", 	nonWeightedExtra 	);
	}

	for ( int i = 0; i < numChildren(); i++ )
	{
	    if ( child(i)->hasWeight( primary ) )
	    {
		// Weighted children will get their share.

		childSize[i] = distributableSize * child(i)->weight( primary ) / totalWeight;

		if ( childSize[i] < child(i)->nicesize( primary ) )
		{
		    y2debug ( "Resizing child widget #%d ( %s ) below its nice size of %ld to %ld "
			      "- check the layout!",
			      i, child(i)->widgetClass(), child(i)->nicesize( primary ), childSize[i] );
		}
	    }
	    else
	    {
		// Non-weighted children will get their nice size.

		childSize[i] = child(i)->nicesize( primary );


		if ( child(i)->stretchable( primary ) )
		{
		    // If there are only non-weighted children ( and only then ),
		    // the stretchable children will get their fair share of the
		    // extra space.

		    childSize[i] += nonWeightedExtra;
		}

		if ( child(i)->isLayoutStretch( primary ) )
		{
		    // If there is more than the total nice size and there
		    // are rubber bands, distribute surplus space among the
		    // rubber bands.

		    childSize[i] += rubberBandExtra;
		}
	    }

	    childPos[i] = pos;
	    pos += childSize[i];

	    if ( debugLayout )
	    {
		y2debug( "child #%d ( %s ) will get %ld "
			 "( nice size: %ld, weight: %ld, stretchable: %s ), pos %ld",
			 i, child(i)->widgetClass(),
			 childSize[i],
			 child(i)->nicesize( primary ),
			 child(i)->weight( primary ),
			 child(i)->stretchable( primary ) ? "yes" : "no",
			 childPos[i] );
	    }
	}
    }
    else	// The pathological case : Not enough space.
    {
	/*
	 * We're in deep shit.
	 *
	 * Not only is there nothing to distribute among the weighted children,
	 * we also need to resize the non-weighted children below their nice
	 * sizes. Let's at least treat them equally bad - divide the lost space
	 * among them as fair as possible.
	 */

	long tooSmall		= -distributableSize;
	int  loserCount		= 0;
	long dividedLoss	= 0L;

	y2debug ( "Not enough space - %ld too small - check the layout!", tooSmall );


	// Calculate initial sizes

	for ( int i = 0; i < numChildren(); i++ )
	{
	    if ( ! child(i)->hasWeight( primary ) )
	    {
		loserCount++;
		childSize[i] = child(i)->nicesize( primary );
	    }
	    else
	    {
		// Weighted children will get nothing anyway if there is nothing
		// to distribute.

		childSize[i] = 0L;
	    }
	}


	// Distribute loss

	do
	{
	    if ( debugLayout )
	    {
		y2debug( "Distributing insufficient space of %ld amoung %d losers",
			 tooSmall, loserCount );
	    }

	    if ( loserCount > 0 )	// avoid division by zero
		dividedLoss	= max( tooSmall / loserCount, 1L );

	    for ( int i = 0; i < numChildren() && tooSmall > 0; i++ )
	    {
		if ( childSize[i] < dividedLoss )
		{
		    // This widget is too small to take its share of the
		    // loss. We'll have to re-distribute the rest of the
		    // loss among the others. Arrgh.

		    if ( childSize[i] > 0L )
		    {
			tooSmall	-= childSize[i];
			childSize[i]	 = 0L;
			loserCount--;

			if ( loserCount > 0 )
			    dividedLoss = max( tooSmall / loserCount, 1L );
		    }
		}
		else
		{
		    childSize[i]	-= dividedLoss;
		    tooSmall		-= dividedLoss;
		}

		if ( debugLayout )
		{
		    y2debug( "child #%d ( %s ) will get %ld - %ld too small "
			     "(nice size: %ld, weight: %ld, stretchable: %s), pos %ld",
			     i, child(i)->widgetClass(),
			     childSize[i],
			     child(i)->nicesize( primary ) - childSize[i],
			     child(i)->nicesize( primary ),
			     child(i)->weight( primary ),
			     child(i)->stretchable( primary ) ? "yes" : "no",
			     childPos[i] );
		}
	    }
	} while ( tooSmall > 0 && loserCount > 0 );


	// Calculate postitions

	for ( int i = 0, pos=0; i < numChildren(); i++ )
	{
	    childPos[i] = pos;
	    pos	     += childSize[i];

	}

    }
}


void YSplit::calcSecondaryGeometry( long	newSize,
				    sizeVector & childSize,
				    posVector  & childPos )
{
    for ( int i = 0; i < numChildren(); i++ )
    {
	long nice = child(i)->nicesize( secondary );

	if ( child(i)->stretchable( secondary ) || newSize < nice )
	{
	    childSize[i] = newSize;
	    childPos [i] = 0L;
	}
	else // child is not stretchable and there is more space than it wants
	{
	    childSize[i] = nice;
	    childPos [i] = ( newSize - nice ) / 2;	// center
	}

	if ( childSize[i] < nice )
	{
	    y2debug ( "Resizing child widget #%d ( %s ) below its nice size of %ld to %ld "
		      "- check the layout!",
		      i, child(i)->widgetClass(), nice, childSize[i] );
	}

	if ( debugLayout )
	{
	    y2debug( "child #%d ( %s ) will get %ld "
		     "(nice size: %ld, weight: %ld, stretchable: %s), pos %ld",
		     i, child(i)->widgetClass(),
		     childSize[i],
		     nice,
		     child(i)->weight( secondary ),
		     child(i)->stretchable( secondary ) ? "yes" : "no",
		     childPos[i] );
	}
    }
}


void YSplit::doResize( sizeVector & width,
		       sizeVector & height,
		       posVector  & x_pos,
		       posVector  & y_pos  )
{
    for ( int i = 0; i < numChildren(); i++ )
    {
	child(i)->setSize( width[i], height[i] );
	moveChild( child(i), x_pos[i], y_pos[i] );
    }
}

