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

   File:	SymbolEntry.h
		symbol entry class

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef YSymbolEntry_h
#define YSymbolEntry_h

#include <y2util/Ustring.h>
#include <y2util/RepDef.h>

#include <y2/SymbolEntry.h>

#include "ycp/YCPValue.h"
#include "ycp/Type.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/YCode.h"

#include <stack>

class Y2Namespace;

DEFINE_DERIVED_POINTER (YSymbolEntry, SymbolEntry);

class YSymbolEntry : public SymbolEntry {
    REP_BODY (YSymbolEntry);

    /**
     * the default (initial) value ('payload') of the entry
     * -> set by YSVariable and YSFunction
     *
     * It is grossly overloaded:
     *	c_builtin:	declaration_t*
     *  c_module:	Y2Namespace*
     *  c_namespace:	SymbolTable *
     *	c_self		n/a (just uses m_name)
     *  c_predefined	n/a (just uses m_name)
     *	c_filename	n/a (just uses m_name)
     */
    union payload {
	Y2Namespace *m_namespace;
	declaration_t *m_decl;
	SymbolTable *m_table;
    } m_payload;

    /*
     * Valid for
     *  c_variable:	YCode* (any value)
     *  c_reference:	YCode* (YEReference*)
     *  c_function:	YCode* (YFunction* to be precise)
     */
    YCodePtr m_code;

public:
    // create symbol beloging to namespace (at position) - overloaded
    YSymbolEntry (const Y2Namespace* name_space, unsigned int position, const char *name, category_t cat, constTypePtr type, YCodePtr payload = 0);

    // create builtin symbol (category == c_builtin), name_space != 0 for symbols inside namespace
    YSymbolEntry (const char *name, constTypePtr type, declaration_t *payload, const Y2Namespace *name_space = 0);

    // create namespace symbol (category == c_namespace)
    YSymbolEntry (const char *name, constTypePtr type, SymbolTable *payload);

    // create declaration point symbol (category == c_filename)
    YSymbolEntry (const char *filename);

    YSymbolEntry (bytecodeistream & str, const Y2Namespace *name_space = 0);

    // payload access for variables and functions
    void setCode (YCodePtr code);
    YCodePtr code () const;    

    // returns true for a declared symbol which isn't defined yet.
    virtual bool onlyDeclared () const;

    // payload access for builtins
    void setDeclaration (declaration_t *decl);
    declaration_t *declaration () const;

    // payload access for namespace symbols
    void setTable (SymbolTable *table);
    SymbolTable *table() const;

    // symbols' link to the defining namespace
    Y2Namespace *payloadNamespace () const;
    void setPayloadNamespace (Y2Namespace *name_space);

    virtual string toString (bool with_type = true) const;
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;
};

#endif // YSymbolEntry_h
