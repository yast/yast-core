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

   File:	StaticDeclaration.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>


   Import static declarations as 'builtins' to the global
   symbol table.

$Id$
/-*/


#include <unistd.h>
#include <stdio.h>
#include <string>
#include <map>
using namespace std;

#include "ycp/StaticDeclaration.h"
#include "ycp/SymbolTable.h"
#include "ycp/SymbolEntry.h"
#include "ycp/YBlock.h"

#include "ycp/Bytecode.h"

#include "ycp/y2log.h"

#define DECLSIZE 127

StaticDeclaration::StaticDeclaration()
{
    declTable = new SymbolTable(211);
    y2debug ("declTable %p", declTable);
}

StaticDeclaration::~StaticDeclaration ()
{
    delete declTable;
}

void
StaticDeclaration::registerDeclarations (const char *filename,
					 declaration_t *declarations)
{
    if (declarations == 0)
    {
	return;
    }

    SymbolTable *table = declTable;
    const YBlock *namespace_block = 0;

    while (declarations->name != 0)
    {
	const char *name = declarations->name;
	TypeCode type (declarations->type);

	if ((declarations->flags & (DECL_WILD|DECL_SYMBOL)) == (DECL_WILD|DECL_SYMBOL))
	{
	    fprintf (stderr, "Declaration of %s::%s combines wildcard and symbol\n", filename, name);
	}
	else if ((declarations->flags & DECL_NAMESPACE) != 0)		// switch namespace
	{
y2debug ("NAMESPACE (%s)", name);
	    TableEntry *tentry = table->find (name);
	    if (tentry != 0)						// name already exists
	    {
		namespace_block = tentry->sentry()->block();
	    }
	    else if (*name == 0)					// reset namespace
	    {
		table = declTable;
		namespace_block = 0;
	    }
	    else							// open up new namespace
	    {
		YBlock * block = new YBlock (YBlock::b_namespace);
		block->setName (string (name));
		SymbolTable *namespaceTable = new SymbolTable (211);
		SymbolEntry *entry = new SymbolEntry (name, TypeCode::Unspec, namespaceTable);
		entry->setBlock (block);
		table->enter (name, entry, 0);
		table = namespaceTable;					// enter all further decls to new namespace
		namespace_block = block;
	    }
	}
	else if (!type.isValidReturn ())
	{
	    fprintf (stderr, "Declaration of %s::%s has bad return type '%s'\n", filename, name, declarations->type);
	}
	else
	{
	    TableEntry *tentry = table->find (name);
	    if (tentry != 0)			// check for overloading
	    {
		const SymbolEntry *entry = tentry->sentry();
		declaration_t *decl = entry->declaration();
		if (decl == 0)
		{
		    y2error ("table entry has wrong category");
		    return;
		}

		while (decl->next != 0)
		{
		    decl = decl->next;
		}
		decl->next = declarations;
	    }
	    else		// first entry with that name
	    {
		SymbolEntry *entry = new SymbolEntry (name, type, declarations);
		table->enter (name, entry, 0);
		if (namespace_block)
		{
		    entry->setBlock (namespace_block);
		}
	    }

#ifdef BUILTIN_STATISTICS
	    FILE *fout = fopen ("/tmp/builtin-register.txt", "a");
	    fprintf (fout, "%s %s\n", declarations->name, declarations->type);
	    fclose (fout);
#endif
	}
	declarations++;
    }

    return;
}


void
StaticDeclaration::dumpDeclarations () const
{
    return;
}


// find declaration by name

declaration_t *
StaticDeclaration::findDeclaration (const char *name) const
{
    y2debug ("JJJJ %s", name);

    TableEntry *entry = declTable->find (name);
    if (entry != 0)
    {
	return (declaration_t *)(entry->sentry()->code());
    }
    return 0;
}


// check if a declaration of 'name' matches 'type'
// return the matched declaration or NULL

declaration_t *
StaticDeclaration::findDeclaration (const char *name, const TypeCode & type, bool partial) const
{
    y2debug ("JJJJ %s %s", name, type.asString().c_str ());

    TableEntry *entry = declTable->find (name);

    if (entry == 0)
    {
	return 0;
    }

    declaration_t *decl = entry->sentry()->declaration();

    if (decl == 0)
    {
	return 0;
    }

    return findDeclaration (decl, type, partial);
}


// find declaration by type
// return the matched declaration or NULL
// @param type signature of actual arguments
declaration_t *
StaticDeclaration::findDeclaration (declaration_t *decl, const TypeCode &type, bool partial) const
{
    y2debug ("StaticDeclaration::findDeclaration (%p, %s, %s)", decl, type.asString().c_str (), partial ? "partial" : "full");
    if (decl == 0)
    {
	return 0;
    }

    // remember start values for error reporting

    declaration_t *first_decl = decl;
    const TypeCode &first_type = type.args ();

    // now check all (overloaded) possibilities

    while (decl)
    {
	TypeCode dtype = decl->type;	// declaration type
	TypeCode atype = type;		// argument type

	y2debug ("decl check: declared '%s', actual '%s'", dtype.asString().c_str(), atype.asString().c_str());

	// if only argument types are given, skip the
	// return type in type checking

	if (atype.isArgs ())
	{
	    atype = atype.args ();
	    dtype = dtype.args ();
/* prevents matching funcs with no params. supposed to catch decls without |
	    if (dtype.isUnspec ())
	    {
		return 0;
	    }
*/
	}

	const char *atype_ptr = atype.asString().c_str();
	const char *dtype_ptr = dtype.asString().c_str();
	while (*atype_ptr != 0)
	{
	    y2debug ("param check: declared '%s', actual '%s'", dtype_ptr, atype_ptr);
	    // 
	    bool match = TypeCode::matchParameters (dtype_ptr, atype_ptr);
	    if (!match)
		break;
	} // while *atype_ptr

	// break on complete match
	if ((*atype_ptr == 0)
	    && (partial
		|| (*dtype_ptr == 0)))
	{
	    y2debug ("check yes");
	    break;
	}
	decl = decl->next;
    }

    if (decl == 0)
    {
	if (!partial)
	{
	    ycp2error ("", 0, "No match for '%s (%s)':", first_decl->name, first_type.toStringSequence().c_str());
	    while (first_decl)
	    {
		ycp2error ("", 0, Decl2String (first_decl,true).c_str());
		first_decl = first_decl->next;
	    }
	}
    }

    else
    {
	y2debug ("match for decl '%s'", Decl2String(decl, true).c_str());
#ifdef BUILTIN_STATISTICS
	FILE *fout = fopen ("/tmp/builtin-lookup.txt", "a");
	fprintf (fout, "%s %s\n", decl->name, decl->type);
	fclose (fout);
#endif
    }

    return decl;
}


// give return type
TypeCode
StaticDeclaration::returnType (const declaration_t *declaration) const
{
    if (declaration == 0)
    {
	y2error ("StaticDeclaration::returnType (NULL)");
	return "";
    }

    return TypeCode (declaration->type).returnType ();
}


//---------------------------------------------------------
// bytecode I/O

// write declaration to stream (name and type)
std::ostream &
StaticDeclaration::writeDeclaration (std::ostream & str, const declaration_t *decl) const
{
    Bytecode::writeCharp (str, decl->name);
    TypeCode (decl->type).toStream (str);
    return str;
}


// read declaration from stream (return declaration matching name and type _exactly_)
declaration_t *
StaticDeclaration::readDeclaration (std::istream & str) const
{
    char *name = Bytecode::readCharp (str);
    TypeCode type (str);

    declaration_t *decl = findDeclaration (name, type);
    if (decl == 0)
    {
	ycp2error ("", 0, "No match for '%s (%s)'", name, type.toStringSequence().c_str());
	str.setstate (std::ostream::failbit);
    }
    return decl;
}


string
Decl2String (const declaration_t *declaration, bool full)
{
    if (declaration == 0)
    {
	return "(NULL)";
    }

    const char *name = declaration->name;

    if (!full)
    {
	return string (name);
    }

    TypeCode type = declaration->type;

    if (name == 0)
    {
	return "<NULL>";
    }

    string result = type.return2string ();
#if 0
    result += " /* '";
    result += declaration->type;
    result += "' '";
    result += type;
    result += "' */";
#endif
    result += " ";
    result += name;

    type = type.args ();
    if (type.isEnd ())
    {
	return result;
    }

    result += " (" + type.toStringSequence() + ')';
    return result;
}

// EOF
