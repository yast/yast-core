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

   File:       Y2ScriptComponent.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * Component that starts wfm to execute a YCP script
 *
 */

#include "Y2ScriptComponent.h"
#include "Y2ComponentBroker.h"
#include <ycp/y2log.h>


Y2ScriptComponent::Y2ScriptComponent (string client_name, string fullname,
				      const YCPValue& script)
    : script (script),
      client_name (client_name),
      fullname (fullname)
{
}


Y2ScriptComponent::~Y2ScriptComponent()
{
}



string Y2ScriptComponent::name() const
{
    return client_name;
}


YCPValue
Y2ScriptComponent::evaluate (const YCPValue& command)
{
    return getCallback ()->evaluate (command);
}


YCPValue Y2ScriptComponent::doActualWork(const YCPList& arglist, Y2Component *user_interface)
{
    // Let the component "wfm", the Workflowmanager do the work.
    Y2Component *workflowmanager = Y2ComponentBroker::createClient("wfm");

    if (!workflowmanager)
    {
	y2error ("Can't locate component wfm (Workflowmanager)");
	return YCPVoid();
    }

    setCallback (workflowmanager);
    Y2Component *callback = 0;
    if (user_interface)
    {
	callback = user_interface->getCallback ();
	user_interface->setCallback (workflowmanager);
    }

    // Prepare the arguments. It has the form [script, [clientargs...]]
    YCPList wfm_arglist;
    wfm_arglist->add(script);
    wfm_arglist->add(YCPString(name()));
    wfm_arglist->add (YCPString (fullname));
    wfm_arglist->add(arglist);

    // Let the wfm do the work
    YCPValue result = workflowmanager->doActualWork(wfm_arglist, user_interface);

    // restore callback pointer to old value, windowmanager will be deleted
    if (user_interface)
    {
	user_interface->setCallback (callback);
    }
    delete workflowmanager;
    return result;
}

