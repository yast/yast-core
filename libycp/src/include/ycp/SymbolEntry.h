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

#include "ycp/YCPValue.h"
#include "ycp/Type.h"
#include "ycp/StaticDeclaration.h"

#include <stack>

class YCode;
class YBlock;
class Y2Namespace;
class TableEntry;

/**
 */

class SymbolEntry
{
private:
    // hash for unique strings
    static UstringHash _nameHash;

public:
    typedef enum {
	c_unspec = 0,		// unspecified local symbol (sets m_global = false)
	c_global,		// unspecified global symbol (translates to c_unspec, sets m_global = true)
	c_module,		// a module identifier
	c_variable,		// a variable
	c_reference,		// a reference to a variable
	c_function,		// a defined function
	c_builtin,		// a builtin function
	c_typedef,		// a type
	c_const,		// a constant (a read-only c_variable)
	c_namespace,		// a namespace identifier
	c_self			// the current namespace (namespace prefix used in namespace definition)
    } category_t;

private:
    /*
     * if it's global
     */
    bool m_global;

    /*
     * the block this entry belongs to
     */
    const Y2Namespace *m_block;

    /*
     * position in the block
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
     *  c_variable:	YCode* (any value)
     *  c_reference:	YCode* (YEReference*)
     *  c_function:	YCode* (YFunction* to be precise)
     *	c_builtin:	declaration_t*
     *  c_module:	Y2Namespace*
     *  c_namespace:	SymbolTable *
     */
    union payload {
	YCode *m_code;
	Y2Namespace *m_namespace;
	declaration_t *m_decl;
	SymbolTable *m_table;
    } m_payload;

    /*	the current (actual) value of the entry c_const  */
    YCPValue m_value;
    
    stack<YCPValue> m_recurse_stack;

public:

    // create symbol beloging to block (at position)
    SymbolEntry (const Y2Namespace* block, unsigned int position, const char *name, category_t cat, constTypePtr type, YCode *payload = 0);
    // create builtin symbol (category == c_builtin)
    SymbolEntry (const char *name, constTypePtr type, declaration_t *payload);
    // create namespace symbol (category == c_namespace)
    SymbolEntry (const char *name, constTypePtr type, SymbolTable *payload);

    SymbolEntry (std::istream & str, const YBlock *block = 0);

    // payload access

    // returns true for a declared symbol which isn't defined yet.
    bool onlyDeclared () const;

    // payload access for variables and functions
    void setCode (YCode *code);
    YCode *code () const;
    
    Y2Namespace *name_space () const;
    void setNamespace (Y2Namespace *ns);
    
    // payload access for builtins
    void setDeclaration (declaration_t *decl);
    declaration_t *declaration () const;

    // payload access for namespace symbols
    void setTable (SymbolTable *table);
    SymbolTable *table() const;

    // symbols' link to the defining block
    const Y2Namespace *block () const;
    void setBlock (const Y2Namespace *block);
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
