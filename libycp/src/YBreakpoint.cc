/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	YBreakpoint.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include <libintl.h>
#include <debugger/Debugger.h>

#include "ycp/YBreakpoint.h"

#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

extern Debugger *debugger_instance;

// ------------------------------------------------------------------

IMPL_DERIVED_POINTER(YBreakpoint, YCode);

// ------------------------------------------------------------------
// YBreakpoint

YBreakpoint::YBreakpoint (YCodePtr code, std::string name) : YCode()
    , m_code (code)
    , m_enabled (true)
    , m_name (name)
{
}


YBreakpoint::~YBreakpoint ()
{
    m_code = 0;
}


bool
YBreakpoint::isConstant() const
{
    return m_code->isConstant();
}


bool
YBreakpoint::isStatement() const
{
    return m_code->isStatement();
}


bool
YBreakpoint::isBlock () const
{
    return m_code->isBlock();
}


bool
YBreakpoint::isReferenceable () const
{
    return m_code->isReferenceable ();
}


string
YBreakpoint::toString() const
{
    return m_code->toString ();
}

YCodePtr 
YBreakpoint::code() const
{
    return m_code;
}

// write to stream, see Bytecode for read
std::ostream &
YBreakpoint::toStream (std::ostream & str) const
{
    return m_code->toStream(str);
}


std::ostream &
YBreakpoint::toXml (std::ostream & str, int indent ) const
{
    return m_code->toXml(str, indent);
}


YCPValue
YBreakpoint::evaluate (bool cse)
{
#if DO_DEBUG
    y2debug ("evaluate(%s) = nil", toString().c_str());
#endif
    if (debugger_instance && m_enabled)
    {
	debugger_instance->stashOutput ("Breakpoint hit at " + m_name + "\n");
    	if (m_code->isBlock()) {
	    debugger_instance->setTracing();
	}
	else
	    y2internal ("Debugger: process input failed");
    }
    
    return m_code->evaluate (cse);
}


constTypePtr
YBreakpoint::type () const
{
    return m_code->type ();
}

bool
YBreakpoint::enabled () const
{
    return m_enabled;
}

void
YBreakpoint::setEnabled (bool enable) 
{
    m_enabled = enable;
}


// EOF
