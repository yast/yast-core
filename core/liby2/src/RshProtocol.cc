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

   File:	RshProtocol.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "RshProtocol.h"
#include <ycp/y2log.h>


RshProtocol::RshProtocol (bool is_server, string loginname, string hostname,
			  string componentname, string shell)
    : RemoteProtocol (shell + " -l " + loginname + " " + hostname +
		      " 'echo Y a S T 2 ; exec /usr/lib/YaST2/bin/y2base " +
		      (is_server ? "stdio " + componentname + " -s'" :
		       componentname + " -s stdio'"), true),
      is_server (is_server),
      used_shell (shell),
      loginname (loginname),
      componentname (componentname),
      at_password_prompt (false)
{
}


RemoteProtocol::callStatus
RshProtocol::callComponent (string password, bool passwd_supplied)
{
    const string messages[] =
    {
	"Y a S T 2",             // Login successfull
	"password:",             // password should be entered
	"':",                    // passphrase should be entered
	"continue connecting",   // hostname not yet known
    };

    const int num_messages = sizeof(messages) / sizeof(string);

    // If we left this function last time due to a missing password
    // and we have it now, this means the prompt 'Password:' is still
    // active.

    if (at_password_prompt && passwd_supplied)
    {
	send (password + "\r\n");
	at_password_prompt = false;
	passwd_supplied = false;
    }

    while (true)
    {
	switch (expectOneOf (messages, num_messages, 4096))
	{
	    case 0:   // magic string read
		return RP_OK;
		break;

	    case 1:   // Password required
	    case 2:   // Passphrase required
		if (!passwd_supplied)
		{
		    // If no password is supplied, remember to send it next time
		    at_password_prompt = true;
		    return RP_PASSWD;
		}
		else
		{
		    // Use password only once. If it is wrong, so will it be
		    // on the second try.
		    passwd_supplied = false;
		    send (password + "\n");
		}
		break;

	    case 3:   // host not yet known (continue anyway)
		send ("yes\n");
		break;

	    default:  // every other thing is an error
		return RP_ERROR;
		break;
	}
    }
}


bool
RshProtocol::doesEcho () const
{
    return true;
}
