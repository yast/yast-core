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

//------------------------------------------------------------------------
// constructor / destructor

StaticDeclaration::StaticDeclaration()
{
    declTable = new SymbolTable(211);
    y2debug ("declTable %p", declTable);
}


StaticDeclaration::~StaticDeclaration ()
{
    delete declTable;
}


//------------------------------------------------------------------------
// registration

void
StaticDeclaration::registerDeclarations (const char *filename,
					 declaration_t *declarations)
{
    if (declarations == 0)
    {
	return;
    }

    SymbolTable *table = declTable;
    const Y2Namespace *namespace_block = 0;
    declaration_t *namespace_decl = 0;

    while (declarations->name != 0)
    {
//	y2debug( "Registering %s", declarations->name );
	const char *name = declarations->name;

	if (*name == 0)		// exit on empty name
	    break;

	if ((declarations->flags & (DECL_WILD|DECL_SYMBOL)) == (DECL_WILD|DECL_SYMBOL))
	{
	    fprintf (stderr, "Declaration of %s::%s combines wildcard and symbol\n", filename, name);
	}
	else if ((declarations->flags & DECL_NAMESPACE) != 0)		// switch namespace
	{
	    y2debug ("NAMESPACE (%s)", name);
            declarations->name_space = namespace_decl;

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
		YBlock * block = new YBlock (filename, YBlock::b_namespace);
		block->setName (string (name));
		SymbolTable *namespaceTable = new SymbolTable (211);
		SymbolEntry *entry = new SymbolEntry (name, Type::Unspec, namespaceTable);
		entry->setBlock (block);
		table->enter (name, entry, 0);
		table = namespaceTable;					// enter all further decls to new namespace
		namespace_block = block;
		namespace_decl = declarations;
	    }
	}
	else
	{
	    declarations->name_space = namespace_decl;
	    string signature = declarations->signature;

	    constTypePtr type = Type::fromSignature (signature);
	    if (type == 0
		|| type->isError()
		|| type->isUnspec()
		|| type->isWildcard())
	    {
		y2error ("Invalid signature %s:'%s'\n", name, signature.c_str());
		return;
	    }

#if 0
y2debug("%s sig[%s] type[%s]", name, signature.c_str(), type->toString().c_str());
	    if (type->hasFlex()
		&& (declarations->flags & DECL_FLEX) == 0)
	    {
		y2error ("%s:'%s' without DECL_FLEX", name, declarations->signature);
		return;
	    }
#endif
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
	    if (fout) {
		fprintf (fout, "%s %s\n", declarations->name, declarations->signature);
		fclose (fout);
	    }
#endif
	    declarations->type = type;
	}
	declarations++;
    }

    return;
}


//------------------------------------------------------------------------
// debug

void
StaticDeclaration::dumpDeclarations () const
{
    return;
}


string
StaticDeclaration::Decl2String (const declaration_t *declaration, bool full)
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

    if (name == 0)
    {
	return "<NULL>";
    }

    return string (name) + ":" + declaration->type->toString();
}


//------------------------------------------------------------------------
// find declaration by name
declaration_t *
StaticDeclaration::findDeclaration (const char *name) const
{
    y2debug ("StaticDeclaration::findDeclaration '%s'", name);

    // split the name by the namespace
    char *next = strstr (name, "::");
    y2debug( "Next is %p", next );
    TableEntry *entry = 0;
    if (next == NULL) {
       entry = declTable->find (name);
       y2debug( "No namespace, found %p", entry );
    }
    else
    {
       *next = '\0';
       entry = declTable->find (name);
       y2debug( "Namespace found: %p", entry );
       *next = ':';
       if (entry != 0 && entry->sentry()->table() != 0) {
           // continue recursively;
           // skip delimiter
           next += 2;
    	    y2debug( "Recursive search for %s", next );
           entry = entry->sentry()->table()->find (next);
       }
    }

    if (entry != 0)
    {
	return entry->sentry()->declaration();
    }
    return 0;
}

//------------------------------------------------------------------------
// check if a declaration of 'name' matches 'type'
// return the matched declaration or NULL

declaration_t *
StaticDeclaration::findDeclaration (const char *name, constTypePtr type, bool partial) const
{
    y2debug ("StaticDeclaration::findDeclaration '%s':%s <%s>", name, type->toString().c_str (), partial?"partial":"full");

    declaration_t *decl = findDeclaration (name);
    if (decl == 0)
    {
	return 0;
    }

    return findDeclaration (decl, type, partial);
}


//------------------------------------------------------------------------
// find declaration by type
// return the matched declaration or NULL
// @param type signature of actual arguments

declaration_t *
StaticDeclaration::findDeclaration (declaration_t *decl, constTypePtr type, bool partial) const
{
    y2debug ("StaticDeclaration::findDeclaration (%p, %s, %s)", decl, type->toString().c_str (), partial ? "partial" : "full");
    if (decl == 0)
    {
	return 0;
    }

    // remember start values for error reporting

    declaration_t *first_decl = decl;

    // now check all (overloaded) possibilities

    while (decl)
    {
	constTypePtr dtype = decl->type;		// declaration type
	constTypePtr atype = type;			// argument type

	y2debug ("decl check: declared '%s', actual '%s'", dtype->toString().c_str(), atype->toString().c_str());

	// if only argument types are given, skip the
	// return type in type checking

	if (dtype->isFunction()
	    && atype->isFunction())
	{
	    bool error = false;
	    
	    constFunctionTypePtr fdtype = dtype;
	    constFunctionTypePtr fatype = atype;
	    int dcount = fdtype->parameterCount();		// declaration count
	    int acount = fatype->parameterCount();		// actual count
	    y2debug ("dcount %d, acout %d", dcount, acount);
	    int i;
	    
	    if (acount == 0) 
	    {
		// no arguments actual and declared
		if (dcount==0) break;
		
		// no actual arguments allowed only if declared arguments are wildcard
		error = ! (fdtype->parameterType (0)->isWildcard ());
	    } 
	    else
	    { 
		// need to check all arguments one by one
		constTypePtr dt = 0;
		for (i = 0; i < acount; i++)
		{
		    if (i >= dcount)					// no more parameters in current declaration
		    {
			y2debug ("too few parameters");
			error = true;
			break;
		    }
		    dt = fdtype->parameterType (i);
		    if (fatype->parameterType(i)->match (dt) != 0)		// parameters do not match
		    {
			y2debug ("parameters at %d do not match", i);
			error = true;
			break;
		    }
		    if (dt->isWildcard())					// stop parameter checking on wildcard
		    {
			break;
		    }
		}
		y2debug ("loop exit %d", i);
	    
		// this was not the last argument in declared arguments
		// and the next one is not wildcard (can be omited completely)
		// it's an error
		if (!partial && i != dcount && !dt->isWildcard () && !fdtype->parameterType(i)->isWildcard ())
		{
		    y2debug ("missing parameters");
		    error = true;
		}
	    }

	    if (!error)		// full match
		break;
	}

	decl = decl->next;
    }

    if (decl == 0)
    {
	y2debug ("findDecl failed");
	if (!partial)
	{
//	    ycp2error ("No match for '%s : %s':", first_decl->name, type->toString().c_str());
	    fprintf (stderr, "No match for '%s : %s':\n", first_decl->name, type->toString().c_str());
	    while (first_decl)
	    {
//		ycp2error (Decl2String (first_decl,true).c_str());
		fprintf (stderr, "%s\n", Decl2String (first_decl,true).c_str());
		first_decl = first_decl->next;
	    }
	}
    }
    else
    {
	y2debug ("match for decl '%s'", Decl2String (decl, true).c_str());
#ifdef BUILTIN_STATISTICS
	FILE *fout = fopen ("/tmp/builtin-lookup.txt", "a");
	if (fout) {
	    fprintf (fout, "%s %s\n", decl->name, decl->type->toString().c_str());
	    fclose (fout);
	}
#endif
    }

    return decl;
}


//---------------------------------------------------------
// bytecode I/O

// write declaration to stream (name and type)
std::ostream &
StaticDeclaration::writeDeclaration (std::ostream & str, const declaration_t *decl) const
{
    // build the full name
    string n = decl->name;

    const declaration_t *d = decl;
    while( d->name_space != 0 ) {
        d = d->name_space;
	n = std::string(d->name) + "::" + n;
    }
 
    Bytecode::writeCharp (str, n.c_str());
    decl->type->toStream (str);
    return str;
}


// read declaration from stream (return declaration matching name and type _exactly_)
declaration_t *
StaticDeclaration::readDeclaration (std::istream & str) const
{
    char *name = Bytecode::readCharp (str);
    constTypePtr type = Bytecode::readType (str);

    declaration_t *decl = findDeclaration (name, type);
    if (decl == 0)
    {
//	ycp2error ("No match for '%s (%s)'", name, type->toString().c_str());
	fprintf (stderr, "No match for '%s (%s)'\n", name, type->toString().c_str());
	str.setstate (std::ostream::failbit);
    }
    return decl;
}


// EOF
