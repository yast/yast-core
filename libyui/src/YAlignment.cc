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


YAlignment::YAlignment( YWidgetOpt & opt,
			YAlignmentType halign,
			YAlignmentType valign )
    : YContainerWidget( opt )
{
    align[ YD_HORIZ ] = halign;
    align[ YD_VERT ]  = valign;
}


bool YAlignment::stretchable( YUIDimension dim ) const
{
    if ( align[ dim ] == YAlignUnchanged ) return child(0)->stretchable( dim );
    else return true;
}


void YAlignment::setSize( long newwidth, long newheight )
{
    long newsize[2];
    newsize[ YD_HORIZ ]  = newwidth;
    newsize[ YD_VERT ]   = newheight;

    long newchildsize[2];
    long newchildpos[2];

    for ( YUIDimension dim = YD_HORIZ; dim <= YD_VERT; dim = ( YUIDimension )( dim + 1 ) )
    {
	if ( child(0)->stretchable( dim ) )
	{
	    newchildsize[ dim ] = newsize[ dim ];
	    newchildpos [ dim ] = 0;
	}
	else
	{
	    newchildsize[ dim ] = min( newsize[ dim ], child(0)->nicesize( dim ) );
	    switch ( align[ dim ] )
	    {
		case YAlignUnchanged:
		case YAlignCenter:
		    newchildpos[ dim ] = ( newsize[ dim ] - newchildsize[ dim ] ) / 2;
		    break;

		case YAlignBegin:
		    newchildpos[ dim ] = 0;
		    break;

		case YAlignEnd:
		    newchildpos[ dim ] = newsize[ dim ] - newchildsize[ dim ];
		    break;
	    }
	}
    }

    child(0)->setSize( newchildsize[ YD_HORIZ ], newchildsize[ YD_VERT ] );
    moveChild( child(0), newchildpos[ YD_HORIZ ], newchildpos[ YD_VERT ] );
}

