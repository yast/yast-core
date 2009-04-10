
/*
  DBusServer.h
*/

#ifndef DBUSSERVER_H
#define DBUSSERVER_H

#include "DBusServerBase.h"

class ScriptingAgent;

class DBusServer : public DBusServerBase
{
    public:

	DBusServer();
	virtual ~DBusServer();

	virtual bool connect();


    protected:

	virtual actionList createActionId(const DBusMsg &msg);


    private:

	// SCR access    
	ScriptingAgent *sa;

	// disable copying
	DBusServer(const DBusServer&);
	DBusServer& operator=(const DBusServer&);

	void registerFunctions();

	DBusMsg handler(const DBusMsg &request);

        class Callback : public std::function<DBusMsg (const DBusMsg &request)>
	{
	    DBusServer *ms;

	    public:

		Callback(DBusServer *s) : ms(s) {}
		~Callback() {}

		DBusMsg operator()(const DBusMsg &request)
		{ return ms ? ms->handler(request) : DBusMsg(); }
	};

	Callback cb;
};


#endif

