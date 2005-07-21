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

   File:	SuProtocol.cc

   Author:	Thomas Roelz <tom@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "SuProtocol.h"
#include <ycp/y2log.h>


SuProtocol::SuProtocol (bool is_server, string loginname, string componentname)
    : RemoteProtocol ("su " + loginname + " -c 'echo Y a S T 2 ; "
		      "exec /usr/lib/YaST2/bin/y2base " +
		      (is_server ? "stdio " + componentname + " -s'" :
		       componentname + " -s stdio'"), true),
      is_server (is_server),
      loginname (loginname),
      componentname (componentname),
      at_password_prompt (false)
{
}


RemoteProtocol::callStatus
SuProtocol::callComponent (string password, bool passwd_supplied)
{
    y2debug ("password: %s, supplied: %i", password.c_str(), passwd_supplied );

    const string messages[] =
    {
	"Password:",                  // password should be entered
	"incorrect password",         // login not successful (no retry)
	"Y a S T 2",                  // Magic string (written by us and echoed)
	"invalid option",             // error in command line
    };

    const int num_messages = sizeof(messages) / sizeof(string);

    // If we left this function last time due to a missing password
    // and we have it now, this means the prompt 'Password:' is still
    // active.

    if (at_password_prompt && passwd_supplied)
    {
	send(password + "\r\n");
	at_password_prompt = false;
	passwd_supplied = false;
    }

    while (true)
    {
	switch (expectOneOf (messages, num_messages, 4096))
	{
	    case 0: // Password
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

	    case 1: // Login not successful
		return RP_ERROR;

	    case 2: // Magic string read
		return RP_OK;

	    default:   // every other thing is fatal
		return RP_ERROR;
	}
    }
}


bool
SuProtocol::doesEcho () const
{
    return true;
}

