

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "AnyAgent.h"


typedef Y2AgentComp <AnyAgent> Y2AnyAgentComp;

Y2CCAgentComp <Y2AnyAgentComp> g_y2ccag_anyagent ("ag_anyagent");

