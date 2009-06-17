/*
  DBusModulesServer.h
*/

#ifndef DBUSMODUELSSERVER_H
#define DBUSMODULESSERVER_H

#include "config.h"

#include <DBusServerBase.h>
#include <DBusMsg.h>

#include <ycp/TypePtr.h>
#include <y2/Y2Component.h>

#include <functional>
#include <list>

class Y2Namespace;

class DBusModulesServer : public DBusServerBase
{
    public:

	typedef std::list<std::string> NameSpaceList;

	DBusModulesServer(const NameSpaceList &name_spaces,  bool use_session_bus);
	virtual ~DBusModulesServer();

	virtual bool connect();


    protected:

	virtual actionList createActionId(const DBusMsg &msg);

	// handle unknown requests
	virtual DBusMsg unknownRequest(const DBusMsg &request);

    private:

	// the wrapped Yast namespaces
	typedef std::map<const std::string, Y2Namespace *> NameSpaceMap;
	NameSpaceMap nsmap;

	constTypePtr searchFuncType(const std::string &objname, const std::string &fname) const;
	std::string Y2Dtype(constTypePtr type) const;

	// try to cast from a DBus value to the expected YCPValue
	YCPValue AutoCast(const YCPValue &value, constTypePtr requested_type);

	// import a Yast namespace
	bool importNamespace(const std::string &nspace);
	bool registerFunction(const std::string &nspace, const char *fname, constFunctionTypePtr fptr);
	bool registerNamespace(const Y2Namespace *ns);

	void registerManager();

	DBusMsg handler(const DBusMsg &request);

        class Callback : public std::function<DBusMsg (const DBusMsg &request)>
	{
	    DBusModulesServer *ms;

	    public:

		Callback(DBusModulesServer *s) : ms(s) {}
		~Callback() {}

		DBusMsg operator()(const DBusMsg &request)
		{ return ms ? ms->handler(request) : DBusMsg(); }
	};

	Callback e;

	DBusMsg managerHandler(const DBusMsg &request);

        class ManagerCallback : public std::function<DBusMsg (const DBusMsg &request)>
	{
	    DBusModulesServer *ms;

	    public:

		ManagerCallback(DBusModulesServer *s) : ms(s) {}
		~ManagerCallback() {}

		DBusMsg operator()(const DBusMsg &request)
		{ return ms ? ms->managerHandler(request) : DBusMsg(); }
	};

	ManagerCallback manager_callback;


	Y2Component *wfm;

	void init_wfm();

	bool m_use_session_bus;
};


#endif
