#ifndef Y2SCRCOMPONENT_H
#define Y2SCRCOMPONENT_H

#include <scr/Y2AgentComponent.h>
#include "ScriptingAgent.h"

class Y2SCRComponent : public Y2AgentComp<ScriptingAgent>
{
public:
  Y2SCRComponent(const char* root);

  ~Y2SCRComponent();
};

#endif
