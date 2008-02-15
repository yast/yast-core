

/*
 *  Author: Ladislav Slezák <lslezak@novell.com>
 */


#include <scr/run_agent.h>

#include "../src/ProcessAgent.h"


int
main (int argc, char *argv[])
{
    run_agent <ProcessAgent> (argc, argv, true);
}
