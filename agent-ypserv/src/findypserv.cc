/*
 * A test program for findYpservers
 * $Id$
 */

#include "FindYpserv.h"
#include <cstdio>

int main (int argc, char **argv)
{
    if (argc < 2)
    {
	fprintf (stderr, "Usage: %s <nis_domain>\n", argv[0]);
	return 1;
    }
    else
    {
	typedef std::set<std::string> servers_t;
	servers_t servers = findYpservers (argv[1]);
	for (servers_t::iterator
		 i = servers.begin (),
		 e = servers.end ();
	     i != e;
	     ++i)
	{
	    printf ("%s\n", i->c_str ());
	}
	return 0;
    }
}
