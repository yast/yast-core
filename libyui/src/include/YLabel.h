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

  File:	      YLabel.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YLabel_h
#define YLabel_h

#include "YWidget.h"
#include <ycp/YCPString.h>

/**
 * Implementation of the Label and Heading widgets
 */
class YLabel : public YWidget
{
public:

    /**
     * Creates a new new label
     * @param text the initial text of the label
     */
    YLabel( YWidgetOpt & opt, YCPString text );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YLabel"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the label text. Overload this, but call
     * YLabel::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );

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


#endif // YLabel_h
