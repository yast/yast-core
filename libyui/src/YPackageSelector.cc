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

  File:	      YPackageSelector.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui-pkg"
#include <ycp/y2log.h>

#include "YPackageSelector.h"


YPackageSelector::YPackageSelector( YWidgetOpt &opt )
    : YWidget( opt)
{
    y2debug( "YPackageSelector" );

    // Derived classes need to check opt.shrinkable!

    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


