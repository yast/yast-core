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

  File:		YLayoutBox.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui-layout"
#include <ycp/y2log.h>

#include "YLayoutBox.h"
#include "YAlignment.h"
#include "YSpacing.h"
#include "YUI.h"

#include <algorithm>


struct YLayoutBoxPrivate
{
    /**
     * Constructor
     **/
    YLayoutBoxPrivate( YUIDimension prim )
	: primary( prim )
	, secondary( prim == YD_HORIZ ? YD_VERT : YD_HORIZ )
	, debugLayout( false )
	{}

    //
    // Data members
    //

    YUIDimension	primary;
    YUIDimension 	secondary;
    bool 		debugLayout;
};




YLayoutBox::YLayoutBox( YWidget * parent, YUIDimension primaryDimension )
    : YWidget( parent )
    , priv( new YLayoutBoxPrivate( primaryDimension ) )
{
    YUI_CHECK_NEW( priv );
    setChildrenManager( new YWidgetChildrenManager( this ) );
}


YLayoutBox::~YLayoutBox()
{
    // NOP
}


YUIDimension
YLayoutBox::primary() const
{
    return priv->primary;
}


YUIDimension
YLayoutBox::secondary() const
{
    return priv->secondary;
}


bool
YLayoutBox::debugLayout() const
{
    return priv->debugLayout;
}

void
YLayoutBox::setDebugLayout( bool deb )
{
    priv->debugLayout = deb;

    y2debug( "YLayoutBox: Layout debugging %s",
	     deb ? "enabled" : "disabled" );

}


int
YLayoutBox::preferredSize( YUIDimension dimension )
{
    if ( dimension == secondary() )	// the easy case first: secondary dimension
    {
	return childrenMaxPreferredSize( dimension );
    }
    else
    {
	/*
	 * In the primary dimension things are much more complicated: We want to
	 * honor any weights specified under all circumstances.  So we first
	 * need to determine the "dominating child" - the widget that determines the
	 * overall size with respect to its weight in that dimension. Once we
	 * know that, we need to stretch all other weighted children accordingly
	 * so the weight ratios are respected.
	 *
	 * As a final step, the preferred sizes of all children that don't have
	 * a weight attached are summed up.
	 */

	int size = 0L;

	// Search for the dominating child
	YWidget * dominatingChild = findDominatingChild();

	if ( dominatingChild )
	{
	    // Calculate size of all weighted widgets.

	    size = dominatingChild->preferredSize( primary() )
		* childrenTotalWeight( primary() )
		/ dominatingChild->weight( primary() );

	    // Maintain this order of calculation in order to minimize integer
	    // rounding errors!
	}


	// Add up the size of all non-weighted children;
	// they will get their respective preferred size.

	size += totalNonWeightedChildrenPreferredSize( primary() );

	return size;
    }
}


int YLayoutBox::preferredWidth()
{
    return preferredSize( YD_HORIZ );
}


int YLayoutBox::preferredHeight()
{
    return preferredSize( YD_VERT );
}


/*
 * Search for the "dominating child" widget.
 *
 * This is the widget that determines the overall size of the
 * container with respect to all children's weights: It is the child
 * with the maximum ratio of preferred size and weight. All other
 * weighted children need to be stretched accordingly so the weight
 * ratios can be maintained.
 *
 * Returns 0 if there is no dominating child, i.e. if there are only
 * non-weighted children.
 */

YWidget *
YLayoutBox::findDominatingChild()
{
    YWidget *	dominatingChild	= 0;
    double 	dominatingRatio	= 0.0;
    double 	ratio;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	YWidget * child = *it;

	if ( child->weight( primary() ) != 0 )	// avoid division by zero
	{
	    ratio = ( ( double ) child->preferredSize( primary() ) )
		/ child->weight( primary() );

	    if ( ratio > dominatingRatio ) // we have a new dominating child
	    {
		dominatingChild = child;
		dominatingRatio = ratio;
	    }
	}
    }


    if ( debugLayout() )
    {
	if ( dominatingChild >= 0 )
	{
	    y2debug( "YLayoutBox::findDominatingChild(): "
		     "Found dominating child: %s - preferred size: %d, weight: %d",
		     dominatingChild->widgetClass(),
		     dominatingChild->preferredSize( primary() ),
		     dominatingChild->weight( primary() ) );
	}
	else
	{
	    y2debug( "YLayoutBox::findDominatingChild(): "
		     "This layout doesn't have a dominating child." );
	}
    }

    return dominatingChild;
}


int
YLayoutBox::childrenMaxPreferredSize( YUIDimension dimension )
{
    int maxPreferredSize = 0L;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	maxPreferredSize = std::max( (*it)->preferredSize( dimension ), maxPreferredSize );
    }

    return maxPreferredSize;
}


int
YLayoutBox::childrenTotalWeight( YUIDimension dimension )
{
    int totalWeight = 0L;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	totalWeight += (*it)->weight( dimension );
    }

    return totalWeight;
}


int
YLayoutBox::totalNonWeightedChildrenPreferredSize( YUIDimension dimension )
{
    int size = 0L;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	if ( ! (*it)->hasWeight( dimension ) ) // non-weighted children only
	    size += (*it)->preferredSize( dimension );
    }

    return size;
}


int
YLayoutBox::countNonWeightedChildren( YUIDimension dimension )
{
    int count = 0;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	if ( ! (*it)->hasWeight( dimension ) )
	    count++;
    }

    return count;
}


int
YLayoutBox::countStretchableChildren( YUIDimension dimension )
{
    int count = 0;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	if ( ! (*it)->hasWeight( dimension ) &&
	     (*it)->stretchable( dimension ) )
	    count++;
    }

    return count;
}


int
YLayoutBox::countLayoutStretchChildren( YUIDimension dimension )
{
    int count = 0;

    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	if ( ! (*it)->hasWeight( dimension ) &&
	     isLayoutStretch( *it, dimension ) )
	    count++;
    }

    return count;
}


bool
YLayoutBox::isLayoutStretch( YWidget * child, YUIDimension dimension )
{
    if ( ! child )
	return false;

    YSpacing * spacing = dynamic_cast<YSpacing *> (child);

    if ( spacing && spacing->stretchable( dimension ) )
	return true;
    else
	return false;
}



bool
YLayoutBox::stretchable( YUIDimension dimension ) const
{
    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it )
    {
	if ( (*it)->stretchable( dimension ) ||
	     (*it)->hasWeight( dimension ) )
	    return true;
    }

    return false;
}


void
YLayoutBox::setSize( int newWidth, int newHeight )
{
    int count = childrenCount();
    sizeVector	widths	( count );
    sizeVector	heights ( count );
    posVector	x_pos	( count );
    posVector	y_pos	( count );

    if ( primary() == YD_HORIZ )
    {
	calcPrimaryGeometry  ( newWidth,  widths,  x_pos );
	calcSecondaryGeometry( newHeight, heights, y_pos );
    }
    else
    {
	calcPrimaryGeometry  ( newHeight, heights, y_pos );
	calcSecondaryGeometry( newWidth,  widths,  x_pos );
    }

    if ( YUI::reverseLayout() )
    {
	// Mirror the widget X geometry for languages with left-to-right
	// writing direction (Arabic, Hebrew).

	for ( int i = 0; i < childrenCount(); i++ )
	    x_pos[i] = newWidth - x_pos[i] - widths[i];
    }

    doResize( widths, heights, x_pos, y_pos );
}


void
YLayoutBox::calcPrimaryGeometry( int		newSize,
				 sizeVector &	childSize,
				 posVector  &	childPos )
{
    int pos = 0L;
    int distributableSize = newSize - totalNonWeightedChildrenPreferredSize( primary() );

    if ( distributableSize >= 0L )
    {
	// The (hopefully) normal case: There is enough space.
	// The non-weighted children will get their preferred sizes,
	// the rest will be distributed among the weighted children
	// according to their respective weight ratios.

	int nonWeightedExtra 	= 0L;
	int totalWeight	= childrenTotalWeight( primary() );
	int  rubberBands	= 0;
	int rubberBandExtra	= 0L;

	if ( totalWeight <= 0 )
	{
	    // If there are no weighted children, equally divide the
	    // extra space among the stretchable children (if any).
	    // This includes any layout stretch spaces.

	    int stretchableChildren = countStretchableChildren( primary() );

	    if ( stretchableChildren > 0 )	// avoid division by zero
		nonWeightedExtra = distributableSize / stretchableChildren;
	}
	else
	{
	    // If there are weighted children and there are rubber band
	    // widgets, equally divide any surplus space (i.e. space that
	    // exceeds the weighted children's preferred sizes with respect to
	    // their weights) between the rubber bands.
	    //
	    // This offers an easy way to make nicely even spaced buttons
	    // of equal size: Give all buttons a weight of 1 and insert a
	    // stretch (without weight!) between each.

	    int surplusSize = newSize - preferredSize( primary() );

	    if ( surplusSize > 0L )
	    {
		rubberBands = countLayoutStretchChildren( primary() );

		if ( rubberBands > 0 )
		{
		    rubberBandExtra 	  = surplusSize / rubberBands;
		    distributableSize -= rubberBandExtra * rubberBands;
		}
	    }
	}

	if ( debugLayout() )
	{
	    y2debug( "Distributing extra space" );
	    y2debug( "new size: %d", 			newSize			);
	    y2debug( "distributable size: %d", 	distributableSize	);
	    y2debug( "rubber band extra: %d",		rubberBandExtra		);
	    y2debug( "rubber bands: %d", 		rubberBands		);
	    y2debug( "total weight: %d", 		totalWeight		);
	    y2debug( "non weighted extra: %d", 	nonWeightedExtra 	);
	}

	int i=0;
	for ( YWidgetListConstIterator it = childrenBegin();
	      it != childrenEnd();
	      ++it, i++ )
	{
	    YWidget * child = *it;

	    if ( child->hasWeight( primary() ) )
	    {
		// Weighted children will get their share.

		childSize[i] = distributableSize * child->weight( primary() ) / totalWeight;

		if ( childSize[i] < child->preferredSize( primary() ) )
		{
		    y2debug( "Resizing child widget #%d (%s) below its preferred size of %d to %d "
			     "- check the layout!",
			     i, child->widgetClass(), child->preferredSize( primary() ), childSize[i] );
		}
	    }
	    else
	    {
		// Non-weighted children will get their preferred size.

		childSize[i] = child->preferredSize( primary() );


		if ( child->stretchable( primary() ) )
		{
		    // If there are only non-weighted children (and only then),
		    // the stretchable children will get their fair share of the
		    // extra space.

		    childSize[i] += nonWeightedExtra;
		}

		if ( isLayoutStretch( child, primary() ) )
		{
		    // If there is more than the total preferred size and there
		    // are rubber bands, distribute surplus space among the
		    // rubber bands.

		    childSize[i] += rubberBandExtra;
		}
	    }

	    childPos[i] = pos;
	    pos += childSize[i];
	}
    }
    else	// The pathological case: Not enough space.
    {
	/*
	 * We're in deep shit.
	 *
	 * Not only is there nothing to distribute among the weighted children,
	 * we also need to resize the non-weighted children below their preferred
	 * sizes. Let's at least treat them equally bad - divide the lost space
	 * among them as fair as possible.
	 */

	int 	tooSmall	 	= -distributableSize;
	int  	loserCount 	 	= 0;
	int 	totalMargins	 	= 0L;
	int 	remainingMargins	= 0L;
	double	marginScale		= 0.0;

	y2debug ( "Not enough space - %d too small - check the layout!", tooSmall );


	// Maybe some of the children are YAlignments with margins that can be reduced

	for ( YWidgetListConstIterator it = childrenBegin();
	      it != childrenEnd();
	      ++it )
	{
	    if ( ! (*it)->hasWeight( primary() ) ) // children with weights will get nothing anyway
	    {
		YAlignment * alignment = dynamic_cast<YAlignment *> (*it);

		if ( alignment )
		{
		    totalMargins += alignment->totalMargins( primary() );
		    y2debug( "Found alignment with margins" );
		}
	    }
	}


	if ( totalMargins > tooSmall )	// We can make up for insufficient space just by reducing margins
	{
	    remainingMargins = totalMargins - tooSmall;
	    tooSmall = 0L;
	    marginScale = ( (double) remainingMargins ) / totalMargins;

	    y2debug( "Making up for insufficient space by reducing margins to %.3g%% - %d left for margins",
		     100.0 * marginScale, remainingMargins );
	}
	else				// Reducing all margins to zero still doesn't solve the problem
	{
	    tooSmall -= totalMargins;

	    y2debug( "Reducing all margins to 0, but still %d too small", tooSmall );
	}


	// Calculate initial sizes

	int i=0;
	for ( YWidgetListConstIterator it = childrenBegin();
	      it != childrenEnd();
	      ++it, i++ )
	{
	    if ( ! (*it)->hasWeight( primary() ) )
	    {
		loserCount++;
		childSize[i] = (*it)->preferredSize( primary() );

		YAlignment * alignment = dynamic_cast<YAlignment *> (*it);

		if ( alignment )		// Alignment widgets may have margins we can reduce
		{
		    int margins = alignment->totalMargins( primary() );
		    childSize[i] -= margins;			// Strip off original margin

		    if ( remainingMargins > 0 )			// Anything left to redistribute?
		    {
			margins		 = (int) marginScale * margins;	// Scale down margin
			childSize[i]	 += margins;		// Add the scaled-down margin
			remainingMargins -= margins;		// Deduct from redistributable margin
		    }
		}
	    }
	    else
	    {
		// Weighted children will get nothing anyway if there is nothing
		// to distribute.

		childSize[i] = 0L;
	    }
	}


	// Distribute loss

	while ( tooSmall > 0 && loserCount > 0 )
	{
	    if ( debugLayout() )
	    {
		y2debug( "Distributing insufficient space of %d amoung %d losers",
			 tooSmall, loserCount );
	    }

	    int dividedLoss = std::max( tooSmall / loserCount, 1 );

	    int i=0;
	    for ( YWidgetListConstIterator it = childrenBegin();
		  it != childrenEnd() && tooSmall > 0;
		  ++it, i++ )
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
			    dividedLoss = std::max( tooSmall / loserCount, 1 );
		    }
		}
		else
		{
		    childSize[i]	-= dividedLoss;
		    tooSmall		-= dividedLoss;
		}

		if ( debugLayout() )
		{
		    YWidget * child = *it;

		    y2debug( "child #%d ( %s ) will get %d - %d too small "
			     "(preferred size: %d, weight: %d, stretchable: %s), pos %d",
			     i, child->widgetClass(),
			     childSize[i],
			     child->preferredSize( primary() ) - childSize[i],
			     child->preferredSize( primary() ),
			     child->weight( primary() ),
			     child->stretchable( primary() ) ? "yes" : "no",
			     childPos[i] );
		}
	    }
	}


	// Calculate postitions

	for ( int i = 0, pos=0; i < childrenCount(); i++ )
	{
	    childPos[i] = pos;
	    pos	     += childSize[i];

	}

    }
}


void
YLayoutBox::calcSecondaryGeometry( int	 	newSize,
				   sizeVector & childSize,
				   posVector  & childPos )
{
    int i=0;
    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it, i++ )
    {
	YWidget * child = *it;
	int preferred = child->preferredSize( secondary() );

	if ( child->stretchable( secondary() ) || newSize < preferred )
	{
	    childSize[i] = newSize;
	    childPos [i] = 0L;
	}
	else // child is not stretchable and there is more space than it wants
	{
	    childSize[i] = preferred;
	    childPos [i] = ( newSize - preferred ) / 2;	// center
	}

	if ( childSize[i] < preferred )
	{
	    y2debug( "Resizing child widget #%d (%s) below its preferred size of %d to %d "
		     "- check the layout!",
		     i, child->widgetClass(), preferred, childSize[i] );
	}

	if ( debugLayout() )
	{
	    y2debug( "child #%d (%s) will get %d "
		     "(preferred size: %d, weight: %d, stretchable: %s), pos %d",
		     i, child->widgetClass(),
		     childSize[i],
		     preferred,
		     child->weight( secondary() ),
		     child->stretchable( secondary() ) ? "yes" : "no",
		     childPos[i] );
	}
    }
}


void
YLayoutBox::doResize( sizeVector & width,
		      sizeVector & height,
		      posVector  & x_pos,
		      posVector  & y_pos  )
{
    int i=0;
    for ( YWidgetListConstIterator it = childrenBegin();
	  it != childrenEnd();
	  ++it, i++ )
    {
	YWidget * child = *it;

	child->setSize( width[i], height[i] );
	moveChild( child, x_pos[i], y_pos[i] );

	if ( debugLayout() )
	{
	    string label = child->debugLabel();

	    if ( ! label.empty() )
		label = "\"" + label + "\"";

	    y2milestone( "x: %4d  y: %4d  w: %4d  h: %4d %s %s",
			 x_pos[i], y_pos[i], width[i], height[i],
			 child->widgetClass(), label.c_str() );
	}
    }
}


