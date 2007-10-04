/*-----------------------------------------------------------*- c++ -*-\
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

   File:       Y2ErrorComponent.h

   Author:     Martin Vidner <mvidner@suse.cz>

/-*/

/*
 * Component that represents an error while creating another component
 * (the component creator would otherwise keep trying while we want to cry foul)
 *
 * BTW we do not need a matching component creator because we do not
 * WANT errors, they just happen, pesky things.
 */

#ifndef Y2ErrorComponent_h
#define Y2ErrorComponent_h

#include "Y2Component.h"

/**
 * @short A dummy component representing an error
 */
class Y2ErrorComponent : public Y2Component
{
public:

    /**
     * Constructor.
     */
    Y2ErrorComponent () {}

    /**
     * Returns "ErrorComponent".
     */
    string name() const { return "ErrorComponent"; }

};


#endif // Y2ErrorComponent_h
