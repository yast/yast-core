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


#define y2log_component "ui"
#include <ycp/y2log.h>
#include <ycp/YCPTerm.h>

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
    // All wizard widgets have a fixed ID `wizard
    YWidget::setId( YCPSymbol( YWizardID ) );

    setDefaultStretchable( YD_HORIZ, true );
    setDefaultStretchable( YD_VERT,  true );
}


YCPValue YWizard::command( const YCPTerm & command )
{
    y2error( "YWizard::command() not reimplemented!" );

    return YCPNull();
}

