
/*

  DBus server

 */

#include <dbus/dbus.h>

#include "DBusServerBase.h"
#include "DBusMsg.h"

#include <ycp/y2log.h>

#include <algorithm>

// timeout for reading a DBus message (in miliseconds)
#define YAST_DBUS_TIMEOUT_VALUE 15000
// the count of timeout events before exiting
#define YAST_DBUS_SHUTDOWN_TIMEOUT_COUNT 8

int timeout_counter = YAST_DBUS_SHUTDOWN_TIMEOUT_COUNT;

DBusServerBase::DBusServerBase()
{
    dbus_threads_init_default();
}

DBusServerBase::~DBusServerBase()
{
}


bool DBusServerBase::connect(DBusType bustype, const std::string &sname)
{
    DBusBusType bt = (bustype == SYSTEM) ? DBUS_BUS_SYSTEM : DBUS_BUS_SESSION;

    service_name = sname;

    // connect to DBus, request a service name
    return connection.connect(bt, service_name.c_str());
}

bool DBusServerBase::canFinish()
{
    // check if clients are still running,
    // remove finished clients
    for(Clients::iterator it = clients.begin();
	it != clients.end();)
    {
	DBusCaller caller = it->second;

	if (!caller.isRunning())
	{
	    Clients::iterator remove_it(it);

	    // move the current iterator
	    it++;

	    y2milestone("Removing client %s (pid %d) from list", (remove_it->first).c_str(), caller.getPid());

	    clients.erase(remove_it);
	}
	else
	{
	    // the process is still running
	    // no need to check the other clients
	    // we have to still run for at least this client
	    y2debug("Client %s PID %d is still running", (it->first).c_str(), caller.getPid());
	    break;
	}
    }

    // if there is no client the server can be finished
    return clients.size() == 0;
}

std::string DBusServerBase::CreateObjectIntrospection(const ObjectData &od)
{
    std::string introspect;

    // iterate over the interfaces
    for(ObjectData::const_iterator ii = od.begin();
	ii != od.end();
	++ii)
    {
	introspect += " <interface name='" + ii->first + "'>\n";

	InterfaceData id = ii->second;

	// iterate over all methods
	for(InterfaceData::const_iterator iii = id.begin();
	    iii != id.end();
	    ++iii)
	{
	    // iterate over the methods in the interface
	    introspect += "  <method name='" + iii->first + "'>";

	    // iterate over all parameters
	    DBusSignature sig = iii->second.second; 

	    // add somthing like "<arg name='xml_data' type='s' direction='out'/>"
	    introspect += sig.asXML();

	    introspect += "</method>\n";
	}

	introspect += " </interface>\n";
    }

    return introspect;
}

/**
 * Server that exposes a method call and waits for it to be called
 */
void DBusServerBase::run(bool forever) 
{
    y2milestone("Listening for incoming DBus messages...");

    if (forever)
	y2milestone("Shutdown timer disabled");

    // mainloop
    while (true)
    {
	// try reading a message from DBus
	DBusMsg request(connection.receive());

	// check if a message was received
	if (request.empty())
	{
	    // the time is over
	    if (!timeout_counter)
	    {
		y2debug("Timout reached");

		if (canFinish())
		{
		    break;
		}
		else
		{
		    // reset the flag
		    timeout_counter = YAST_DBUS_SHUTDOWN_TIMEOUT_COUNT;
		}
	    }

	    // wait for a DBus message
	    connection.setTimeout(YAST_DBUS_TIMEOUT_VALUE);

	    // decrease the timeout counter
	    if (! forever)
		timeout_counter--;

	    continue; 
	}
	else
	{
	    // reset the flag
	    timeout_counter = YAST_DBUS_SHUTDOWN_TIMEOUT_COUNT;
	}

	// create a reply to the message
	DBusMsg reply;

	y2milestone("Received request from %s: object: %s, interface: %s, method: %s", request.sender().c_str(),
	    request.path().c_str(), request.interface().c_str(), request.method().c_str());
     
	// handle Introspection request from "org.freedesktop.DBus.Introspectable" "Introspect"
	if (request.isMethodCall(DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
	{
	    y2milestone("Introspecting object: %s", request.path().c_str());
	    std::string introspect(
		// introcpection data for the root node
		DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE
		"\n<node>\n"
		" <interface name='"DBUS_INTERFACE_INTROSPECTABLE"'>\n"
		"  <method name='Introspect'>\n"
		"   <arg name='xml_data' type='s' direction='out'/>\n"
		"  </method>\n"
		" </interface>\n");

	    // add "static" methods (with empty object)
	    std::string requested_path(request.path());

	    std::list<std::string> announced_objects;

	    for(Objects::const_iterator i = registered_objects.begin();
		i != registered_objects.end();
		++i)
	    {
		std::string object(i->first);

		// exact match or a subtree match?
		if (object != requested_path)
		{
		    std::string obj_prefix(object, 0, requested_path.size());

		    // subtree match
		    if (obj_prefix == requested_path)
		    {
			y2debug("Object: %s, Requested path: %s, Prefix: %s", object.c_str(),
			    requested_path.c_str(), obj_prefix.c_str()
			);

			std::string sub_path(object, obj_prefix.size());
			if (sub_path.size() > 0 && sub_path[0] == '/')
			{
			    sub_path.erase(sub_path.begin());
			}

			y2debug("sub_path: %s", sub_path.c_str());

			std::string node_name(sub_path, 0, sub_path.find("/"));
			if (node_name.size() > 0 && node_name[0] == '/')
			{
			    node_name.erase(node_name.begin());
			}

			y2debug("Using node name: %s", node_name.c_str());

			// check if it has been already added
			if (find(announced_objects.begin(), announced_objects.end(), node_name)
			    == announced_objects.end())
			{
			    introspect += " <node name='" + node_name + "'/>\n";
			    announced_objects.push_back(node_name);
			}
		    }
		}
		else
		{
		    introspect += CreateObjectIntrospection(i->second);
		}
	    }

	    // close the node section
	    introspect += "</node>\n";

	    // create a reply to the request
	    reply.createReply(request);
	    reply.addString(introspect.c_str());

	    y2debug("Introspection data: %s", introspect.c_str());
	}
	else if (request.type() == DBUS_MESSAGE_TYPE_METHOD_CALL)
	{
	    DBusError dbus_error;
	    dbus_error_init(&dbus_error);

#ifdef HAVE_POLKIT
	    // check for changes in policykit config
	    policykit.checkPolkitChanges();
#endif

	    // check the policy using PolicyKit
	    if (isActionAllowed(request, &dbus_error))
	    {
		// find the registered object
		std::string objname = request.path();

		// search the object
		Objects::const_iterator i = registered_objects.find(objname);

		bool found = false;

		if (i != registered_objects.end())
		{
		    ObjectData::const_iterator ii = i->second.find(request.interface());

		    if (ii != i->second.end())
		    {
			InterfaceData::const_iterator iii = ii->second.find(request.method());

			if (iii != ii->second.end())
			{
			    MethodData md = iii->second;
			    methodHandler mh = md.first;

			    y2milestone("Evaluating method: object %s interface: %s, method: %s",
				request.path().c_str(), request.interface().c_str(), request.method().c_str());

			    // call the registered callback
			    found = true;
			    reply = mh(request);
			}
			else
			{
			    y2warning("Object %s does not provide method %s in interface %s",
				objname.c_str(), request.path().c_str(), request.interface().c_str());
			}
		    }
		    else
		    {
			y2warning("Object %s does not provide interface %s",
			    objname.c_str(), request.interface().c_str());
		    }
		}
		else
		{
		    y2warning("Object %s is not registered", objname.c_str());
		}

		if (!found)
		{
		    // call unknown method handler
		    y2debug("Calling unknownRequest handler...");
		    reply = unknownRequest(request);
		}
	    }
	    else
	    {
		if (dbus_error_is_set(&dbus_error))
		{
		    y2warning("Returning PolicyKit error: %s: %s", dbus_error.name, dbus_error.message);
		    reply.createError(request, dbus_error.message, dbus_error.name);
		    dbus_error_free(&dbus_error);
		}
		else
		{
		    // report error
		    reply.createError(request, "PolicyKit check failed, unknown error",
			DBUS_ERROR_ACCESS_DENIED);
		}
	    }
	}
	else if (request.type() == DBUS_MESSAGE_TYPE_ERROR)
	{
	    DBusError error;
	    dbus_error_init(&error);

	    dbus_set_error_from_message(&error, request.getMessage());

	    if (dbus_error_is_set(&error))
	    {
		y2error("Received an error: %s: %s", error.name, error.message);
	    }

	    dbus_error_free(&error);
	}
	else if (request.type() == DBUS_MESSAGE_TYPE_SIGNAL)
	{
	    // singals are not supported
	    y2warning("Ignoring a received signal: interface: %s method: %s", request.interface().c_str(), request.method().c_str());
	}

	// was a reply set?
	if (!reply.empty())
	{
	    // send the reply
	    if (!connection.send(reply))
	    {
		y2error("Cannot send the result");
	    }
	    else
	    {
		y2debug("Flushing connection...");
		connection.flush();
		y2debug("...done");
	    }
	}

	y2milestone("Message processed");
    }

    y2milestone("Finishing the DBus service");
}

void DBusServerBase::register_method(const Object &obj, const Interface &intf, const Method &m, const DBusSignature &sig, methodHandler h)
{
    y2milestone("Registering DBus path: object %s interface %s method %s...", obj.c_str(), intf.c_str(), m.c_str());
    Objects::iterator i = registered_objects.find(obj);

    // create a new data item
    MethodData md = std::make_pair(h, sig);

    if (i == registered_objects.end())
    {
	// create a new object
	InterfaceData id;
	id[m] = md;

	ObjectData od;
	od[intf] = id;

	registered_objects[obj] = od;
    }
    else
    {
	ObjectData od = i->second;

	// search the interface
	ObjectData::iterator ii = od.find(intf);

	if (ii == od.end())
	{
	    // the interface is not registered
	    // create a new object
	    InterfaceData id;
	    id[m] = md;

	    od[intf] = id;

	    registered_objects[obj] = od;
	}
	else
	{
	    InterfaceData id = ii->second;
    
	    // search the method
	    InterfaceData::iterator iii = id.find(m);

	    if (iii == id.end())
	    {
		// the method is not registered
		// create a new object
		id[m] = md;

		od[intf] = id;

		registered_objects[obj] = od;
	    }
	    else
	    {
		y2warning("Object %s has already registered method %s in interface %s, updating...", obj.c_str(), m.c_str(), intf.c_str());

		// update the registered data
		id[m] = md;

		od[intf] = id;

		registered_objects[obj] = od;
	    }
	}
    }
}


bool DBusServerBase::isActionAllowed(const DBusMsg &msg, DBusError *err)
{
#ifdef HAVE_POLKIT
    // create actionId
    actionList actions_id(createActionId(msg));

    if (actions_id.empty())
    {
	// no actionId -> return the default (forbidden)
	return false;
    }

    for(actionList::iterator it = actions_id.begin();
	it != actions_id.end(); ++it)
    {
	if (!PolKit::isValidActionID(*it))
	{
	    y2error("Invalid action ID: %s", it->c_str());
	    return false;
	}

	if (dbus_error_is_set(err))
	{
	    dbus_error_free(err);
	}

	// check the policy here
	if (policykit.isDBusUserAuthorized(*it, msg.sender(), connection.getConnection(), err))
	{
	    y2security("User is authorized to do action %s", it->c_str());
	    return true;
	}
	else
	{
	    y2security("User is NOT authorized to do action %s", it->c_str());
	}
    }

    return false;
#else
    // no PolicyKit -> enable action
    return true;
#endif
}

DBusServerBase::actionList DBusServerBase::createActionId(const DBusMsg &msg)
{
    y2debug("Using default empty list of action IDs");
    // default implementation is empty list
    // the inherited classes should redefine it
    return actionList();
}

void DBusServerBase::unregister_client(const std::string &bus_id)
{
    Clients::iterator it = clients.find(bus_id);

    if (it != clients.end())
    {
	y2milestone("Unregistering client %s", bus_id.c_str());
	clients.erase(it);
    }
}

void DBusServerBase::register_client(const std::string &bus_id)
{
    // remember the client
    if (clients.find(bus_id) == clients.end())
    {
	// insert the dbus name and PID
	DBusCaller caller(bus_id, connection);

	// pid 0 = an error at DBus query
	if (caller.getPid() > 0)
	{
	    Clients::value_type new_client = make_pair(bus_id, caller);
	    y2milestone("Registered a new client %s (pid %d)", bus_id.c_str(), caller.getPid());
	    clients.insert(new_client);
	}
	else
	{
	    y2error("Cannot register client %s, pid query failed", bus_id.c_str());
	}
    }
}

DBusMsg DBusServerBase::unknownRequest(const DBusMsg &request)
{
    DBusMsg err_reply;
    y2error("Unknown object, interface or method");
    err_reply.createError(request, "Unknown object, interface or method", DBUS_ERROR_UNKNOWN_METHOD);

    return err_reply;
}
