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

   File:	Y2ErrorComponent.cc

   Component that represents an error while creating another component

   Author:     Martin Vidner <mvidner@suse.cz>

/-*/

#include "Y2ErrorComponent.h"

// the default implementation of doActualWork logs an error and returns nil,
// which is fine for us
