#include "quotes.h"

using namespace std;

string quote( const string & unquoted_string)
{
    string dest /*= "'"*/;

    for( string::const_iterator sit = unquoted_string.begin(); sit < unquoted_string.end(); sit++)
        if( *sit == '\'')
            dest += "'\\''";
        else
            dest += *sit;

    if( dest != unquoted_string)
        dest = "'" + dest + "'";

//    dest += "'";

    return dest;
}

// helper
// preconditions:
// - starts with ['"]
// - as a consequence: size > 0
bool parse_quoted_string( string::const_iterator & sit, string::const_iterator last, string & ret)
{
    char quote_char = *( sit++);

    if( quote_char == '"')
    {
        while( sit < last)
        {
            switch( *sit)
            {
                case '"':
                    return true;

                case '\\':
                    switch( *(++sit))
                    {
                        case '"':
                        case '\\':
                        case '`':
                        case '$':
                        case '\n':
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
    }
    else
    {
        while( sit < last)
        {
            if( *sit == '\'')
            {
                return true;
            }
            else
            {
                ret += *(sit++);
            }
        }
    }

    // missing closing quote
    return false;
}

string unquote( const string & quoted_string)
{
    string res;
    string::const_iterator sit = quoted_string.begin();
    string::const_iterator end = quoted_string.end();

    while( sit < end)
    {
        while(  sit < end && 
                *sit != '"' && 
                *sit != '\'')
        {   
/*
            if( *sit == '\\')
            {
                if( *(++sit) != '\n')
                    res += *sit;
            }
            else
                res += *sit;

            sit++;
*/
            res += *(sit++);
        }

        if( !( sit < end))
            break;

        string substr;
        if( parse_quoted_string( sit, end, substr))
            res += substr;
        sit++;
    }
    return res;
}