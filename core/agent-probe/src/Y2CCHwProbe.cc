

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "HwProbe.h"


typedef Y2AgentComp <HwProbe> Y2HwProbeComp;

Y2CCAgentComp <Y2HwProbeComp> g_y2ccag_hwprobe ("ag_hwprobe");

