

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/run_agent.h>

#include "../src/ModulesAgent.h"


int
main (int argc, char *argv[])
{
    run_agent <ModulesAgent> (argc, argv, true);
}
