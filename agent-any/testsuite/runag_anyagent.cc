

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/run_agent.h>

#include "../src/AnyAgent.h"


int
main (int argc, char *argv[])
{
    run_agent <AnyAgent> (argc, argv, true);
}
