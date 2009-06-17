/*

*/

#include "DBusModulesServer.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{

    bool forever = false;
    bool badopts = false;
    bool use_session_bus = false;

    DBusModulesServer::NameSpaceList modules;

    if (argc > 1) {

	int index = 1;

	while(index < argc)
	{
	    if (!strcmp(argv[index], "--help"))
	    {
		badopts = true;
	    }
	    else if (!strcmp(argv[index], "--disable-timer")) // like in packagekitd
	    {
		forever = true;
	    }
	    else if (!strcmp(argv[index], "--session"))
	    {
		use_session_bus = true;
	    }
	    else
	    {
		modules.push_back(argv[index]);
	    }

	    index++;
	}
    }

    if (badopts)
    {
	std::cerr << "Usage: " << argv[0] << " [--help] [--disable-timer] [--session] <namespace> <namespace>..." << std::endl;
	std::cerr << "       --help            Print this text\n";
	std::cerr << "       --disable-timer   Disable automatic shutdown of the service, useful for debugging\n";
	std::cerr << "       --session         Connect to the session bus (system is the default), useful for debugging\n";
	std::cerr << "       <namespace>       Preload an yast namespace and export it on DBus\n";
	return 1;
    }

    DBusModulesServer server(modules, use_session_bus);
    bool connected = server.connect();

    if (connected)
    {
	server.run(forever);
	return 0;
    }

    // indicate an error
    return 1;
}
