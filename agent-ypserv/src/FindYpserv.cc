/*
 * FindYpserv.cc
 *
 * Does a RPC broadcast to find NIS servers for a given domain
 *
 * Authors:
 *   Martin Vidner <mvidner@suse.cz> (adaptation to yast)
 *   Thorsten Kukuk <kukuk@suse.de>  (rpc code)
 *
 * $Id$
 */

// needs linking against libnsl (in glibc.rpm) because of xdr_domainname

#include "FindYpserv.h"

#include <arpa/inet.h> // sockaddr_in
#include <sys/time.h>
#include <rpc/pmap_clnt.h>
#include <rpcsvc/yp.h>

// holds intermediate results as they arrive
static std::set<std::string> servers;

/*
  timeout handling:
  http://www.linuxhaven.de/dlhp/HOWTO-test/DE-RPC-Programmierung-HOWTO-6.html#ss6.5

  If we keep asking for more results by returning 0 in eachresult,
  it will finally timeout after 4+6+8+10+12+14 = 54 seconds,
  which may be too long.
  So let's watch the clock and stop after 10 seconds have elapsed.
  Unfortunately, we cannot set a timeout for the case
  when there are no responses, but that's life.
*/
static double start;

// stolen from Y2PluginComponent
static double
exact_time ()
{
    struct timeval time;
    gettimeofday (&time, 0);
    return (double) (time.tv_sec + 1.0e-6 * (double)(time.tv_usec));
}

static bool_t
eachresult (caddr_t resp, struct sockaddr_in *addr)
{
    if (*resp && addr)
    {
	servers.insert (inet_ntoa (addr->sin_addr));
    }
    else
    {
//	printf ("eachresult called\n");
    }

    double diff = exact_time () - start;
//    fprintf (stderr, "%g: %d\n", diff, *resp);

    // Are we statisfied with the results?
    // Yes, if we have waited long enough.
    return diff > 10.0;
}

std::set<std::string>
findYpservers (const std::string &domain)
{
    const char * cdomain = domain.c_str ();
    bool_t out;
    enum clnt_stat status;

    servers.clear ();
    start = exact_time ();
    status = clnt_broadcast (YPPROG, YPVERS, YPPROC_DOMAIN_NONACK,
			     (xdrproc_t) xdr_domainname, (char *)&cdomain,
			     (xdrproc_t) xdr_bool, (char *)&out,
			     (resultproc_t) eachresult);

    // status will be RPC timeout because we keep asking for more servers
    // Not anymore
    if (status != RPC_SUCCESS)
    {
//	fprintf (stderr, "broadcast: %s.\n", clnt_sperrno(status));
    }
    return servers;
}
