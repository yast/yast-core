

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/Y2AgentComponent.h>
#include <scr/Y2CCAgentComponent.h>

#include "ScriptingAgent.h"


// create Agent called Y2SCRComp out of ScriptingAgent
typedef Y2AgentComp <ScriptingAgent> Y2SCRComp;

// create Component Creator for Y2SCRComp
Y2CCAgentComp <Y2SCRComp> g_y2ccscr ("scr");

