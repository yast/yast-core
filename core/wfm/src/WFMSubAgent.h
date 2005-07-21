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

   File:	WFMSubAgent.h

   Author:	Arvin Schnell <arvin@suse.de>
   Maintainer:  Arvin Schnell <arvin@suse.de>

/-*/

#ifndef WFMSubAgent_h
#define WFMSubAgent_h

#include <string>

using std::string;

#include <y2/Y2Component.h>


/**
 * A simple class handling a agent.
 */
class WFMSubAgent
{

public:

    /**
     * Constructor for a subagent. Takes the name and handle as arguments.
     */
    WFMSubAgent (const string&, int);

    /**
     * Destructor for a subagent. Terminates the Y2Component if necessary.
     */
    ~WFMSubAgent ();

    /**
     * Starts the subagent. Returns true on success otherwise false.
     */
    bool start ();

    /**
     * Starts the subagent and evaluates one term to ensure that the component
     * is created (mainly for remote components). Can also check for the
     * correct SuSE Version. Returns true on success otherwise false and sets
     * the error number.
     */
    bool start_and_check (bool, int*);

    /**
     * Returns the name of the subagent.
     */
    string get_name () const { return my_name; }

    /**
     * Returns the handle of the subagent.
     */
    int get_handle () const { return my_handle; }

    /**
     * Returns the Y2Component of the subagent. This does not call start ().
     * Is 0 if start () was not called or failed.
     */
    Y2Component* comp () { return my_comp; }

    /**
     * Returns the SCRAgent of the subagent. This does not call start ().
     * Is 0 if start () was not called or failed or the Y2Component does not
     * support the getSCRAgent () function.
     */
    SCRAgent* agent () { return my_agent ? my_agent : (my_comp ? my_comp->getSCRAgent () : 0); }

private:

    /**
     * The name.
     */
    const string my_name;

    /**
     * The handle.
     */
    const int my_handle;

    /**
     * The component.
     */
    Y2Component* my_comp;
    
    /**
     * The agent if component does not provide one
     */
    SCRAgent* my_agent;

    WFMSubAgent (const WFMSubAgent&);		// disallow
    void operator = (const WFMSubAgent&);	// disallow

};


inline bool
wfmsubagent_less (const WFMSubAgent* a, int handle) // FIXME
{
    return a->get_handle () < handle;
}


#endif // WFMSubAgent_h
