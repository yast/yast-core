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

  File:	      YDate.h

  Author:     Anas Nashif <nashif@suse.de>

/-*/

#ifndef YDate_h
#define YDate_h

#include "YWidget.h"
#include <ycp/YCPString.h>

/**
 * Implementation of the Date and Heading widgets
 */
class YDate : public YWidget
{
public:

    /**
     * Creates a new date
     * @param text the initial text of the date
     */
    YDate( const YWidgetOpt & opt, const YCPString & label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YDate"; }

    /**
     * Implements the ui command changeWidget.
     */
    YCPValue changeWidget( const YCPSymbol & property, const YCPValue & newvalue );

    /**
     * Implements the ui command queryWidget
     */
    YCPValue queryWidget( const YCPSymbol & property );


    /**
     * Set the date in the entry to a new value
     */
    virtual void setNewDate( const YCPString & text ) = 0;
    
    /**
     * get the date currently entered in the Date entry
     */
    virtual YCPString getDate() = 0;

    /**
     * change the label of the Date  entry. Overload this, but call
     * YTextEntry::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );


protected:
    /**
     * Current label
     */
    YCPString label;
    

};


#endif // YDate_h
