/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Ini agent implementation
 *
 * Authors:
 *   Petr Blahos <pblahos@suse.cz>
 *
 * $Id$
 */

#include "Y2IniAgentComponent.h"
#include <scr/SCRInterpreter.h>
#include "IniAgent.h"


Y2IniAgentComponent::Y2IniAgentComponent()
    : interpreter(0),
      agent(0)
{
}


Y2IniAgentComponent::~Y2IniAgentComponent()
{
    if (interpreter) {
        delete interpreter;
        delete agent;
    }
}


bool
Y2IniAgentComponent::isServer() const
{
    return true;
}

string
Y2IniAgentComponent::name() const
{
    return "ag_ini";
}


YCPValue Y2IniAgentComponent::evaluate(const YCPValue& value)
{
    if (!interpreter) {
        agent = new IniAgent();
        interpreter = new SCRInterpreter(agent);
    }
    
    return interpreter->evaluate(value);
}

