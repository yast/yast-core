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

   File:       RemoteProtocol.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef RemoteProtocol_h
#define RemoteProtocol_h

#include <y2util/ExternalProgram.h>

class RemoteProtocol : public ExternalProgram
{
public:
    /**
     * Initializes the remote protocol. Starts the external program
     * like telnet, ssh, su, rsh, ...
     * @param string commandline parsed by the shell
     * @param use_pty, set to true, if you want to communicate over ptys instead
     * of pipes.
     */
    RemoteProtocol (std::string commandline, bool use_pty = false);

    /**
     * Returns values of @ref #callComponent.
     */
    enum callStatus { RP_OK, RP_PASSWD, RP_ERROR };

    /**
     * Tries to do the handshake, this is supply password, enter loginname
     * and answer other interactive questions. Also starts the Y2 component
     * on the other machine.
     * @param password A password for the login.
     * @param bool passwd_supplied This flag determines whether the password given
     * should be used.
     * @return RP_OK, if the remote component is now running, RP_PASSWD,
     * if the protocol needs a login password (provide a password and try again)
     * or RP_ERROR, if an unrecoverable error occured.
     */
    virtual callStatus callComponent (std::string password = "",
				      bool passwd_supplied = false) = 0;

    /**
     * Determines, whether output to the protocol program (telnet, rsh, ...) is
     * echoed.
     */
    virtual bool doesEcho () const = 0;

protected:
    /**
     * Reads from the input (i.e. output of the login program)
     * characters until one of a list of given strings will match the
     * tail of the read characters.
     * @param strings an array of strings to wait for.
     * @param number_strings size of this array.
     * @param maxread the number of characters that should be read
     * before we give it up
     * @return index of the string that has been read. -1 if
     * we gave up, i.e. no matching string has been found within
     * maxread characters or if no further characters could be read.
     */
    int expectOneOf (const std::string *strings, int number_strings, int maxread);
};

#endif // RemoteProtocol_h
