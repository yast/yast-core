#include "quotes.h"

using namespace std;

string quote( string & unquoted_string)
{
    string dest = "'";

    for( string::iterator sit = unquoted_string.begin(); sit < unquoted_string.end(); sit++)
        if( *sit == '\'')
            dest += "'\\''";
        else
            dest += *sit;

    dest += "'";

    return dest;
}

// helper
// preconditions:
// - starts with ['"]
// - as a consequence: size > 0
bool parse_quoted_string( string::iterator & sit, string::iterator last, string & ret)
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

string unquote( string & quoted_string)
{
    string res;
    string::iterator sit = quoted_string.begin();

    while( sit < quoted_string.end())
    {
        while(  sit < quoted_string.end() && 
                *sit != '"' && 
                *sit != '\'')
        {   
            if( *sit == '\\')
            {
                if( *(++sit) != '\n')
                    res += *sit;
            }
            else
                res += *sit;

            sit++;
        }

        if( !( sit < quoted_string.end()))
            break;

        string substr;
        if( parse_quoted_string( sit, quoted_string.end(), substr))
            res += substr;
        sit++;
    }
    return res;
}
