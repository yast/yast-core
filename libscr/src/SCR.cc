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

   File:	SCR.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "scr/SCR.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPString.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"
#include "scr/SCRAgent.h"

#include "DBusConn.h"
#include "DBusMsg.h"

extern StaticDeclaration static_declarations;

bool SCR::registered = false;

// DBus related variables
DBusConn *connection;

bool check_dbus_env()
{
    const char* y2dbus = ::getenv("Y2DBUS");

    if (y2dbus != NULL)
    {
	if (::strcmp(y2dbus, "1") == 0)
	    return true;
    }

    return false;
}

static YCPValue CallDBus(const char *method, const YCPPath &path = YCPNull(), const YCPValue &args = YCPNull (),
    const YCPValue &opt = YCPNull ())
{

    y2milestone("Calling DBus method %s: path: %s, args: %s, opt: %s", method, path.isNull() ? "NULL" : path->toString().c_str(), args.isNull() ? "NULL" : args->toString().c_str(), opt.isNull() ? "NULL" : opt->toString().c_str());

	DBusMsg msg;
	// TODO create constants for these strings
	msg.createCall("org.opensuse.yast.SCR", "/SCR", "org.opensuse.yast.SCR.Methods", method);

	if (!path.isNull())
	{
	    msg.addYCPValue(path);
	}

	if (!args.isNull())
	{
	    msg.addYCPValue(args);
	}

	if (!opt.isNull())
	{
	    msg.addYCPValue(opt);
	}

	// send the message and get the response
	DBusMsg reply(connection->call(msg));

	y2debug("Received reply type: %d", reply.type());

	// return the first argument from the reply
	YCPValue ret = reply.getYCPValue(0);

	// validate the reply (check for exceptions)
	if (reply.type() != DBUS_MESSAGE_TYPE_METHOD_RETURN)
	{
	    if (reply.type() == DBUS_MESSAGE_TYPE_ERROR)
	    {
		if (!ret.isNull())
		{
		    y2error("Received Error reply: %s", ret->toString().c_str());
		}
	    }

	    return YCPVoid();
	}

	if (!ret.isNull())
	{
	    y2milestone("Received reply: %s", ret->toString().c_str());
	}
	else
	{
	    y2error("Received YCPNull or the value is missing");
	}

	return ret;
}

static YCPValue 
SCRRead3 (const YCPPath &path, const YCPValue &args = YCPNull (), const YCPValue &opt = YCPNull ()) {

    if (connection != NULL)
    {
	return CallDBus("Read", path, args, opt);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    YCPValue ret = SCRAgent::instance()->Read( path, args, opt );
    
    return ret;
}

static YCPValue 
SCRRead2 (const YCPPath &path, const YCPValue &arg) {
    return SCRRead3 (path, arg);
}

static YCPValue 
SCRRead (const YCPPath &path) {
    return SCRRead3 (path);
}

static YCPValue 
SCRWrite2 (const YCPPath &path, const YCPValue& value_n) {
    YCPValue value = value_n.isNull()? YCPVoid(): value_n; // bnc#406138

    if (connection != NULL)
    {
	return CallDBus("Write", path, value);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Write (2 args)  on SCR agent %p", SCRAgent::instance () );
    return SCRAgent::instance ()->Write (path, value);
}

static YCPValue 
SCRWrite3 (const YCPPath &path, const YCPValue& value_n, const YCPValue& arg_n) {
    YCPValue value = value_n.isNull()? YCPVoid(): value_n; // bnc#406138
    YCPValue arg =   arg_n.isNull()?   YCPVoid(): arg_n; // bnc#406138

    if (connection != NULL)
    {
	return CallDBus("Write", path, value, arg);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Write (3 args) on SCR agent %p", SCRAgent::instance () );
    return SCRAgent::instance ()->Write (path, value, arg);
}

static YCPValue 
SCRDir (const YCPPath& path) {
    if (connection != NULL)
    {
	return CallDBus("Dir", path);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Dir on SCR agent %p", SCRAgent::instance () );
    return SCRAgent::instance ()->Dir (path);
}

static YCPValue 
SCRExecute (const YCPPath &path) {
    if (connection != NULL)
    {
	return CallDBus("Execute", path);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Execute on SCR agent %p", SCRAgent::instance () );
    return SCRAgent::instance ()->Execute (path);
}

static YCPValue 
SCRError (const YCPPath &path) {
    if (connection != NULL)
    {
	return CallDBus("Error", path);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Error on SCR agent %p", SCRAgent::instance () );
    y2debug( "path: %s", path->toString ().c_str () );
    
    return SCRAgent::instance ()->Error (path);
}

static YCPValue 
SCRExecute2 (const YCPPath &path, const YCPValue &arg_n) {
    YCPValue arg = arg_n.isNull()? YCPVoid(): arg_n; // bnc#406138

    if (connection != NULL)
    {
	return CallDBus("Execute", path, arg);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Execute on SCR agent %p", SCRAgent::instance () );
    y2debug( "path: %s", path->toString ().c_str () );
    y2debug( "args: %s", arg->toString ().c_str () );
    
    return SCRAgent::instance ()->Execute (path, arg);
}

static YCPValue 
SCRExecute3 (const YCPPath &path, const YCPValue &arg_n, const YCPValue &opt_n) {
    YCPValue arg = arg_n.isNull()? YCPVoid(): arg_n; // bnc#406138
    YCPValue opt = opt_n.isNull()? YCPVoid(): opt_n; // bnc#406138

    if (connection != NULL)
    {
	return CallDBus("Execute", path, arg, opt);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::Execute on SCR agent %p", SCRAgent::instance () );
    y2debug( "path: %s", path->toString ().c_str () );
    y2debug( "args: %s,%s", arg->toString ().c_str (), opt->toString ().c_str () );
    
    return SCRAgent::instance ()->Execute (path, arg, opt);
}

static YCPValue 
SCRRegisterAgentS (const YCPPath &path, const YCPString &arg) {
    if (connection != NULL)
    {
	return CallDBus("RegisterAgent", path, arg);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::RegisterAgent on SCR agent %p", SCRAgent::instance () );
    y2debug( "Path: %s", path->toString ().c_str () );
    y2debug( "Arg: %s", arg->toString ().c_str () );
    return SCRAgent::instance ()->RegisterAgent (path, arg);
}

static YCPValue 
SCRRegisterAgentT (const YCPPath &path, const YCPTerm &arg) {
    if (connection != NULL)
    {
	return CallDBus("RegisterAgent", path, arg);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::RegisterAgent on SCR agent %p", SCRAgent::instance () );
    y2debug( "Path: %s", path->toString ().c_str () );
    y2debug( "Arg: %s", arg->toString ().c_str () );
    return SCRAgent::instance ()->RegisterAgent (path, arg);
}

static YCPValue 
SCRUnregisterAgent (const YCPPath &path) {
    if (connection != NULL)
    {
	return CallDBus("UnregisterAgent", path);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::UnregisterAgent on SCR agent %p", SCRAgent::instance () );
    y2debug( "Path: %s", path->toString ().c_str () );
    return SCRAgent::instance ()->UnregisterAgent (path);
}

static YCPValue 
SCRUnregisterAllAgents () {
    if (connection != NULL)
    {
	return CallDBus("UnregisterAllAgents");
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::UnregisterAllAgents on SCR agent %p", SCRAgent::instance () );
    return SCRAgent::instance ()->UnregisterAllAgents ();
}

static YCPValue 
SCRUnmountAgent (const YCPPath &path) {
    if (connection != NULL)
    {
	return CallDBus("UnmountAgent", path);
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::UnmountAgent on SCR agent %p", SCRAgent::instance () );
    y2debug( "Path: %s", path->toString ().c_str () );
    return SCRAgent::instance ()->UnmountAgent (path);
}

static YCPValue 
SCRRegisterNewAgents () {
    if (connection != NULL)
    {
	return CallDBus("RegisterNewAgents");
    }

    if (! SCRAgent::instance())
    {
	ycperror ( "No SCR instance found" );
	return YCPVoid ();
    }
    y2debug( "Running SCR::RegisterNewAgents on SCR agent %p", SCRAgent::instance () );
    return SCRAgent::instance ()->RegisterNewAgents ();
}

SCR::SCR ()
{
    // already done, we must avoid double registration
    if (registered) return;
    
    y2debug( "Registering SCR builtins in %p", &static_declarations );

    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
#define ETC 0, NULL, constTypePtr(), NULL
#define ETCf   NULL, constTypePtr(), NULL
	{ "SCR",		"",				NULL,         DECL_NAMESPACE, ETCf },
	{ "Read",		"any (path)",			(void *)SCRRead,	       ETC },
	{ "Read",		"any (path, any)",		(void *)SCRRead2,	       ETC },
	{ "Read",		"any (path, any, any)",		(void *)SCRRead3,	       ETC },
	{ "Write",		"boolean (path, any)",		(void *)SCRWrite2, DECL_NIL,  ETCf },
	{ "Write",		"boolean (path, any, any)",	(void *)SCRWrite3,	       ETC },
	{ "Dir",		"list<string> (path)",		(void *)SCRDir,		       ETC },
	{ "Execute",		"any (path)",			(void *)SCRExecute,            ETC },
	{ "Execute",		"any (path, any)",		(void *)SCRExecute2,	       ETC },
	{ "Execute",		"any (path, any, any)",		(void *)SCRExecute3,	       ETC },
	{ "Error",		"map<string,any> (path)",	(void *)SCRError,	       ETC },
	{ "RegisterAgent",	"boolean (path, string)",	(void *)SCRRegisterAgentS,     ETC },
	{ "RegisterAgent",	"boolean (path, term)",		(void *)SCRRegisterAgentT,     ETC },
	{ "UnregisterAgent",	"boolean (path)",		(void *)SCRUnregisterAgent,    ETC },
	{ "UnregisterAllAgents","boolean ()",			(void *)SCRUnregisterAllAgents,ETC },
	{ "UnmountAgent",	"boolean (path)",		(void *)SCRUnmountAgent,       ETC },
	{ "RegisterNewAgents",  "boolean ()",			(void *)SCRRegisterNewAgents,  ETC },
	{ NULL, NULL, NULL, ETC }
#undef ETC
#undef ETCf
    };

    static_declarations.registerDeclarations ("SCR", declarations);
    
    registered = true;

    bool y2dbus = check_dbus_env();

    // use std SCR or DBUS SCR?
    if (y2dbus)
    {
	y2debug("Y2DBUS is set, using DBUS for SCR access");
	connection = new DBusConn;

	// connect to the system bus
	if (!connection->connect(DBUS_BUS_SYSTEM))
	{
	    y2error("Cannot connect to DBus, exiting...");
	    exit(1);
	}
    }
    else
    {
	y2debug("Y2DBUS is not set, using embedded SCR");
    }
}

SCR::~SCR()
{
    if (connection != NULL)
    {
	delete connection;
	connection = NULL;
    }
}
