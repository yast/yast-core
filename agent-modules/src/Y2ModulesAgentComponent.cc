/* Y2ModulesAgentComponent.cc
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 */

#include "Y2ModulesAgentComponent.h"
#include <scr/SCRInterpreter.h>
#include "ModulesAgent.h"


Y2ModulesAgentComponent::Y2ModulesAgentComponent()
    : interpreter(0),
      agent(0)
{
}


Y2ModulesAgentComponent::~Y2ModulesAgentComponent()
{
    if (interpreter)
    {
	delete interpreter;
	if (agent)
	{
	    delete agent;
	}
    }
}


bool
Y2ModulesAgentComponent::isServer() const
{
    return true;
}

string
Y2ModulesAgentComponent::name() const
{
    return "ag_modules";
}


YCPValue Y2ModulesAgentComponent::evaluate(const YCPValue& value)
{
    if (!interpreter) {
	agent = new ModulesAgent();
	interpreter = new SCRInterpreter(agent);
    }
    
    return interpreter->evaluate(value);
}

