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
#include "YUI.h"


YSpacing::YSpacing( const YWidgetOpt & opt, float layoutUnits, bool horizontal, bool vertical )
    : YWidget( opt )
{
    _size[ YD_HORIZ ] = horizontal ? YUI::ui()->deviceUnits( YD_HORIZ, layoutUnits ) : 0;
    _size[ YD_VERT  ] = vertical   ? YUI::ui()->deviceUnits( YD_VERT , layoutUnits ) : 0;
}


long YSpacing::nicesize( YUIDimension dim )
{
    return _size[ dim ];
}


bool YSpacing::isLayoutStretch( YUIDimension dim ) const
{
    return _stretch[ dim ];
}



