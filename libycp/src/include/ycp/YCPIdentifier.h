/*---------------------------------------------------------------------\
|                                                                      |  
|                      __   __    ____ _____ ____                      |  
|                      \ \ / /_ _/ ___|_   _|___ \                     |  
|                       \ V / _` \___ \ | |   __) |                    |  
|                        | | (_| |___) || |  / __/                     |  
|                        |_|\__,_|____/ |_| |_____|                    |  
|                                                                      |  
|                               core system                            | 
|                                                        (C) SuSE GmbH |  
\----------------------------------------------------------------------/ 

   File:       YCPIdentifier.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPIdentifier_h
#define YCPIdentifier_h

#include "ycp/YCPSymbol.h"
#include "ycp/YCPValue.h"


/**
 * @short YCP identifier.
 * Identifiers appear as symbolic values in statements.
 * Identifiers may carry a module (module) prefix.
 *
 * YCP Syntax: A symbol, optionally prefixed by another symbol and ::
 * <pre>an_indentifier  module::name_in_module  ::global_symbol</pre>
 */
class YCPIdentifierRep : public YCPValueRep
{
    string n;		// module, "_" is global
    YCPSymbol s;	// symbol

protected:
    friend class YCPIdentifier;

    /**
     * Creates new identifier from a const char *pointer.
     */
    YCPIdentifierRep(const char *i_symbol, const char *i_module, bool lvalue);

    /**
     * Creates new identifier from a string.
     */
    YCPIdentifierRep(string i_symbol, string i_module, bool lvalue);

    /**
     * Creates new identifier from a symbol and a string
     */
    YCPIdentifierRep(const YCPSymbol& i_symbol, string i_module);

    /**
     * Creates new identifier from a symbol and a const char*
     */
    YCPIdentifierRep(const YCPSymbol& i_symbol, const char *i_module);

public:
    /**
     * Returns the identifier's module.
     */
    string module() const;

    /**
     * Returns the identifier's symbol.
     */
    YCPSymbol symbol() const;

    /**
     * Returns the identifier's module as const char * pointer.
     */
    const char *module_cstr() const;

    /**
     * Compares two YCPIdentifiers for equality, greaterness or smallerness.
     * Identifiers are compared by their names interpreted as strings.
     * @param v value to compare against
     * @return YO_LESS, if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPIdentifier &v) const;

    /**
     * Returns the ASCII representation of the symbol.
     */
    string toString() const;

    /**
     * Returns YT_IDENTIFIER. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPIdentifierRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPIdentifierRep
 * with the arrow operator. See @ref YCPIdentifierRep.
 */
class YCPIdentifier : public YCPValue
{
    DEF_COMMON(Identifier, Value);
public:
    YCPIdentifier(const char *i_symbol, const char *i_module = "", bool lvalue = false) : YCPValue(new YCPIdentifierRep (i_symbol, i_module, lvalue)) {}
    YCPIdentifier(string i_symbol, string i_module = string(""), bool lvalue = false) : YCPValue(new YCPIdentifierRep (i_symbol, i_module, lvalue)) {}
    YCPIdentifier(const YCPSymbol& i_symbol, string i_module = string("")) : YCPValue(new YCPIdentifierRep (i_symbol, i_module)) {}
};

#endif   // YCPIdentifier_h

