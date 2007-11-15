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

  File:		YPackageSelector.cc

  Author:	Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui-pkg"
#include <ycp/y2log.h>

#include "YPackageSelector.h"


YPackageSelector::YPackageSelector( YWidget * parent, long modeFlags )
    : YWidget( parent )
    , _modeFlags( modeFlags )
{
    y2milestone( "YPackageSelector flags: 0x%lx", modeFlags );

    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


