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

   File:	SuProtocol.h

   Author:	Thomas Roelz <tom@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/


#ifndef SuProtocol_h
#define SuProtocol_h

#include "RemoteProtocol.h"


class SuProtocol : public RemoteProtocol
{
    const bool is_server;

    std::string loginname;
    std::string componentname;

    /**
     * Set to true, if su just asked for the password
     * and we haven't answered yet.
     */
    bool at_password_prompt;

public:
    /**
     * Create a su session
     * @param user_interface the user interface in case the user must
     * be asked for the password
     * @param loginname the username for the login
     * @param componentname component to start on the remote machine
     */
    SuProtocol (bool is_server, std::string loginname, std::string componentname);

    callStatus callComponent (std::string password = "", bool passwd_supplied = false);

    /**
     * Returns true, if output is output appears at input again.
     */
    bool doesEcho () const;

};


#endif // SuProtocol_h
