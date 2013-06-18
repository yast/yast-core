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

   File:	YSymbolEntry.cc
		symbol entry class for YCP

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-
#include <stdio.h>

#include <string>
using std::string;

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "ycp/y2log.h"
#include "y2/SymbolEntry.h"
#include "ycp/SymbolTable.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/Bytecode.h"
#include "ycp/Xmlcode.h"

IMPL_DERIVED_POINTER(YSymbolEntry, SymbolEntry);


YSymbolEntry::YSymbolEntry (const Y2Namespace* name_space, unsigned int position, const char *name, category_t cat, constTypePtr type, YCodePtr code)
    : SymbolEntry ( name_space, position, name, cat, type)
    , m_code (code)
{
}


// builtin
YSymbolEntry::YSymbolEntry (const char *name, constTypePtr type, declaration_t *decl, const Y2Namespace* name_space)
    : SymbolEntry ( name_space, 0, name, c_builtin, type)
    , m_code (0)
{
    m_global = true;
    m_payload.m_decl = decl;
    m_value = YCPNull();
}


// namespace
YSymbolEntry::YSymbolEntry (const char *name, constTypePtr type, SymbolTable *table)
    : SymbolEntry ( 0, 0, name, c_namespace, type)
    , m_code (0)
{
    m_global = true;
    m_payload.m_table = table;
    m_value = YCPNull();
}


// filename
YSymbolEntry::YSymbolEntry (const char *filename)
    : SymbolEntry ( 0, 0, filename, c_filename, Type::Unspec)
    , m_code (0)
{
    m_global = true;
    m_value = YCPNull();
}

// bytecode constructor

YSymbolEntry::YSymbolEntry (bytecodeistream & str, const Y2Namespace *name_space)
    : SymbolEntry ( 0, 0, "", c_unspec, Type::Unspec)
    , m_code (0)
{
    m_global = Bytecode::readBool (str);
    m_namespace = name_space;
    m_position = Bytecode::readInt32 (str);
    m_name = Ustring (*_nameHash, "");		// no default constructor for Ustring available
    m_value = YCPNull();			// value stays NULL to enforce re-initialization from payload

    // read name, put to Ustring, delete name
    //  thats the only way to preserve the argument to Ustring
    //  without breaking existing bytecode

    char *mname = Bytecode::readCharp (str);
    m_name = Ustring (*_nameHash, mname);
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


void
YSymbolEntry::setCode (YCodePtr code)
{
    if (m_category == c_builtin || m_category == c_module)
    {
	y2error ("setDeclaration: Wrong category (%s)", toString().c_str());
	return;
    }
    m_code = code;
}


YCodePtr
YSymbolEntry::code () const
{
    if (m_category == c_builtin || m_category == c_module)
    {
	return 0;
    }
    return m_code;
}


bool
YSymbolEntry::onlyDeclared () const
{
    return (m_category == c_function)			// only functions may be 'only declared'
	   && (((YFunctionPtr)m_code)->definition() == 0);
}


string 
YSymbolEntry::toString (bool with_type) const
{
    string s = (with_type && m_global) ? "global " : "";
#if DO_DEBUG
    y2debug ("YSymbolEntry::toString %p: name '%s'", this, m_name.asString().c_str());
    y2debug ("YSymbolEntry::toString %p: with_type %d, cat %s, name '%s'", this, with_type, catString().c_str(), m_name.asString().c_str());
#endif
    switch (m_category)
    {
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
	    return SymbolEntry::toString (with_type);
	}
	break;
    }

    return "?SymbolEntry?";
}


std::ostream &
YSymbolEntry::toStream (std::ostream & str) const
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


std::ostream &
YSymbolEntry::toXml (std::ostream & str, int indent ) const
{
    str << Xmlcode::spaces( indent ) << "<symbol";
    if (m_global) str << " global=\"1\"";
    str << " category=\"" << catString(); str << "\"";
    str << " type=\""; str << m_type->toXmlString(); str << "\"";
    str << " name=\""; str << m_name.asString(); str << "\"";
    string ns = nameSpace()->name();
    if (!ns.empty())
      str << " ns=\"" << ns << "\"";
    str << "/>";
    return str;
    // value is never written
}


void
YSymbolEntry::setDeclaration (declaration_t *decl)
{
    if (m_category != c_builtin)
    {
	y2error ("setDeclaration: Wrong category (%s)", toString().c_str());
	return;
    }
    m_payload.m_decl = decl;
}


declaration_t *
YSymbolEntry::declaration () const
{
    if (m_category != c_builtin)
    {
	return 0;
    }
    return m_payload.m_decl;
}


Y2Namespace *
YSymbolEntry::payloadNamespace () const
{
    if (m_category != c_module)
    {
	return 0;
    }
    return m_payload.m_namespace;
}


void
YSymbolEntry::setPayloadNamespace (Y2Namespace *name_space)
{
    if (m_category != c_module)
    {
	y2error ("setPayloadNamespace: Wrong category (%s)", toString().c_str());
	return;
    }
    m_payload.m_namespace = name_space;
}

void
YSymbolEntry::setTable (SymbolTable *table)
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
YSymbolEntry::table() const
{
    if (m_category != c_namespace)
    {
	return 0;
    }
    return m_payload.m_table;
}


