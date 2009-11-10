
/*
  DBusConn
*/

#ifndef DBusConn_h
#define DBusConn_h

#include <dbus/dbus.h>
#include <string>

#include "DBusMsg.h"

class DBusConn
{
    private:

	DBusConnection *connection;
	DBusError dbus_error;

	// disable copying
	DBusConn(const DBusConn &);
	DBusConn& operator=(const DBusConn&);

    public:

	DBusConn();
	~DBusConn();

	bool connect(DBusBusType type, const std::string& service = std::string());
	void setTimeout(int miliseconds);
	bool send(const DBusMsg &msg);
	DBusMsg call(const DBusMsg &msg);
	void flush();
	// it the msg is empty, there was a timeout
	DBusMsg receive();
	DBusConnection *getConnection() const;
};

#endif


