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

   File:       YCPIdentifier.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

   This implements the YCPIndentifier data type which is an
   extension of the YCPSymbol data type. An identifier carries
   a module and a symbol name.

   $Id$
/-*/

#include "y2log.h"
#include "YCPIdentifier.h"



// YCPIdentifierRep

YCPIdentifierRep::YCPIdentifierRep(const char *i_symbol, const char *i_module, bool lvalue)
    : n(i_module)
    , s(YCPSymbol (i_symbol, lvalue))
{
}


YCPIdentifierRep::YCPIdentifierRep(string i_symbol, string i_module, bool lvalue)
    : n(i_module)
    , s(YCPSymbol (i_symbol, lvalue))
{
}


YCPIdentifierRep::YCPIdentifierRep(const YCPSymbol& i_symbol, string i_module)
    : n(i_module)
    , s(i_symbol)
{
}


YCPIdentifierRep::YCPIdentifierRep(const YCPSymbol& i_symbol, const char *i_module)
    : n(i_module)
    , s(i_symbol)
{
}


string YCPIdentifierRep::module() const
{
    return n;
}


YCPSymbol YCPIdentifierRep::symbol() const
{
    return s;
}


const char *YCPIdentifierRep::module_cstr() const
{
    return n.c_str();
}


YCPOrder YCPIdentifierRep::compare(const YCPIdentifier& id) const
{
    if (n == id->n) {
	return s->compare (id->s);
    }
    else return n < id->n ? YO_LESS : YO_GREATER;
}


string YCPIdentifierRep::toString() const
{
    if (n == "") return s->toString();
    else return ((n == "_") ? string("") : n) + string("::") + s->toString();
}


YCPValueType YCPIdentifierRep::valuetype() const
{
    return YT_IDENTIFIER;
}

