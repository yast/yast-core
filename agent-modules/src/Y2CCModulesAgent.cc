/* Y2CCModulesAgent.cc
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 */

#include "Y2CCModulesAgent.h"
#include "Y2ModulesAgentComponent.h"

Y2CCModulesAgent::Y2CCModulesAgent()
    : Y2ComponentCreator(Y2ComponentBroker::BUILTIN)
{
}


bool
Y2CCModulesAgent::isServerCreator() const
{
    return true;
}


Y2Component *
Y2CCModulesAgent::create(const char *name) const
{
    if (!strcmp(name, "ag_modules")) return new Y2ModulesAgentComponent();
    else return 0;
}


Y2CCModulesAgent g_y2ccag_modules;
