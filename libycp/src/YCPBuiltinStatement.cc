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

   File:       YCPBuiltinStatement.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * YCPBuiltinStatement data type
 *
 */

#include "y2log.h"
#include "YCPBuiltin.h"
#include "YCPBuiltinStatement.h"


// YCPBuiltinStatementRep

YCPBuiltinStatementRep::YCPBuiltinStatementRep(int lineno, const builtin_t c, const YCPValue v)
    : YCPStatementRep (lineno)
    , c(c)
    , v(v)
{
}


YCPStatementType YCPBuiltinStatementRep::statementtype() const
{
    return YS_BUILTIN;
}


builtin_t YCPBuiltinStatementRep::code() const
{
    return c;
}


YCPValue YCPBuiltinStatementRep::value() const
{
    return v;
}


//compare BuiltinStatements
YCPOrder YCPBuiltinStatementRep::compare(const YCPBuiltinStatement& s) const
{
    return (c < s->c) ? YO_LESS : ((c > s->c) ? YO_GREATER : YO_EQUAL);
}


// get the statement as string
string YCPBuiltinStatementRep::toString() const
{
    return YCPBuiltin (c, v)->toString();
}
