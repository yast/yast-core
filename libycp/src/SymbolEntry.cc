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

$Id$
/-*/
// -*- c++ -*-
#include <stdio.h>

#include <string>
using std::string;

#include "ycp/y2log.h"
#include "ycp/SymbolEntry.h"
#include "ycp/SymbolTable.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/YBlock.h"

#include "ycp/Bytecode.h"

UstringHash SymbolEntry::_nameHash;

/**
 * constructor
 */

SymbolEntry::SymbolEntry (const YBlock* block, unsigned int position, const char *name, category_t cat, const TypeCode & type, YCode *code)
    : m_global (cat == c_global)
    , m_block (block)
    , m_position (position)
    , m_name (Ustring (_nameHash, name))
    , m_category (m_global ? c_unspec : cat)
    , m_type (type)
    , m_value (YCPNull())
{
    m_payload.m_code = code;
}

// builtin
SymbolEntry::SymbolEntry (const char *name, const TypeCode & type, declaration_t *decl)
    : m_global (true)
    , m_block (0)
    , m_position (0)
    , m_name (Ustring (_nameHash, name))
    , m_category (c_builtin)
    , m_type (type)
    , m_value (YCPNull())
{
    m_payload.m_decl = decl;
}


// namespace
SymbolEntry::SymbolEntry (const char *name, const TypeCode & type, SymbolTable *table)
    : m_global (true)
    , m_block (0)
    , m_position (0)
    , m_name (Ustring (_nameHash, name))
    , m_category (c_namespace)
    , m_type (type)
    , m_value (YCPNull())
{
    m_payload.m_table = table;
}


const YBlock *
SymbolEntry::block () const
{
    return m_block;
}


bool
SymbolEntry::isGlobal () const
{
    return m_global;
}


void
SymbolEntry::setBlock (const YBlock *block)
{
    m_block = block;
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
SymbolEntry::setCode (YCode *code)
{
    m_payload.m_code = code;
}


YCode *
SymbolEntry::code () const
{
    return m_payload.m_code;
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
    y2debug ("SymbolEntry::setValue (%s = '%s')", m_name.asString().c_str(), value.isNull() ? "nil" : value->toString().c_str());
    return m_value = value;
}


YCPValue
SymbolEntry::value () const
{
    return m_value;
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


TypeCode
SymbolEntry::type () const
{
    return m_type;
}


void
SymbolEntry::setType (const TypeCode & type)
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
	default:
	break;
    }

    return "?cat?";
}


string 
SymbolEntry::toString (bool with_type) const
{
    string s = (with_type && m_global) ? "global " : "";

//y2debug ("SymbolEntry::toString %p: with_type %d, cat %s, name %s", this, with_type, catString().c_str(), m_name.asString().c_str());

    switch (m_category)
    {
	case c_unspec:
	{
	    return s + catString() + " '" + m_type.toString() + " " + m_name.asString() + "'";
	}
	break;
	case c_module:
	{
#if 1
	    return s + catString() + " \"" + m_name.asString() + "\"";
#else
	    return s + catString() + " \"" + m_name.asString() + "\"/*" + ((with_type && m_payload.m_code) ? m_payload.m_code->toString() : "") + "*/";
#endif
	}
	break;
	case c_variable:
	case c_function:
	case c_builtin:
	{
		y2debug ("m_block %p[%s], m_global %d, with_type %d", m_block, m_block?m_block->name().c_str():"",  m_global, with_type);
	    if (with_type)
	    {
		s += m_type.toString() + " ";
		if (m_global
		    && !m_block->name().empty())
		{
		    s += (m_block->name() + "::");
		}
		s += m_name.asString();
		if (m_category == c_builtin)
		{
		    s += "(" + TypeCode (m_payload.m_decl->type).toStringSequence() + ")";
		}
		return s;
	    }
	    else
	    {
		return string (m_global ? (m_block->name() + "::") : "") + m_name.asString();
	    }
	}
	break;
	case c_typedef:
	{
	    return s + catString() + " " + m_name.asString() + " " + m_type.toString() + ";";
	}
	break;
	case c_const:
	{
	    return s + m_type.toString() + " " + m_name.asString();
	}
	break;
	case c_namespace:
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


SymbolEntry::SymbolEntry (std::istream & str, const YBlock *block)
    : m_global (Bytecode::readBool (str))
    , m_block (block)
    , m_position (Bytecode::readInt32 (str))
    , m_name (Ustring (_nameHash, Bytecode::readCharp (str)))
    , m_category ((category_t)Bytecode::readInt32 (str))
    , m_type (str)
    , m_value (YCPNull())    // value stays NULL to enforce re-initialization from payload
{
    m_payload.m_code = 0; y2debug ("SymbolEntry::fromStream (%s)", toString().c_str());
    if (m_category == c_builtin)
    {
	// re-create declaration from name and type
	extern StaticDeclaration static_declarations;
	m_payload.m_decl = static_declarations.findDeclaration (m_name.asString().c_str(), m_type);
	if (m_payload.m_decl == 0)
	{
	    y2error ("Undefined builtin '%s (%s)'", m_name.asString().c_str(), m_type.toStringSequence().c_str());
	}
    }
    else
    {
	m_payload.m_code = Bytecode::readCode (str);
    }
    y2debug ("SymbolEntry::fromStream (%s) done", toString().c_str());
}


std::ostream &
SymbolEntry::toStream (std::ostream & str) const
{
    y2debug ("SymbolEntry::toStream (%s)", toString().c_str());
    Bytecode::writeBool (str, m_global);
    Bytecode::writeInt32 (str, m_position);
    Bytecode::writeCharp (str, m_name.asString().c_str());
    Bytecode::writeInt32 (str, m_category);
    m_type.toStream (str);
    if (m_category != c_builtin)
    {
	m_payload.m_code->toStream (str);
    }
    return str;
    // value is never written
}

