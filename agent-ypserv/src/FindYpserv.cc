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

#include <rpc/pmap_clnt.h>
#include <rpcsvc/yp.h>

// holds intermediate results as they arrive
static std::set<std::string> servers;

static bool_t
eachresult (bool_t *out, struct sockaddr_in *addr)
{
    if (*out && addr)
    {
	servers.insert (inet_ntoa (addr->sin_addr));
    }
    else
    {
//	printf ("eachresult called\n");
    }

    return 0;
}

std::set<std::string>
findYpservers (const std::string &domain)
{
    const char * cdomain = domain.c_str ();
    bool_t out;
    enum clnt_stat status;

    servers.clear ();
    status = clnt_broadcast (YPPROG, YPVERS, YPPROC_DOMAIN_NONACK,
			     (xdrproc_t) xdr_domainname, (char *)&cdomain,
			     (xdrproc_t) xdr_bool, (char *)&out,
			     (resultproc_t) eachresult);

    // status will be RPC timeout because we keep asking for more servers
    if (status != RPC_SUCCESS)
    {
//	fprintf (stderr, "broadcast: %s.\n", clnt_sperrno(status));
    }
    return servers;
}
