/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Y2SystemFunction.cc

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/


#include <y2util/y2log.h>
#include <ycp/SymbolTable.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPString.h>

#include <y2/Y2ProgramComponent.h>

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "Y2SystemNamespace.h"
#include "Y2SystemFunction.h"

#include "ycp/SymbolTable.h"

Y2SystemFunction::Y2SystemFunction (Y2Function* local_call, constFunctionTypePtr type) :
    m_local (local_call)
    , m_remote (0)
    , m_use_remote (false)
    , m_type (type)
{
}

Y2SystemFunction::~Y2SystemFunction ()
{
    // FIXME deletes
    
    // FIXME unregister in Y2Namespace
}

bool 
Y2SystemFunction::attachParameter (const YCPValue& arg, const int position)
{
    Y2Function* cur = m_use_remote ? m_remote : m_local;
    
    return cur->attachParameter (arg, position);
}

constTypePtr 
Y2SystemFunction::wantedParameterType () const
{
    Y2Function* cur = m_use_remote ? m_remote : m_local;
    
    return cur->wantedParameterType ();
}


bool 
Y2SystemFunction::appendParameter (const YCPValue& arg)
{
    Y2Function* cur = m_use_remote ? m_remote : m_local;
    
    return cur->appendParameter (arg);
}


bool 
Y2SystemFunction::finishParameters ()
{
    Y2Function* cur = m_use_remote ? m_remote : m_local;
    
    return cur->finishParameters ();
}


YCPValue 
Y2SystemFunction::evaluateCall ()
{
    Y2Function* cur = m_use_remote ? m_remote : m_local;
    
    y2milestone ("Going to call proxied function '%s'", m_local->name ().c_str ());
    
    return cur->evaluateCall ();
}


bool 
Y2SystemFunction::reset ()
{
    Y2Function* cur = m_use_remote ? m_remote : m_local;
    
    return cur->reset ();
}


void
Y2SystemFunction::useRemote (Y2Function* remote_call)
{
    m_remote = remote_call;
    m_use_remote = true;
    // FIXME: delete old remote?
    
    y2milestone ("'%s': switched to remote", m_local->name ().c_str ());
}


void
Y2SystemFunction::useLocal ()
{
    m_use_remote = false;
    m_remote = 0; // FIXME: delete?
}


string
Y2SystemFunction::name () const
{
    return m_local->name ();
}


constFunctionTypePtr
Y2SystemFunction::type () const
{
    return m_type;
}
