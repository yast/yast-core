/*
 * HalAgent.cc
 *
 * An agent for some hal commands.
 *
 * Authors:	Arvin Schnell <aschnell@suse.de>
 */

#include "config.h"

#include <YCP.h>
#include <ycp/y2log.h>

#include "HalAgent.h"


HalAgent::HalAgent()
    : hal_ctx(NULL),
      initialised(false)
{
    hal_ctx = libhal_ctx_new();
    if (hal_ctx == NULL)
    {
	y2error("libhal_ctx_new failed");
	return;
    }

    DBusError error;
    dbus_error_init(&error);

    DBusConnection* connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (connection == NULL)
    {
	y2error("dbus_bus_get failed: %s: %s", error.name, error.message);
	LIBHAL_FREE_DBUS_ERROR(&error);
	return;
    }

    if (!libhal_ctx_set_dbus_connection(hal_ctx, connection))
    {
	y2error("libhal_ctx_set_dbus_connection failed");
	return;
    }

    if (!libhal_ctx_init(hal_ctx, &error))
    {
	if (dbus_error_is_set(&error))
	{
	    y2error("libhal_ctx_init failed: %s: %s\n", error.name, error.message);
	    dbus_error_free(&error);
	}
	y2error("could not initialise connection to hald.");
	return;
    }

    initialised = true;
}


HalAgent::~HalAgent ()
{
    if (hal_ctx)
    {
	DBusError error;
	dbus_error_init(&error);

	libhal_ctx_shutdown (hal_ctx, &error);
	if (dbus_error_is_set(&error))
	{
	    y2error("libhal_ctx_shutdown failed: %s: %s\n", error.name, error.message);
	    dbus_error_free(&error);
	}

	libhal_ctx_free(hal_ctx);
    }
}


bool
HalAgent::acquire_global_interface_lock(const string& interface, bool exclusive)
{
    if (!initialised)
	return false;

    DBusError error;
    dbus_error_init(&error);

    bool ret = libhal_acquire_global_interface_lock(hal_ctx, interface.c_str(), exclusive, &error);
    if (dbus_error_is_set(&error))
    {
	y2error("libhal_acquire_global_interface_lock failed: %s: %s:", error.name, error.message);
	dbus_error_free (&error);
    }

    return ret;
}


bool
HalAgent::release_global_interface_lock(const string& interface)
{
    if (!initialised)
	return false;

    DBusError error;
    dbus_error_init(&error);

    bool ret = libhal_release_global_interface_lock(hal_ctx, interface.c_str(), &error);
    if (dbus_error_is_set(&error))
    {
	y2error("libhal_release_global_interface_lock failed: %s: %s:", error.name, error.message);
	dbus_error_free (&error);
    }

    return ret;
}


YCPValue
HalAgent::Read(const YCPPath& path, const YCPValue& arg, const YCPValue&)
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
HalAgent::Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg)
{
    y2debug("Write(%s)", path->toString().c_str());

    if (path->isRoot())
    {
	ycp2error("Write() called without sub-path");
	return YCPBoolean(false);
    }

    const string cmd = path->component_str(0); // just a shortcut

    ycp2error("Undefined subpath for Write(%s)", path->toString().c_str());
    return YCPBoolean (false);
}


YCPValue
HalAgent::Execute(const YCPPath& path, const YCPValue& value, const YCPValue& arg)
{
    y2debug ("Execute (%s)", path->toString().c_str());

    if (path->isRoot())
    {
	return YCPError("Execute() called without sub-path");
    }

    if (value.isNull())
    {
	return YCPError(string("Execute(")+path->toString()+") without argument.");
    }

    const string cmd = path->component_str(0); // just a shortcut

    if (cmd == "acquire_global_interface_lock")
    {
	if (value.isNull() || !value->isString())
	    return YCPError("Bad interface in Execute(.hal.acquire_global_interface_lock, "
			    "string interface, boolean exclusive)", YCPBoolean(false));
	string interface = value->asString()->value();

	if (arg.isNull() || !arg->isBoolean())
	    return YCPError("Bad exclusive in Execute(.hal.acquire_global_interface_lock, "
			    "string interface, boolean exclusive)", YCPBoolean(false));
	bool exclusive = arg->asBoolean()->value();

	return YCPBoolean(acquire_global_interface_lock(interface, exclusive));
    }
    else if (cmd == "release_global_interface_lock")
    {
	if (value.isNull() || !value->isString())
	    return YCPError("Bad interface in Execute(.hal.release_global_interface_lock, "
			    "string interface)", YCPBoolean(false));
	string interface = value->asString()->value();

	return YCPBoolean(release_global_interface_lock(interface));
    }

    return YCPError(string("Undefined subpath for Execute(") + path->toString() + ")");
}
