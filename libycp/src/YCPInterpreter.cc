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

   File:       YCPInterpreter.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

static const char *
YCPInterpreterId __attribute__((unused)) =
"$Id$";

#define INNER_DEBUG 1

#include <stdarg.h>

#include "StaticDeclaration.h"
#include "YCPInterpreter.h"
// FIXME #include "YCPDebugger.h"
#include "y2log.h"

// YCPInterpreter


// FIXME YCPDebugger* YCPInterpreter::debugger = NULL;


YCPInterpreter::YCPInterpreter ()
    : m_filename ("<unknown>")
      , m_line (0)
{
#if 0 // FIXME
    if (!debugger)
    {
	char *p = getenv ("Y2DEBUGGER");
	int i = p != NULL ? strtol (p, NULL, 10) : 0;
	if (i == 1 || i == 2)
	    debugger = new YCPDebugger (i == 2);
    }

    if (debugger)
	debugger->add_interpreter (this);
#endif
}


YCPInterpreter::~YCPInterpreter ()
{
#if 0 // FIXME
    if (debugger)
    {
	debugger->delete_interpreter (this);
	if (debugger->number_of_interpreters () == 0)
	{
	    delete debugger;
	    debugger = NULL;
	}
    }
#endif
}


string
YCPInterpreter::interpreter_name () const
{
    return "YCP";	// must be upper case
}


YCPValue YCPInterpreter::callback (const YCPValue& value)
{
    y2internal ("dummy callback (%s)", value->toString().c_str());
    return YCPVoid();
}

YCPValue YCPInterpreter::evaluateWFM (const YCPValue& value)
{
    y2internal ("dummy evaluateWFM (%s)", value->toString().c_str());
    return YCPVoid();
}

YCPValue YCPInterpreter::evaluateUI (const YCPValue& value)
{
    y2internal ("dummy evaluateUI (%s)", value->toString().c_str());
    return YCPVoid();
}

YCPValue YCPInterpreter::evaluateSCR (const YCPValue& value)
{
    y2internal ("dummy evaluateSCR (%s)", value->toString().c_str());
    return YCPVoid();
}


YCPValue YCPInterpreter::evaluate(const YCPValue& value)
{
    if (value.isNull())
    {
	return value;
    }

#if INNER_DEBUG
    y2debug ("[%s] evaluate (%s)", interpreter_name().c_str(), value->toString().c_str());
#endif

#if 0 //FIXME
    if (debugger)
	debugger->debug (this, YCPDebugger::Interpreter, value);
#endif

    YCPValue v = YCPNull();
    switch (value->valuetype())
    {
	case YT_ERROR:
	{
#if INNER_DEBUG
	    y2debug ("Error");
#endif
	    YCPError err = value->asError();
	    ycp2error (m_filename, m_line, "%s\n", err->message().c_str());
	    v = err->value();
	}
	break;
	case YT_CODE:
	{
#if INNER_DEBUG
	    y2debug ("Code");
#endif
	    v = evaluateCode	   (value->asCode());
	}
	break;

	// Simple value like integer or boolean are evaluated to
	// themselves.

	default:
	{
	    v = value;
	}
	break;
    }

#if INNER_DEBUG
    y2debug ("END [%s] evaluate (%s)", interpreter_name().c_str(), value->toString().c_str());
    y2debug ("[%s] result (%s)", interpreter_name().c_str(), (v.isNull()) ? "NULL" : v->toString().c_str());
#endif
    if (!v.isNull() && v->isError())
    {
	v = evaluate (v);
    }
    return v;
}


// -----------------------------------------------------------------------

YCPValue YCPInterpreter::evaluateCode(const YCPCode& code)
{
    YCPValue retval = code->evaluate ();
    return retval;
}

YCPValue YCPInterpreter::evaluateInstantiatedTerm(const YCPTerm& term)
{
    y2debug ("virtual evaluateInstantiatedTerm(%s)\n", term->toString().c_str());
    return YCPNull();
}


YCPValue YCPInterpreter::evaluateBuiltinTerm (const YCPTerm& term)
{
    y2debug ("virtual evaluateBuiltinTerm (%s)\n", term->toString().c_str());
    return YCPNull();
}

int
YCPInterpreter::lineNumber () const
{
    return m_line;
}

void
YCPInterpreter::setLineNumber (int line)
{
    m_line = line;
    return;
}

const char *
YCPInterpreter::filename () const
{
    return m_filename;
}

void
YCPInterpreter::setFilename (const char *filename)
{
    y2debug ("setFilename (%s)\n", filename);
    if (filename)
	m_filename = filename;
    return;
}

void YCPInterpreter::reportError (enum loglevel_t severity, const char *message, ...) const
{
    // Prepare info text
    va_list ap;
    va_start(ap, message);
    y2_vlogger (severity, interpreter_name().c_str(), m_filename, m_line, "", message, ap);
    va_end(ap);
}

