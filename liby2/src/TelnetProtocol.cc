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

   File:	TelnetProtocol.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "TelnetProtocol.h"
#include <ycp/y2log.h>


TelnetProtocol::TelnetProtocol (bool is_server, string loginname, string hostname,
				string componentname)
    : RemoteProtocol ("telnet -l " + loginname + " " + hostname, true),
      is_server (is_server),
      loginname (loginname),
      componentname (componentname)
{
}


RemoteProtocol::callStatus
TelnetProtocol::callComponent (string password, bool passwd_supplied)
{
    y2debug ("password: %s, supplied: %i", password.c_str(), passwd_supplied);
    const string messages[] =
    {
	"Last login:", // this must be ignored!
	"login: ",
	"Password: ",
	"Login incorrect",
	"Have a lot of fun...",
	"Y a S T 2",
	"Unknown host",
	"invalid option",
    };

    const int num_messages = sizeof(messages) / sizeof(string);

    while (true)
    {
	switch (expectOneOf (messages, num_messages, 4096))
	{
	    case 0: // Last login:   ignore this line
		break;

	    case 1: // login
		send (loginname + "\n");
		break;

	    case 2: // Password
		if (!passwd_supplied) {
		    // If no password is supplied, we simple hit <Enter>
		    // at the password prompt. Otherwise our automata
		    // would get stuck here.
		    send ("\n");
		}
		else
		{
		    // Use password only once. If it is wrong, so will it be
		    // on the second try.
		    passwd_supplied = false;
		    send (password + "\n");
		}
		break;

	    case 3: // Login incorrect
		return RP_PASSWD;

	    case 4: // Have a lot of fun... (Bash starting up)
		// Start command. Force output of magic string.
		// We need to detect when the output stream of the newly
		// started YaST2 component begins.
		if (is_server)
		    send ("echo Y  a  S  T  2 ; exec /usr/lib/YaST2/bin/y2base stdio "
			  + componentname + " -s\n");
		else
		    send ("echo Y  a  S  T  2 ; exec /usr/lib/YaST2/bin/y2base "
			  + componentname + " -s stdio\n");
		break;

	    case 5: // Magic string read
		return RP_OK;

	    default:
		return RP_ERROR;
	}
    }
}


bool
TelnetProtocol::doesEcho () const
{
    return true;
}
