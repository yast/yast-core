/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	Y2CCRemote.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

/*
 * Component Creator that creates remote components
 */

#include <stdio.h>
#include <regex.h>

#include <ycp/y2log.h>
#include "Y2CCRemote.h"
#include "Y2RemoteComponent.h"


Y2CCRemote::Y2CCRemote (bool creates_servers)
    : Y2ComponentCreator (Y2ComponentBroker::BUILTIN),
      creates_servers (creates_servers)
{
    make_rxs ();
}


bool
Y2CCRemote::isServerCreator () const
{
    return creates_servers;
}


void
Y2CCRemote::make_rxs () const
{
    extern int _nl_msg_cat_cntr;
    my_nl_msg_cat_cntr = _nl_msg_cat_cntr;

    // protocol://user:password@host/component
    regcomp (&rx1, "\\(telnet\\|.sh\\|rlogin\\)://\\([[:alpha:]][[:alnum:]_]*\\):"
	     "\\([^@]\\+\\)@\\([[:alnum:]._]\\+\\)/\\(.*\\)", 0);

    // protocol://user@host/component
    regcomp (&rx2, "\\(telnet\\|.sh\\|rlogin\\)://\\([[:alpha:]][[:alnum:]_]*\\)@"
	     "\\([[:alnum:]._]\\+\\)/\\(.*\\)", 0);

    // protocol://user/component
    regcomp (&rx3, "\\(su\\|sudo\\)://\\([[:alpha:]][[:alnum:]_]*\\)/\\(.*\\)", 0);
}


void
Y2CCRemote::free_rxs () const
{
    regfree (&rx1);
    regfree (&rx2);
    regfree (&rx3);
}


Y2Component*
Y2CCRemote::create (const char* componentname) const
{
    string protocol, loginname, password, hostname, cname;
    if (analyseURL (componentname, protocol, loginname, password, hostname, cname))
	return new Y2RemoteComponent (creates_servers, protocol, loginname,
				      password, hostname, cname);
    return 0;
}


Y2Component*
Y2CCRemote::provideNamespace(const char*)
{
    y2debug ("Y2RemoteComponent cannot import namespaces");
    return NULL;
}


bool
Y2CCRemote::analyseURL (const char* componentname, string& protocol,
			string& loginname, string& password, string& hostname,
			string& cname) const
{
    extern int _nl_msg_cat_cntr;
    if (_nl_msg_cat_cntr != my_nl_msg_cat_cntr)
    {
	free_rxs ();
	make_rxs ();
    }

    regmatch_t matches[6];

    if (regexec (&rx1, componentname, 6, matches, 0) == 0)
    {
	protocol = string (componentname + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
	loginname = string (componentname + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
	password = string (componentname + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);
	hostname = string (componentname + matches[4].rm_so, matches[4].rm_eo - matches[4].rm_so);
	cname = string (componentname + matches[5].rm_so, matches[5].rm_eo - matches[5].rm_so);

	return true;
    }

    if (regexec (&rx2, componentname, 5, matches, 0) == 0)
    {
	protocol = string (componentname + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
	loginname = string (componentname + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
	password = string ("");
	hostname = string (componentname + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);
	cname = string (componentname + matches[4].rm_so, matches[4].rm_eo - matches[4].rm_so);

	return true;
    }

    if (regexec (&rx3, componentname, 4, matches, 0) == 0)
    {
	protocol = string (componentname + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
	loginname = string (componentname + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
	password = string ("");
	hostname = string ("");
	cname = string (componentname + matches[3].rm_so, matches[3].rm_eo - matches[3].rm_so);

	return true;
    }

    return false;
}


Y2CCRemote g_y2ccremote0 (true);
Y2CCRemote g_y2ccremote1 (false);
