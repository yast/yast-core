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

   File:	SudoProtocol.cc

   Author:	Thomas Roelz <tom@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include "SudoProtocol.h"
#include <ycp/y2log.h>


SudoProtocol::SudoProtocol (bool is_server, string loginname, string componentname)
    : RemoteProtocol ("sudo -u " + loginname + " echo Y a S T 2 ; "
		      "exec /usr/lib/YaST2/bin/y2base " +
		      (is_server ? "stdio " + componentname + " -s" :
		       componentname + " -s stdio"), true),
      is_server (is_server)
{
}


RemoteProtocol::callStatus
SudoProtocol::callComponent (string password, bool passwd_supplied)
{
    y2debug ("password: %s, supplied: %i", password.c_str(), passwd_supplied );

    const string messages[] =
    {
	"no passwd entry",            // login not successful (no retry)
	"Y a S T 2",                  // Magic string (written by us and echoed)
	"usage",                      // error in command line
    };

    const int num_messages = sizeof(messages) / sizeof(string);

    while (true)
    {
	switch (expectOneOf (messages, num_messages, 4096))
	{
	    case 0: // Login not successful
		return RP_ERROR;

	    case 1: // Magic string read
		return RP_OK;

	    default:   // every other thing is fatal
		return RP_ERROR;
	}
    }
}


bool
SudoProtocol::doesEcho () const
{
    return true;
}

