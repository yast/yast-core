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

   File:	Y2RemoteComponent.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

/*
 * Component that communicates via a telnet/rsh/ssh session
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include <unistd.h>

#include "Y2RemoteComponent.h"
#include <y2util/ExternalProgram.h>
#include <ycp/Parser.h>
#include <ycp/y2log.h>

#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPCode.h>

#include "TelnetProtocol.h"
#include "RloginProtocol.h"
#include "RshProtocol.h"
#include "SuProtocol.h"
#include "SudoProtocol.h"


Y2RemoteComponent::Y2RemoteComponent (bool is_server, string protocol,
				      string loginname, string password,
				      string hostname, string componentname)
    : is_server (is_server),
      protocol (protocol),
      loginname (loginname),
      password (password),
      hostname (hostname),
      componentname (componentname),
      rp (0),
      is_up (false)
{
    y2debug ("Creating %s://%s:<password>@%s/%s", protocol.c_str (),
	     loginname.c_str (), hostname.c_str (), componentname.c_str ());
}


string
Y2RemoteComponent::name () const
{
    return componentname;
}


bool
Y2RemoteComponent::connect ()
{
    rp = 0;
    if (protocol == "telnet")
	rp = new TelnetProtocol (is_server, loginname, hostname, componentname);
    else if (protocol == "rlogin")
	rp = new RloginProtocol (is_server, loginname, hostname, componentname);
    else if (protocol == "rsh")
	rp = new RshProtocol (is_server, loginname, hostname, componentname, "rsh");
    else if (protocol == "ssh")
	rp = new RshProtocol (is_server, loginname, hostname, componentname, "ssh");
    else if (protocol == "su")
	rp = new SuProtocol (is_server, loginname, componentname);
    else if (protocol == "sudo")
	rp = new SudoProtocol (is_server, loginname, componentname);

    if (!rp)
    {
	y2internal ("Protocol %s not yet implemented", protocol.c_str ());
	return false;
    }

    // Check if it is running
    if (!rp->running ())
    {
	delete rp;
	return false;
    }

    return true;
}


YCPValue
Y2RemoteComponent::evaluate (const YCPValue& command)
{
    y2debug ("evaluate %s", command->toString ().c_str ());

    if (is_server)
    {
	if (!is_up)
	{
	    if (!connect ())
		return YCPVoid (); // FIXME

	    RemoteProtocol::callStatus status;
	    status = rp->callComponent (password, true);

	    if (status == RemoteProtocol::RP_PASSWD)
	    {
		y2error ("Remote login was not successful: wrong password");
		delete rp;
		return YCPVoid ();
	    }

	    if (status != RemoteProtocol::RP_OK)
	    {
		y2error ("Remote login was not successful to %s://%s@%s/%s",
			 protocol.c_str (), loginname.c_str (), hostname.c_str (),
			 componentname.c_str ());
		delete rp;
		return YCPVoid ();
	    }

	    parser.setInput (rp->inputFile (), "<remote>");
	    YCPValue ret = receive (); // eat first ([])

	    is_up = true;
	}

	send (command, true);

	while (true)
	{

	    YCPValue ret = receive ();

	    if (!ret.isNull ())
		return ret;

	    y2error ("Couldn't read return value from remote. Aborting!");
	    exit (10);
	    return YCPVoid ();
	}
    }
    else
    {
	return YCPNull ();
    }
}


YCPValue
Y2RemoteComponent::doActualWork (const YCPList& arglist,
				 Y2Component *user_interface)
{
    this->user_interface = user_interface;

    // Initialize the connection to the remote host

    if (!connect ())
	return YCPVoid ();	// FIXME

    // Do interactive logging in
    bool have_password = false;
    string passwd = "";
    RemoteProtocol::callStatus status;

    while (true)
    {
	status = rp->callComponent (passwd, have_password);
	if (status == RemoteProtocol::RP_PASSWD)
	{
            // Either wrong or missing password
	    passwd = askPassword (have_password);
	    if (!have_password)
		break; // User pressed cancel
	}
	else break;
    }

    if (status != RemoteProtocol::RP_OK)
    {
	y2error ("Remote login was not successful to %s://%s@%s/%s",
		 protocol.c_str (), loginname.c_str (), hostname.c_str (),
		 componentname.c_str ());
	delete rp;
	return YCPVoid ();
    }

    // Prepare parser

    parser.setInput (rp->inputFile (), "<remote>");

    is_up = true;

    // Send client arguments to remote client
    send (arglist, true);

    // Do communication until result () or EOF

    YCPValue value = YCPNull ();
    while (!(value = receive ()).isNull ())
    {
	if (value->isTerm () && value->asTerm ()->size () == 1 &&
	    value->asTerm ()->name () == "result")
	{
	    y2debug ("got result from remote module: %s",
		     value->toString ().c_str ());
	    break;
	}
	else
	{
	    y2debug ("got command for UI from remote module: %s",
		     value->toString ().c_str ());
	    send (user_interface->evaluate (value), true);
	}
    }

    delete rp;
    rp = 0;

    if (value.isNull ())
    {
	y2warning ("Communication ended prior to result () message");
	return YCPVoid ();
    }
    else // got result value correctly. returning result.
    {
	return value->asTerm ()->value (0);
    }
}


void
Y2RemoteComponent::send (const YCPValue& v, bool eat_echo)
{
    string s = "(" + (v.isNull () ? "(nil)" : v->toString ()) + ")\n";
    // y2debug ("send begin %s", s.c_str ());
    fwrite (s.c_str (), s.length (), 1, rp->outputFile ());
    fflush (rp->outputFile ());
    if (eat_echo && rp->doesEcho ())
	parser.parse ();
    // y2debug ("send end %s", v->toString ().c_str ());
}


YCPValue
Y2RemoteComponent::receive ()
{
    // y2debug ("receive begin");
    YCPValue v = YCPCode (parser.parse ());
    // y2debug ("receive end %s", v->toString ().c_str ());
    return v;
}


string
Y2RemoteComponent::askPassword (bool& ok)
{
#if 0
//FIXME:
    // Ask password
    YCPValue password = callModule (string ("password"), YCPList (),
				    user_interface);
    if (!password.isNull () && password->isString ())
    {
	ok = true;
	return password->asString ()->value ();
    }
    else
#endif
    {
	ok = false;
	return "";
    }
}


bool
Y2RemoteComponent::remote () const
{
    return true;
}
