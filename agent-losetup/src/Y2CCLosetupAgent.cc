

/*
 *  Author: Stanislav Visnovsky <visnov@suse.cz>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "LosetupAgent.h"


typedef Y2AgentComp <LosetupAgent> Y2LosetupAgentComp;

Y2CCAgentComp <Y2LosetupAgentComp> g_y2ccag_losetup ("ag_losetup");

