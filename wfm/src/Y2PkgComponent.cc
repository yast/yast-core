
#include <y2util/y2log.h>
#include <y2/Y2Namespace.h>
#include <y2/Y2Component.h>
#include <y2/Y2ComponentCreator.h>

#include "Y2PkgComponent.h"

#include "PkgModule.h"

Y2Namespace *Y2PkgComponent::import (const char* name)
{
    // FIXME: for internal components, we should track changes in symbol numbering
    if ( strcmp (name, "Pkg") == 0)
    {
	return PkgModule::instance ();
    }
	
    return NULL;
}

Y2PkgComponent* Y2PkgComponent::m_instance = NULL;

Y2PkgComponent* Y2PkgComponent::instance ()
{
    if (m_instance == NULL)
    {
        m_instance = new Y2PkgComponent ();
    }

    return m_instance;
}

