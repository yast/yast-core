/* Y2ModulesAgentComponent.h
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 */

#ifndef Y2ModulesAgentComponent_h
#define Y2ModulesAgentComponent_h

#include "Y2.h"

class SCRInterpreter;
class ModulesAgent;


class Y2ModulesAgentComponent : public Y2Component
{
    SCRInterpreter *interpreter;
    ModulesAgent *agent;
    
public:
    
    /**
     * Default constructor
     */
    Y2ModulesAgentComponent();
    
    /**
     * Destructor
     */
    ~Y2ModulesAgentComponent();
    
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
