
/*
  DBusSignature
  */

#include "DBusSignature.h"

DBusSignature::DBusSignature(const DBusArgument &ret, const Params &pars): params(pars), retval(ret)
{
}

DBusSignature::~DBusSignature()
{
}

std::string DBusSignature::asXML() const
{
    std::string ret;

    // add return value (false = direction is out)
    ret += retval.asXML(false);

    // add parameters
    for(Params::const_iterator i = params.begin();
	i != params.end();
	++i)
    {
	ret += i->asXML();
    }

    return ret;
}
