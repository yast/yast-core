
/*
  PolicyKit access
*/

#ifndef PolKit_h
#define PolKit_h


#include <string>
#include <list>

#include <dbus/dbus.h>
#include <polkit-dbus/polkit-dbus.h>

extern "C"
{ 
#include <sys/time.h>
}

class PolKit
{
    public:

	PolKit();
	~PolKit();

	bool isDBusUserAuthorized(const std::string &action_id, const std::string &dbus_caller,
	    DBusConnection *con, DBusError*err);

	void checkPolkitChanges();

	void addWatch(int fd);
	void removeWatch(int fd);
	void configChanged();

	static std::string createActionId(const std::string &prefix, const std::string &path,
	    const std::string &method, const std::string &arg = std::string(),
	    const std::string &opt = std::string());

	static std::string makeValidActionID(const std::string &s);

	static bool isValidActionID(const std::string &action);

    private:

	PolKitContext *context;

	typedef std::list<int> WatchListType;

	WatchListType fd_watch_list;

	// select() timeout (set to 0 to return immediately)
	struct timeval select_timeout;

};

#endif

