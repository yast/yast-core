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

   File:	TelnetProtocol.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-


#ifndef TelnetProtocol_h
#define TelnetProtocol_h

#include "RemoteProtocol.h"


class TelnetProtocol : public RemoteProtocol
{
    const bool is_server;

    std::string loginname;
    std::string componentname;

public:
    /**
     * Create a telnet session
     * @param user_interface the user interface in case the user must
     * be asked for the password
     * @param loginname the username for the login
     * @param hostname the host to log into
     * @param componentname component to start on the remote machine
     */
    TelnetProtocol (bool is_server, std::string loginname, std::string hostname,
		    std::string componentname);

    callStatus callComponent (std::string password = "", bool passwd_supplied = false);

    /**
     * Returns true, if output is output appears at input again.
     */
    bool doesEcho () const;

};


#endif // TelnetProtocol_h
