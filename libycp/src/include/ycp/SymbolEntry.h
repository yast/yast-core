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

#include <YaST2/y2util/Ustring.h>

#include "ycp/YCPValue.h"
#include "ycp/TypeCode.h"
#include "ycp/StaticDeclaration.h"

class YCode;
class YBlock;
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
	c_function,		// a defined function
	c_builtin,		// a builtin function
	c_typedef,		// a type
	c_const,		// a constant
	c_namespace		// a namespace identifier
    } category_t;

private:
    /*
     * if it's global
     */
    bool m_global;

    /*
     * the block this entry belongs to
     */
    const YBlock *m_block;

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
    TypeCode m_type;

    /**
     * the default (initial) value ('payload') of the entry
     * -> set by YSVariable and YSFunction
     *
     * It is grossly overloaded:
     *  c_variable:	YCode* (any value)
     *  c_function:	YCode* (YBlock* to be precise)
     *	c_builtin:	declaration_t*
     *  c_module:	YCode* (YBlock* to be precise)
     *  c_namespace:	SymbolTable *
     */
    union payload {
	YCode *m_code;
	declaration_t *m_decl;
	SymbolTable *m_table;
    } m_payload;

    /*	the current (actual) value of the entry c_const  */
    YCPValue m_value;

public:

    SymbolEntry (const YBlock* block, unsigned int position, const char *name, category_t cat, const TypeCode &type, YCode *code = 0);
    // builtin
    SymbolEntry (const char *name, const TypeCode &type, declaration_t *decl);
    // namespace
    SymbolEntry (const char *name, const TypeCode &type, SymbolTable *table);

    SymbolEntry (std::istream & str, const YBlock *block = 0);

    // payload access
    void setCode (YCode *code);
    YCode *code () const;
    void setDeclaration (declaration_t *decl);
    declaration_t *declaration () const;
    void setTable (SymbolTable *table);
    SymbolTable *table() const;

    const YBlock *block () const;
    void setBlock (const YBlock *block);
    unsigned int position () const;
    void setPosition (unsigned int position);
    bool isGlobal () const;
    const char *name () const;
    category_t category () const;
    void setCategory (category_t cat);
    TypeCode type () const;
    string catString () const;
    void setType (const TypeCode &type);
    YCPValue setValue (YCPValue value);
    YCPValue value () const;

    string toString (bool with_type = true) const;
    std::ostream & toStream (std::ostream & str) const;
};


#endif // SymbolEntry_h
