/*
 *  Basic setup for the standalone "scr" binary.
 *
 *  Author: Stanislav Visnovsky <visnov@suse.cz>
 */

#include "scr/SCR.h"
#include "ScriptingAgent.h"

// register builtins
SCR scr_builtins;

// create agent for SCR calls
ScriptingAgent agent;
