

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "DummyAgent.h"


typedef Y2AgentComp <DummyAgent> Y2DummyAgentComp;

Y2CCAgentComp <Y2DummyAgentComp> g_y2ccag_dummy ("ag_dummy");

