

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/run_agent.h>

#include "../src/ScriptingAgent.h"


int
main (int argc, char *argv[])
{
    // Here we must not load the scr files since they must be evaluated
    // in the subagents. So the last argument is false.
    run_agent <ScriptingAgent> (argc, argv, false);
}
