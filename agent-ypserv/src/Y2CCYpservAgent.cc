/*
 *  Author: Arvin Schnell <arvin@suse.de>
 *	    Martin Vidner <mvidner@suse.cz>
 */

#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>
#include <scr/SCRInterpreter.h>

#include "YpservAgent.h"

typedef Y2AgentComp <YpservAgent> Y2YpservAgentComp;

Y2CCAgentComp <Y2YpservAgentComp> g_y2ccag_ypserv ("ag_ypserv");
