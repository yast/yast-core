/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */

#include <scr/run_agent.h>

#include "../src/ResolverAgent.h"


int
main (int argc, char *argv[])
{
    run_agent <ResolverAgent> (argc, argv, true);
}
