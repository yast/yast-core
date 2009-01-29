

/*
 *  Author: Arvin Schnell <aschnell@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "DbusAgent.h"


typedef Y2AgentComp<DbusAgent> Y2DbusAgentComp;

Y2CCAgentComp<Y2DbusAgentComp> g_y2ccag_dbus("ag_dbus");

