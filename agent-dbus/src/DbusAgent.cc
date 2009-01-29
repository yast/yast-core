/*
 * File:	DbusAgent.cc
 *
 * Authors:	Arvin Schnell <aschnell@suse.de>
 */

#include "config.h"

#include <YCP.h>
#include <ycp/y2log.h>

#include "DbusAgent.h"


DbusAgent::DbusAgent()
{
    y2milestone("connecting dbus");

    dbus_error_init(&error);

    connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
	y2error("dbus_bus_get failed (%s)\n", error.message);
	dbus_error_free(&error);
    }

    if (connection == NULL)
    {
	y2error("connecting dbus failed");
    }
}


DbusAgent::~DbusAgent()
{
}


YCPValue
DbusAgent::Read(const YCPPath& path, const YCPValue& arg, const YCPValue&)
{
    y2debug("Read(%s)", path->toString().c_str());

    if (path->isRoot())
    {
	ycp2error("Read() called without sub-path");
	return YCPNull();
    }

    const string cmd = path->component_str(0); // just a shortcut

    return YCPError(string("Undefined subpath for Read(") + path->toString() + ")");
}


YCPBoolean
DbusAgent::Write(const YCPPath& path, const YCPValue& value,
		 const YCPValue& arg)
{
    y2debug("Write(%s)", path->toString().c_str());

    if (path->isRoot())
    {
	ycp2error("Write() called without sub-path");
	return YCPBoolean(false);
    }

    const string cmd = path->component_str(0); // just a shortcut

    ycp2error("Undefined subpath for Write(%s)", path->toString ().c_str ());
    return YCPBoolean(false);
}


YCPValue
DbusAgent::Execute(const YCPPath& path, const YCPValue& value,
		   const YCPValue& arg)
{
    y2debug("Execute(%s)", path->toString().c_str());

    if (path->isRoot())
    {
	return YCPError("Execute() called without sub-path");
    }

    if (value.isNull())
    {
	return YCPError(string("Execute(")+path->toString()+") without argument.");
    }

    const string cmd = path->component_str(0); // just a shortcut

    if (cmd == "method")
    {
	/**
	 * @builtin Execute(.dbus.method, map params, list args) -> boolean
	 *
	 * params must contain parameters for dbus_message_new_method_call()
	 * and args must contain arguments for dbus method call.
	 */

	if (!connection)
	    return YCPError("dbus connection failed");

	if (!value->isMap())
	    return YCPError("value not a map");

	if (!arg->isList())
	    return YCPError("arg not a list");

	YCPMap tmp1 = value->asMap();

	YCPValue destination = tmp1.value(YCPSymbol("destination"));
	if (!destination->isString())
	    return YCPError("Missing or wrong type for 'destination'");

	YCPValue path = tmp1.value(YCPSymbol("path"));
	if (!path->isString())
	    return YCPError("Missing or wrong type for 'path'");

	YCPValue interface = tmp1.value(YCPSymbol("interface"));
	if (!interface->isString())
	    return YCPError("Missing or wrong type for 'interface'");

	YCPValue method = tmp1.value(YCPSymbol("method"));
	if (!method->isString())
	    return YCPError("Missing or wrong type for 'method'");

	DBusMessage* message = dbus_message_new_method_call(destination->asString()->value_cstr(),
							    path->asString()->value_cstr(),
							    interface->asString()->value_cstr(),
							    method->asString()->value_cstr());
	if (NULL == message)
	{
	    return YCPError("dbus_message_new_method_call() failed");
	}

	DBusMessageIter args;

	dbus_message_iter_init_append(message, &args);

	YCPList tmp2 = arg->asList();
	for (YCPListIterator it = tmp2.begin(); it != tmp2.end(); ++it)
	{
	    const YCPValue& tmp3 = *it;

	    if (tmp3->isString())
	    {
		const char* param = tmp3->asString()->value_cstr();
		if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &param))
		    return YCPError("dbus_message_iter_append_basic() failed");
	    }
	    else if (tmp3->isBoolean())
	    {
		bool param = tmp3->asBoolean()->value();
		if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &param))
		    return YCPError("dbus_message_iter_append_basic() failed");
	    }
	    else
	    {
		return YCPError("Unsupported type");
	    }
	}

	DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection, message, -1, &error);

	dbus_message_unref(message);

	if (dbus_error_is_set(&error))
	{
	    dbus_error_free(&error);
	    return YCPError("dbus_connection_send_with_reply_and_block() failed");
	}

	if (reply == NULL)
	{
	    return YCPError("dbus_connection_send_with_reply_and_block() failed");
	}

	dbus_message_unref(reply);

	return YCPBoolean(true);
    }

    return YCPError(string("Undefined subpath for Execute(") + path->toString() + ")");
}
