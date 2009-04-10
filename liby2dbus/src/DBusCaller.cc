
/*
  DBusCaller.cc
*/

#include "DBusCaller.h"
#include "DBusConn.h"
#include "DBusMsg.h"

#include <ycp/y2log.h>

#include <dbus/dbus.h>

extern "C"
{
// stat()
#include <sys/stat.h>
}

// ostringstream
#include <sstream>

DBusCaller::DBusCaller(const std::string &busid, DBusConn &connection) : pid(0)
{
    DBusMsg query;

    // ask the DBus server for the PID of the caller
    query.createCall(DBUS_SERVICE_DBUS, DBUS_PATH_DBUS"/Bus",
	DBUS_INTERFACE_DBUS, "GetConnectionUnixProcessID");

    query.addString(busid);

    // send the request
    DBusMsg reply(connection.call(query));
    
    // read the answer
    DBusMessageIter iter;
    dbus_message_iter_init(reply.getMessage(), &iter);

    int type = dbus_message_iter_get_arg_type(&iter);
    y2debug("Message type: %d, %c", type, (char)type);

    if (type == DBUS_TYPE_UINT32)
    {
	dbus_message_iter_get_basic(&iter, &pid);
    }
    else
    {
	y2internal("Unexpected type in PID reply %d (%c)", type, (char)type);
    }

    y2debug("Created DBusCaller with PID %d", pid);
}

DBusCaller::~DBusCaller()
{
}

bool DBusCaller::isRunning()
{
    if (!pid)
	return false;

    ostringstream sstr;
    sstr << "/proc/" << pid;

    struct stat stat_result;
    bool ret = ::stat(sstr.str().c_str(), &stat_result) == 0;

    y2milestone("Process /proc/%d is running: %s", pid, ret ? "true" : "false");
    return ret;
}

pid_t DBusCaller::getPid()
{
    return pid;
}

