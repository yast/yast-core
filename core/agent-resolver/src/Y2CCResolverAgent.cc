

/*
 *  Author: Stanislav Visnovsky <visnov@suse.cz>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "ResolverAgent.h"


typedef Y2AgentComp <ResolverAgent> Y2ResolverAgentComp;

Y2CCAgentComp <Y2ResolverAgentComp> g_y2ccag_resolver ("ag_resolver");

