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

  File:	      YFrame.h

  Author:     Stefan Hundhammer <sh@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YFrame_h
#define YFrame_h

#include <ycp/YCPString.h>
#include "YContainerWidget.h"

/**
 * @short Implementation of the Frame widget
 */
class YFrame : public YContainerWidget
{
public:
    /**
     * Constructor
     */
    YFrame( YWidgetOpt &opt, const YCPString &label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YFrame"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget(const YCPSymbol& property, const YCPValue& newvalue);

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue queryWidget(const YCPSymbol& property);
    
    /**
     * Change the Frame label. Overload this, but call
     * YFrame::setLabel at the end of your own function.
     */
    virtual void setLabel(const YCPString& label);

    /**
     * Get the current label.
     */
    YCPString getLabel();

    /**
     * The name of the widget property that holds the keyboard shortcut.
     * Inherited from YWidget.
     */
    const char *shortcutProperty() { return YUIProperty_Label; }


protected:
    /**
     * The current frame label.
     */
    YCPString label;
};


#endif // YFrame_h

