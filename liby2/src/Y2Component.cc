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

   File:       Y2Component.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * Base class of all Y2 Components
 *
 */

#include <stdio.h>

#include <ycp/y2log.h>
#include "Y2Component.h"
#include "Y2ComponentBroker.h"


Y2Component::Y2Component()
{
    callbackComponent = 0;
}


Y2Component::~Y2Component()
{
}


YCPValue Y2Component::evaluate(const YCPValue&)
{
    y2internal ("component %s: stub function Y2Component::evaluate() called", name ().c_str ());
    return YCPNull();
}


void Y2Component::result(const YCPValue&)
{
    // No warning. It is legal to ignore the result
}


void Y2Component::setServerOptions(int, char **)
{
    // No warning. It is legal to ignore the options
}


Y2Component *Y2Component::getCallback (void) const
{
#if 0
    y2warning ("getCallback of %s (%p) equals %s (%p)", name ().c_str (), this,
	     callbackComponent ? callbackComponent->name ().c_str () : "none", callbackComponent);
#endif
    return callbackComponent;
}


void Y2Component::setCallback (Y2Component *callback)
{
#if 0
    y2warning ("setCallback of %s (%p) set to %s (%p)", name ().c_str (), this,
	     callback ? callback->name ().c_str () : "none", callback);
#endif
    callbackComponent = callback;
}


YCPValue Y2Component::doActualWork(const YCPList&, Y2Component *)
{
    y2internal ("component %s: stub function Y2Component::doActualWork() called",
		name().c_str());
    return YCPNull();
}


YCPValue Y2Component::callModule (string modulename, const YCPList& arglist,
				  Y2Component *user_interface)
{
    Y2Component *client = Y2ComponentBroker::createClient(modulename.c_str());

    if (!client)
    {
	y2error ("Can't find client component %s", modulename.c_str());
	return YCPNull();
    }

    y2debug ("Calling module %s", modulename.c_str ());

    YCPValue result = client->doActualWork (arglist, user_interface);

    delete client;

    return result;
}


SCRAgent *
Y2Component::getSCRAgent ()
{
    return NULL;
}
