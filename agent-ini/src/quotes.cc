/*
 * Copyright (c) 2012 Novell, Inc.
 *
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may
 * find current contact information at www.novell.com.
 */
/**
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Functions for shell like (un)quoting
 *
 * Authors:
 *   Michal Filka <mfilka@suse.cz>
 *
 * $Id$
 */

#include "quotes.h"
#include <ycp/y2log.h>
#include <stdexcept>

using namespace std;

string quote( const string & unquoted_string)
{
    string dest = "'";

    for( string::const_iterator sit = unquoted_string.begin(); sit != unquoted_string.end(); sit++)
    {
        if( *sit == '\'')
            dest += "'\\''";
        else
            dest += *sit;
    }

    dest += "'";

    return dest;
}

string parse_dquoted_string( string::const_iterator & sit, const string::const_iterator & last)
{
    string ret;

    while( sit != last)
    {
        switch( *sit)
        {
            case '"':
                return ret;

            case '\\':
                switch( *(++sit))
                {
                    case '"':
                    case '\\':
                    case '`':
                    case '$':
                    case '\n':
                        if( sit != last)
                            ret += *(sit++);
                        break;

                    default:
                        ret += '\\';
                        break;
                }
                break;

            default:
                ret += *(sit++);
                break;
        }
    }

    throw invalid_argument( "Missing enclosing dquote.");
    return "";
}

string parse_squoted_string( string::const_iterator & sit, const string::const_iterator & last)
{
    string ret;

    while( sit != last)
    {
        if( *sit == '\'')
        {
            return ret;
        }
        else
        {
            ret += *(sit++);
        }
    }

    throw invalid_argument( "Missing enclosing squote.");
    return "";
}

/*
 * helper which parses one quoted block.
 * example:
 * "string" -> string
 *
 * preconditions:
 * - starts with ['"] 
 * - as a consequence: size > 0
 */
string parse_quoted_string( string::const_iterator & sit, const string::const_iterator & last)
{
    char quote_char = *( sit++);

    return quote_char == '"' ? parse_dquoted_string( sit, last) : parse_squoted_string( sit, last);
}

string unquote( const string & quoted_string)
{
    string res;
    string::const_iterator sit = quoted_string.begin();
    string::const_iterator end = quoted_string.end();

    try
    {
        while( sit != end)
        {
            while(  sit != end && 
                    *sit != '"' && 
                    *sit != '\'')
            {   
                if( *sit == '\\')
                {
                    if( ++sit == end)
                        return res;
                }
                res += *(sit++);
            }

            if( !( sit != end))
                break;

            res += parse_quoted_string( sit, end);
            sit++;
        }
    }
    catch( invalid_argument &ia)
    {
        res = "";
        ycp2error( "Unquoting error: %s Unquoted value: <%s>.", ia.what(), quoted_string.c_str());
    }

    return res;
}
