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

  File:	      YTime.h

  Author:     Anas Nashif <nashif@suse.de>

/-*/

#ifndef YTime_h
#define YTime_h

#include "YWidget.h"
#include <ycp/YCPString.h>

/**
 * Implementation of the Time and Heading widgets
 */
class YTime : public YWidget
{
public:

    /**
     * Creates a new date
     * @param text the initial text of the date
     */
    YTime( const YWidgetOpt & opt, const YCPString & label );

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass() { return "YTime"; }

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
    virtual void setNewTime( const YCPString & text ) = 0;
    
    /**
     * get the date currently entered in the Time entry
     */
    virtual YCPString getTime() = 0;

    /**
     * change the label of the Time  entry. Overload this, but call
     * YTime::setLabel at the end of your own function.
     */
    virtual void setLabel( const YCPString & label );


protected:
    /**
     * Current label
     */
    YCPString label;
    
    /**
     * Flag: This property holds whether the editor automatically advances to the next section.
     **/
    bool autoAdvance;

};


#endif // YTime_h
