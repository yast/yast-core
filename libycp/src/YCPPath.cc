/*---------------------------------------------------------------------\
|								      |  
|		      __   __    ____ _____ ____		      |  
|		      \ \ / /_ _/ ___|_   _|___ \		     |  
|		       \ V / _` \___ \ | |   __) |		    |  
|			| | (_| |___) || |  / __/		     |  
|			|_|\__,_|____/ |_| |_____|		    |  
|								      |  
|			       core system			    | 
|							(C) SuSE GmbH |  
\----------------------------------------------------------------------/ 

   File:       YCPPath.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/YCPPath.h"
#include "ycp/Bytecode.h"
#include <ctype.h>

// YCPPathRep

YCPPathRep::YCPPathRep()
{
}


YCPPathRep::YCPPathRep(const char *r)
{
    if (strcmp (r, ".") == 0)
	return; // Root path is empty

    string p;
    enum { dot, simple, complex_first, complex } state = dot;
    const char*start = NULL;
    for (const char*c = r; *c; c++)
    {
	switch (state)
	{
	case dot: // there must be ." or .[a-zA-Z0-9_]. Scanner guarantees this.
	    state = ('"' == *(c+1)) ? complex_first : simple;
	    start = c+1;
	    break;
	case simple:
	    if ('.' == *(c+1)
		|| '\0' == *(c+1))
	    {
		if ('-' == *start
		    || '-' == *c)
		{
		    ycp2error ("bad path constant: dash before/after dot not allowed");
		    components.clear();
		    return;
		}
		p.assign (start, c-start+1);
		components.push_back (Component(p));
		state = dot;
	    }
	    break;
	case complex_first:
	    state = complex;
	    break;
	case complex:
	    if ('\\'==*c)
		c++;
	    else if ('"' == *c)
	    {	// end of component
		p.assign (start,c-start+1);
		components.push_back (Component(p));
		state = dot;
	    }
	    break;
	}
    }
}


bool
YCPPathRep::isRoot() const
{
    return components.empty();
}


void
YCPPathRep::append(const YCPPath&p)
{
    int len = p->length();
    for (int i = 0; i<len; i++)
	components.push_back (p->components[i]);
}


void
YCPPathRep::append(const Component&c)
{
    components.push_back(c);
}


void
YCPPathRep::append(string c)
{
    Component added;
    added.component = Ustring (*SymbolEntry::_nameHash, c);
    added.complex = true;  //it would be nicer if we checked if it is really complex, but this is faster
    components.push_back(added);
}


YCPValue
YCPPathRep::select(const YCPValue& val)
{
    return val; 
    // TODO: do real operation
}


long
YCPPathRep::length() const
{
    return components.size();
}


bool
YCPPathRep::isPrefixOf(const YCPPath& path) const
{
    if (length() > path->length()) return false;

    for (int c=0; c<length(); c++)
	if (component_str(c) != path->component_str(c)) return false;
    return true;
}


YCPPath
YCPPathRep::at(long index) const
{
    YCPPath postfix;
    for (int i=index; i<length(); i++)
	postfix->append(components[i]);
    return postfix;
}


string
YCPPathRep::component_str(long index) const
{
    return components[index].component.asString();
}


YCPOrder
YCPPathRep::compare(const YCPPath& p) const
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


string
YCPPathRep::toString() const
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


YCPValueType
YCPPathRep::valuetype() const
{
    return YT_PATH;
}


YCPPathRep::Component::Component(string s)
    : component (SymbolEntry::emptyUstring)
{
    string comp;
    if (s[0] == '"')
    {
	char num;
	complex = true;
	comp.erase();
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
			    comp+= '\\';
			    return;
			case '\\':   comp+= '\\';   break;
			case 'n':    comp+= '\n';   break;
			case 't':    comp+= '\t';   break;
			case 'r':    comp+= '\r';   break;
			case 'b':    comp+= '\b';   break;
			case 'f':    comp+= '\f';   break;
			case '"':    comp+= '"';    break;
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
				ycp2error("\\x00 not allowed in path constant. Skipping.");
			    comp+= num;
			    break;
			default:
			    comp+= *c;
			}
	    }
	    else if ('"' == *c)
	    {  // this must be " at the end. Return anyway to save some comparison
		break;
	    }
	    else
		comp+= *c;
	}
    }
    else
    {
	comp = s;
	complex = string::npos == comp.find_first_not_of ("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_-") ? false : true;
    }
    component = Ustring (*SymbolEntry::_nameHash, comp);
}


string
YCPPathRep::Component::toString() const
{
    if (!complex)
	return component.asString();
    string s = "\"";
    for (const char*c = component->c_str();*c;c++)
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
	    default:
	    {
		if (isprint (*c))
		    s+= *c;
		else
		{
		    char buf[5];
		    snprintf (buf, 4, "%02X", *(unsigned char*)c);
		    s += "\\x";
		    s += buf;
		}
	    }
	}
    }
    s+= '"';
    return s;
}


/**
 * Input value as bytecode from stream
 */


YCPPathRep::Component::Component(bytecodeistream & str)
    : component (SymbolEntry::emptyUstring)
{
    char v;
    if (str.get (v))
    {
	complex = (v != '\x00');
	component = Bytecode::readUstring (str);
    }
}


/**
 * Output value as bytecode to stream
 */

std::ostream &
YCPPathRep::Component::toStream (std::ostream & str) const
{
    str.put (complex ? '\x01' : '\x00');
    return Bytecode::writeUstring (str, component);
}

std::ostream &
YCPPathRep::toStream (std::ostream & str) const
{
#if 0
    if (Bytecode::writeInt32 (str, components.size()))
    {
	for (unsigned c = 0; c < components.size(); c++)
	{
	    if (!components[c].toStream (str))
		break;
	}
    }
    return str;
#else
    return Bytecode::writeString (str, toString());
#endif
}



// --------------------------------------------------------

static string
fromStream (bytecodeistream & str)
{
    string s;
    Bytecode::readString (str, s);
    return s;
}

YCPPath::YCPPath (bytecodeistream & str)
    : YCPValue (new YCPPathRep (fromStream(str).c_str()))
{
#if 0
    u_int32_t count = Bytecode::readInt32 (str);
    if (str.good())
    {
	YCPPathRep *p = new YCPPathRep ();
	for (unsigned c = 0; c < count; c++)
	{
	    p->append (YCPPathRep::Component (str));
	}
	if (str.good())
	    element = p;
    }
#endif
}

