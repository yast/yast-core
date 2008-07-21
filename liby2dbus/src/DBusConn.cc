
/*
  DBusConn.cc
*/

#include "DBusConn.h"

#include <ycp/y2log.h>

DBusConn::DBusConn() : connection(NULL)
{
    dbus_error_init(&dbus_error);
}

DBusConn::~DBusConn()
{
    if (connection)
    {
        dbus_connection_unref(connection);
    }

    dbus_error_free(&dbus_error);

    y2milestone("DBusConn finished");
}

bool DBusConn::connect(DBusBusType type, const std::string& service)
{
    if (connection != NULL)
    {
	// already connected
	return true;
    }

    y2milestone("Connecting to DBus...");
   
    // connect to the system bus
    connection = dbus_bus_get(type, &dbus_error);

    if (dbus_error_is_set(&dbus_error))
    {
	y2error("Cannot connect to the system bus: %s", dbus_error.message);
	return false;
    }

    if (connection == NULL)
    {
	y2error("DBus connection is NULL");
	return false;
    }

    if (!service.empty())
    {
	y2milestone("Registering service %s", service.c_str());
       
	// set the service name, replace the existing owner if it's allowed
	int result = dbus_bus_request_name(connection, service.c_str(), DBUS_NAME_FLAG_REPLACE_EXISTING, &dbus_error);
	if (dbus_error_is_set(&dbus_error))
	{
	    y2error("Cannot register service %s: %s", service.c_str(), dbus_error.message);
	    return false;
	}

	// primary owner?
	if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
	{
	    y2error("Service %s is already registered", service.c_str());
	    return false;
	}

	y2milestone("Service %s successfuly registered", service.c_str());
    }

    return true;
}

void DBusConn::setTimeout(int miliseconds)
{
    if (connection != NULL)
    {
	dbus_connection_read_write(connection, miliseconds);
    }
}

DBusMsg DBusConn::receive()
{
    DBusMsg msg;

    DBusMessage *m = dbus_connection_pop_message(connection);

    if (m != NULL)
    {
	msg.setMessage(m);
    }

    return msg;
}

bool DBusConn::send(const DBusMsg &msg)
{
    // dbus_uint32_t serial = 0;
    // NULL = we are not interested in the serial number
    return dbus_connection_send(connection, msg.getMessage(), NULL);
}

void DBusConn::flush()
{
    dbus_connection_flush(connection);
}

DBusConnection *DBusConn::getConnection() const
{
    return connection;
}

DBusMsg DBusConn::call(const DBusMsg &msg)
{
    DBusMessage *reply = dbus_connection_send_with_reply_and_block(connection, msg.getMessage(), -1, &dbus_error);

    if (dbus_error_is_set(&dbus_error))
    {
	y2error("DBus error: %s", dbus_error.message);
	dbus_error_free(&dbus_error);
    }

    DBusMsg ret;
    if (reply != NULL)
    {
	ret.setMessage(reply);
    }

    return ret;
}
