
/*
  DBusServer.h
*/

#ifndef DBUSSERVER_H
#define DBUSSERVER_H

#include "config.h"

#include <dbus/dbus.h>

#include "DBusConn.h"
#include "ScriptingAgent.h"

#ifdef HAVE_POLKIT
#include "PolKit.h"
#endif

extern "C"
{
#include <unistd.h>
#include <signal.h>
}

class DBusServer
{
    public:

	DBusServer();
	~DBusServer();

	bool connect();
	/**
	 * Runs the server
	 * @param forever for debugging, disables exiting after timeout
	 */
	void run(bool forever = false);

    private:

	DBusConn connection;

#ifdef HAVE_POLKIT
	PolKit policykit;
	bool isActionAllowed(const std::string &caller, const std::string &path,
	    const std::string &method, const std::string &arg, const std::string &opt);
#endif

	// SCR access    
	ScriptingAgent *sa;

	// disable copying
	DBusServer(const DBusServer&);
	DBusServer& operator=(const DBusServer&);

	void resetTimer();
	void registerSignalHandler();
	bool isProcessRunning(pid_t pid);
	bool canFinish();
	pid_t callerPid(const std::string &bus_name);

	typedef std::map<std::string, pid_t> Clients;

	Clients clients;
};


#endif

