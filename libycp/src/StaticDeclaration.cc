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

/-*/


#include <unistd.h>
#include <stdio.h>
#include <string>
#include <map>
using namespace std;

#include "ycp/StaticDeclaration.h"
#include "ycp/SymbolTable.h"
#include "ycp/YSymbolEntry.h"
#include "ycp/YBlock.h"

#include "ycp/Bytecode.h"
#include "ycp/Import.h"
#include "ycp/Point.h"

#include "ycp/y2log.h"

#define DECLSIZE 127

#ifndef DO_DEBUG
#define DO_DEBUG 0
#endif

//
// list of namespace prefixes to mark as 'predefined'
// They will be auto-loaded by the scanner on first appearance
//
static char *predefined[] = {
  "UI", "WFM", "SCR", "Pkg", 0
};
//------------------------------------------------------------------------
// constructor / destructor

StaticDeclaration::StaticDeclaration()
{
    m_declTable = new SymbolTable(-1);
#if DO_DEBUG
    y2debug ("m_declTable %p", m_declTable);
#endif
    char **pptr = predefined;
    SymbolEntryPtr sentry;
    Point *point = new Point ("<predefined>");
    while (*pptr != 0)
    {
	sentry = new YSymbolEntry (0, 0, *pptr, SymbolEntry::c_predefined, Type::Unspec, 0);
	m_declTable->enter (*pptr, sentry, new Point (sentry, 0, point));
	pptr++;
    }
}


StaticDeclaration::~StaticDeclaration ()
{
    delete m_declTable;
}


//------------------------------------------------------------------------
// registration
//
// WARNING: It is not possible to register the same static data twice!
// It creates a neverending cycle via declaration_t::next.

void
StaticDeclaration::registerDeclarations (const char *filename,
					 declaration_t *declarations)
{
    if (declarations == 0)
    {
	return;
    }

    SymbolTable *table = m_declTable;
    const Y2Namespace *name_space = 0;
    static const Point *builtin_point = new Point ("<builtin>");
    const Point *namespace_point = builtin_point;		// declarations default to builtin
    declaration_t *namespace_decl = 0;

    std::pair <std::string, Y2Namespace *> *track_info = 0;

    while (declarations->name != 0)
    {
#if DO_DEBUG
	y2debug( "Registering %s", declarations->name );
#endif
	const char *name = declarations->name;

	if (*name == 0)		// exit on empty name
	    break;

	if ((declarations->flags & (DECL_WILD|DECL_SYMBOL)) == (DECL_WILD|DECL_SYMBOL))
	{
	    y2internal ("Declaration of %s::%s combines wildcard and symbol\n", filename, name);
	}
	else if ((declarations->flags & DECL_NAMESPACE) != 0)		// switch namespace
	{
	    // new namespace, clear possibly old track_info
	    if (track_info != 0)
	    {
		new Import (track_info->first, track_info->second);	// remember which predefined got activated
		m_active_predefined.push_back (*track_info);
		track_info = 0;
	    }
#if DO_DEBUG
	    y2debug ("NAMESPACE (%s)", name);
#endif
            declarations->name_space = namespace_decl;

	    TableEntry *tentry = table->find (name);
	    if (tentry != 0						// name already exists
		&& tentry->sentry()->isNamespace())			//   as namespace
	    {
		name_space = tentry->sentry()->nameSpace();
	    }
	    else if (*name == 0)					// reset namespace
	    {
		table = m_declTable;
		name_space = 0;
		namespace_point = builtin_point;
	    }
	    else							// open up new namespace
	    {
		bool is_predefined = false;

		if (tentry != 0
		    && tentry->sentry()->isPredefined())		//   replace predefined
		{
		    table->remove (tentry);
		    is_predefined = true;
		}

		// create definition container for namespace
		YBlock *block = new YBlock (filename, YBlock::b_namespace);
		block->setName (string (name));

		Y2Namespace *namespaceNamespace = (Y2Namespace *)block;
		namespaceNamespace->createTable();
		SymbolTable *namespaceTable = block->table();

		// create SymbolEntry::c_namespace for the namespace to be entered into the global table
		SymbolEntryPtr sentry = new YSymbolEntry (name, Type::Unspec, namespaceTable);

#if DO_DEBUG
		y2debug ("Entered Namespace '%s' (block %p) into namespaceTable %p", name, block, namespaceTable);
#endif

		// enter into global table
		namespace_point = new Point (filename);
		table->enter (name, sentry, namespace_point);

		// all further definitions go into this namespace
		//   -> make it the new global table
		table = namespaceTable;
		namespace_decl = declarations;

		if (is_predefined)
		{
		    // save tracking info, trigger 'Import' _after_ all symbols have been added
		    track_info = new std::pair<std::string, Y2Namespace *> (name, namespaceNamespace);
		}
	    }
	}
	else	// normal entry, not namespace
	{
	    declarations->name_space = namespace_decl;
	    string signature = declarations->signature;

	    constTypePtr type = Type::fromSignature (signature);
	    if (type == 0
		|| type->isError()
		|| type->isUnspec()
		|| type->isWildcard())
	    {
		y2error ("Invalid signature %s::%s:'%s'\n", filename, name, signature.c_str());
		return;
	    }

#if DO_DEBUG
y2debug("%s sig[%s] type[%s]", name, signature.c_str(), type->toString().c_str());
#endif
#if 0
	    if (type->hasFlex()
		&& (declarations->flags & DECL_FLEX) == 0)
	    {
		y2error ("%s:'%s' without DECL_FLEX", name, declarations->signature);
		return;
	    }
#endif
	    declarations->type = type;

	    SymbolEntryPtr sentry = new YSymbolEntry (name, type, declarations, name_space);
	    declarations->tentry = table->enter (name, sentry, new Point (sentry, 0, namespace_point));

#ifdef BUILTIN_STATISTICS
	    FILE *fout = fopen ("/tmp/builtin-register.txt", "a");
	    if (fout) {
		fprintf (fout, "%s %s\n", declarations->name, declarations->signature);
		fclose (fout);
	    }
#endif
	}
	declarations++;
    }

    // clear possibly old track_info
    if (track_info != 0)
    {
	new Import (track_info->first, track_info->second);	// remember which predefined got activated
	m_active_predefined.push_back (*track_info);
	track_info = 0;
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

    const declaration_t *name_space = declaration->name_space;
    const char *name = declaration->name;

    if (!full)
    {
	return (name_space ? string (name_space->name) + "::" + string (name) : string (name));
    }

    if (name == 0)
    {
	return "<NULL>";
    }

    return string (name) + " : " + declaration->type->toString();
}


//------------------------------------------------------------------------
// find declaration by name
declaration_t *
StaticDeclaration::findDeclaration (const char *name) const
{
#if DO_DEBUG
    y2debug ("StaticDeclaration::findDeclaration '%s'", name);
#endif

    // split the name by the namespace
    char *next = strstr (name, "::");

#if DO_DEBUG
    y2debug( "Next is %p", next );
#endif

    TableEntry *tentry = 0;
    if (next == NULL)
    {
   	tentry = m_declTable->find (name);
#if DO_DEBUG
	y2debug( "No namespace, found %p", tentry );
#endif
    }
    else
    {
	*next = '\0';
	tentry = m_declTable->find (name);
#if DO_DEBUG
	y2debug( "Namespace found: %p", tentry );
#endif
	*next = ':';
	if (tentry != 0
	   && ((YSymbolEntryPtr)(tentry->sentry()))->table() != 0)
	{
	    // continue recursively;
	    // skip delimiter
	    next += 2;
#if DO_DEBUG
	    y2debug( "Recursive search for %s", next );
#endif
	    tentry = ((YSymbolEntryPtr)(tentry->sentry()))->table()->find (next);
	}
    }

    if (tentry != 0)
    {
	return ((YSymbolEntryPtr)tentry->sentry())->declaration();
    }
    return 0;
}

//------------------------------------------------------------------------
// check if a declaration of 'name' matches 'type'
// return the matched declaration or NULL

declaration_t *
StaticDeclaration::findDeclaration (const char *name, constTypePtr type, bool partial) const
{
#if DO_DEBUG
    y2debug ("StaticDeclaration::findDeclaration '%s':%s <%s>", name, type->toString().c_str (), partial?"partial":"full");
#endif

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
#if DO_DEBUG
    y2debug ("StaticDeclaration::findDeclaration (%p, %s, %s)", decl, type->toString().c_str (), partial ? "partial" : "full");
#endif
    if (decl == 0)
    {
	return 0;
    }

    // now check all (overloaded) possibilities

// FIXME: properly check Templates (FlexT and NFlexT)

    while (decl)
    {
	constTypePtr dtype = decl->type;		// declaration type
	constTypePtr atype = type;			// argument type

#if DO_DEBUG
	y2debug ("decl check: declared '%s', actual '%s'", dtype->toString().c_str(), atype->toString().c_str());
#endif

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
#if DO_DEBUG
	    y2debug ("dcount %d, acout %d", dcount, acount);
#endif
	    int i;
	    
	    if (acount == 0)
	    {
		// no arguments actual and declared
		if (dcount == 0) break;
		
		// no actual arguments allowed only if declared arguments are wildcard
		error = ! (fdtype->parameterType (0)->isWildcard ());
	    } 
	    else
	    { 
		// need to check all arguments one by one
		constTypePtr dt = 0;

		for (i = 0; i < acount; i++)
		{
		    if (i >= dcount)				// no more parameters in current declaration
		    {
#if DO_DEBUG
			y2debug ("too few parameters");
#endif
			error = true;
			break;
		    }
		    dt = fdtype->parameterType (i);

		    if (fatype->parameterType(i)->match (dt) != 0)		// parameters do not match
		    {
#if DO_DEBUG
			y2debug ("parameters at %d do not match: decl '%s', actual '%s'", i, dt->toString().c_str(), fatype->parameterType(i)->toString().c_str());
#endif
			error = true;
			break;
		    }
		    if (dt->isWildcard())					// stop parameter checking on wildcard
		    {
			break;
		    }
		}
#if DO_DEBUG
		y2debug ("loop exit %d", i);
#endif
	    
		// this was not the last argument in declared arguments
		// and the next one is not wildcard (can be omited completely)
		// it's an error
		if (!partial					// full match required
		    && i != dcount				// not the last argument
		    && !dt->isWildcard ()			// declared type is not wildcard
		    && !fdtype->parameterType(i)->isWildcard ())
		{
#if DO_DEBUG
		    y2debug ("missing parameters");
#endif
		    error = true;
		}
	    }

	    if (!error)		// full match
		break;
	}
	
	if (decl->tentry->next_overloaded () != 0)
	{
	    decl = ((YSymbolEntryPtr)decl->tentry->next_overloaded ()->sentry ())->declaration ();
	}
	else
	{
	    decl = 0;
	}
    }

    if (decl == 0)
    {
#if DO_DEBUG
	y2debug ("findDecl failed");
#endif
    }
    else
    {
#if DO_DEBUG
	y2debug ("match for decl '%s'", Decl2String (decl, true).c_str());
#endif
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
    while( d->name_space != 0 )
    {
        d = d->name_space;
	n = std::string(d->name) + "::" + n;
    }
#if DO_DEBUG
    y2debug ("StaticDeclaration::writeDeclaration('%s':%s)", n.c_str(), decl->type->toString().c_str());
#endif
 
    Bytecode::writeCharp (str, n.c_str());
    decl->type->toStream (str);
    return str;
}


// read declaration from stream (return declaration matching name and type _exactly_)
declaration_t *
StaticDeclaration::readDeclaration (bytecodeistream & str) const
{
    char *name = Bytecode::readCharp (str);
    constTypePtr type = Bytecode::readType (str);
#if DO_DEBUG
    y2debug ("StaticDeclaration::readDeclaration('%s':%s)", name, type->toString().c_str());
#endif
    declaration_t *decl = findDeclaration (name, type);
    if (decl == 0)
    {
	ycp2error ("No match for '%s (%s)'", name, type->toString().c_str());
	str.setstate (std::ostream::failbit);
    }
    delete [] name;
    return decl;
}


void
StaticDeclaration::errorNoMatch (Logger* problem_logger, constFunctionTypePtr orig, declaration_t* first_decl)
{
    problem_logger->error (string("No match for '")+first_decl->name+" : "+orig->toString ()+"'");
    problem_logger->error ("Please fix parameter types to match one of:");
    while (first_decl)
    {
        problem_logger->error (string("'")+StaticDeclaration::Decl2String (first_decl,true)+"'");

        if (first_decl->tentry->next_overloaded () != 0)
        {
            first_decl = ((YSymbolEntryPtr)first_decl->tentry->next_overloaded ()->sentry ())->declaration ();
        }
        else
        {
	    return;
        }
    }
}

// EOF
