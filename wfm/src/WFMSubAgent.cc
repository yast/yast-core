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

   File:	WFMSubAgent.cc

   Author:	Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/


#include <ycp/y2log.h>
#include <y2/Y2ComponentBroker.h>
#include <scr/SCRAgent.h>
#include "../../scr/src/StdioSCRAgent.h"
#include <WFMSubAgent.h>


WFMSubAgent::WFMSubAgent (const string& name, int handle)
    : my_name (name),
      my_handle (handle),
      my_comp (0),
      my_agent (0)
{
    // TODO when perl-bootloader die, remove whole system namespacing
    // check if name is scr ( can be prepended by chroot like "chroot=/mnt:scr" )
    if (name.find("scr") == string::npos && name.find("chroot") != string::npos)
    {
      y2internal("WFMSubAgent support chrrot only for scr component, but not '%s'.",
        name.c_str());
      abort();
    }
}


WFMSubAgent::~WFMSubAgent ()
{
    if (my_agent)
    {
	delete my_agent;
    }
}


bool
WFMSubAgent::start ()
{
    if (!my_comp)
    {
	y2debug ("Creating SubAgent: %d %s", my_handle, my_name.c_str ());
        // different behavior in chroot and non-chroot as for chroot we need to
        // handle specially system namespace via scr_remote
        // TODO remove after perl-bootloader die, then else branch is enough
        if (my_name.find("chroot") != string::npos)
        {
            Y2Component *scr_comp = Y2ComponentBroker::createServer (my_name.c_str ());
            my_comp = Y2ComponentBroker::createServer ((my_name+"_remote").c_str ());
            if (!scr_comp)
            {
	        ycp2error ("Can't create component 'scr'");
                return false;
	    }
            // WFM subagent use specialized agent for remote and scr_remote as
            // component
            // see Y2WFMComponent::SCRSetDefault implementation for details 
            // how it works
            my_agent = scr_comp->getSCRAgent();
        }
        else
        {
            my_comp = Y2ComponentBroker::createServer (my_name.c_str ());
        }
    }

    return my_comp != 0;
}


bool
WFMSubAgent::start_and_check (bool check_version, int* error)
{
    if (!start ())
    {
	*error = -1;
	return false;
    }

    YCPValue q1 = YCPTerm ("SuSEVersion");
    YCPValue q2 = YCPVoid ();

    YCPValue a = my_comp->evaluate (check_version ? q1 : q2);

    if (a.isNull ())
    {
	*error = -1;
	return false;
    }

    if (check_version)
    {
	y2debug ("SuSEVersion \"%s\" %s", SUSEVERSION, a->toString ().c_str ());

	if (!a->isString () || a->asString ()->value () != SUSEVERSION)
	{
	    *error = -2;
	    return false;
	}
    }

    return true;
}

