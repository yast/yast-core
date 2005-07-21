

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/run_agent.h>

#include "../src/SystemAgent.h"


int
main (int argc, char *argv[])
{
    run_agent <SystemAgent> (argc, argv, true);
}
