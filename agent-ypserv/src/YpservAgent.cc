/*
 * YpservAgent.cc
 *
 * An agent for finding NIS servers
 *
 * Authors: Martin Vidner <mvidner@suse.cz>
 *
 * $Id$
 */


#include "YpservAgent.h"
#include "FindYpserv.h"
#include <YCP.h>
#include <ycp/y2log.h>
#include <string>
#include <set>
using std::string;
using std::set;

/**
 * Read function
 */
YCPValue
YpservAgent::Read (const YCPPath& path, const YCPValue& arg)
{
    y2debug ("Read (%s)", path->toString().c_str());

    if (path->isRoot())
    {
	return YCPError ("Read () called without sub-path");
    }

    const string cmd = path->component_str (0); // just a shortcut

    if (cmd == "find")
    {
	if (path->length () < 2)
	{
	    return YCPError ("Read (.find) called without domain");
	}
	else
	{
	    const string domain = path->component_str (1);

	    // just in case the timeout does not work
	    y2debug ("Calling findYpservers (\"%s\").", domain.c_str ());

	    // The real thing. The rest is glue.
	    set<string> servers = findYpservers (domain);

	    y2debug ("Returned from findYpservers.");

	    // now copy the result to a YCP form
	    YCPList ycpservers;
	    ycpservers->reserve (servers.size ());
	    for (set<string>::iterator
		     i = servers.begin (),
		     e = servers.end ();
		 i != e;
		 ++i)
	    {
		ycpservers->add (YCPString (*i));
	    }
	    return ycpservers;
	}
    }

    return YCPError (string("Undefined subpath for Read (") + path->toString() + ")");
}


/**
 * Write function
 */
YCPValue
YpservAgent::Write (const YCPPath& path, const YCPValue& value,
		    const YCPValue& arg)
{
    y2debug ("Write (%s)", path->toString().c_str());

    return YCPError (string("Undefined subpath for Write (") + path->toString() + ")",
		     YCPBoolean (false));
}


/**
 * Execute functions
 */
YCPValue
YpservAgent::Execute (const YCPPath& path, const YCPValue& value,
		      const YCPValue& arg)
{
    y2debug ("Execute (%s)", path->toString().c_str());

    return YCPError (string("Undefined subpath for Execute (") + path->toString() + ")");
}

/**
 * Get a list of all subtrees
 */
YCPValue
YpservAgent::Dir (const YCPPath& path)
{
    y2debug ("Dir (%s)", path->toString().c_str());
    
    if (path->isRoot())
    {
	YCPList ret;
	ret->add (YCPString ("find"));
	return ret;
    }
    else
    {
	// don't bother with errors
	return YCPList ();
    }
}

/* EOF */
