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

   File:       Y2PluginComponent.h

   Author:     Arvin Schnell <arvin@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component that starts a plugin
 */

#ifndef Y2PluginComponent_h
#define Y2PluginComponent_h

#include "Y2.h"
#include <ycp/YCPParser.h>

class Y2PluginComponent : public Y2Component
{
    /**
     * It this component a server or a client?
     */
    const bool is_server;

    /**
     * The name of the plugin library.
     */
    string filename;

    /**
     * Name of the global component creator.
     */
    string creator_name;

    /**
     * Name of the component that is implemented by the plugin.
     */
    string component_name;

    /**
     * Stores options for a server plugin.
     */
    int argc;

    /**
     * Stores options for a server plugin.
     */
    char** argv;

    /**
     * The component level the plugin was started in.
     */
    int level;

    /**
     * Handle of the dynamic loaded library.
     */
    void* handle;

    /**
     * The created Y2Component.
     */
    Y2Component* comp;

    /**
     * The saved callback pointer to be passed to the component
     * after creating (plugin loaded).
     */
    Y2Component* m_callback;

public:

    Y2PluginComponent (bool is_server, string filename, const char* creator_name,
		       const char* component_name, int level);

    /**
     * Frees internal data.
     */
    ~Y2PluginComponent ();

    /**
     * Returns the name of this component.
     */
    string name () const;

    /**
     * Let the server evaluate a command.
     *
     * This method is only valid, if the component is a server.
     */
    YCPValue evaluate (const YCPValue& command);

    /**
     * Returns the SCRAgent of the Y2Component or NULL if it doesn't have one.
     */
    SCRAgent* getSCRAgent ();

    /**
     * Tells this server, that the client doesn't need it's services
     * any longer and that the exit code of the client is result.
     *
     * This method is only valid, if the component is a server.
     */
    void result (const YCPValue& result);

    /**
     * Sets the commandline options of the server.
     *
     * This method is only valid, if the component is a server.
     */
    void setServerOptions (int argc, char** argv);

    /**
     * Functions to pass callback information
     * The callback is a pointer to a Y2Component with
     * a valid evaluate() function.
     * Override the Y2Component functions here since the plugin
     * component isn't the 'real' component but just a wrapper
     * which contains a pointer to the real one.
     * So any callback information must be passed by the Y2PluginComponent
     * to the component loaded via plugin.
     */

    Y2Component* getCallback (void) const;
    void setCallback (Y2Component *callback);

    /**
     * Launches the plugin with the previously set parameters.
     */
    YCPValue doActualWork (const YCPList& arglist, Y2Component* user_interface);

private:
    /**
     * Does actually load the plugin.
     */
    bool loadPlugin ();

    /**
     * Tries to locate the global componentcreator via dlsym.
     */
    Y2ComponentCreator* locateSym (int num);

};


#endif // Y2PluginComponent_h
