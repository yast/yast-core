

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "SystemAgent.h"


typedef Y2AgentComp <SystemAgent> Y2SystemAgentComp;

Y2CCAgentComp <Y2SystemAgentComp> g_y2ccag_system ("ag_system");

