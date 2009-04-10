
/*
  PolKit implementation
*/

#include "PolKit.h"

#include <ycp/y2log.h>

#include<map>

extern "C"
{
#include <sys/select.h>
#include <errno.h>
}

#include <cstring>

typedef std::map<PolKitContext *, PolKit*> PolKitMapping;

// PolKitContext * -> PolKit * mapping
// for routing the policykit callbacks to the correct PolKit object
PolKitMapping polkit_mapping;

PolKit* findPolKitObj(PolKitContext *context)
{
    PolKitMapping::const_iterator it = polkit_mapping.find(context);

    if (it == polkit_mapping.end())
    {
	y2error("Cannot find PolKit object for PolKitContext %p", context);
	return NULL;
    }
    else
    {
	return it->second;
    }
}

static void _polkitConfigChanged(PolKitContext *context, void *data)
{
    y2debug("PolicyKit context %p has been changed", context);

    PolKit *pk = findPolKitObj(context);

    if (pk != NULL)
    {
	pk->configChanged();
    }
}

static int _polkitIOAddWatch(PolKitContext *context, int fd)
{
    y2debug("PolicyKit context %p: adding IO watch: %d", context, fd);

    PolKit *pk = findPolKitObj(context);

    if (pk == NULL)
    {
	return 0;
    }
    else
    {
	pk->addWatch(fd);
    }

    // TODO: Polkit doc says the result must be unique ID, is this OK??
    return fd;
}

static void _polkitIORemoveWatch(PolKitContext *context, int fd)
{
    y2debug("PolicyKit context %p removing IO watch: %d", context, fd);

    PolKit *pk = findPolKitObj(context);

    if (pk != NULL)
    {
	pk->removeWatch(fd);
    }
}

PolKit::PolKit()
{
    context = polkit_context_new();

    // add object mapping
    polkit_mapping.insert(std::make_pair(context, this));

    // set PolicyKit config change callback
    polkit_context_set_config_changed(context, _polkitConfigChanged, NULL);

    // set PolicyKit config watch callbacks
    polkit_context_set_io_watch_functions(context, _polkitIOAddWatch, _polkitIORemoveWatch);

    PolKitError *polkit_error = NULL;
    polkit_context_init(context, &polkit_error);

    if (polkit_error)
    {
	y2error("PolicyKit error: %s: %s", polkit_error_get_error_name(polkit_error),
	    polkit_error_get_error_message(polkit_error));

        polkit_error_free(polkit_error);
    }

    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 0;
}

PolKit::~PolKit()
{
    // release this object from mapping
    polkit_mapping.erase(context);

    // release the PolKitContext object
    polkit_context_unref(context);
}


bool PolKit::isDBusUserAuthorized(const std::string &action_id, const std::string &dbus_caller, DBusConnection *con, DBusError *err)
{
    y2debug("Checking action %s from %s", action_id.c_str(), dbus_caller.c_str());

    PolKitCaller *pk_caller = polkit_caller_new_from_dbus_name(con, dbus_caller.c_str(), err);

    if (dbus_error_is_set(err))
    {
	// PolKit sometimes sets the error even if the PolKitCaller object has been successfully returned
	// see bnc#439150
	if (pk_caller == NULL)
	{
	    y2error ("DBus error: creating PolKitCaller object failed: %s: %s", err->name, err->message);
	    return false;
	}
	else
	{
	    // reset the error flag, no error
	    dbus_error_free(err);
	}
   }

    if (pk_caller == NULL)
    {
	y2error("PolKitCaller is NULL!");
	return false;
    }

    PolKitAction *pk_action = polkit_action_new();
    polkit_action_set_action_id (pk_action, action_id.c_str());

    PolKitError *polkit_error = NULL;
    PolKitResult pk_result = polkit_context_is_caller_authorized(
	context, pk_action, pk_caller, TRUE, &polkit_error);

    y2debug("polkit_context_is_caller_authorized() result: %s", polkit_result_to_string_representation(pk_result));

    if (pk_result != POLKIT_RESULT_YES)
    {
	if (!polkit_dbus_error_generate(pk_action, pk_result, err))
	{
	    y2error("Cannot set DBus error from PolicyKit result");
	}
    }

    polkit_action_unref (pk_action);

    if (polkit_error)
    {
	y2error("PolicyKit error: %s: %s", polkit_error_get_error_name(polkit_error),
	    polkit_error_get_error_message(polkit_error));

        polkit_error_free(polkit_error);
    }

    polkit_caller_unref(pk_caller);

    return pk_result == POLKIT_RESULT_YES;
}

std::string PolKit::makeValidActionID(const std::string &s)
{
    if (s.empty())
	return s;

    std::string ret;
    // reserve enough space in advance, but not more than 255 characters
    ret.reserve(s.size() & 255);

    bool was_invalid_char = false;

    for (std::string::size_type i = 0; i < s.length(); ++i)
    {
	char ch = s[i];

	// skip valid charcters
	if (islower(ch) || isdigit(ch) || ch == '.' || ch == '-')
	{
	    ret.push_back(ch);
	    was_invalid_char = false;
	}
	// convert uppercase to lowercase
	else if (isupper(ch))
	{
	    ret.push_back(tolower(ch));
	    was_invalid_char = false;
	}
	else
	{
	    if (!was_invalid_char)
	    {
		// replace invalid characters
		ret.push_back('-');
		was_invalid_char = true;
	    }
	}

	if (ret.size() == 255)
	    break;
    }

    return ret;
}

std::string PolKit::createActionId(const std::string &prefix, const std::string &path, const std::string &method,
	    const std::string &arg, const std::string &opt)
{
    std::string action_id(prefix + "." + method + path);

    // use arg and opt for generic agents (like .target.bash) to allow only some arguments
    if(::strncmp(path.c_str(), ".target.", ::strlen(".target.")) == 0 ||
	::strncmp(path.c_str(), ".background.", ::strlen(".background.")) == 0 ||
	::strncmp(path.c_str(), ".process.", ::strlen(".process.")) == 0 ||
	method == "RegisterAgent")
    {
	action_id += arg + opt;
    }

    // actionID must contain only [a-z][0-9] and .- characters, max. length is 255 characters
    action_id = makeValidActionID(action_id);

    return action_id;
}

bool PolKit::isValidActionID(const std::string &action)
{
    return polkit_action_validate_id(action.c_str());
}


// check the registered file descriptors here,
// if there is something to read then call
// polkit_context_io_func(context, ready_fd)
// to process the changes by PolicyKit
//
// this method must be called from the main loop

void PolKit::checkPolkitChanges()
{
    y2debug("Checking changes in PolicyKit config...");

    // filedescriptor set
    fd_set rfds;

    // init to empty set
    FD_ZERO(&rfds);

    int max_fd = -1;

    for(WatchListType::const_iterator it = fd_watch_list.begin();
	it != fd_watch_list.end();
	++it)
    {
	// add the FD to the watch set
	FD_SET(*it, &rfds);

	if (max_fd < *it)
	{
	    max_fd = *it;
	}
    }

    // check whether there is something to read, timeout is 0 (return immediately)
    int retval = ::select(max_fd + 1, &rfds, NULL, NULL, &select_timeout);

    y2debug("select() result: %d", retval);

    // error?
    if (retval == -1)
    {
	y2error("Error in select() call: %s", ::strerror(errno));
    }
    // data available?
    else if (retval > 0)
    {
	for(WatchListType::const_iterator it = fd_watch_list.begin();
	    it != fd_watch_list.end();
	    ++it)
	{
	    // check the FD in the result
	    if (FD_ISSET(*it, &rfds))
	    {
		y2debug("File descriptor %d has data available", *it);

		// call the PolicyKit IO handler
		// (the config changed callbacked will be called
		// if the config has been changed)
		polkit_context_io_func(context, *it);
	    }
	}
    }
}

void PolKit::addWatch(int fd)
{
    y2milestone("Adding Polkit watch fd: %d", fd);

    // add the fd to the internal list
    fd_watch_list.push_back(fd);

    y2debug("%zd file descriptors in the watch list", fd_watch_list.size());
}

void PolKit::removeWatch(int fd)
{
    y2milestone("Removing Polkit watch fd: %d", fd);

    // remove the fd from the internal list
    fd_watch_list.remove_if(std::bind2nd(std::equal_to<int>(), fd));

    y2debug("%zd file descriptors in the watch list", fd_watch_list.size());
}

void PolKit::configChanged()
{
    y2milestone("PolicyKit config has been changed");
}
