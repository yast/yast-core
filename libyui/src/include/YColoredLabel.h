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

  File:	      YColoredLabel.h

  Author:     Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YColoredLabel_h
#define YColoredLabel_h

#include "YWidget.h"
#include <ycp/YCPString.h>

/**
 * Implementation of the YColoredLabel widget
 */
class YColoredLabel : public YWidget
{
public:

    /**
     * Creates a new new label
     * @param text the initial text of the label
     */
    YColoredLabel(YWidgetOpt &opt, YCPString text);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YColoredLabel"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget(const YCPSymbol & property, const YCPValue & newvalue);

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget(const YCPSymbol & property);

    /**
     * Change the label text. Overload this, but call
     * YColoredLabel::setLabel at the end of your own function.
     */
    virtual void setLabel(const YCPString & label);

    /**
     * Get the current label text. This method cannot be overidden.
     * The value of the label cannot be changed other than by calling setLabel,
     * i.e. not by the ui. Therefore setLabel stores the current label in #label.
     */
    YCPString getLabel();

protected:
    /**
     * Current label text
     */
    YCPString text;
};


#endif // YColoredLabel_h
