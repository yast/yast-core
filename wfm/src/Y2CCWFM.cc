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

   File:	Y2CCWFM.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Mathias Kettner <kettner@suse.de>

/-*/

#include <Y2CCWFM.h>
#include <Y2WFMComponent.h>


Y2CCWFM::Y2CCWFM ()
    : Y2ComponentCreator (Y2ComponentBroker::BUILTIN)
{
}


bool
Y2CCWFM::isServerCreator () const
{
    return false;
}


Y2Component *
Y2CCWFM::create (const char *name) const
{
    if (!strcmp(name, "wfm"))
	return new Y2WFMComponent();
    else
	return 0;
}


Y2CCWFM g_y2ccwfm;
