// -*- c++ -*-

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef Y2AgentComponent_h
#define Y2AgentComponent_h


#include <ycp/y2log.h>
#include <y2/Y2Component.h>


class SCRInterpreter;
class SCRAgent;


/**
 * Template class for a Y2AgentComp of an Agent.
 */
template <class Agent> class Y2AgentComp : public Y2Component
{

public:

    /**
     * Constructor for a Y2AgentComp.
     */
    Y2AgentComp (const char*);

    /**
     * Clean up.
     */
    ~Y2AgentComp ();

    /**
     * Returns the name of the component.
     */
    string name () const { return my_name; }

    /**
     * Evaluates a command to the agent.
     */
    YCPValue evaluate (const YCPValue &command);

    /**
     * Returns the SCRAgent of the Y2Component.
     */
    SCRAgent* getSCRAgent ();

private:

    /**
     * Name of my agent.
     */
    const char* my_name;

    /**
     * Pointer to my agent.
     */
    Agent* agent;

    /**
     * Pointer to my scr interpreter.
     */
    SCRInterpreter* interpreter;

};


template <class Agent>
Y2AgentComp<Agent>::Y2AgentComp (const char* my_name)
    : my_name (my_name),
      agent (0),
      interpreter (0)
{
}


template <class Agent>
Y2AgentComp<Agent>::~Y2AgentComp ()
{
    if (interpreter)
    {
        delete interpreter;
        delete agent;
    }
}


template <class Agent> YCPValue
Y2AgentComp<Agent>::evaluate (const YCPValue& value)
{
    y2debug ("evaluate (%s)", value->toString ().c_str ());

    if (!interpreter)
	getSCRAgent ();

    return interpreter->evaluate (value);
}


template <class Agent> SCRAgent*
Y2AgentComp<Agent>::getSCRAgent ()
{
    if (!interpreter)
    {
	agent = new Agent ();
	interpreter = new SCRInterpreter (agent);
    }

    return agent;
}


#endif // Y2AgentComponent_h
