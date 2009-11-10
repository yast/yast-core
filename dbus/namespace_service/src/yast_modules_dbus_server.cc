/*

*/

#include "DBusModulesServer.h"
#include <cstring>
#include <iostream>

int main(int argc, char **argv)
{

    bool forever = false;
    bool badopts = false;
    bool test_mode = false;

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
	    else if (!strcmp(argv[index], "--test"))
	    {
		test_mode = true;
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
	std::cerr << "       --test            Set the test mode - Connect to the session bus (system is the default),\n";
	std::cerr << "                         disable PolicyKit checks. Useful for testing or debugging.\n";
	std::cerr << "       <namespace>       Preload an yast namespace and export it on DBus\n";
	return 1;
    }

    DBusModulesServer server(modules, test_mode);
    bool connected = server.connect();

    if (connected)
    {
	server.run(forever);
	return 0;
    }

    // indicate an error
    return 1;
}
