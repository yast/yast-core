// -*- c++ -*-

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef Y2CCAgentComponent_h
#define Y2CCAgentComponent_h


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

};


template <class AgentComp>
Y2CCAgentComp<AgentComp>::Y2CCAgentComp (const char* my_name)
    : Y2ComponentCreator (Y2ComponentBroker::BUILTIN),
      my_name (my_name)
{
}


template <class AgentComp> Y2Component*
Y2CCAgentComp<AgentComp>::create (const char* name) const
{
    if (strcmp (name, my_name) == 0)
	return new AgentComp (my_name);

    return 0;
}


#endif // Y2CCAgentComponent_h
