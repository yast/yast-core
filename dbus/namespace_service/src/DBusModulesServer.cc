/*

  DBus server

 */

#include "DBusModulesServer.h"
#include "DBusMsg.h"

#include <ycp/Import.h>
#include <y2/Y2Namespace.h>
#include <y2/Y2Function.h>
#include <y2/Y2PluginComponent.h>
#include <ycp/Parser.h>

#include <ycp/y2log.h>
#include <YCP.h>

#include <functional>

#include <Y2WFMComponent.h>

#include "yast_dbus_names.h"

void DBusModulesServer::init_wfm()
{
    if (Y2WFMComponent::instance() == NULL)
    {
	wfm = Y2ComponentBroker::createClient("wfm");

	if (wfm == NULL)
	{
	    y2error("Cannot create WFM component");
	}
    }
}

std::string MakeValidObjectName(const std::string &name)
{
    std::string ret;
    // optimization: preallocate enough space, avoid memory reallocations
    ret.reserve(name.size());

    for(std::string::const_iterator it = name.begin();
	it != name.end();
	++it)
    {
	// replace "::" -> "/"
	if (*it == ':')
	{
	    std::string::const_iterator it2(it);
	    it2++;

	    if (it2 != name.end() && *it2 == ':')
	    {
		ret += '/';
		it++;
		continue;
	    }
	}

	if (!isupper(*it) && !islower(*it) && (*it) != '_')
	{
	    ret += '_';
	}
	else
	{
	    ret += *it;
	}
    }

    return ret;
}

bool DBusModulesServer::importNamespace(const std::string &nspace)
{
    NameSpaceMap::const_iterator nsiter = nsmap.find(nspace);

    if (nsiter != nsmap.end())
    {
	y2internal("Namespace %s has already been imported", nspace.c_str());
	return true;
    }

    Import import(nspace);     // has an iternal static cache

    Y2Namespace *ns = import.nameSpace();

    if (ns == NULL)
    {
	y2error("Import name space %s failed", nspace.c_str());
	return false;
    }
    else
    {
	ns->initialize();
	std::string objname(nspace);
	objname = MakeValidObjectName(objname);

	y2milestone("Imported name space: %s, using object name: %s", nspace.c_str(), objname.c_str());

	nsmap[YAST_DBUS_OBJ_PREFIX"/" + objname] = ns;

	return registerNamespace(ns);
    }
}

bool DBusModulesServer::registerFunction(const std::string &nspace, const char *fname, constFunctionTypePtr fptr)
{
    if (fptr)
    {
	constTypePtr rettype = fptr->returnType();

	DBusSignature sig_marshalled;
	DBusSignature sig_raw;

	// add return type
	std::string rett(Y2Dtype(rettype));

	if (!rett.empty())
	{
	    sig_raw.retval = DBusArgument("ret", rett);
	}

	// TODO FIXME void() ?
	sig_marshalled.retval = DBusArgument("ret", "(bsv)");

	// add parameter types
	int params = fptr->parameterCount();
	int parindex = 0;
	DBusSignature::Params p_marshalled;
	DBusSignature::Params p_raw;

	bool params_ok_raw = true;

	while(parindex < params)
	{
	    std::string partype_raw(Y2Dtype(fptr->parameterType(parindex)));

	    if (!partype_raw.empty())
	    {
		DBusArgument param("param", partype_raw);
		p_raw.push_back(param);
	    }
	    else
	    {
		y2warning("Function %s is not exported due to an unsupported parameter type", fname);
		params_ok_raw = false;
	    }

	    DBusArgument param("param", "(bsv)");
	    p_marshalled.push_back(param);

	    parindex++;
	}

	std::string namespace_str(nspace);
	namespace_str = MakeValidObjectName(namespace_str);

	if (params_ok_raw)
	{
	    sig_raw.params = p_raw;
	    // register the function: register_function(object, interface, method, signature, handler)
	    register_method(YAST_DBUS_OBJ_PREFIX"/" + namespace_str, YAST_DBUS_RAW_INTERFACE, fname, sig_raw, e);
	}

	sig_marshalled.params = p_marshalled;
	register_method(YAST_DBUS_OBJ_PREFIX"/" + namespace_str, YAST_DBUS_YCP_INTERFACE, fname, sig_marshalled, e);

	return true;
    }

    return false;
}


bool DBusModulesServer::registerNamespace(const Y2Namespace *ns)
{
    if (ns)
    {
	unsigned symbols = ns->symbolCount();
	unsigned index = 0;

	while(index < symbols)
	{
	    SymbolEntryPtr symbol = ns->symbolEntry(index);

	    if (symbol)
	    {
		// name of the method
		const char *name = symbol->name();
		// type
		constTypePtr type = symbol->type();

		if (type->isFunction())
		{
		    constFunctionTypePtr fptr = constFunctionTypePtr(type);

		    registerFunction(ns->name(), name, fptr);
		}
		else
		{
		    y2debug("Symbol %s::%s is not a function", ns->name().c_str(), name);
		}
	    }

	    index++;
	}

	return true;
    }

    return false;
}


DBusModulesServer::DBusModulesServer(const NameSpaceList &name_spaces, bool use_session_bus)
    : e(this)
    , manager_callback(this)
    , wfm(NULL)
    , m_use_session_bus(use_session_bus)
{
    init_wfm();

    for (NameSpaceList::const_iterator it = name_spaces.begin();
	it != name_spaces.end();
	++it)
    {
	importNamespace(*it);
    }
}

DBusModulesServer::~DBusModulesServer()
{
    if (wfm != NULL)
    {
	y2milestone("Deleting WFM component");
	delete wfm;
	wfm = NULL;
    }
}

void DBusModulesServer::registerManager()
{
    DBusSignature::Params p;
    DBusArgument param("module_name", DBUS_TYPE_STRING_AS_STRING);
    p.push_back(param);

    DBusSignature sig;
    sig.retval = DBusArgument("ret", DBUS_TYPE_BOOLEAN_AS_STRING);
    sig.params = p;

    // register the manager object: register_function(object, interface, method, signature, handler)
    register_method(YAST_DBUS_OBJ_PREFIX, YAST_DBUS_MGR_INTERFACE, YAST_DBUS_MANAGER_IMPORT_METHOD, sig, manager_callback);

    DBusSignature void_sig;
    // register the manager object: register_function(object, interface, method, signature, handler)
    register_method(YAST_DBUS_OBJ_PREFIX, YAST_DBUS_MGR_INTERFACE, YAST_DBUS_MANAGER_UNLOCK_METHOD, void_sig, manager_callback);
    register_method(YAST_DBUS_OBJ_PREFIX, YAST_DBUS_MGR_INTERFACE, YAST_DBUS_MANAGER_LOCK_METHOD, void_sig, manager_callback);
}

std::string DBusModulesServer::Y2Dtype(constTypePtr type) const
{
    YCPValueType vt = type->valueType();
    std::string ret;

    switch (vt)
    {
	case(YT_VOID) : ret = ""; break;
	case(YT_BOOLEAN) : ret = DBUS_TYPE_BOOLEAN_AS_STRING; break;
	case(YT_INTEGER) : ret = DBUS_TYPE_INT64_AS_STRING; break;
	case(YT_FLOAT) : ret = DBUS_TYPE_DOUBLE_AS_STRING; break;
	case(YT_STRING) : ret = DBUS_TYPE_STRING_AS_STRING; break;
	case(YT_PATH) : ret = DBUS_TYPE_STRING_AS_STRING; break;
	case(YT_SYMBOL) : ret = DBUS_TYPE_STRING_AS_STRING; break;
	case(YT_LIST) : ret = DBUS_TYPE_ARRAY_AS_STRING DBUS_TYPE_VARIANT_AS_STRING; break; /* av */
	case(YT_MAP) : ret = DBUS_TYPE_ARRAY_AS_STRING
	    DBUS_DICT_ENTRY_BEGIN_CHAR_AS_STRING DBUS_TYPE_STRING_AS_STRING
	    DBUS_TYPE_VARIANT_AS_STRING DBUS_DICT_ENTRY_END_CHAR_AS_STRING; break; /* a{sv} */
	case(YT_TERM) : ret = DBUS_TYPE_STRING_AS_STRING; break;
//	case(YT_CODE) : ret = DBUS_TYPE_STRING_AS_STRING; break;

	default : y2warning("Value type %d is not supported", vt); break;
    }

    return ret;
}

constTypePtr DBusModulesServer::searchFuncType(const std::string &objname, const std::string &fname) const
{
    if (nsmap.find(objname) == nsmap.end())
    {
	y2error("Object: %s not found", objname.c_str());
	return NULL;
    }
    Y2Namespace *ns = nsmap.find(objname)->second;

    if (ns)
    {
	unsigned symbols = ns->symbolCount();
	unsigned index = 0;

	while(index < symbols)
	{
	    SymbolEntryPtr symbol = ns->symbolEntry(index);

	    if (symbol)
	    {
		const char *name = symbol->name();

		if (fname == name)
		{
		    constTypePtr type = symbol->type();

		    if (type->isFunction())
		    {
			return type;
		    }
		}
	    }

	    index++;
	}
    }

    return NULL;
}

bool DBusModulesServer::connect()
{
    registerManager();
    return DBusServerBase::connect(m_use_session_bus? SESSION: SYSTEM,
				   YAST_DBUS_SERVICE_NAME);
}

/*
  convert string starting with '`' to YCPSymbol or YCPTerm
  convert string starting with '.' to YCPPath
*/
YCPValue DBusModulesServer::AutoCast(const YCPValue &value, constTypePtr requested_type)
{
    if (value->isString())
    {
	std::string value_str(value->asString()->value());

	if (requested_type->isSymbol())
	{
	    if (!value_str.empty() && value_str[0] == '`')
	    {
		// remove the leading backtick
		value_str.erase(value_str.begin());

		y2milestone("Automatic cast: string -> YCPSymbol");
		return YCPSymbol(std::string(value_str));
	    }
	}
	else if (requested_type->isPath())
	{
	    if (!value_str.empty() && value_str[0] == '.')
	    {
		y2milestone("Automatic cast: string -> YCPPath");
		return YCPPath(value_str);
	    }
	}
	else if (requested_type->isTerm())
	{
	    if (!value_str.empty() && value_str[0] == '`')
	    {
		y2milestone("Automatic cast: string -> YCPTerm");

		// parse the string, recreate the YCPTerm again
		Parser parser(value_str.c_str());
		parser.setBuffered();
		YCodePtr p = parser.parse();
		YCPValue contents = YCPNull ();
		
		if (!p)
		{
		    y2error("Parse error in YCP value: %s", value_str.c_str());
		}
		else
		{
		    // for security reasons do the evaluation only if it's a term!
		    constTypePtr tptr = p->type();

		    if (tptr && tptr->isTerm())
		    {
			return p->evaluate(true);
		    }

		    y2error("Not an YCP term: %s", value_str.c_str());
		}
	    }
	}
	else
	{
	    y2debug("Automatic cast is not defined for the current arguments");
	}
    }

    return value;
}

DBusMsg DBusModulesServer::handler(const DBusMsg &request)
{
    YCPValue ret;
    DBusMsg reply;

    std::string method(request.method());
    std::string object(request.path());
    std::string interface(request.interface());

    y2debug("Requested object: %s, interface: %s, method: %s", object.c_str(), interface.c_str(), method.c_str());

    constTypePtr t = searchFuncType(object, method);

    if (t)
    {
	constFunctionTypePtr fptr(t);

	if (fptr)
	{
	    int reqarg = fptr->parameterCount();

	    if (request.arguments() == reqarg)
	    {
		// TODO improve it, Y2Namespace was already found in searchFuncType()
		Y2Namespace *ns = nsmap[object];

		if (ns)
		{
		    // create function call
		    Y2Function *y2func = ns->createFunctionCall(method.c_str(), t);

		    if (y2func)
		    {
			int index = 0;
			bool valid_arguments = true;

			while(index < reqarg)
			{
			    YCPValue arg = request.getYCPValue(index);

			    if (arg.isNull())
			    {
				y2error("NULL parameter");
				valid_arguments = false;
				break;
			    }

			    // check the data type compatibility
			    constTypePtr argtype(fptr->parameterType(index));

			    if (argtype->matchvalue(arg) < 0)
			    {
				if (interface == YAST_DBUS_RAW_INTERFACE)
				{
				    // try converting DBus types to YCPValues in the raw interface
				    YCPValue casted = AutoCast(arg, argtype);

				    if (casted.isNull())
				    {
					y2error("Automatic cast failed (nil returned)");
					valid_arguments = false;
				    }
				    else
				    {
					arg = casted;
					// do the typecheck again
					valid_arguments = argtype->matchvalue(arg) >= 0;
				    }
				}
				else
				{
				    valid_arguments = false;
				}

				if (!valid_arguments)
				{
				    y2error("Invalid argument %d (%s): Function %s::%s() requires type '%s' instead of '%s'",
					index, arg->toString().c_str(), object.c_str(), method.c_str(),
					argtype->toString().c_str(), Type::vt2type(arg->valuetype())->toString().c_str()
				    );
				    break;
				}
			    }

			    y2func->appendParameter(arg);
			    index ++;
			}

			if (valid_arguments && y2func->finishParameters())
			{
			    // call the function
			    ret = y2func->evaluateCall();
			}
			else
			{
			    y2error("Wrong parameters to function %s::%s", object.c_str(), method.c_str());
			}
		    }
		}
	    }
	    else
	    {
		y2error("Function %s::%s got %d parameters instead of %d", object.c_str(), method.c_str(), request.arguments(), fptr->parameterCount());
	    }
	}
	else
	{
	    y2internal("Function %s::%s was not found although it was registered", object.c_str(), method.c_str());
	}
    }
    else
    {
	y2internal("Function %s::%s was not found although it was registered", object.c_str(), method.c_str());
    }

    reply.createReply(request);

    if (!ret.isNull())
    {
	y2milestone("Result: %s", ret->toString().c_str());

	// return empty message for void functions
	if (!ret->isVoid())
	{
	    if (interface == YAST_DBUS_RAW_INTERFACE)
	    {
		y2debug("Returning direct DBus value");
		reply.addValue(ret);
	    }
	    else
	    {
		y2debug("Returning (bsv) encoded YCP value");
		reply.addYCPValue(ret);
	    }
	}
	else
	{
	    y2debug("Result is void");
	}
    }

    return reply;
}

DBusMsg DBusModulesServer::managerHandler(const DBusMsg &request)
{
    DBusMsg reply;

    std::string method(request.method());
    std::string object(request.path());
    std::string interface(request.interface());

    y2milestone("ModuleManager request: object: %s, method: %s, interface: %s",
	object.c_str(), method.c_str(), interface.c_str());

    YCPValue ret;

    if (object == YAST_DBUS_OBJ_PREFIX)
    {
	if (interface == YAST_DBUS_MGR_INTERFACE)
	{
	    if (method == YAST_DBUS_MANAGER_IMPORT_METHOD)
	    {
		if (request.arguments() == 1)
		{
		    YCPValue arg = request.getYCPValue(0);

		    if (arg.isNull())
		    {
			y2error("NULL parameter");
		    }
		    else
		    {
			if (arg->isString())
			{
			    std::string required_namespace(arg->asString()->value());

			    NameSpaceMap::const_iterator nsiter = nsmap.find(required_namespace);
			    if (nsiter != nsmap.end())
			    {
				ret = YCPBoolean(true);
			    }
			    else
			    {
				bool retval = importNamespace(arg->asString()->value());

				ret = YCPBoolean(retval);
			    }
			}
			else
			{
			    y2error("Expecting 'string' parameter, got '%s'", Type::vt2type(arg->valuetype())->toString().c_str());
			}
		    }

		}
		else
		{
		    y2error("ModuleManager function %s got %d parameters instead of 1", method.c_str(), request.arguments());
		}
	    }
	    else if (method == YAST_DBUS_MANAGER_UNLOCK_METHOD)
	    {
		unregister_client(request.sender());
	    }
	    else if (method == YAST_DBUS_MANAGER_LOCK_METHOD)
	    {
		register_client(request.sender());
	    }
	}
    }

    reply.createReply(request);

    if (!ret.isNull())
    {
	y2milestone("Result: %s", ret->toString().c_str());
	reply.addValue(ret);
    }

    return reply;
}

DBusModulesServer::actionList DBusModulesServer::createActionId(const DBusMsg &msg)
{
    // actionId: <prefix>.<namespace>.<method>
    std::string ret;
    std::string method(msg.method());

    if (msg.interface() == YAST_DBUS_MGR_INTERFACE)
    {
	// use a different prefix, do not take namespace from object path
	ret = YAST_POLKIT_PREFIX_MANAGER;
	// Lock and Unlock methods share the same PolicyKit action Id "lock"
	if (method == YAST_DBUS_MANAGER_UNLOCK_METHOD)
	    method = YAST_DBUS_MANAGER_LOCK_METHOD;
    }
    else
    {
	ret = YAST_POLKIT_PREFIX_MODULES;

	std::string obj(msg.path());
	// convert object path "/org/opensuse/yast/Label" to PolKit path "Label"
	if (!obj.empty())
	{
	    if (obj[obj.size() - 1] == '/')
	    {
		obj.erase(obj.size() - 1);
	    }

	    // remove the object prefix path
	    obj.erase(0, sizeof(YAST_DBUS_OBJ_PREFIX));

	    if (!obj.empty() && obj[0] == '/')
	    {
		obj.erase(obj.begin());
	    }

	    for(std::string::iterator it = obj.begin();
		it != obj.end();
		++it)
	    {
		// replace "/" -> "."
		if (*it == '/')
		{
		    *it = '.';
		}
	    }

	    y2debug("Object path for PolicyKit action: %s", obj.c_str());

	    if (!obj.empty())
	    {
		ret += '.' + obj;
	    }
	}
    }

    ret += '.' + method;

    // make it valid action ID (lowercase chars, replace invalid chars)
    if (!PolKit::isValidActionID(ret))
    {
	ret = PolKit::makeValidActionID(ret);
    }

    actionList retlist;
    retlist.push_back(ret);

    return retlist;
}

// handle unknown requests
DBusMsg DBusModulesServer::unknownRequest(const DBusMsg &request)
{
    y2debug("Unknown request handler called");
    std::string req_path(request.path());
    DBusMsg reply;

    // subtree match
    if (req_path.substr(0, sizeof(YAST_DBUS_OBJ_PREFIX) - 1) == YAST_DBUS_OBJ_PREFIX
	&& (request.interface() == YAST_DBUS_RAW_INTERFACE || request.interface() == YAST_DBUS_YCP_INTERFACE)
    )
    {
	y2debug("Found object prefix " YAST_DBUS_OBJ_PREFIX " in request");
	std::string req_namespace(req_path, sizeof(YAST_DBUS_OBJ_PREFIX));

	// hack: replace / in request to :: to make a valid namespace
	// DBus object name cannot contain ::
	std::string::size_type pos = req_namespace.find("/");

	while (pos != std::string::npos)
	{
	    req_namespace.erase(pos, 1).insert(pos, "::");
	    pos = req_namespace.find("/");
	}

	y2milestone("Autoimporting namespace %s...", req_namespace.c_str());

	if (importNamespace(req_namespace))
	{
	    y2milestone("Import succeded, calling the handler again...");
	    // call the handler again (polikt has already allowed the access) 
	    reply = handler(request);
	    y2debug("Request handled");
	}
	else
	{
	    reply.createError(request, "Automatic import failed, unknown object", DBUS_ERROR_UNKNOWN_METHOD);
	}
    }
    else
    {
	reply.createError(request, "Unknown object, interface or method", DBUS_ERROR_UNKNOWN_METHOD);
    }

    return reply;
}


