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

   File:	Y2RemoteComponent.h

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component that communicates via stdin/out/err
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef Y2RemoteComponent_h
#define Y2RemoteComponent_h


class RemoteProtocol;

#include <ycp/Parser.h>

#include "Y2Component.h"

/**
 * @short Interface to a component via remote login
 */
class Y2RemoteComponent : public Y2Component
{
    /**
     * It this component a server or a client?
     */
    const bool is_server;

    /**
     * Access to the user interface
     */
    Y2Component *user_interface;

    /**
     *
     */
    string protocol;

    /**
     *
     */
    string loginname;

    /**
     *
     */
    string password;

    /**
     *
     */
    string hostname;

    /**
     *
     */
    string componentname;

public:

    /**
     * Creates a new remote component.
     */
    Y2RemoteComponent (bool is_server, string protocol, string loginname,
		       string hostname, string password, string componentname);

    /**
     *
     */
    string name() const;

    /**
     *
     */
    YCPValue evaluate (const YCPValue& command);

    /**
     * Defined only for the "cat" component: Prints
     * result(..result..) on stdout.
     */
    // void result(const YCPValue& result);

    /**
     * Sets the commandline options of the server. Server options
     * for the cat server are simply ignored.
     *
     * This method is only defined, if the component is a server.
     */
    // void setServerOptions(int argc, char **argv);

    /**
     * Here the client does its actual work.
     * @param arglist YCPList of client arguments.
     * @param user_interface Option display server (user interface)
     * @return The result value (exit code) of the called client. The
     * result code has </i>not<i> yet been sent to the display server.
     * Destroy it after use.
     *
     * This method is only defined, if the component is a client.
     */
    YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);

private:

    /**
     * The remote protocol.
     */
    RemoteProtocol *rp;

    /**
     * The parser.
     */
    Parser parser;

    /**
     * Sends a YCP value to the remote component.
     */
    void send (const YCPValue& v, bool eat_echo);

    /**
     * Receive a YCP value from the remote component.
     */
    YCPValue receive ();

    /**
     * Initiates a telnet session
     */
    bool initTelnetSession(class ExternalProgram &);

    /**
     * Ask the user for the password.
     * @param ok I set this value to false, if the user pressed 'cancel'
     * otherwise I'll set it to true.
     * @return the typed in password
     */
    string askPassword(bool& ok);

    /**
     *
     */
    bool analyseURL(string);

    /**
     * Is the remote connection up so that we can send callbacks there?
     */
    bool is_up;

    /**
     *
     */
    bool connect ();

};


#endif // Y2RemoteComponent_h
