
/*
  DBusServerBase.h
*/

#ifndef DBUSSERVERBASE_H
#define DBUSSERVERBASE_H

#include "config.h"

#include "DBusConn.h"
#include "DBusCaller.h"
#include "DBusSignature.h"

#ifdef HAVE_POLKIT
#include "PolKit.h"
#endif

#include <functional>
#include <map>

class DBusServerBase
{
    public:

	DBusServerBase();
	virtual ~DBusServerBase();

	virtual bool connect() = 0;

	/**
	 * Runs the server
	 * @param forever for debugging, disables exiting after timeout
	 */
	void run(bool forever = false);


    protected:
	enum DBusType{ SESSION, SYSTEM };

	bool connect(DBusType bustype, const std::string &sname);

	typedef std::string Object;
	typedef std::string Interface;
	typedef std::string Method;

	typedef std::function<DBusMsg(const DBusMsg &)> methodHandler;

	void register_method(const Object &obj, const Interface &i, const Method &m, const DBusSignature &sig, methodHandler h);
	void unregister_client(const std::string &bus_id);
	void register_client(const std::string &bus_id);

	typedef std::list<std::string> actionList;

	// create PolicyKit action ID for the received message
	virtual actionList createActionId(const DBusMsg &msg);

	// handle unknown requests
	virtual DBusMsg unknownRequest(const DBusMsg &request);

	std::string service_name;


    private:

	// disable copying
	DBusServerBase(const DBusServerBase&);
	DBusServerBase& operator=(const DBusServerBase&);

	bool canFinish();
	bool isActionAllowed(const DBusMsg &msg, DBusError *err);

	typedef std::pair<methodHandler, DBusSignature> MethodData;
	typedef std::map<Method, MethodData> InterfaceData;
	typedef std::map<Interface, InterfaceData> ObjectData;
	typedef std::map<Object, ObjectData> Objects;

	std::string CreateObjectIntrospection(const ObjectData &od);

	Objects registered_objects;

	typedef std::map<std::string, DBusCaller> Clients;
	Clients clients;

	DBusConn connection;

#ifdef HAVE_POLKIT
	PolKit policykit;
#endif

};

#endif

