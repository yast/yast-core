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

  File:	      YWizard.cc

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/


#define y2log_component "ui-pkg"
#include <ycp/y2log.h>

#include "YWizard.h"


YWizard::YWizard( YWidgetOpt & opt,
		  const YCPValue & backButtonId,	const YCPString & backButtonLabel,
		  const YCPValue & abortButtonId,	const YCPString & abortButtonLabel,
		  const YCPValue & nextButtonId,	const YCPString & nextButtonLabel  )
    : YContainerWidget( opt )
    , _backButtonId( backButtonId )
    , _backButtonLabel( backButtonLabel )
    , _abortButtonId( abortButtonId )
    , _abortButtonLabel( abortButtonLabel )
    , _nextButtonId( nextButtonId )
    , _nextButtonLabel( nextButtonLabel )
{
    y2debug( "YWizard" );

    // Derived classes need to check opt.shrinkable!

    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


