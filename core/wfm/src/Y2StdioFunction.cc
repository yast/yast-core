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

   File:       Y2StdioFunction.cc

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#include "Y2StdioFunction.h"

#include <y2util/y2log.h>
#include <ycp/SymbolTable.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPString.h>

#include <y2/Y2ProgramComponent.h>

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "Y2SystemNamespace.h"
#include "ycp/SymbolTable.h"


// TODO: do some code sharing with Y2YCPFunction (parameter handling)

Y2StdioFunction::Y2StdioFunction (string ns, string name
    , constFunctionTypePtr type, Y2ProgramComponent* sender) :
    m_namespace (ns)
    , m_name (name)
    , m_type (type)
    , m_sender (sender)
{
    uint count = type->parameterCount ();
   
    m_parameters = new YCPValue[count];
      
    for (uint i=0; i < count; i++)
    {
	m_parameters[i] = YCPNull ();
    }
}

Y2StdioFunction::~Y2StdioFunction ()
{
    delete[] m_parameters;
}

bool 
Y2StdioFunction::attachParameter (const YCPValue& arg, const int pos)
{
    if (pos < 0 || pos > m_type->parameterCount ())
    {
	y2error ("Attaching parameter to function '%s::%s' at incorrect position: %d"
	    , m_namespace.c_str(), m_name.c_str (), pos );
	return false;
    }

    m_parameters[pos] = arg;
    return true;
}

constTypePtr 
Y2StdioFunction::wantedParameterType () const
{
    y2internal ("Somebody asking for a parameter type redirector");
    return Type::Any;
}


bool 
Y2StdioFunction::appendParameter (const YCPValue& arg)
{
    if (arg.isNull())
    {
	ycp2error ("NULL parameter to %s::%s", m_namespace.c_str(), m_name.c_str ());
	return false;
    }
			    
    // FIXME: check the type
				
    // lookup the first non-set parameter
    for (int i = 0 ; i < m_type->parameterCount (); i++)
    {
	if (m_parameters[i].isNull ())
	{
#if DO_DEBUG
	    y2debug ("Assigning parameter %d: %s", i, arg->toString ().c_str ());
#endif
	    m_parameters[i] = arg;
	    return true;
	}
    }

    // Our caller should report the place
    // in a script where this happened
    ycp2error ("Excessive parameter to %s::%s"
	, m_namespace.c_str(), m_name.c_str ());
															        // FIXME
    return false;
}


bool 
Y2StdioFunction::finishParameters ()
{
    for (int i = 0 ; i < m_type->parameterCount (); i++)
    {
	if (m_parameters [i].isNull ())
	{
	    y2error ("Missing parameter %d to %s::%s",
	         i, m_namespace.c_str(), m_name.c_str ());
	    return false;
	}
    }
									         // FIXME
    return true;
}


YCPValue 
Y2StdioFunction::evaluateCall ()
{
    y2milestone ("Evaluating remote call to '%s::%s'"
	, m_namespace.c_str (), m_name.c_str ());

    // FIXME: ensure connected

    string params = "";
    
    if (m_type->parameterCount () > 0)
    {
	params = m_parameters[0]->toString ();
	for (int i = 1 ; i < m_type->parameterCount (); i++)
	{
	    params += ", " + m_parameters[i]->toString ();
	}
    }
    
    string call = string ("{ import \"") + m_namespace
	+ "\"; return " + m_namespace + "::" 
	+ m_name + "(" + params + "); }";

#if DO_DEBUG
    y2debug ("Going to evaluate a call: %s", call.c_str ());
#endif
    
    Y2ProgramComponent* sender = dynamic_cast<Y2ProgramComponent*>(m_sender);

    // send command
    sender->sendToExternal (call);

    // get answer
    YCPValue retval = sender->receiveFromExternal();
    return !retval.isNull() ? retval : YCPVoid();
}


bool 
Y2StdioFunction::reset ()
{
    for (int i = 0; i < m_type->parameterCount (); i++)
    {
	m_parameters[i] = YCPNull ();
    }

    return true;
}



string
Y2StdioFunction::name () const
{
    return m_name;
}
