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

  File:	      YSquash.h

  Author:     Mathias Kettner <kettner@suse.de>
  Maintainer: Stefan Hundhammer <sh@suse.de>

/-*/

#ifndef YSquash_h
#define YSquash_h

#include "YContainerWidget.h"

/**
 * @short Implementation of the HSquash, VSquash and HVSquash widgets
 */
class YSquash : public YContainerWidget
{
public:

    /**
     * Constructor
     */
    YSquash( YWidgetOpt &opt, bool hsquash, bool vsquash);

    /**
     * Returns a descriptive name of this widget class for logging,
     * debugging etc.
     */
    virtual char *widgetClass( ) { return "YSquash"; }

    /**
     * In a squashed dimension the widget NOT stretchable.
     * In an unsquashed dimension the widget is stretchable if the
     * child is stretchable.
     */
    bool stretchable( YUIDimension dim);


protected:
    /**
     * In which dimensions to squash the contained widget.
     */
    bool squash[YUIAllDimensions];
};


#endif // YSquash_h
