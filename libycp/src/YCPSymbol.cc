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

   File:       YCPSymbol.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPSymbol data type
 */

#include "y2log.h"
#include "YCPSymbol.h"



// YCPSymbolRep

YCPSymbolRep::YCPSymbolRep(const char *s, bool quoted)
    : v(s)
    , quoted(quoted)
{
}


YCPSymbolRep::YCPSymbolRep(string s, bool quoted)
    : v(s)
    , quoted(quoted)
{
}


string YCPSymbolRep::symbol() const
{
    return v;
}


bool YCPSymbolRep::isQuoted() const
{
    return quoted;
}

const char *YCPSymbolRep::symbol_cstr() const
{
    return v.c_str();
}


YCPOrder YCPSymbolRep::compare(const YCPSymbol& s) const
{
    if (v == s->v) return YO_EQUAL;
    else return v < s->v ? YO_LESS : YO_GREATER;
}


string YCPSymbolRep::toString() const
{
    if (quoted) return string("`") + v;
    else return v;
}


YCPValueType YCPSymbolRep::valuetype() const
{
    return YT_SYMBOL;
}

