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

   File:       Y2ScriptComponent.h

   Author:     Mathias Kettner <kettner@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component that starts wfm to execute a YCP script
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef Y2ScriptComponent_h
#define Y2ScriptComponent_h

#include <y2/Y2Component.h>

class Y2ScriptComponent : public Y2Component
{
    /**
     * The script that implements the component.
     */
    YCPValue script;

    /**
     * The name of the client that is implemented by the script.
     */
    string client_name;

    /**
     * The fullname of the script file.
     */
    string fullname;
    
    Y2Component* m_wfm;
    
    Y2Component* wfm_instance ();
    
    static Y2ScriptComponent* m_instance;

public:

    Y2ScriptComponent ();

    /**
     * Setups this script component.
     * @param the name of the component that is realized be the script.
     * @param script the script. This component clones it, so you can
     * destroy the script after the constructor call.
     */
    void setupComponent (string client_name, string fullname,
		       const YCPValue& script);

    /**
     * Cleans up
     */
    ~Y2ScriptComponent();

    /**
     * Returns the name of the component the script implements.
     */
    virtual string name() const;

    /**
     * Only used for callbacks.
     */
    virtual YCPValue evaluate (const YCPValue& command);

    /**
     * Implements the communication with the server component.
     * It delegates it to the workflowmanager component.
     */
    virtual YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);
    
    virtual Y2Namespace* import (const char* name_space, const char* timestamp = NULL);
    
    static Y2ScriptComponent* instance();

};


#endif // Y2ScriptComponent_h
