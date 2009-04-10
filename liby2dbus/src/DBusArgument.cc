
/*
  class DBusArgument
*/

#include "DBusArgument.h"

DBusArgument::DBusArgument(const std::string &arg_name, const std::string &arg_signature) : name(arg_name), signature(arg_signature)
{
}

DBusArgument::~DBusArgument()
{
}

bool DBusArgument::empty() const
{
    return signature.empty();
}

std::string DBusArgument::asXML(bool direction_in) const
{
    if (empty())
    {
	return std::string();
    }

    return std::string("<arg name='" + name + "' type='" + signature + "' direction='" + (direction_in ? "in" : "out") +  "'/>");
}

