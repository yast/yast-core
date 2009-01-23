

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "HalAgent.h"


typedef Y2AgentComp <HalAgent> Y2HalAgentComp;

Y2CCAgentComp <Y2HalAgentComp> g_y2ccag_hal ("ag_hal");

