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

   File:	RloginProtocol.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-


#ifndef RloginProtocol_h
#define RloginProtocol_h

#include "RemoteProtocol.h"


class RloginProtocol : public RemoteProtocol
{
    const bool is_server;

    std::string loginname;
    std::string componentname;

    /**
     * Set to true, if rlogin just asked us
     * with 'Password:' and we haven't answered yet.
     */
    bool at_password_prompt;

public:
    /**
     * Create a rlogin session
     * @param user_interface the user interface in case the user must
     * be asked for the password
     * @param loginname the username for the login
     * @param hostname the host to log into
     * @param componentname component to start on the remote machine
     */
    RloginProtocol (bool is_server, std::string loginname, std::string hostname,
		    std::string componentname);

    callStatus callComponent (std::string password = "", bool passwd_supplied = false);

    /**
     * Returns true, if output is output appears at input again.
     */
    bool doesEcho () const;

};


#endif // RloginProtocol_h
