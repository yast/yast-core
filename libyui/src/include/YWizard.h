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

  File:	      YWizard.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YWizard_h
#define YWizard_h

#include "YContainerWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPValue.h>

class YMacroRecorder;

#define YWizardContentsReplacePointID "contents"

/**
 * @short Implementation of the SelectionBox widget.
 */
class YWizard: public YContainerWidget
{
public:

    /**
     * Constructor
     */
    YWizard( YWidgetOpt & opt,
	     const YCPValue & backButtonId,	const YCPString & backButtonLabel,
	     const YCPValue & abortButtonId,	const YCPString & abortButtonLabel,
	     const YCPValue & nextButtonId,	const YCPString & nextButtonLabel  );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YWizard"; }

protected:
    
    YCPValue 	_backButtonId;
    YCPString 	_backButtonLabel;
    YCPValue 	_abortButtonId;
    YCPString 	_abortButtonLabel;
    YCPValue 	_nextButtonId;
    YCPString 	_nextButtonLabel;
};


#endif // YWizard_h
