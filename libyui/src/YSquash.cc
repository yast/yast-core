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

   File:       YSquash.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#include "YSquash.h"

YSquash::YSquash( YWidgetOpt & opt, bool hsquash, bool vsquash )
    : YContainerWidget( opt )
{
    squash[ YD_HORIZ ] = hsquash;
    squash[ YD_VERT ]  = vsquash;
}

bool YSquash::stretchable( YUIDimension dim )
{
    return ! squash[ dim ] && child(0)->stretchable( dim );
}

