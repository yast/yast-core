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

  File:	      YPackageSelector.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YPackageSelector_h
#define YPackageSelector_h

#include "YWidget.h"
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>

class YMacroRecorder;

/**
 * @short Implementation of the SelectionBox widget.
 */
class YPackageSelector : public YWidget
{
public:

    /**
     * Constructor
     * @param opt the widget options
     */
    YPackageSelector( YWidgetOpt & opt );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YPackageSelector"; }


};


#endif // YPackageSelector_h
