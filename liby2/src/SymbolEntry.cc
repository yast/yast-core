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

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

#include "ycp/y2log.h"
#include "y2/SymbolEntry.h"
#include "ycp/SymbolTable.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPCode.h"

IMPL_BASE_POINTER(SymbolEntry);

UstringHash* SymbolEntry::_nameHash = NULL;
Ustring SymbolEntry::emptyUstring = Ustring ( *( SymbolEntry::_nameHash ? SymbolEntry::_nameHash : (SymbolEntry::_nameHash = new UstringHash)), ""); 

#ifdef D_MEMUSAGE
void __UUsage ()
{
//    fprintf (stderr, "%d Ustrings using %ld bytes\n", SymbolEntry::_nameHash.size(), SymbolEntry::_nameHash.sum());
}
#endif

/**
 * constructor
 */

SymbolEntry::SymbolEntry (const Y2Namespace* name_space, unsigned int position, const char *name, category_t cat, constTypePtr type)
    : m_global ((cat == c_global)||(cat == c_filename))
    , m_namespace (name_space)
    , m_position (position)
    , m_name ( Ustring ( *( _nameHash ? _nameHash : (_nameHash = new UstringHash)), name) )
    , m_category ((cat == c_filename) ? cat : (m_global ? c_unspec : cat))
    , m_type (type)
    , m_value (YCPNull())
    , m_recurse_stack (NULL)
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
    if (! m_recurse_stack)
    {
	m_recurse_stack = new valuestack_t;
    }
    m_recurse_stack->push (m_value);
}

void
SymbolEntry::pop ()
{
    if (! m_recurse_stack)
	return;

    m_value = m_recurse_stack->top ();
    m_recurse_stack->pop ();
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
	    return s + catString() + " \"" + m_name.asString() + "\"";
	}
	break;
	case c_variable:
	case c_reference:
	case c_function:
#if DO_DEBUG
	    y2debug ("m_namespace %p[%s], m_global %d, with_type %d", m_namespace, m_namespace ? m_namespace->name().c_str() : "", m_global, with_type);
#endif
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




