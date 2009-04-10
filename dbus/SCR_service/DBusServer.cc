
/*

  DBus server

 */

#include "ScriptingAgent.h"

#include "DBusServer.h"
#include "DBusMsg.h"

#ifdef HAVE_POLKIT
#include "PolKit.h"
#endif

#include <ycp/y2log.h>

#include "scr_names.h"

#define TIMEOUT 15 /* 30 secs idle timeout */


DBusServer::DBusServer() : cb(this)
{
    sa = new ScriptingAgent();
}

DBusServer::~DBusServer()
{
    if (sa != NULL)
    {
	delete sa;
    }
}

void DBusServer::registerFunctions()
{
    // parameters
    DBusArgument param1("parmeter1", DBusMsg::YCPValueSignature());
    DBusArgument param2("parmeter2", DBusMsg::YCPValueSignature());
    DBusArgument param3("parmeter3", DBusMsg::YCPValueSignature());

    // parameter list
    DBusSignature::Params p;
    p.push_back(param1);

    DBusSignature sig_0param;
    sig_0param.retval = DBusArgument("ret", DBusMsg::YCPValueSignature());

    DBusSignature sig_1param(sig_0param);
    sig_1param.params = p;

    DBusSignature sig_2param(sig_1param);
    p.push_back(param2);
    sig_2param.params = p;

    DBusSignature sig_3param(sig_2param);
    p.push_back(param3);
    sig_3param.params = p;

    std::string object(SCR_OBJECT_PATH);

    if (!object.empty() && object[0] == '/')
    {
	object.erase(object.begin());
    }

    // register the manager object: register_function(object, interface, method, signature, handler)
    register_method(object, YAST_SCR_INTERFACE, METHOD_READ, sig_3param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_WRITE, sig_3param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_EXECUTE, sig_3param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_DIR, sig_1param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_ERROR, sig_1param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_UNREGISTER, sig_1param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_UNREGISTER_ALL, sig_0param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_REGISTER_NEW, sig_0param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_REGISTER, sig_2param, cb);
    register_method(object, YAST_SCR_INTERFACE, METHOD_UNMOUNT, sig_1param, cb);
}

bool DBusServer::connect()
{
    registerFunctions();
    return DBusServerBase::connect(SYSTEM, YAST_SCR_SERVICE);
}

DBusMsg DBusServer::handler(const DBusMsg &request)
{
    // create a reply to the message
    DBusMsg reply;
    reply.createReply(request);

    y2milestone("Received request from %s: interface: %s, method: %s, arguments: %d", request.sender().c_str(),
	request.interface().c_str(), request.method().c_str(), request.arguments());
  
    // check this is a method call for the right object, interface & method
    if (request.type() == DBUS_MESSAGE_TYPE_METHOD_CALL
	&& request.interface() == YAST_SCR_INTERFACE
	&& request.path() == SCR_OBJECT_PATH)
    {
	std::string method(request.method());

	YCPValue arg0;
	YCPPath pth;

	bool check_ok = false;

	// check missing arguments
	if (method == METHOD_READ
	    || method == METHOD_WRITE
	    || method == METHOD_EXECUTE
	    || method == METHOD_DIR
	    || method == METHOD_ERROR
	    || method == METHOD_UNREGISTER
	    || method == METHOD_UNMOUNT
	    || method == METHOD_REGISTER)
	{
	    if (request.arguments() == 0)
	    {
		// return an ERROR
		reply.createError(request, "Missing arguments", DBUS_ERROR_INVALID_ARGS);
	    }
	    else
	    {
		arg0 = request.getYCPValue(0);

		if (arg0.isNull() || !arg0->isPath())
		{
		    // return an ERROR
		    reply.createError(request, "Expecting YCPPath as the first argument", DBUS_ERROR_INVALID_ARGS);
		}
		else
		{
		    pth = arg0->asPath();
		    check_ok = true;
		}
	    }
	}
	else if (method == METHOD_UNREGISTER_ALL || method != METHOD_REGISTER_NEW)
	{
	    check_ok = true;
	}

	y2internal("check_ok: %d", check_ok);

	if (check_ok)
	{
	    YCPValue arg = request.getYCPValue(1);
	    YCPValue opt = request.getYCPValue(2);

	    std::string caller(request.sender());

		y2debug("Request from: %s", caller.c_str());

	    // remember the client
	    register_client(caller);

	    YCPValue ret;

	    if (method == METHOD_READ)
		ret = sa->Read(pth, arg, opt);
	    else if (method == METHOD_WRITE)
		ret = sa->Write(pth, arg, opt);
	    else if (method == METHOD_EXECUTE)
		ret = sa->Execute(pth, arg, opt);
	    else if (method == METHOD_DIR)
	    {
		ret = sa->Dir(pth);
		if (ret.isNull())
		    ret = YCPList();
	    }
	    else if (method == METHOD_ERROR)
		ret = sa->Error(pth);
	    else if (method == METHOD_UNREGISTER)
		ret = sa->UnregisterAgent(pth);
	    else if (method == METHOD_UNREGISTER_ALL)
		ret = sa->UnregisterAllAgents();
	    else if (method == METHOD_UNMOUNT)
		ret = sa->UnmountAgent(pth);
	    else if (method == METHOD_REGISTER_NEW)
		ret = sa->RegisterNewAgents();
	    else if (method == METHOD_REGISTER)
		ret = sa->RegisterAgent(pth, arg);
	    else
		y2internal("Unhandled method %s", method.c_str());

	    if (!ret.isNull())
	    {
		y2milestone("Result: %s", ret->toString().c_str());
		reply.addYCPValue(ret);
	    }
	    else
		reply.addYCPValue(YCPVoid());
	}
    }

    y2internal("Finishing the callback");

    return reply;
}

DBusServer::actionList DBusServer::createActionId(const DBusMsg &msg)
{
    actionList ret;

#ifdef HAVE_POLKIT
    // check the access right to all methods at first (see bnc#449794)
    std::string action_id(PolKit::createActionId(POLKIT_PREFIX, "", msg.method(), "", ""));

    ret.push_back(action_id);

    YCPValue arg = msg.getYCPValue(1);
    YCPValue opt = msg.getYCPValue(2);

    std::string arg_str, opt_str;
    
    if (!arg.isNull())
    {
	arg_str = arg->toString();
    }

    if (!opt.isNull())
    {
	opt_str = opt->toString();
    }

    action_id = PolKit::createActionId(POLKIT_PREFIX, msg.path(), msg.method(), arg_str, opt_str);

    ret.push_back(action_id);
#endif

    return ret;
}
