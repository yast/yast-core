

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include <scr/run_agent.h>

#include "../src/IniAgent.h"
#include "../../scr/src/ScriptingAgent.h"

int
main (int argc, char *argv[])
{
    // find the ".ycp" argument
    string scrconf = process_options (argc, argv);
    // substitute
    string::size_type p = scrconf.rfind (".ycp");
    if (p != string::npos)
    {
	scrconf.replace (p, 4, ".scr");
    }

    SCRAgent *agent = new ScriptingAgent (scrconf);
    if (!agent)
    {
	fprintf (stderr, "Failed to create Agent\n");
	exit (EXIT_FAILURE);
    }

    run_agent_instance (argc, argv, false, agent);

    delete agent;
    exit (EXIT_SUCCESS);
}
