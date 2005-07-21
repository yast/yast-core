

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "ModulesAgent.h"


typedef Y2AgentComp <ModulesAgent> Y2ModulesAgentComp;

Y2CCAgentComp <Y2ModulesAgentComp> g_y2ccag_modules ("ag_modules");

