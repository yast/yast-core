/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Ini agent implementation
 *
 * Authors:
 *   Petr Blahos <pblahos@suse.cz>
 *
 * $Id$
 */

#ifndef Y2IniAgentComponent_h
#define Y2IniAgentComponent_h

#include "Y2.h"

class SCRInterpreter;
class IniAgent;


class Y2IniAgentComponent : public Y2Component
{
    private:
        SCRInterpreter *interpreter;
        IniAgent *agent;
    
    public:
    
        /**
         * Default constructor
         */
        Y2IniAgentComponent();
        
        /**
         * Destructor
         */
        ~Y2IniAgentComponent();
        
        /**
         * Returns true: The scr is a server component
         */
        bool isServer() const;
        
        /**
         * Returns the name of the module.
         */
        virtual string name() const;
        
        /**
         * Starts the server, if it is not already started and does
         * what a server is good for: Gets a command, evaluates (or
         * executes) it and returns the result.
         * @param command The command to be executed. Any YCPValueRep
         * can be executed. The execution is performed by some
         * YCPInterpreter.
         */
        virtual YCPValue evaluate(const YCPValue& command);
};

#endif
