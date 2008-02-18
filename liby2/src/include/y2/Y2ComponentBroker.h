/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Y2ComponentBroker.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef Y2ComponentBroker_h
#define Y2ComponentBroker_h

#include <string.h>
#include <map>
#include <vector>
#include <string>

using std::vector;
using std::map;
using std::string;

class Y2ComponentCreator;
class Y2Component;

/**
 * @short Looks for and creates YaST2 components
 * This class has no instances and only static methods.
 * There are two reasons for this:
 *
 * a) Only one component broker is needed
 *
 * b) The data must be accessable before the first global
 *    constructor is called
 *
 * The component broker is the one that you can ask for
 * if you need a certain component. Components are specified
 * by names. A component name is an arbitrary string.
 * The component broker does not statically know what kinds components
 * exist. During global constructor call time (before main), the
 * constructors of the @ref ComponentCreator classes <i>register</i>
 * themselves to the component broker.
 */
class Y2ComponentBroker
{
public:
    /**
     * Constants for the different types of component
     * creators.
     */
    enum order_t { BUILTIN          = 0,
		   PLUGIN           = 1,
		   SCRIPT           = 2,
		   EXTERNAL_PROGRAM = 3,
		   NETWORK          = 4,
		   MAX_ORDER        = 5 };

private:

    struct ltstr
    {
	bool operator()(const char* s1, const char* s2) const
	{
	    return strcmp(s1, s2) < 0;
	}
    };

    static map<const char *, const Y2Component *, ltstr> namespaces;

    /**
     * Storage for the component creators.
     */
    static vector<const Y2ComponentCreator *> *creators[MAX_ORDER];

    /**
     * This flag stops the registry of components at the broker.
     * It must be set to true before any plugin (dynamic loadable
     * library) is loaded!
     */
    static bool stop_register;
    
    /**
     * A map containing a namespace exceptions. This will be honoured in
     * getNamespaceComponent to give an explicit preference for a
     * namespace to be created by a preffered component.
     */
    static map<string, string> namespace_exceptions;
    
public:
    /**
     * Enters a component creator into the list of
     * component creators. Is called by @ref Y2ComponentCreator#Y2ComponentCreator.
     * @param creator the component creator the register
     * @param order The orders define the order how the creators
     * are looked up. A creator with a lower order is looked up before one
     * with a higher order. It is very important that the compiled-in components
     * must be created with the lowest order to prevent an infinitive loop of starting
     * external components.
     * @param force override the stop_register flag.
     * See @ref #order_t for the possible orders.
     */
    static void registerComponentCreator(const Y2ComponentCreator *creator, order_t order, bool force=false);

private:
    /**
     * Tries to create or find a YaST2 component.
     * @param spec Specifies which component to find.
     * @param look_for_clients Set this to true if you are looking for clients.
     * If set to false only servers are created.
     * @return A pointer to the new component if one has been found, 0 if no
     * component matching spec has been found.
     */
    static Y2Component *createComponent(const char *name, bool look_for_clients);

public:
    /**
     * Is a wrapper for @ref #createComponent, but only looks for clients.
     */
    static Y2Component *createClient(const char *name);

    /**
     * Is a wrapper for @ref #createComponent, but only looks for servers.
     */
    static Y2Component *createServer(const char *name);
    
    /**
     * Provide a component which implements the given namespace.
     *
     * @param name	the name of the requested namespace
     * @return 		a component instance or 0 if unsuccessful
     */
    static Y2Component *getNamespaceComponent(const char *name);
    
    /**
     * Register a new namespace exception to be used by getNamespaceComponent.
     * @param name_space	the namespace to be changed
     * @param component_name	the component which should provide the namespace
     * @return 	true on success, false on failure (for example, a namespace is already
     * instantiated by another component.
     */
    static bool registerNamespaceException(const char* name_space, const char* component_name);

private:
    /**
     * Initializes @ref #creators.
     */
    static void initializeLists();
};


#endif //Y2ComponentBroker_h
