/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Y2Component.h

   Author:     Mathias Kettner <kettner@suse.de>
	       Thomas Roelz <tom@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef Y2Component_h
#define Y2Component_h

#include <string>

using std::string;

class SCRAgent;
class Y2Namespace;
class YCPValue;
class YCPList;

/**
 * @short Communication handle to a YaST2 component.
 * @see Y2ComponentBroker.
 * YaST2 is a network oriented client/server architecture.
 * Currently there exist five differnt types of components:
 * userinterfaces, modules, the workflowmanagers, the
 * repository (scr) and scr agents. All components are
 * communicating using the same protocol.
 *
 * The communication between two components is asymmetric. On
 * component is said to be the <i>server</i>, the other to be
 * the <i>client</i>. The protocol begins with the client sending
 * an YCPValueRep, the <i>command</i> to the server. The server
 * <i>evaluates</i> it and returns an YCPValueRep, the <i>answer</i>.
 * Now the client sends the next command and so on. When the client
 * has finished the work with the server, it sends a <i>result</i>
 * value to the server and both sides close the connection. The result
 * is a special YCPValueRep that is a term with the symbol "result" and
 * exactly one argument.
 *
 * Each component may be realized by a binary program (under Linux: ELF),
 * by a process listening to an internet-domain socket, by a shared
 * library plugin in ELF format (ending in .so), by a YCP script that
 * is executed by the Workflowmanager (WFM), or by a C++ class that
 * is linked to the Generic YaST2 Frontend (GF).
 *
 * When you want to implement a component you need to subclass Y2Component
 * and define the virtual functions (some of them only for a client component, some
 * of them only for a server component). Furthermore you need to subclass
 * @ref Y2ComponentCreator and create a global variable from this class.
 *
 * When you implement a component that needs the services of another
 * component then you ask the @ref Y2ComponentBroker to get a handle
 * to it. That handle is of type Y2Component. Once you
 * have got the handle, you can use it communicate with the component,
 * regardless whether it is realized by a program, a plugin or what so ever.
 *
 * See @ref Y2ComponentBroker for examples.
 */

class Y2Component
{
public:

    /* ================ common ================ */

    Y2Component();

    /**
     * Base class must have virtual destructor.
     */
    virtual ~Y2Component();

    /**
     * Returns the name of the module.
     */
    virtual string name() const = 0;

    /* ================ server ================ */

    /**
     * Starts the server, if it is not already started and
     * does what a server is good for: Gets a command, evaluates
     * (or executes) it and returns the result.
     * @param command The command to be executed. Any YCPValueRep can
     * be executed. The execution is performed by some @ref YCPInterpreter.
     * @return the result. Destroy it after use with @ref YCPElementRep#destroy.
     *
     * This method is only defined, if the component is a server.
     */
    virtual YCPValue evaluate(const YCPValue& command);

    /**
     * Tells this server, that the client doesn't need it's services
     * any longer and that the exit code of the client is result.
     *
     * This method is only defined, if the component is a server.
     */
    virtual void result(const YCPValue& result);

    /**
     * Sets the commandline options of the server.
     * @param argc number of arguments including argv[0], the
     * name of the component
     * @param argv a pointer to a field of argc+1 char *,
     * where the last one must be 0. The caller of the function
     * must assure that the data field is persistent. This method
     * will _not_ make a copy of it.
     *
     * This method is only defined, if the component is a server.
     */
    virtual void setServerOptions(int argc, char **argv);

    /**
     * Try to import a given namespace. This method is used
     * for transparent handling of namespaces (YCP modules)
     * through whole YaST.
     * NOTICE: there is no reverse operation to import.
     * Semantics of YCP modules is there is a single instance
     * and it is available from the first import
     * until the end of YaST run.
     * @param name_space the name of the required namespace
     * @return on errors, NULL should be returned. The
     * error reporting must be done by the component itself
     * (typically using y2log). On success, the method
     * should return a proper instance of the imported namespace
     * ready to be used. The returned instance is still owned
     * by the component, any other part of YaST will try to
     * free it. Thus, it's possible to share the instance.
     */
    virtual Y2Namespace* import(const char* name_space);

    /* ================ client ================ */

    /**
     * This function must be overridden by an actual client.
     * Here the client does its actual work.
     * @param arglist YCPList of client arguments.
     * @param user_interface Option display server (user interface)
     * Most clients need interaction with the user. The different
     * YaST2 user interfaces are servers. If the user interace
     * is already active and running, give a handle to it with this
     * parameter. Give 0 here if no user interface is running yet and
     * either the module launches the user interace itself or it it
     * does not need one.
     * @return The result value (exit code) of the called client. The
     * result code has <i>not</i> yet been sent to the display server.
     * Destroy it after use.
     *
     * This method is only defined, if the component is a client.
     */
    virtual YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);
    
    /* ================ misc ================ */

    /**
     * Returns the SCRAgent of the Y2Component or NULL, if it doesn't have
     * one. Note: This might trigger the creation of the Interpreter and
     * Agent associated with the Y2Component. For plugins, this might trigger
     * the loading of the plugin as well.
     */
    virtual SCRAgent * getSCRAgent ();
};

#endif // Y2Component_h
