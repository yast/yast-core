
/*
  DBusSignature
  */

#ifndef DBUSSIGNATURE_H
#define DBUSSIGNATURE_H

#include <string>

#include "DBusArgument.h"
#include <list>

class DBusSignature
{
    public:
	typedef std::list<DBusArgument> Params;

	// create void(void) method by default
	DBusSignature(const DBusArgument &ret = DBusArgument(), const Params &pars = Params());
	~DBusSignature();

	std::string asXML() const;

	Params params;

	DBusArgument retval;
};

#endif

