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

  File:	      YReplacePoint.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YReplacePoint_h
#define YReplacePoint_h

#include "YContainerWidget.h"

/**
 * @short Implementation of the ReplacePoint widget.
 */
class YReplacePoint : public YContainerWidget
{
public:
    /**
     * Constructor
     */
    YReplacePoint( YWidgetOpt & opt);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YReplacePoint"; }

    /**
     * Inherited from YWidget. Returns true.
     */
    bool isReplacePoint( ) const;
};


#endif // YReplacePoint_h

