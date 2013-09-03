#ifndef Y2CCSCR_H
#define Y2CCSCR_H

#include <y2/Y2ComponentCreator.h>
#include <map>
#include <string>

#include "ScriptingAgent.h"
#include "Y2SCRComponent.h"

class Y2CCSCR : public Y2ComponentCreator
{
public:

    /**
     * Constructor of a SCR component creator.
     */
    Y2CCSCR ();


    /**
     * Destructor of a SCR component creator.
     */
    ~Y2CCSCR ();


    /**
     * Returns true since all agents are server components.
     */
    bool isServerCreator () const { return true; }

    /**
     * Creates a new @ref Y2Component if the name matches the one
     * provided in the constructor.
     */
    Y2Component* create (const char*) const;

    /**
     * Agent components do not provide any namespaces.
     */
    Y2Component* provideNamespace (const char*) { return NULL; }

private:

    /* SCR instances, owned by this class, keyed by target root.
     * Target root must be valid absolute path like "/" or "/mnt".*/
    mutable std::map<std::string, Y2SCRComponent*> scr_instances;
};

#endif
