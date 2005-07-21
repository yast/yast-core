

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/run_agent.h>

#include "../src/HwProbe.h"


int
main (int argc, char* argv[])
{
    run_agent <HwProbe> (argc, argv, true);
}
