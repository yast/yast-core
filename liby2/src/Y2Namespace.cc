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

   File:       Y2Namespace.cc

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/
/*
 * Base class of all Y2 namespaces providing a default implementations
 *
 */


#include <ycp/y2log.h>
#include <ycp/SymbolTable.h>

#include "Y2Namespace.h"
#include "Y2NamespaceCPP.h"

#include "md5.h"

Y2Namespace::Y2Namespace ()
    : m_table ( new SymbolTable(211) )
    , m_symbolcount (0)
{}

Y2Namespace::~Y2Namespace ()
{
    delete m_table;
}

const string Y2Namespace::timestamp ()
{
    // this function uses SymbolTable::toStringSymbols() to get a string representation
    // of a global symbols. This representation must be unique
    // against reordering, type and name changes

    MD5Context md5;
    MD5Init (&md5);
    MD5Update (&md5, (unsigned char*)(table()->toStringSymbols ().c_str ()), table()->toStringSymbols ().length ());
    
    char res[17];
    res[16] = 0;
    
    MD5Final ((unsigned char*)res, &md5);
    
    return string(res);
}

unsigned int Y2Namespace::symbolCount ()
{
    return m_symbolcount;
}

string Y2Namespace::toString () const
{
    SymbolTable* t = m_table;
    string s = t->toString ();
    
    return s;
}

SymbolEntry* Y2Namespace::symbolEntry (unsigned int position)
{
    return m_symbols[position];
}


void Y2Namespace::enterSymbol( string name, SymbolEntry* symbol, int lineno )
{
    // FIXME: that strdup can hurt
    m_table->enter ( strdup(name.c_str ()), symbol, lineno);
    m_symbols[symbol->position ()] = symbol;
    m_symbolcount++;
}

SymbolTable* Y2Namespace::table ()
{
    return m_table;
}

// ************************** Y2NamespaceCPP.h helpers ***********************************

Y2CPPFunction::Y2CPPFunction(Y2Namespace* parent, string name, Y2CPPFunctionCallBase* call_impl)
    : YFunction (new YBlock (parent->name (), YBlock::b_unknown))
    , m_name (name)
    , m_parent (parent)
    , m_impl (call_impl)
{
    call_impl->registerParameters (declaration ());
    
    setDefinition (call_impl);
}

SymbolEntry* Y2CPPFunction::sentry(unsigned int position)
{
    // FIXME: strdup
    SymbolEntry* morefun = new SymbolEntry (m_parent, position, strdup(m_name.c_str()), SymbolEntry::c_global, 
            Type::fromSignature ( m_impl->m_signature ), this );
    morefun->setCategory (SymbolEntry::c_function); 
    return morefun;
}

void Y2CPPFunctionCallBase::newParameter (YBlock* decl, unsigned pos, constTypePtr type)
{
    string name;
    
    // FIXME: do it nicer
    switch (pos)
    {
	case 1: name = "param1"; break;
	case 2: name = "param2"; break;
	case 3: name = "param3"; break;
	case 4: name = "param4"; break;
    }
    // FIXME: strdup
    SymbolEntry* param = decl->newEntry ( strdup(name.c_str()), SymbolEntry::c_global, type, 0)->sentry ();
    param->setCategory (SymbolEntry::c_variable);
    
    switch (pos)
    {
	case 1: m_param1 = param; break;
	case 2: m_param2 = param; break;
	case 3: m_param3 = param; break;
	case 4: m_param4 = param; break;
    }
}
