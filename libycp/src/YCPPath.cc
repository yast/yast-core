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

   File:       YCPPath.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCPPath data type
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include <stdio.h>

#include "y2log.h"
#include "YCPPath.h"
#include <ctype.h>

// YCPPathRep

YCPPathRep::YCPPathRep()
{
}


YCPPathRep::YCPPathRep(const char *r)
{
    if (!strcmp(r, ".")) return; // Root path is empty

    string p;
    enum { dot, simple, complex_first, complex } state = dot;
    const char*start = NULL;
    for (const char*c = r;*c;c++) {
        switch (state) {
        case dot: // there must be ." or .[a-zA-Z0-9_]. Scanner guarantees this.
            state = '"'==*(c+1) ? complex_first : simple;
            start = c+1;
            break;
        case simple:
            if ('.'==*(c+1) || '\0'==*(c+1)) {
                if ('-' ==*start || '-'==*c) {
                    y2error("bad path constant: dash before/after dot not allowed");
                    components.clear();
                    return;
                }
                p.assign(start,c-start+1);
                components.push_back(Component(p));
                state = dot;
            }
            break;
        case complex_first:
            state = complex;
            break;
        case complex:
            if ('\\'==*c)
                c++;
            else if('"'==*c) { // end of component
                p.assign(start,c-start+1);
                components.push_back(Component(p));
                state = dot;
            }
            break;
        }
    }
}

bool YCPPathRep::isRoot() const
{
    return components.empty();
}

void YCPPathRep::append(const YCPPath&p)
{
    int len = p->length();
    for(int i = 0;i<len;i++)
        components.push_back(p->components[i]);
}

void YCPPathRep::append(const Component&c)
{
    components.push_back(c);
}

void YCPPathRep::append(string c)
{
    Component added;
    added.component = c;
    added.complex = true;  //it would be nicer if we checked if it is really complex, but this is faster
    components.push_back(added);
}

YCPValue YCPPathRep::select(const YCPValue& val)
{
    return val; 
    // TODO: do real operation
}


long YCPPathRep::length() const
{
    return components.size();
}


bool YCPPathRep::isPrefixOf(const YCPPath& path) const
{
    if (length() > path->length()) return false;

    for (int c=0; c<length(); c++)
	if (component_str(c) != path->component_str(c)) return false;
    return true;
}


YCPPath YCPPathRep::at(long index) const
{
    YCPPath postfix;
    for (int i=index; i<length(); i++)
	postfix->append(components[i]);
    return postfix;
}


string YCPPathRep::component_str(long index) const
{
    return components[index].component;
}

YCPOrder YCPPathRep::compare(const YCPPath& p) const
{
    for (unsigned c=0; c<components.size(); c++)
    {
	if (p->components.size() <= c) return YO_GREATER;
	int comp = components[c].compare(p->components[c]);
	if (comp < 0) return YO_LESS;
	else if (comp > 0) return YO_GREATER;
    }
    if ( components.size() == p->components.size() ) return YO_EQUAL;
    return components.size() < p->components.size() ? YO_LESS : YO_GREATER;
}


string YCPPathRep::toString() const
{
    if (components.empty()) return ".";
    string v;
    for (unsigned c=0; c<components.size(); c++)
    {
	v += ".";
	v += components[c].toString();
    }
    return v;
}


YCPValueType YCPPathRep::valuetype() const
{
    return YT_PATH;
}

YCPPathRep::Component::Component(string s)
{
    if (s[0] == '"')
        {
            char num;
            complex = true;
            component.erase();
            for (const char*c = s.c_str()+1;*c;c++)
                {
                    if ('\\' == *c)
                        {
                            // handles \\, \n, \t, \r, \b, \f, \x00, \"
                            c++;
                            switch (*c)
                                {
                                case '\0':
                                    // this should never happen because scanner gives us
                                    // value that ends up with " (not with \)
                                    component+= '\\';
                                    return;
                                case '\\':   component+= '\\';   break;
                                case 'n':    component+= '\n';   break;
                                case 't':    component+= '\t';   break;
                                case 'r':    component+= '\r';   break;
                                case 'b':    component+= '\b';   break;
                                case 'f':    component+= '\f';   break;
                                case '"':    component+= '"';    break;
                                case 'x':
                                    // there must be at least one number
                                    // we know that there is at least " and \0 so this is safe
                                    num = 0;
                                    if (isxdigit(*(c+1)) && isxdigit(*(c+2)))
                                        {
                                            num =
                                                (isdigit(*(c+2)) ? *(c+2)-'0' : isupper(*(c+2)) ? *(c+2)-'A'+10 : *(c+2)-'a'+10)
                                                |
                                                ((isdigit(*(c+1)) ? *(c+1)-'0' : isupper(*(c+1)) ? *(c+1)-'A'+10 : *(c+1)-'a'+10) << 4)
                                                ;
                                            c+= 2;
                                        }
                                    else if (isxdigit(*(c+1)))
                                        {
                                            num =
                                                isdigit(*(c+1)) ? *(c+1)-'0' : isupper(*(c+1)) ? *(c+1)-'A'+10 : *(c+1)-'a'+10
                                                ;
                                            c++;
                                        }
                                    if (!num)
                                        y2error("\\x00 not allowed in path constant. Skipping.");
                                    component+= num;
                                    break;
                                default:
                                    component+= *c;
                                }
                        }
                    else if ('"' == *c)
                        {  // this must be " at the end. Return anyway to save some comparison
                            break;
                        }
                    else
                        component+= *c;
                }
        }
    else
        {
            component = s;
            complex = string::npos == component.find_first_not_of ("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-") ? false : true;
        }
}

string YCPPathRep::Component::toString() const
{
    if (!complex)
        return component;
    string s = "\"";
    for(const char*c = component.c_str();*c;c++)
        {
            switch (*c)
                {
                case '"': s+= "\\\""; break;
                case '\\':    s+= "\\\\";  break;
                case '\n':    s+= "\\n";   break;
                case '\t':    s+= "\\t";   break;
                case '\r':    s+= "\\r";   break;
                case '\b':    s+= "\\b";   break;
                case '\f':    s+= "\\f";   break;
                default:{
                    if (isprint (*c))
                        s+= *c;
                    else {
                        char buf[5];
                        snprintf(buf,4,"%02X",*(unsigned char*)c);
                        s+= "\\x";
                        s+= buf;
                    }
                }
                }
        }
    s+= '"';
    return s;
}
