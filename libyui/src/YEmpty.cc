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

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/


#include "YEmpty.h"

YEmpty::YEmpty( YWidgetOpt & opt )
    :YWidget( opt )
{
}


long YEmpty::nicesize( YUIDimension dim )
{
    return 0;
}


bool YEmpty::isLayoutStretch( YUIDimension dim ) const
{
    return stretchable( dim );
}

