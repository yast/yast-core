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

  File:	      YEmpty.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#include "YSpacing.h"


YSpacing::YSpacing( const YWidgetOpt & opt, float size, bool horizontal, bool vertical )
    : YWidget( opt )
{
    relativeSize[ YD_HORIZ ] = horizontal ? size : 0.0;
    relativeSize[ YD_VERT  ] = vertical   ? size : 0.0;
}


long YSpacing::nicesize( YUIDimension dim )
{
    return absoluteSize( dim, relativeSize[ dim ] );
}


bool YSpacing::isLayoutStretch( YUIDimension dim ) const
{
    return _stretch[ dim ];
}


/**
 * Default implementation. Overwrite this for UI specific units.
 */
long YSpacing::absoluteSize( YUIDimension dim, float relativeSize )
{
    return ( long ) ( relativeSize + 0.5 );
}

