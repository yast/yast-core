
/*
  DBusCaller
*/

#ifndef DBUSCALLER_H
#define DBUSCALLER_H

#include <string>

extern "C"
{
#include <sys/types.h>
}

class DBusConn;

// represents a DBus client process
class DBusCaller
{

    public:

	DBusCaller(const std::string &busid, DBusConn &connection);
	~DBusCaller();

	bool isRunning();
	pid_t getPid();

    private:

	pid_t pid;
};

#endif

