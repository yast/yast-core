

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#undef Y2LOG
#define Y2LOG "scr"
#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>
#undef Y2LOG
#define Y2LOG "agent-ini"

#include "IniAgent.h"


typedef Y2AgentComp <IniAgent> Y2IniAgentComp;

Y2CCAgentComp <Y2IniAgentComp> g_y2ccag_ini ("ag_ini");

