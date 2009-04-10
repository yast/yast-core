
/*
  class DBusArgument 
*/

#ifndef DBUSARGUMENT_H
#define DBUSARGUMENT_H

#include <string>

class DBusArgument
{

    public:

	DBusArgument(const std::string &arg_name = std::string(), const std::string &arg_signature = std::string());
	~DBusArgument();

	bool empty() const;

	// default direction is "in" (input parameter)
	std::string asXML(bool direction_in = true) const;

	std::string name;
	std::string signature;
};

#endif
