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

  File:	      YRichText.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YRichText_h
#define YRichText_h

#include "YWidget.h"
#include <ycp/YCPString.h>

/**
 * @short Implementation of the RichText widget.
 * Derived classes need to check opt.plainTextMode!
 */
class YRichText : public YWidget
{
public:
    /**
     * Constructor
     * @param text the initial text of the RichText
     * @param opt the widget options
     */
    YRichText( const YWidgetOpt & opt, YCPString text );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YRichText"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );

    /**
     * Change the RichText text. Overload this, but call
     * YRichText::setText at the end of your own function.
     */
    virtual void setText( const YCPString & RichText );

    /**
     * Get the current RichText text. This method cannot be overidden. The value
     * of the RichText cannot be changed other than by calling setRichText,
     * i.e. not by the ui. Therefore setRichText stores the current RichText in
     * #RichText.
     */
    YCPString getText();


protected:
    /**
     * Current text of the RichText
     */
    YCPString text;


    /**
     * Flag: Should the text automatically scroll all the way down upon text change?
     **/
    bool autoScrollDown;
};


#endif // YRichText_h
