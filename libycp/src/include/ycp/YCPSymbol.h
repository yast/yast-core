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

   File:       YCPSymbol.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

#ifndef YCPSymbol_h
#define YCPSymbol_h


#include "YCPValue.h"
#include <y2util/Ustring.h>


/**
 * @short YCP symbol.
 * Symbols appear as components of pathes, as names of
 * structure elements and as term names. The ASCII representation
 * of a symbol is a letter or underscore followed by an arbitrary
 * number of letters, digits and underscores.
 *
 * YCP Syntax: A letter or underscore followed by an arbitrary number
 * of digits, letters and underscores.
 * <pre>hElP   _8   a_45</pre>
 */
class YCPSymbolRep : public YCPValueRep
{
    Ustring v;

protected:
    friend class YCPSymbol;

    /**
     * Creates new symbol from a const char *pointer.
     */
    YCPSymbolRep(const char *s);

    /**
     * Creates a new symbol from a string.
     */
    YCPSymbolRep(string s);

public:
    /**
     * Returns the symbol's string.
     */
    string symbol() const;

    /**
     * Returns the symbol's string as const char * pointer.
     */
    const char *symbol_cstr() const;

    /**
     * Compares two YCPSymbols for equality, greaterness or smallerness.
     * Symbols are compared by their names interpreted as strings.
     * @param v value to compare against
     * @return YO_LESS, if this is smaller than v,
     *         YO_EQUAL,   if this is equal to v,
     *         YO_GREATER, if this is greater to v
     */
    YCPOrder compare(const YCPSymbol &v) const;

    /**
     * Returns the ASCII representation of the symbol.
     */
    string toString() const;

    /**
     * Output value as bytecode to stream
     */
    std::ostream & toStream (std::ostream & str) const;
    std::ostream & toXml (std::ostream & str, int indent ) const;

    /**
     * Returns YT_SYMBOL. See @ref YCPValueRep#valuetype.
     */
    YCPValueType valuetype() const;
};

/**
 * @short Wrapper for YCPSymbolRep
 * This class realizes an automatic memory management
 * via @ref YCPElement. Access the functionality of YCPSymbolRep
 * with the arrow operator. See @ref YCPSymbolRep.
 */
class YCPSymbol : public YCPValue
{
    DEF_COMMON(Symbol, Value);
public:
    YCPSymbol(const char *s) : YCPValue(new YCPSymbolRep(s)) {}
    YCPSymbol(string s) : YCPValue(new YCPSymbolRep(s)) {}
    YCPSymbol(bytecodeistream & str);
};

#endif   // YCPSymbol_h
