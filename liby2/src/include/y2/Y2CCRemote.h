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

   File:       Y2CCRemote.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component Creator that creates remote components
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef Y2CCRemote_h
#define Y2CCRemote_h

#include <string>
#include "Y2ComponentCreator.h"

class Y2CCRemote : public Y2ComponentCreator
{
    /**
     * Specifies, whether to create server or client components.
     */
    bool creates_servers;

    /**
     * regular expression
     */
    mutable regex_t rx1, rx2, rx3;

    mutable int my_nl_msg_cat_cntr;
    void make_rxs () const;
    void free_rxs () const;

public:

    Y2CCRemote (bool creates_servers);

    bool isServerCreator () const;

    Y2Component* create (const char* name) const;

    /**
     * Importing a namespace from a remote subcomponent is not possible.
     */    
    virtual Y2Component* provideNamespace(const char* name_space);

private:
    /**
     * Analyses an URL of the form protocol://login:password@host/componentname
     * @param componentname URL
     * @param protocol Outputparam for the protocol (telnet, rsh, ssh)
     * @param loginname Outputparam for the login name
     * @param password Outputparam for the password
     * @param hostname Outputparam for the hostname
     * @param cname Outputparam for the component name on the remote host
     */
    bool analyseURL (const char* componentname, string& protocol,
		     string& loginname, string& password, string& hostname,
		     string& cname) const;

};


#endif // Y2CCRemote_h
