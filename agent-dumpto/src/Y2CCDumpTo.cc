

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>
#include <scr/SCRInterpreter.h>

#include "DumpTo.h"


typedef Y2AgentComp <DumpTo> Y2DumpToAgentComp;

Y2CCAgentComp <Y2DumpToAgentComp> g_y2ccag_dumpto ("ag_dumpto");

