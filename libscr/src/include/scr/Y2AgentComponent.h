// -*- c++ -*-

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#ifndef Y2AgentComponent_h
#define Y2AgentComponent_h


#include <ycp/y2log.h>
#include <ycp/YCPTerm.h>
#include <y2/Y2Component.h>

#include <ycp/YCPVoid.h>

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

    YCPValue Read (const YCPPath &path);
    
private:

    /**
     * Name of my agent.
     */
    const char* my_name;

    /**
     * Pointer to my agent.
     */
    Agent* agent;

};


template <class Agent>
Y2AgentComp<Agent>::Y2AgentComp (const char* my_name)
    : my_name (my_name),
      agent (0)
{
}


template <class Agent>
Y2AgentComp<Agent>::~Y2AgentComp ()
{
    if (agent)
    {
        delete agent;
    }
}


template <class Agent> YCPValue
Y2AgentComp<Agent>::evaluate (const YCPValue& v)
{
    y2debug ("evaluate (%s)", v->toString ().c_str ());

    if (!agent)
	getSCRAgent ();

    y2debug ("Going to evaluate %s", v->toString ().c_str ());
	
    YCPValue value = v;
    if (value->isCode ())
	value = value->asCode ()->evaluate ();
	
    if (value.isNull () || value->isVoid ())
	return value;

    y2debug ("After code evaluation: %s", value->toString ().c_str ());

    if( value->isTerm () ) {
	YCPTerm term = value ->asTerm ();
	string command = term->name ();
	YCPList args = term->args ();

	// evaluate the term in native functions
	if( command == "Read" ) {
	    return getSCRAgent ()-> Read (args->value (0)->asPath (), args->size() > 1 ? args->value (1) : YCPNull ()) ;
	}
	else if( command == "Write" ) {
	    return getSCRAgent ()-> Write (args->value (0)->asPath (), args->value (1), args->size () > 2 ? args->value (2) : YCPNull ()) ;
	}
	else if( command == "Dir" ) {
	    return getSCRAgent ()-> Dir (args->value (0)->asPath ()) ;
	}
	else if( command == "Error" ) {
	    return getSCRAgent ()-> Error (args->value (0)->asPath ()) ;
	}
	else if( command == "Execute" ) {
	    y2debug( "Execute, arg size is %d", args->size() );
	    switch( args->size() ) {
		case 1:
		    return getSCRAgent ()-> Execute (args->value (0)->asPath ()) ;
		case 2:
		    return getSCRAgent ()-> Execute (args->value (0)->asPath (), args->value (1)) ;
		default:
		    return getSCRAgent ()-> Execute (args->value (0)->asPath (), args->value (1), args->value (2)) ;
	    }
	}
	else {
	    y2debug( "Passing term to otherCommand" );
	    return getSCRAgent ()-> otherCommand (term);
	}
    }
#if 0
    if( value->isCode () ) {
	y2debug( "Passing (evaluated) code to otherCommand" );
	return getSCRAgent ()-> otherCommand (value->asCode ()->evaluate ()->asTerm ());
    }
#endif

    y2error( "Unhandled value (%d): %s", value->valuetype (), value->toString ().c_str () );

    return YCPVoid();
}


template <class Agent> SCRAgent*
Y2AgentComp<Agent>::getSCRAgent ()
{
    if (!agent)
    {
	agent = new Agent ();
    }

    return agent;
}

template <class Agent> YCPValue Y2AgentComp<Agent>::Read (const YCPPath &path)
{
    y2error( "Y2AgentComp::Read" );
    return getSCRAgent()->Read (path);
}

#endif // Y2AgentComponent_h
