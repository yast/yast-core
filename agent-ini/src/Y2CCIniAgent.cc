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

#include "Y2CCIniAgent.h"
#include "Y2IniAgentComponent.h"


Y2CCIniAgent::Y2CCIniAgent()
    : Y2ComponentCreator(Y2ComponentBroker::BUILTIN)
{
}


bool
Y2CCIniAgent::isServerCreator() const
{
    return true;
}


Y2Component *
Y2CCIniAgent::create(const char *name) const
{
    if (!strcmp(name, "ag_ini")) return new Y2IniAgentComponent();
    else return 0;
}


Y2CCIniAgent g_y2ccag_ini;
