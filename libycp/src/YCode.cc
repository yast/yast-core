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

   File:	YCode.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/
// -*- c++ -*-
#include <libintl.h>

#include "ycp/YCode.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPByteblock.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPError.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPList.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPCode.h"

#include "ycp/YBlock.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"


// ------------------------------------------------------------------
// YCode

YCode::YCode (ycode code)
    : m_code (code)
{
}


YCode::~YCode ()
{
}


YCode::ycode
YCode::code() const
{
    return m_code;
}


bool
YCode::isConstant() const
{
    return (m_code < ycConstant);
}


bool
YCode::isError() const
{
    return (m_code == yxError);
}


bool
YCode::isStatement() const
{
    return ((m_code > yeExpression)
	    && (m_code < ysStatement));
}


bool
YCode::isBlock () const
{
    return (m_code == yeBlock);
}


string
YCode::toString (ycode code)
{
    static char *names[] = {
	"yxError",
	// [1] Constants	(-> YCPValue, except(!) locale and term -> yeLocale, yeTerm)
	"ycVoid", "ycBoolean", "ycInteger", "ycFloat",	// constants
	"ycString", "ycByteblock", "ycPath", "ycSymbol",
	"ycList",					// list
	"ycMap",					// map
	"ycEntry",

	"ycConstant",		// -- placeholder --
	"ycLocale",		// locale constant (gettext)
	"ycFunction",		// function definition (parameters and body)

	// [15] Expressions	(-> declaration_t + values)
	"yePropagate",		// type propagation (value, type)
	"yeUnary",		// unary (prefix) operator
	"yeBinary",		// binary (infix) operator
	"yeTriple",		// <exp> ? <exp> : <exp>
	"yeCompare",		// compare
	"yePrefix",		// normal function

	// [21] Value expressions (-> values + internal)
	"yeLocale",		// locale expression (ngettext)
	"yeList",		// list expression
	"yeMap",		// map expression
	"yeTerm",		// <name> ( ...)
	"yeLookup",		// lookup ()
	"yeIs",			// is()
	"yeBracket",		// <name> [ <expr>, ... ] : <expr>

	// [28] Block (-> linked list of statements)
	"yeBlock",		// block expression
	"yeReturn",		// quoted expression

	// [30] Symbolref (-> SymbolEntry)
	"yeVariable",		// variable ref
	"yeBuiltin",		// builtin ref + args
	"yeSymFunc",		// builtin ref with symbols + args
	"yeFunction",		// function ref + args

	"yeExpression",		// -- placeholder --

	// [35] Statements	(-> YCode + next)
	"ysTypedef",		// typedef
	"ysVariable",		// variable definition
	"ysFunction",		// function definition
	"ysAssign",		// variable assignment
	"ysBracket",		// <name> [ <expr>", ... ] = <expr>
	"ysIf",			// if", then", else
	"ysWhile",		// while () do ...
	"ysDo",			// do ... while ()
	"ysRepeat",		// repeat ... until ()
	"ysExpression",		// any expression (function call)
	"ysReturn",
	"ysBreak",
	"ysContinue",
	"ysTextdomain",
	"ysInclude",
	"ysFilename",

	"ysStatement"		// -- placeholder --
    };

    if ((int)code < 0 || (unsigned int)code >= (sizeof (names) / sizeof (*names)))
    {
	y2error ("Bad ycode %d", code);
	return "*** BAD YCode";
    }
    char buf[16]; sprintf (buf, "[%d]", code);
    return string (buf) + names[code];
}


string
YCode::toString() const
{
    if (isError())
    {
	return "YError";
    }
    return toString (m_code);
}


// write to stream, see Bytecode for read
std::ostream &
YCode::toStream (std::ostream & str) const
{
    y2debug ("YCode::toStream (%d:%s)", (int)m_code, YCode::toString (m_code).c_str());
    return str.put ((char)m_code);
}


YCPValue
YCode::evaluate (bool cse)
{
    y2debug ("evaluate(%s) = nil", toString().c_str());
    if (isError())
    {
	return YCPError ("*** Error", YCPNull());
    }
    return YCPNull();
}

// ------------------------------------------------------------------
// constant (-> YCPValue)

YConst::YConst (ycode code, YCPValue value)
    : YCode (code)
    , m_value (value)
{
}


YConst::YConst (ycode code, std::istream & str)
    : YCode (code)
    , m_value (YCPNull())
{
    if (Bytecode::readBool (str))		// not nil
    {
	y2debug ("YConst::YConst (%d:%s)", (int)code, YCode::toString (code).c_str());
	switch (code)
	{
	    case YCode::ycVoid:
	    {
		m_value = YCPVoid (str);
	    }
	    break;
	    case YCode::ycBoolean:
	    {
		m_value = YCPBoolean (str);
	    }
	    break;
	    case YCode::ycInteger:
	    {
		m_value = YCPInteger (str);
	    }
	    break;
	    case YCode::ycFloat:
	    {
		m_value = YCPFloat (str);
	    }
	    break;
	    case YCode::ycString:
	    {
		m_value = YCPString (str);
	    }
	    break;
	    case YCode::ycByteblock:
	    {
		m_value = YCPByteblock (str);
	    }
	    break;
	    case YCode::ycPath:
	    {
		m_value = YCPPath (str);
	    }
	    break;
	    case YCode::ycSymbol:
	    {
		m_value = YCPSymbol (str);
	    }
	    break;
	    case YCode::ycList:
	    {
		m_value = YCPList (str);
	    }
	    break;
	    case YCode::ycMap:
	    {
		m_value = YCPMap (str);
	    }
	    break;
	    case YCode::ycEntry:
	    {
		m_value = YCPEntry ((SymbolEntry*)0);		// FIXME
	    }
	    break;
	    default:
	    {
		y2error ("YConst stream code %d", code);
		break;
	    }
	}
	if (!m_value.isNull())
	{
	    y2debug ("m_value '%s'", m_value->toString().c_str());
	}
    }
    else
    {
	y2warning ("YConst::YConst(%d:%s) NIL", (int)code, YCode::toString(code).c_str());
    }
}


YCPValue
YConst::value() const
{
    return m_value;
}


string
YConst::toString() const
{
    if (m_value.isNull())
    {
	switch (m_code)
	{
	    case ycVoid:
		return "nil"; break;
	    case ycBoolean:
		return "nilboolean"; break;
	    case ycInteger:
		return "nilinteger"; break;
	    case ycFloat:
		return "nilfloat"; break;
	    case ycString:
		return "nilstring"; break;
	    case ycByteblock:
		return "nilbyteblock"; break;
	    case ycPath:
		return "nilpath"; break;
	    case ycSymbol:
		return "nilsymbol"; break;
	    case ycList:
		return "nillist"; break;
	    case ycMap:
		return "nilmap"; break;
	    case yeTerm:
		return "nilterm"; break;
	    case yeBlock:
		return "nilblock"; break;
	    case ycLocale:
	    case yeLocale:
		return "nillocale"; break;
	    default:
		return "nilWHAT?"; break;
	}
    }
    return m_value->toString();
}


YCPValue
YConst::evaluate (bool cse)
{
    y2debug("evaluate(%s) = %s", toString().c_str(), m_value.isNull()?"NULL":m_value->toString().c_str());
    return m_value;
}


std::ostream &
YConst::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    if (m_value.isNull())
	return Bytecode::writeBool (str, false);
    Bytecode::writeBool (str, true);
    return m_value->toStream (str);
}


// ------------------------------------------------------------------
// locale

YLocale::YLocale (const char *locale, const char *textdomain)
    : YCode (ycLocale)
    , m_locale (locale)
    , m_domain (textdomain)
{
}


YLocale::YLocale (std::istream & str)
    : YCode (ycLocale)
{
    m_locale = Bytecode::readCharp (str);
    m_domain = Bytecode::readCharp (str);
}


YLocale::~YLocale ()
{
    free ((void *)m_locale);
}


const char *
YLocale::value () const
{
    return m_locale;
}


const char *
YLocale::domain () const
{
    return m_domain;
}


std::ostream &
YLocale::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeCharp (str, m_locale);
    return Bytecode::writeCharp (str, m_domain);
}

string
YLocale::toString() const
{
    return "_(\"" + string (m_locale) + "\")";
}


YCPValue
YLocale::evaluate (bool cse)
{
    if (cse) return YCPNull();

    const char *ret = dgettext (m_domain, m_locale);
    y2debug ("localize <%s> to <%s>", m_locale, ret);
    return YCPString (ret);
}


// ------------------------------------------------------------------
// declaration (-> declaration_t)

YDeclaration::YDeclaration (ycode code, declaration_t *value)
    : YCode (code)
    , m_value (value)
{
}


declaration_t *
YDeclaration::value() const
{
    return m_value;
}


string
YDeclaration::toString() const
{
    return Decl2String (m_value);
}


YCPValue
YDeclaration::evaluate (bool cse)
{
    y2debug("evaluate(declaration %s) = nil", toString().c_str());
    return YCPNull();
}


std::ostream &
YDeclaration::toStream (std::ostream & str) const
{
    y2warning ("oops?!");
    return str;
}

// ------------------------------------------------------------------
// function definition

YFunction::YFunction (YBlock *parameterblock, YBlock *body)
    : YCode (ycFunction)
    , m_parameterblock (parameterblock)
    , m_body (body)
{
}


YFunction::~YFunction ()
{
    if (m_parameterblock) delete m_parameterblock;
    if (m_body) delete m_body;
}


YBlock *
YFunction::body() const
{
    return m_body;
}


void
YFunction::setBody (YBlock *body)
{
    m_body = body;
    body->setKind (YBlock::b_body);
    return;
}


unsigned int
YFunction::parameterCount (void) const
{
    return (m_parameterblock ? m_parameterblock->symbolCount() : 0);
}


SymbolEntry *
YFunction::parameter (unsigned int position) const
{
    return (m_parameterblock ? m_parameterblock->symbolEntry (position) : 0);
}


string
YFunction::toString() const
{
    string s = " (";

    for (unsigned int p = 0; p < parameterCount(); p++)
    {
	if (p > 0)
	{
	    s += ", ";
	}
	s += parameter(p)->toString();
    }

    s += ")\n";

    if (m_body != 0)
    {
	s += m_body->toString();
    }

    return s;
}


YCPValue
YFunction::evaluate (bool cse)
{
    y2debug ("YFunction::evaluate(%s)\n", toString().c_str());
    // there's nothing to evaluate for a function _definition_
    // its all in the function call.
    return YCPNull();
}


YFunction::YFunction (std::istream & str)
    : YCode (ycFunction)
    , m_parameterblock (0)
    , m_body (0)
{
    if (Bytecode::readBool (str))
    {
	y2debug ("YFunction::YFunction: need_parameterblock !");
	m_parameterblock = (YBlock *)Bytecode::readCode (str);
	if ((m_parameterblock == 0)
	    || (!m_parameterblock->isBlock()))
	{
	    y2error ("Error reading parameterblock");
	}
    }
    else
    {
	y2debug ("YFunction::YFunction: no parameterblock !");
    }
    if (Bytecode::readBool (str))
    {
	y2debug ("YFunction::YFunction: have body!");
	m_body = (YBlock *)Bytecode::readCode (str);
	if ((m_body == 0)
	    || (!m_body->isBlock()))
	{
	    y2error ("Error reading body");
	}
    }
    else
    {
	y2debug ("YFunction::YFunction: no body!");
    }
}


std::ostream &
YFunction::toStream (std::ostream & str) const
{
    YCode::toStream (str);

    bool need_parameterblock = ((m_parameterblock != 0) && (m_parameterblock->symbolCount() > 0));
    Bytecode::writeBool (str, need_parameterblock);
    y2debug ("YFunction::toStream, need_parameterblock %d", need_parameterblock);
    if (need_parameterblock)
    {
	m_parameterblock->toStream (str);
    }

    bool need_body = (m_body != 0);
    Bytecode::writeBool (str, need_body);
    y2debug ("YFunction::toStream, need_body %d", need_body);
    if (need_body)
    {
	m_body->toStream (str);
    }

    return str;
}


// ------------------------------------------------------------------
//
// error
//

YError::YError (int line, const char *msg)
    : YCode (yxError)
    , m_line (line)
    , m_msg (msg)
{
    y2debug ("YError::YError %p: m_line %d, msg %s", this, m_line, m_msg);
}


YCPValue
YError::evaluate (bool cse)
{
    y2debug ("YError::evaluate %p: m_line %d, msg %s", this, m_line, m_msg);
    if (m_line > 0)
    {
	extern ExecutionEnvironment ee;
	ee.setLinenumber (m_line);
    }
    return YCPError (toString());
}


string
YError::toString()
{
    return (m_msg ? m_msg : "*** Error");
}


std::ostream &
YError::toStream (std::ostream & str) const
{
    y2warning ("oops?!");
    return str;
}

// EOF
