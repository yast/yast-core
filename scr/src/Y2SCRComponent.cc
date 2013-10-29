#include "Y2SCRComponent.h"
#include "ScriptingAgent.h"


Y2SCRComponent::Y2SCRComponent(const char *root) :
  Y2AgentComp<ScriptingAgent>("scr")
{
  agent = new ScriptingAgent(root);
}

Y2SCRComponent::~Y2SCRComponent()
{
  //agent is destructed in parent
}
