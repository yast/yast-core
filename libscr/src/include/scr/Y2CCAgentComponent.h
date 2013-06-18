// -*- c++ -*-

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef Y2CCAgentComponent_h
#define Y2CCAgentComponent_h


#include <vector>
#include <y2/Y2ComponentCreator.h>


/**
 * Template class for a Y2ComponentCreator of an Y2AgentComp.
 */
template <class AgentComp> class Y2CCAgentComp : public Y2ComponentCreator
{

public:

    /**
     * Constructor of a Y2CCAgentComp object.
     */
    Y2CCAgentComp (const char*);


    /**
     * Destructor of a Y2CCAgentComp object.
     */
    ~Y2CCAgentComp () { 
      for (typename std::vector<AgentComp*>::iterator i = agent_instances.begin();
           i != agent_instances.end();
           ++i)
        delete *i;
    }


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

    /**
     * Name of my agent.
     */
    const char* my_name;

    /**
     * Component for given name
     */
    mutable typename std::vector<AgentComp*> agent_instances;
};


template <class AgentComp>
Y2CCAgentComp<AgentComp>::Y2CCAgentComp (const char* my_name)
    : Y2ComponentCreator (Y2ComponentBroker::BUILTIN),
      my_name (my_name)
{}


template <class AgentComp> Y2Component*
Y2CCAgentComp<AgentComp>::create (const char* name) const
{
    if (strcmp (name, my_name) == 0)
    {
        // Agent cannot share one component instance because IniAgent component
        // contain specific settings for given IniParser and it cannot be shared
        AgentComp *instance = new AgentComp(my_name);
        agent_instances.push_back(instance);
        return instance;
    }

    return 0;
}


#endif // Y2CCAgentComponent_h
