/* Y2CCModulesAgent.h
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 */

#ifndef Y2CCModulesAgent_h
#define Y2CCModulesAgent_h

#include "Y2.h"

/**
 * @short And a component creator for the component
 */
class Y2CCModulesAgent : public Y2ComponentCreator
{
public:
    /**
     * Enters this component creator into the global list of component creators.
     */
    Y2CCModulesAgent();
    
    /**
     * Specifies, whether this creator creates Y2Servers.
     */
    virtual bool isServerCreator() const;
    
    /**
     * Implements the actual creating of the component.
     */
    virtual Y2Component *create(const char *name) const;
};


#endif
