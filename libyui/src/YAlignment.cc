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

  File:	      YAlignment.cc

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui"
#include <ycp/y2log.h>
#include "YAlignment.h"

using std::min;


YAlignment::YAlignment( const YWidgetOpt & opt,
			YAlignmentType halign,
			YAlignmentType valign )
    : YContainerWidget( opt )
    , _leftMargin(0)
    , _rightMargin(0)
    , _topMargin(0)
    , _bottomMargin(0)
    , _minWidth(0)
    , _minHeight(0)
{
    align[ YD_HORIZ ] = halign;
    align[ YD_VERT  ] = valign;
}


bool YAlignment::stretchable( YUIDimension dim ) const
{
    if ( align[ dim ] == YAlignUnchanged ) return child(0)->stretchable( dim );
    else return true;
}


long YAlignment::nicesize( YUIDimension dim )
{
    long nice_size = child(0)->nicesize( dim );
    long min_size;

    if ( dim == YD_HORIZ )
    {
	nice_size += leftMargin() + rightMargin();
	min_size = minWidth();
    }
    else
    {
	nice_size += topMargin() + bottomMargin();
	min_size = minHeight();
    }
    
    return max( min_size, nice_size );
}


void YAlignment::setSize( long newWidth, long newHeight )
{
    if ( ! child(0) )
    {
	y2error( "No child in %s", widgetClass() );
	return;
    }

    
    long newSize[ YUIAllDimensions ];
    newSize[ YD_HORIZ ]  = newWidth;
    newSize[ YD_VERT  ]  = newHeight;
    
    long offset[ YUIAllDimensions ];
    offset[ YD_HORIZ ] = leftMargin();
    offset[ YD_VERT  ] = topMargin();

    long totalMargin[ YUIAllDimensions ];
    totalMargin[ YD_HORIZ ] = leftMargin() + rightMargin();
    totalMargin[ YD_VERT  ] = topMargin()  + bottomMargin();

    long newChildSize[ YUIAllDimensions ];
    long newChildPos [ YUIAllDimensions ];

    
    for ( YUIDimension dim = YD_HORIZ; dim <= YD_VERT; dim = (YUIDimension) (dim+1) )
    {
	long childNiceSize = child(0)->nicesize( dim );
	long niceSize      = childNiceSize + totalMargin[ dim ];
	
	if ( newSize[ dim ] >= niceSize )
	    // Optimum case: enough space for the child and all margins
	{
	    if ( align[ dim ] == YAlignUnchanged && child(0)->stretchable( dim ) )
	    {
		newChildSize[ dim ] = newSize[ dim ] - totalMargin[ dim ];
	    }
	    else
	    {
		newChildSize[ dim ] = childNiceSize;
	    }
	}
	else if ( newSize[ dim ] >= childNiceSize )
	    // Still enough space for the child, but not for all margins
	{
	    newChildSize[ dim ] = childNiceSize; // Give the child as much space as it needs

	    // Reduce the margins

	    if ( totalMargin[ dim ] > 0 ) // Prevent division by zero
	    {
		// Redistribute remaining space according to margin ratio
		// (disregarding integer rounding errors - we don't care about one pixel)
		    
		long remaining = newSize[ dim ] - childNiceSize;
		offset     [ dim ] = remaining * offset[ dim ] / totalMargin[ dim ];
		totalMargin[ dim ] = remaining;
	    }

	}
	else // Not even enough space for the child - forget about the margins
	{
	    newChildSize[ dim ] = newSize[ dim ];
	    offset	[ dim ] = 0;
	    totalMargin [ dim ] = 0;
	}
	    

	switch ( align[ dim ] )
	{
	    case YAlignCenter:
		newChildPos[ dim ] = ( newSize[ dim ] - newChildSize[ dim ] - totalMargin[ dim ] ) / 2;
		break;

	    case YAlignUnchanged:
	    case YAlignBegin:
		newChildPos[ dim ] = 0;
		break;

	    case YAlignEnd:
		newChildPos[ dim ] = newSize[ dim ] - newChildSize[ dim ] - totalMargin[ dim ];
		break;
	}

	newChildPos[ dim ] += offset[ dim ];
    }

    child(0)->setSize( newChildSize[ YD_HORIZ ], newChildSize[ YD_VERT ] );
    moveChild( child(0), newChildPos[ YD_HORIZ ], newChildPos[ YD_VERT ] );
}



long YAlignment::totalMargins( YUIDimension dim ) const
{
    if ( dim == YD_HORIZ )	return leftMargin() + rightMargin();
    else			return topMargin()  + bottomMargin();
}



void YAlignment::setBackgroundPixmap( string pixmap )
{
    if ( pixmap.length() > 0 &&
	 pixmap[0] != '/'  &&	// Absolute path?
	 pixmap[0] != '.'    )	// Path relative to $CWD ?
    {
	// Prepend theme dir ("/usr/share/YaST2/theme/current/")
	pixmap = pixmap.insert( 0, string( THEMEDIR "/" ) );
    }
    
    _backgroundPixmap = pixmap;
}
