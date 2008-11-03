
/*
  PolKit implementation
*/

#include "PolKit.h"

#include <ycp/y2log.h>

PolKit::PolKit() : polkit_error(NULL)
{
    dbus_error_init (&dbus_error);
    context = polkit_context_new();
    polkit_context_init(context, &polkit_error);
}

PolKit::~PolKit()
{
    dbus_error_free(&dbus_error);
}


bool PolKit::isDBusUserAuthorized(const std::string &action_id, const std::string &dbus_caller, DBusConnection *con)
{
    y2debug("Checking action %s from %s", action_id.c_str(), dbus_caller.c_str());

    PolKitCaller *pk_caller = polkit_caller_new_from_dbus_name(con, dbus_caller.c_str(), &dbus_error);

    if (dbus_error_is_set (&dbus_error))
    {
	// PolKit sometimes sets the error even if the PolKitCaller object has been successfully returned
	// see bnc#439150
	if (pk_caller == NULL)
	{
	    y2error ("dbus error: %s: %s", dbus_error.name, dbus_error.message);
	}

	dbus_error_free (&dbus_error);

	if (pk_caller == NULL)
	{
	    return false;
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

    polkit_caller_unref (pk_caller);
    polkit_action_unref (pk_action);

    if (polkit_error)
        polkit_error_free(polkit_error);

    return pk_result == POLKIT_RESULT_YES;
}

std::string makeValidActionID(const std::string &s)
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
