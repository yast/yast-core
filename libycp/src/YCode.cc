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

/-*/
// -*- c++ -*-

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include <libintl.h>

#include "ycp/YCode.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPFloat.h"
#include "ycp/YCPByteblock.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPList.h"
#include "ycp/YCPMap.h"
#include "ycp/YCPTerm.h"
#include "ycp/YCPCode.h"

#include "ycp/YBlock.h"
#include "ycp/YBreakpoint.h"

#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

#include "ycp/y2log.h"
#include "ycp/ExecutionEnvironment.h"

// ------------------------------------------------------------------

IMPL_BASE_POINTER(YCode);
IMPL_DERIVED_POINTER(YConst, YCode);
IMPL_DERIVED_POINTER(YLocale, YCode);
IMPL_DERIVED_POINTER(YFunction, YCode);

// ------------------------------------------------------------------
// YCode

YCode::YCode ()
{
  comment_before = comment_after = NULL;
}


YCode::~YCode ()
{
  if (comment_after != NULL)
    delete[] comment_after;
  if (comment_before != NULL)
    delete[] comment_before;
}


bool
YCode::isConstant() const
{
    return false;
}


bool
YCode::isError() const
{
    return false; 		// TODO unused in practice?
}


bool
YCode::isStatement() const
{
    return false;
}


bool
YCode::isBlock () const
{
    return false;
}


bool
YCode::isReferenceable () const
{
    return false;
}


string
YCode::toString (ykind kind)
{
    static const char *names[] = {
	"yxError",
	// [1] Constants	(-> YCPValue, except(!) locale -> yeLocale)
	"ycVoid", "ycBoolean", "ycInteger", "ycFloat",	// constants
	"ycString", "ycByteblock", "ycPath", "ycSymbol",
	"ycList",					// list
	"ycMap",					// map
	"ycTerm",					// term
	"ycEntry",

	"ycConstant",		// -- placeholder --
	"ycLocale",		// locale constant (gettext)
	"ycFunction",		// function definition (declaration and definition)

	// [16] Expressions	(-> declaration_t + values)
	"yePropagate",		// type propagation (value, type)
	"yeUnary",		// unary (prefix) operator
	"yeBinary",		// binary (infix) operator
	"yeTriple",		// <exp> ? <exp> : <exp>
	"yeCompare",		// compare

	// [21] Value expressions (-> values + internal)
	"yeLocale",		// locale expression (ngettext)
	"yeList",		// list expression
	"yeMap",		// map expression
	"yeTerm",		// <name> ( ...)
	"yeIs",			// is()
	"yeBracket",		// <name> [ <expr>, ... ] : <expr>

	// [27] Block (-> linked list of statements)
	"yeBlock",		// block expression
	"yeReturn",		// quoted expression

	// [29] Symbolref (-> SymbolEntry)
	"yeVariable",		// variable ref
	"yeBuiltin",		// builtin ref + args
	"yeFunction",		// function ref + args
	"yeReference",		// reference
        // SuSE Linux v9.2
        "yeFunctionPointer",      // function pointer

	"yeExpression",		// -- placeholder --

	// [35] Statements	(-> YCode + next)
	"ysTypedef",		// typedef
	"ysVariable",		// variable definition (-> YSAssign)
	"ysFunction",		// function definition
	"ysAssign",		// variable assignment or definition
	"ysBracket",		// <name> [ <expr>, ... ] = <expr>
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
	"ysImport",
	"ysBlock",		// a block
// FIXME ysSwitch is missing
	"ysStatement"		// -- placeholder --
    };

    if ((int)kind < 0 || (unsigned int)kind >= (sizeof (names) / sizeof (*names)))
    {
	y2error ("Bad ykind %d", kind);
	return "*** BAD YCode";
    }
    char buf[16]; sprintf (buf, "[%d]", kind);
    return string (buf) + names[kind];
}


string
YCode::toString() const
{
    return toString (kind ());
}


// write to stream, see Bytecode for read
std::ostream &
YCode::toStream (std::ostream & str) const
{
    ykind k = kind ();
#if DO_DEBUG
    y2debug ("YCode::toStream (%d:%s)", (int)k, YCode::toString (k).c_str());
#endif
    return str.put ((char)k);
}


std::string
YCode::commentToXml () const
{
  string x;
  if (comment_before != NULL)
    x += " comment_before=\"" + Xmlcode::xmlify(comment_before) + '"';
  if (comment_after != NULL)
    x += " comment_after=\""  + Xmlcode::xmlify(comment_after)  + '"';

  return x;
}

std::ostream &
YCode::commentToXml (std::ostream & str ) const
{
  return str << commentToXml();
}

std::ostream &
YCode::toXml (std::ostream & str, int /*indent*/ ) const
{
    ykind k = kind ();
#if DO_DEBUG
    y2debug ("YCode::toStream (%d:%s)", (int)k, YCode::toString (k).c_str());
#endif
    return str << "<ycode code=" << k << "/>";
}


YCPValue
YCode::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug ("evaluate(%s) = nil", toString().c_str());
#endif
    if (isError())
    {
	y2error ("evaluating error code");
    }
    return YCPNull();
}


constTypePtr
YCode::type () const
{
    return Type::Unspec;
}

void
YCode::setCommentBefore (const char* comment)
{
  if (strlen(comment) > 0 )
  {
    if (comment_before && strlen(comment_before) > 0) // there is already something
    {
      char * newstr = new char[strlen(comment_before) + strlen(comment) + 1];
      strcpy(newstr, comment);
      strcat(newstr, comment_before);
      delete[] comment_before;
      delete[] comment;
      comment_before = newstr;
    }
    else
      comment_before = comment;
  }
  y2debug("set comment '%s' before for code %s", comment, toString().c_str());
}

void
YCode::setCommentAfter (const char* comment)
{
  if (strlen(comment) > 0 )
    comment_after = comment;
  y2debug("set comment '%s' after for code %s", comment, toString().c_str());
}

// ------------------------------------------------------------------
// constant (-> YCPValue)

YConst::YConst (ykind kind, YCPValue value)
    : m_kind (kind)
    , m_value (value)
{
}


YConst::YConst (ykind kind, bytecodeistream & str)
    : m_kind (kind)
    , m_value (YCPNull())
{
    if (Bytecode::readBool (str))		// not nil
    {
#if DO_DEBUG
	y2debug ("YConst::YConst (%d:%s)", (int)kind, YCode::toString (kind).c_str());
#endif
	switch (kind)
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
	    case YCode::ycTerm:
	    {
		m_value = YCPTerm (str);
	    }
	    break;
	    case YCode::ycEntry:
	    {
#if DO_DEBUG
		y2debug ("YCode::ycEntry:");
#endif
		m_value = YCPEntry (Bytecode::readEntry (str));
	    }
	    break;
	    default:
	    {
		y2error ("YConst stream kind %d", kind);
		break;
	    }
	}
	if (!m_value.isNull())
	{
#if DO_DEBUG
	    y2debug ("m_value '%s'", m_value->toString().c_str());
#endif
	}
    }
    else
    {
	y2warning ("YConst::YConst(%d:%s) NIL", (int)kind, YCode::toString(kind).c_str());
    }
}


YCode::ykind
YConst::kind() const
{
    return m_kind;
}

YCPValue
YConst::value() const
{
    return m_value;
}


string
YConst::toString() const
{
    if (!m_value.isNull())
	return m_value->toString();

    switch (m_kind)
    {
	case ycVoid:
	case ycBoolean:
	case ycInteger:
	case ycFloat:
	case ycString:
	case ycByteblock:
	case ycPath:
	case ycSymbol:
	case ycList:
	case ycMap:
	case ycTerm:
	case yeBlock:
	case ycLocale:
	case yeLocale:
	    return "nil";
	    break;
	default:
	    break;
    }
    return "nilWHAT?";
}

YCPValue
YConst::evaluate (bool /*cse*/)
{
    YCPValue v = m_value;
#if DO_DEBUG
    y2debug("evaluate(%s) = %s", toString().c_str(), v.isNull() ? "NULL" : v->toString().c_str());
#endif
    return v;
}


std::ostream &
YConst::toStream (std::ostream & str) const
{
    if (m_kind == ycConstant)
    {
	y2error ("Internal error, a constant not supposed to be written to bytecode");
    }
    YCode::toStream (str);
    if (m_value.isNull())
    {
	return Bytecode::writeBool (str, false);
    }
    Bytecode::writeBool (str, true);
    return m_value->toStream (str);
}

std::ostream &
YConst::toXml (std::ostream & str, int /*indent*/ ) const
{
    if (m_kind == ycConstant) {
	y2error ("Internal error, a constant not supposed to be written to xml");
    }
    if (m_value.isNull())
	return str << "<null/>";
    str << "<yconst" << commentToXml() << '>';
    m_value->toXml (str, 0 );
    str << "</yconst>";
    return str;
}


constTypePtr
YConst::type () const
{
    switch (m_kind)
    {
	case ycVoid:		return Type::ConstVoid; break;
	case ycBoolean:		return Type::ConstBoolean; break;
	case ycInteger:		return Type::ConstInteger; break;
	case ycFloat:		return Type::ConstFloat; break;
	case ycString:		return Type::ConstString; break;
	case ycByteblock:	return Type::ConstByteblock; break;
	case ycPath:		return Type::ConstPath; break;
	case ycSymbol:		return Type::ConstSymbol; break;
	case ycList:		return Type::ListUnspec; break;
	case ycMap:		return Type::MapUnspec; break;
	case ycTerm:		return Type::ConstTerm; break;
	case yeBlock:		return Type::ConstAny; break;
	case ycLocale:		return Type::ConstLocale; break;
	default:
	    break;
    }
    return Type::Unspec;
}


// ------------------------------------------------------------------
// locale

YLocale::t_uniquedomains YLocale::domains;

YLocale::YLocale (const char *locale, const char *textdomain)
    : YCode ()
    , m_locale (locale)
{
    if (domains.find (textdomain) == domains.end ())
    {
	domains.insert (std::make_pair(textdomain,false));
    }

    m_domain = domains.find (textdomain);
}


YLocale::YLocale (bytecodeistream & str)
    : YCode ()
{
    m_locale = Bytecode::readCharp (str);		// the string to be translated

    const char * dom = Bytecode::readCharp (str);

    if (domains.find (dom) != domains.end ())
    {
	m_domain = domains.find (dom);
	// the textdomain was already there, we can free the memory allocated in readCharp
	delete[] dom;
    }
    else
    {
	domains.insert (std::make_pair(dom,false));
	m_domain = domains.find (dom);
    }    
}


YLocale::~YLocale ()
{
    delete[] m_locale;
}


const char *
YLocale::value () const
{
    return m_locale;
}


const char *
YLocale::domain () const
{
    return m_domain->first;
}


std::ostream &
YLocale::toStream (std::ostream & str) const
{
    YCode::toStream (str);
    Bytecode::writeCharp (str, m_locale);
    return Bytecode::writeCharp (str, m_domain->first);
}

// see also YELocale::toXml
std::ostream &
YLocale::toXml (std::ostream & str, int /*indent*/ ) const
{
    str << "<locale domain=\"" << m_domain->first << "\" text=\"" << Xmlcode::xmlify( m_locale ) << "\"/>";
    return str;
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

    const char *ret = dgettext (m_domain->first, m_locale);
#if DO_DEBUG
    y2debug ("localize <%s> to <%s>", m_locale, ret);
#endif
    return YCPString (ret);
}

// FIXME: why we use pair string:bool in domains if we used only true value?

YLocale::t_uniquedomains::const_iterator
YLocale::setDomainStatus (const string& domain, bool status)
{
    if (domains.find (domain.c_str ()) == domains.end ())
    {
	domains.insert (std::make_pair(strdup (domain.c_str ()),status));
    }
    else
    {
	domains [domain.c_str ()] = status;
    }
    return domains.find (domain.c_str ());
}

bool
YLocale::findDomain(const string& domain)
{
    return (domains.find (domain.c_str ()) == domains.end ()) ? false : true;
}


void 
YLocale::ensureBindDomain (const string& domain)
{
    if (domains.find (domain.c_str ()) == domains.end () 
	|| ! domains[domain.c_str ()])
    {
	bindDomainDir (domain, LOCALEDIR);
    }
}


void 
YLocale::bindDomainDir (const string& domain, const string& domain_path)
{

#if DO_DEBUG
    y2debug ("going to bind a domain %s with path %s", domain.c_str(),  domain_path.c_str());
#endif
    bindtextdomain (domain.c_str (), domain_path.c_str());
    bind_textdomain_codeset (domain.c_str (), "UTF-8");
    setDomainStatus (domain, true);
 
}

// ------------------------------------------------------------------
// function definition

YFunction::YFunction (YBlockPtr declaration, const SymbolEntryPtr entry)
    : YCode ()
    , m_declaration (declaration)
    , m_definition (0)
    , m_is_global (entry ? entry->isGlobal() : true)
{
}


YFunction::~YFunction ()
{
}


YCodePtr
YFunction::definition() const
{
    return m_definition;
}


YBlockPtr
YFunction::declaration() const
{
    return m_declaration;
}


void
YFunction::setDefinition (YBlockPtr definition)
{
    m_definition = definition;
    definition->setKind (YBlock::b_definition);
    return;
}


void
YFunction::setDefinition (YBreakpointPtr definition)
{
    m_definition = definition;
    // skip setKind call, we are just setting a breakpoint wrapper here
    return;
}


void
YFunction::setDefinition (bytecodeistream & str)
{
    if (Bytecode::readBool (str))
    {
#if DO_DEBUG
	y2debug ("YFunction::YFunction: have definition!");
#endif

	if (m_declaration != 0)
	{
	    Bytecode::pushNamespace (m_declaration->nameSpace());
	}
	
	YBlockPtr def = (YBlockPtr)Bytecode::readCode (str);
	def->setKind (YBlock::b_definition);

	m_definition = def;

	if (m_declaration != 0)
	{
	    Bytecode::popNamespace (m_declaration->nameSpace());
	}

	if ((m_definition == 0)
	    || (!m_definition->isBlock()))
	{
	    y2error ("Error reading definition");
	}
    }
    else
    {
#if DO_DEBUG
	y2debug ("YFunction::setDefinition(stream): no definition!");
#endif
    }

    return;
}


unsigned int
YFunction::parameterCount (void) const
{
    return (m_declaration ? m_declaration->symbolCount() : 0);
}


SymbolEntryPtr
YFunction::parameter (unsigned int position) const
{
    return (m_declaration ? m_declaration->symbolEntry (position) : 0);
}


string
YFunction::toStringDeclaration () const
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

    s += ")";

    return s;
}

string
YFunction::toString() const
{
    string  s = toStringDeclaration ();
    if (m_definition != 0)
    {
	s += "\n";
	s += m_definition->toString();
    }

    return s;
}


YCPValue
YFunction::evaluate (bool /*cse*/)
{
#if DO_DEBUG
    y2debug ("YFunction::evaluate(%s)\n", toString().c_str());
#endif
    // there's nothing to evaluate for a function _definition_
    // its all in the function call.
    return YCPCode (YCodePtr (this));		// YCPCode will take care of YCodePtr
}


// read function (prototype only !) from stream
//
YFunction::YFunction (bytecodeistream & str)
    : YCode ()
    , m_declaration (0)
    , m_definition (0)
    , m_is_global (false)		// don't care about globalness any more
{
#if DO_DEBUG
    y2debug ("YFunction::YFunction (from stream)");
#endif
    if (Bytecode::readBool (str))
    {
#if DO_DEBUG
	y2debug ("YFunction::YFunction: need_declaration !");
#endif
	m_declaration = (YBlockPtr)Bytecode::readCode (str);
	if ((m_declaration == 0)
	    || (!m_declaration->isBlock()))
	{
	    y2error ("Error reading declaration");
	}
    }
    // read of definition block is done in YSFunction(str)
}


// just write the definition

std::ostream &
YFunction::toStreamDefinition (std::ostream & str) const
{
    bool need_declaration = ((m_declaration != 0) && (m_declaration->symbolCount() > 0));
#if DO_DEBUG
    y2debug ("YFunction::toStreamDefinition, need_declaration %d", need_declaration);
#endif

    // definition

    bool need_definition = (m_definition != 0);
    Bytecode::writeBool (str, need_definition);
#if DO_DEBUG
    y2debug ("YFunction::toStreamDefinition, need_definition %d", need_definition);
#endif
    if (need_definition)
    {
	if (need_declaration)
	{
	    Bytecode::pushNamespace (m_declaration->nameSpace());	// keep the declaration accessible during definition write
	}
	m_definition->toStream (str);
	if (need_declaration)
	{
	    Bytecode::popNamespace (m_declaration->nameSpace());
	}
    }

    return str;
}


std::ostream &
YFunction::toXmlDefinition (std::ostream & str, int /*indent*/ ) const
{
    // see toXml(...)
    return str;
}


// writing a function to a stream is done in two (well, in fact three) parts
// 1. the SymbolEntry (done when writing the block this function is defined in)
// 2. the declaration block
// 3. the definition block
//
// Parts 2 & 3 might be splitted if this function is defined in a module as global
// In this case, part 2 (defining the prototype) is written alongside the SymbolTable
// of the module and part 3 when the function definition statement is encountered.
// See TableEntry::toStream() and YSFunction::toStream() respectively.

std::ostream &
YFunction::toStream (std::ostream & str) const
{
    YCode::toStream (str);

    // declaration

    bool need_declaration = ((m_declaration != 0) && (m_declaration->symbolCount() > 0));
#if DO_DEBUG
    y2debug ("YFunction::toStream, need_declaration %d", need_declaration);
#endif
    Bytecode::writeBool (str, need_declaration);
    if (need_declaration)
    {
	m_declaration->toStream (str);
    }
    return str;
}


std::ostream &
YFunction::toXml (std::ostream & str, int indent ) const
{
    bool need_declaration = ((m_declaration != 0) && (m_declaration->symbolCount() > 0));
    bool need_definition = (m_definition != 0);

    if (need_declaration || need_definition) {
	if (need_declaration)
	{
	    Xmlcode::pushNamespace (m_declaration->nameSpace());	// keep the declaration accessible during definition write
	    str << Xmlcode::spaces( indent+2 ) << "<declaration>\n";
	    m_declaration->toXml (str, indent+4 );
	    str << Xmlcode::spaces( indent+2 ) << "</declaration>\n";
	}
	m_definition->toXml (str, indent+2 );
	if (need_declaration)
	{
	    Xmlcode::popNamespace (m_declaration->nameSpace());
	}
    }

    return str;
}


constTypePtr
YFunction::type () const
{
    // FIXME: return type undefined
    FunctionTypePtr f = new FunctionType (Type::Unspec);
    for (unsigned int p = 0; p < parameterCount(); p++)
    {
	f->concat (parameter(p)->type());
    }
    return f;
}

// EOF
