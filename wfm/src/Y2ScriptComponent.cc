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

   Author:     Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Component that starts wfm to execute a YCP script
 *
 */

#include "Y2ScriptComponent.h"

#include <y2/Y2ComponentBroker.h>
#include <ycp/y2log.h>


Y2ScriptComponent::Y2ScriptComponent ()
    : script (YCPNull()),
      client_name (""),
      fullname (""),
      m_wfm (0)
{
}


Y2ScriptComponent::~Y2ScriptComponent()
{
    if (m_wfm) delete m_wfm;
}



void Y2ScriptComponent::setupComponent (string cn, string fn,
				      const YCPValue& sc)
{
    script = sc;
    client_name = cn;
    fullname = fn;
//    m_wfm = 0;
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


Y2Component * Y2ScriptComponent::wfm_instance ()
{
    if (m_wfm != 0) return m_wfm;
    
    // Let the component "wfm", the Workflowmanager do the work.
    m_wfm = Y2ComponentBroker::createClient("wfm");

    if (!m_wfm)
    {
	y2error ("Can't locate component wfm (Workflowmanager)");
    }
    return m_wfm;
}

YCPValue Y2ScriptComponent::doActualWork(const YCPList& arglist, Y2Component *user_interface)
{

    if (! wfm_instance() ) return YCPVoid ();

    // Prepare the arguments. It has the form [script, [clientargs...]]
    YCPList wfm_arglist;
    wfm_arglist->add(script);
    wfm_arglist->add(YCPString(name()));
    wfm_arglist->add (YCPString (fullname));
    wfm_arglist->add(arglist);

    // Let the wfm do the work
    YCPValue result = wfm_instance()->doActualWork(wfm_arglist, user_interface);
    
    // remove reference to the script
    script = YCPNull();

    return result;
}

Y2Namespace* Y2ScriptComponent::import (const char *name_space)
{
    if (! wfm_instance ()) return 0;
    
    return wfm_instance()->import (name_space);
}

Y2ScriptComponent* Y2ScriptComponent::m_instance = NULL;

Y2ScriptComponent* Y2ScriptComponent::instance ()
{
    if (m_instance == NULL ) m_instance = new Y2ScriptComponent ();
    
    return m_instance;
}
