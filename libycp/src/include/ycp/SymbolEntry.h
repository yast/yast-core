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

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef SymbolEntry_h
#define SymbolEntry_h

#include <y2util/Ustring.h>
#include <y2util/RepDef.h>

#include "ycp/YCPValue.h"
#include "ycp/Type.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/YCode.h"

#include <stack>

class YBlock;
class Y2Namespace;
class TableEntry;

/**
 */

class SymbolEntry : public Rep, public MemUsage
{
    REP_BODY (SymbolEntry);

public:
    // hash for unique strings
    static UstringHash _nameHash;
    static Ustring emptyUstring;

public:
    typedef enum {
	c_unspec = 0,		//  0 unspecified local symbol (sets m_global = false)
	c_global,		//  1 unspecified global symbol (translates to c_unspec, sets m_global = true)
	c_module,		//  2 a module identifier
	c_variable,		//  3 a variable
	c_reference,		//  4 a reference to a variable
	c_function,		//  5 a defined function
	c_builtin,		//  6 a builtin function
	c_typedef,		//  7 a type
	c_const,		//  8 a constant (a read-only c_variable)
	c_namespace,		//  9 a namespace identifier
	c_self,			// 10 the current namespace (namespace prefix used in namespace definition)
	c_predefined,		// 11 a predefined namespace identifier
	c_filename		// 12 a filename (used in conjunction with TableEntry to store definition locations)
    } category_t;

private:
    /*
     * if it's global
     */
    bool m_global;

    /*
     * the namespace this entry belongs to
     */
    const Y2Namespace *m_namespace;

    /*
     * position in the namespace
     */
    unsigned int m_position;

    /*
     * the name of the entry
     */
    Ustring m_name;

    /*
     * the category of the entry
     */
    category_t m_category;

    /*
     * the type (string) of the entry
     */
    constTypePtr m_type;

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

    /*	the current (actual) value of the entry c_const  */
    YCPValue m_value;
    
    stack<YCPValue> m_recurse_stack;

public:

    // create symbol beloging to namespace (at position)
    SymbolEntry (const Y2Namespace* name_space, unsigned int position, const char *name, category_t cat, constTypePtr type, YCodePtr payload = 0);

    // create builtin symbol (category == c_builtin), name_space != 0 for symbols inside namespace
    SymbolEntry (const char *name, constTypePtr type, declaration_t *payload, const Y2Namespace *name_space = 0);

    // create namespace symbol (category == c_namespace)
    SymbolEntry (const char *name, constTypePtr type, SymbolTable *payload);

    // create declaration point symbol (category == c_filename)
    SymbolEntry (const char *filename);

    SymbolEntry (std::istream & str, const Y2Namespace *name_space = 0);

    virtual ~SymbolEntry ();

    // symbols' link to the defining namespace
    const Y2Namespace *nameSpace () const;
    void setNamespace (const Y2Namespace *name_space);

    // payload access

    // returns true for a declared symbol which isn't defined yet.
    bool onlyDeclared () const;

    // payload access for variables and functions
    void setCode (YCodePtr code);
    YCodePtr code () const;
    
    // payload access for builtins
    void setDeclaration (declaration_t *decl);
    declaration_t *declaration () const;

    // payload access for namespace symbols
    void setTable (SymbolTable *table);
    SymbolTable *table() const;

    // symbols' link to the defining namespace
    Y2Namespace *payloadNamespace () const;
    void setPayloadNamespace (Y2Namespace *name_space);

    // this is the position of the entry in the namespace (>= 0)
    //   or in the xref table (< 0), see YSImport()
    unsigned int position () const;
    void setPosition (unsigned int position);

    bool isGlobal () const;

    bool isModule () const { return m_category == c_module; }
    bool isVariable () const { return m_category == c_variable; }
    bool isReference () const { return m_category == c_reference; }
    bool isFunction () const { return m_category == c_function; }
    bool isBuiltin () const { return m_category == c_builtin; }
    bool isNamespace () const { return m_category == c_namespace; }
    bool isSelf () const { return m_category == c_self; }
    bool isFilename () const { return m_category == c_filename; }
    bool isPredefined () const { return m_category == c_predefined; }

    bool likeNamespace () const { return isModule() || isNamespace() || isSelf(); }

    const char *name () const;
    category_t category () const;
    void setCategory (category_t cat);
    constTypePtr type () const;
    string catString () const;
    void setType (constTypePtr type);
    YCPValue setValue (YCPValue value);
    YCPValue value () const;
    
    void push ();
    void pop ();

    string toString (bool with_type = true) const;
    std::ostream & toStream (std::ostream & str) const;
};


#endif // SymbolEntry_h
