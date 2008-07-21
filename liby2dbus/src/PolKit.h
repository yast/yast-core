
/*
  PolicyKit access
*/

#ifndef PolKit_h
#define PolKit_h


#include <string>

#include <dbus/dbus.h>
#include <polkit-dbus/polkit-dbus.h>

class PolKit
{
    public:

	PolKit();
	~PolKit();

	bool isDBusUserAuthorized(const std::string &action_id, const std::string &dbus_caller, DBusConnection *con);

	static std::string createActionId(const std::string &prefix, const std::string &path,
	    const std::string &method, const std::string &arg = std::string(),
	    const std::string &opt = std::string());

    private:

	DBusError dbus_error;
	PolKitError *polkit_error;
	PolKitContext *context;
};

#endif

