/*----------------------------------------------------------------------\
|									|
|		      __   __    ____ _____ ____			|
|		      \ \ / /_ _/ ___|_   _|___ \			|
|		       \ V / _` \___ \ | |   __) |			|
|			| | (_| |___) || |  / __/			|
|			|_|\__,_|____/ |_| |_____|			|
|									|
|				core system				|
|							  (C) SuSE GmbH |
\-----------------------------------------------------------------------/

   File:	SymbolEntry.cc
		symbol entry class

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-
#include <stdio.h>

#include <string>
using std::string;

#define DO_DEBUG 0

#include "ycp/y2log.h"
#include "ycp/SymbolEntry.h"
#include "ycp/SymbolEntryPtr.h"
#include "ycp/SymbolTable.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/YBlock.h"
#include "ycp/YCPCode.h"
#include "ycp/YCPVoid.h"
#include "ycp/Bytecode.h"

IMPL_BASE_POINTER(SymbolEntry);

UstringHash SymbolEntry::_nameHash;

/**
 * constructor
 */

SymbolEntry::SymbolEntry (const Y2Namespace* name_space, unsigned int position, const char *name, category_t cat, constTypePtr type, YCodePtr code)
    : m_global ((cat == c_global)||(cat == c_filename))
    , m_namespace (name_space)
    , m_position (position)
    , m_name (Ustring (_nameHash, name))
    , m_category ((cat == c_filename) ? cat : (m_global ? c_unspec : cat))
    , m_type (type)
    , m_code (code)
    , m_value (YCPNull())
{
}


// builtin
SymbolEntry::SymbolEntry (const char *name, constTypePtr type, declaration_t *decl, const Y2Namespace* name_space)
    : m_global (true)
    , m_namespace (name_space)
    , m_position (0)
    , m_name (Ustring (_nameHash, name))
    , m_category (c_builtin)
    , m_type (type)
    , m_code (0)
    , m_value (YCPNull())
{
    m_payload.m_decl = decl;
}


// namespace
SymbolEntry::SymbolEntry (const char *name, constTypePtr type, SymbolTable *table)
    : m_global (true)
    , m_namespace (0)
    , m_position (0)
    , m_name (Ustring (_nameHash, name))
    , m_category (c_namespace)
    , m_type (type)
    , m_code (0)
    , m_value (YCPNull())
{
    m_payload.m_table = table;
}


// filename
SymbolEntry::SymbolEntry (const char *filename)
    : m_global (true)
    , m_namespace (0)
    , m_position (0)
    , m_name (Ustring (_nameHash, filename))
    , m_category (c_filename)
    , m_type (Type::Unspec)
    , m_code (0)
    , m_value (YCPNull())
{
}

SymbolEntry::~SymbolEntry ()
{
}

const Y2Namespace *
SymbolEntry::nameSpace () const
{
    return m_namespace;
}


void
SymbolEntry::setNamespace (const Y2Namespace *name_space)
{
    m_namespace = name_space;
}


bool
SymbolEntry::isGlobal () const
{
    return m_global;
}


bool
SymbolEntry::onlyDeclared () const
{
    return (m_category == c_function)			// only functions may be 'only declared'
	   && (((YFunctionPtr)m_code)->definition() == 0);
}


unsigned int
SymbolEntry::position () const
{
    return m_position;
}


void
SymbolEntry::setPosition (unsigned int position)
{
    m_position = position;
    return;
}


void
SymbolEntry::setCode (YCodePtr code)
{
    if (m_category == c_builtin || m_category == c_module)
    {
	y2error ("setDeclaration: Wrong category (%s)", toString().c_str());
	return;
    }
    m_code = code;
}


YCodePtr
SymbolEntry::code () const
{
    if (m_category == c_builtin || m_category == c_module)
    {
	return 0;
    }
    return m_code;
}


Y2Namespace *
SymbolEntry::payloadNamespace () const
{
    if (m_category != c_module)
    {
	return 0;
    }
    return m_payload.m_namespace;
}


void
SymbolEntry::setPayloadNamespace (Y2Namespace *name_space)
{
    if (m_category != c_module)
    {
	y2error ("setPayloadNamespace: Wrong category (%s)", toString().c_str());
	return;
    }
    m_payload.m_namespace = name_space;
}


void
SymbolEntry::setDeclaration (declaration_t *decl)
{
    if (m_category != c_builtin)
    {
	y2error ("setDeclaration: Wrong category (%s)", toString().c_str());
	return;
    }
    m_payload.m_decl = decl;
}


declaration_t *
SymbolEntry::declaration () const
{
    if (m_category != c_builtin)
    {
	return 0;
    }
    return m_payload.m_decl;
}


void
SymbolEntry::setTable (SymbolTable *table)
{
    if (m_category != c_namespace)
    {
	y2error ("setTable: Wrong category (%s)", toString().c_str());
    }
    else
    {
	m_payload.m_table = table;
    }
    return;
}


SymbolTable *
SymbolEntry::table() const
{
    if (m_category != c_namespace)
    {
	return 0;
    }
    return m_payload.m_table;
}


YCPValue
SymbolEntry::setValue (YCPValue value)
{
    y2debug ("SymbolEntry::setValue (%s@%p = '%s')", m_name.asString().c_str(), this, value.isNull() ? "nil" : value->toString().c_str());
    if (!value.isNull()
	&& (m_category == c_reference))
    {
	y2debug ("C_REFERENCE");
	if (value->isReference())
	{
	    return m_value = value;
	}

	if (m_value.isNull()
	    || !m_value->isReference())
	{
	    y2error ("Setting uninitialized reference");
	    return YCPNull ();
	}
	return m_value->asReference()->entry()->setValue (value);
    }
    
    // use YCPVoid for nil to avoid problems with function references
    if (value.isNull ())
    {
	value = YCPVoid ();
    }
	
    return m_value = value;
}


YCPValue
SymbolEntry::value () const
{
    if ((m_category == c_reference)
	&& !m_value.isNull()
	&& m_value->isReference())
    {
	y2debug ("DE-REFERENCE");
	return m_value->asReference()->entry()->value();
    }
    return m_value;
}

void
SymbolEntry::push ()
{
    m_recurse_stack.push (m_value);
}

void
SymbolEntry::pop ()
{
    m_value = m_recurse_stack.top ();
    m_recurse_stack.pop ();
}

const char *
SymbolEntry::name () const
{
    return m_name.asString().c_str();
}


SymbolEntry::category_t 
SymbolEntry::category () const
{
    return m_category;
}


void
SymbolEntry::setCategory (SymbolEntry::category_t cat)
{
    m_category = cat;
    return;
}


constTypePtr
SymbolEntry::type () const
{
    return m_type;
}


void
SymbolEntry::setType (constTypePtr type)
{
    m_type = type;
    return;
}


string 
SymbolEntry::catString () const
{
    switch (m_category)
    {
	case c_unspec:
	    return "unspecified";
	break;
	case c_module:
	    return "module";
	break;
	case c_variable:
	    return "variable";
	break;
	case c_reference:
	    return "reference";
	break;
	case c_function:
	    return "function";
	break;
	case c_builtin:
	    return "builtin";
	break;
	case c_typedef:
	    return "typedef";
	break;
	case c_const:
	    return "const";
	break;
	case c_namespace:
	    return "namespace";
	break;
	case c_self:
	    return "self";
	break;
	case c_filename:
	    return "filename";
	break;
	case c_predefined:
	    return "predefined";
	break;
	default:
	break;
    }

    return "?cat?";
}


string 
SymbolEntry::toString (bool with_type) const
{
    string s = (with_type && m_global) ? "global " : "";
#if DO_DEBUG
    y2debug ("SymbolEntry::toString %p: name '%s'", this, m_name.asString().c_str());
    y2debug ("SymbolEntry::toString %p: with_type %d, cat %s, name '%s'", this, with_type, catString().c_str(), m_name.asString().c_str());
#endif
    switch (m_category)
    {
	case c_unspec:
	{
	    return s + catString() + " '" + m_type->toString() + " " + m_name.asString() + "'";
	}
	break;
	case c_module:
	{
#if 1
	    return s + catString() + " \"" + m_name.asString() + "\"";
#else
	    return s + catString() + " \"" + m_name.asString() + "\"/*" + ((with_type && m_code) ? m_code->toString() : "") + "*/";
#endif
	}
	break;
	case c_variable:
	case c_reference:
	case c_function:
#if DO_DEBUG
	    y2debug ("m_namespace %p[%s], m_global %d, with_type %d", m_namespace, m_namespace ? m_namespace->name().c_str() : "", m_global, with_type);
#endif
	case c_builtin:
	{
	    if (with_type)
	    {
		constFunctionTypePtr ftype = m_type;
		s += (((m_category == c_variable)||(m_category ==c_reference)) ? m_type->toString() : ftype->returnType()->toString()) + " ";
		if (m_global
		    && m_namespace != 0
		    && !m_namespace->name().empty())
		{
		    s += (m_namespace->name() + "::");
		}
		s += m_name.asString();
		if ((m_category == c_builtin)
		    && (m_payload.m_decl != 0))
		{
		    s += "(" + m_payload.m_decl->type->toString() + ")";
		}
		else if ((m_category == c_function)
			 && (m_code != 0))
		{
		    s += ((YFunctionPtr)m_code)->toStringDeclaration();
		}
		return s;
	    }
	    else
	    {
		string s = string ((m_global && (m_namespace!=0)) ? (m_namespace->name() + "::") : "") + m_name.asString();
		return s;
	    }
	}
	break;
	case c_typedef:
	{
	    return s + catString() + " " + m_type->toString() + " " + m_name.asString() + ";";
	}
	break;
	case c_const:
	{
	    return s + m_type->toString() + " " + m_name.asString();
	}
	break;
	case c_namespace:
	case c_self:
	case c_filename:
	case c_predefined:
	{
	    return s + catString() + " '" + m_name.asString() + "'";
	}
	break;
	default:
	{
	    y2error ("category %d ?", m_category);
	}
	break;
    }

    return "?SymbolEntry?";
}


// bytecode constructor

SymbolEntry::SymbolEntry (std::istream & str, const Y2Namespace *name_space)
    : m_global (Bytecode::readBool (str))
    , m_namespace (name_space)
    , m_position (Bytecode::readInt32 (str))
    , m_name (Ustring (_nameHash, ""))		// no default constructor for Ustring available
    , m_code (0)
    , m_value (YCPNull())			// value stays NULL to enforce re-initialization from payload
{
    // read name, put to Ustring, delete name
    //  thats the only way to preserve the argument to Ustring
    //  without breaking existing bytecode

    char *mname = Bytecode::readCharp (str);
    m_name = Ustring (_nameHash, mname);
    delete [] mname;

    m_category  = (category_t)Bytecode::readInt32 (str);
    m_type = Bytecode::readType (str);

#if DO_DEBUG
    y2debug ("SymbolEntry::fromStream (%s)", toString().c_str());
#endif
    if (m_category == c_builtin)
    {
	// re-create declaration from name and type
	extern StaticDeclaration static_declarations;
	m_payload.m_decl = static_declarations.findDeclaration (m_name.asString().c_str(), m_type);
	if (m_payload.m_decl == 0)
	{
	    y2error ("Undefined builtin '%s (%s)'", m_name.asString().c_str(), m_type->toString().c_str());
	}
    }
#if 0
    else if (m_category == c_variable)		// variable might have payload (default value)
    {
	if (Bytecode::readBool (str))
	{
	    m_code = Bytecode::readCode (str);
	}
    }
#endif
//    y2debug ("SymbolEntry::fromStream (%s) done", toString().c_str());
}


std::ostream &
SymbolEntry::toStream (std::ostream & str) const
{
#if DO_DEBUG
    y2debug ("SymbolEntry::toStream (%p:%s)", this, toString().c_str());
#endif
    Bytecode::writeBool (str, m_global);
    Bytecode::writeInt32 (str, m_position);
    Bytecode::writeCharp (str, m_name.asString().c_str());
    Bytecode::writeInt32 (str, m_category);
    m_type->toStream (str);
#if 0
    if (m_category == c_variable)
    {
	if (m_payload.m_code != 0)		// formal arguments don't have a payload (a default value)
	{
	    Bytecode::writeBool (str, true);
	    m_payload.m_code->toStream (str);
	}
	else
	{
	    Bytecode::writeBool (str, false);	// mark 'no payload'
	}
    }
#endif
    return str;
    // value is never written
}

