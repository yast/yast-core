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

#ifndef Y2CCIniAgent_h
#define Y2CCIniAgent_h

#include "Y2.h"

/**
 * @short And a component creator for the component
 */
class Y2CCIniAgent : public Y2ComponentCreator
{
    public:
        /**
         * Enters this component creator into the global list of component creators.
         */
        Y2CCIniAgent();
    
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
