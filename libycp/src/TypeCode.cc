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

   File:	TypeCode.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include "ycp/y2log.h"
#include "ycp/TypeCode.h"
#include "ycp/Bytecode.h"

#include <ctype.h>

UstringHash TypeCode::_nameHash;

/* unspecified type  */
const TypeCode TypeCode::Unspec = TypeCode ("");

/* any type  */
const TypeCode TypeCode::Any = TypeCode ("a");

/* void type  */
const TypeCode TypeCode::Void = TypeCode ("v");

/* integer type  */
const TypeCode TypeCode::Integer = TypeCode ("i");

/* float type  */
const TypeCode TypeCode::Float = TypeCode ("f");

/* string type  */
const TypeCode TypeCode::String = TypeCode ("s");

/* symbol type  */
const TypeCode TypeCode::Symbol = TypeCode ("y");

/* term type  */
const TypeCode TypeCode::Term = TypeCode ("t");

/* list type  */
const TypeCode TypeCode::List = TypeCode ("La");

/* map type  */
const TypeCode TypeCode::Map = TypeCode ("Ma");

/* function type  */
const TypeCode TypeCode::Function = TypeCode ("|");

TypeCode
TypeCode::makeBlock (const TypeCode &t)
{
    return newtype ("C", t);
}

TypeCode
TypeCode::makeList (const TypeCode &t)
{
    return newtype ("L", t);
}


TypeCode
TypeCode::makeMap (const TypeCode &t)
{
    return newtype ("M", t);
}


TypeCode
TypeCode::makeReference (const TypeCode &t)
{
    return newtype ("R", t);
}


TypeCode
TypeCode::makeFunction (const TypeCode &t)
{
    return newtype (TypeCode::Function, t);
}



static const char *
typenext (const char *t)
{
    y2debug ("typenext (%s)", t);
    while (strchr ("LMCY", *t) != 0)
    {
	t++;
    }
    y2debug ("-> (%s)", t+1);
    return t + 1;
}


//----------------------------------------------------------------...
// member functions

string
TypeCode::toString () const
{
    const char *c = asString().c_str();
    return typeToString (c);
}


string
TypeCode::toStringSequence () const
{
    const char *typep = asString().c_str ();
    string s;

    while (*typep)
    {
	s += typeToString (typep);
	if (*typep)
	{
	    s += ", ";
	}
    }
    return s;
}


TypeCode
TypeCode::vt2type (enum YCPValueType vt)
{
    // FIXME locale???
    switch (vt)
    {
	case YT_VOID:		return "v";
	case YT_BOOLEAN:	return "b";
	case YT_INTEGER:	return "i";
	case YT_FLOAT:		return "f";
	case YT_STRING :	return "s";
	case YT_BYTEBLOCK:	return "o";
	case YT_PATH:		return "p";
	case YT_SYMBOL:		return "y";
	case YT_LIST:		return "La";
	case YT_TERM:		return "t";
	case YT_MAP:		return "Ma";
	case YT_CODE:		return "C"; //FIXME "Ca"?
	case YT_ERROR:		return "E";
	case YT_ENTRY:		return "Y";
	default:
		return "";
    }

    return "?";
}

YCPValueType
TypeCode::valueType () const
{
    // why can we throw away typed compounds (Lx)?
    switch (asString()[0])
    {
	case 'v':	return YT_VOID;
	case 'b':	return YT_BOOLEAN;
	case 'i':	return YT_INTEGER;
	case 'f':	return YT_FLOAT;
	case 's':	return YT_STRING ;
	case 'o':	return YT_BYTEBLOCK;
	case 'p':	return YT_PATH;
	case 'y':	return YT_SYMBOL;
	case 'L':	return YT_LIST;
	case 't':	return YT_TERM;
	case 'M':	return YT_MAP;
	case 'C':	return YT_CODE;
	case 'E':	return YT_ERROR;
	case 'Y':	return YT_ENTRY;
	default:
	    break;
    }

    return YT_ERROR;
}


/**
 * Construct from a bytecode stream
 * @param istream
 */
TypeCode::TypeCode (std::istream & str)
    : Ustring (_nameHash, Bytecode::readCharp (str))
{
}


/*
 * t will point after the type code
 */

string
TypeCode::typeToString (const char * &t)
{
    string res;

    switch (*t++)
    {
	case 'a':	res = "any"; break;
	case 'A':	res = "any"; break; // same as the argument
	case 'b':	res = "boolean"; break;
	case 'f':	res = "float"; break;
	case 'i':	res = "integer"; break;
	case 'o':	res = "byteblock"; break;
	case 'p':	res = "path"; break;
	case 's':	res = "string"; break;
	case 't':	res = "term"; break;
	case 'v':	res = "void"; break;
	case 'w':	res = "..."; break;
	case 'y':	res = "symbol"; break;
	case '_':	res = "locale"; break;
	case 'C':
	{
	    res = "block(" + typeToString (t) + ')';
	}
	break;
	case 'L':
	{
	    if (*t == 'a')
	    {
		++t;
		res = "list";
	    }
	    else
	    {
		res = "list(" + typeToString (t) + ')';
	    }
	}
	break;
	case 'M':
	{
	    if (*t == 'a')
	    {
		++t;
		res = "map";
	    }
	    else
	    {
		res = "map(" + typeToString (t) + ')';
	    }
	}
	break;
	case 'Y':
	{
	    res = "entry(" + typeToString (t) + ")";
	}
	break;
	case 'R':
	{
	    // "Rxxx" -> "XXX&"
	    res = typeToString (t) + '&';
	}
	break;
	case '|':
	{
	    res = "**func**";
	}
	break;
	case '*': // for YEFunction::attachParameter, excess param
	{
	    res = "**nothing**";
	}
	break;
	case 0:
	{
	    res = "<unspec>";
	}
	break;
	default:
	{
	    res = "**???**";
	    y2error ("Unknown type [%s]", t-1);
	}
	break;
    }

    return res;
}


TypeCode
TypeCode::codify () const
{
    return isCode() ? *this : makeBlock (*this);
}


TypeCode
TypeCode::firstT () const
{
    const char *b = asString().c_str ();
    const char *e = typenext (b);
    return TypeCode (string (b, e - b));
}


TypeCode
TypeCode::next () const
{
    const char *b = asString().c_str ();
    const char *n = typenext (b);
    return TypeCode (n);
}


bool
TypeCode::isValidReturn () const
{
    if (isUnspec ())
    {
	return false;
    }
    char b = asString()[0];
    return isalpha (b) && b != 'w';
}


/**
 * write out to stream
 */

std::ostream & 
TypeCode::toStream (std::ostream & str) const
{
    return Bytecode::writeCharp (str, asString().c_str());
}


/**
 * Extracts the return type-code of a block or function
 * @param type a block (C) or function (|) type code
 */
TypeCode
TypeCode::returnType () const
{
    int pos = 0;
    while (asString()[pos] == 'C')
    {
	pos++;
    }
    // this is firstT()
    const char *b = asString().c_str () + pos;
    const char *e = typenext (b);
    y2debug ("returnType (%s) = %s", b, string (b, e - b).c_str());
    return TypeCode (string (b, e - b));
}


/**
 * Converts the return type-code of a block or function to YCP notation
 * Eg. "Ls|LLs" -> "list(string)"
 * @param type a block (C) or function (|) type code
 */
string
TypeCode::return2string () const
{
    const char *rt = returnType().asString().c_str ();
    string result = typeToString (rt);

    y2debug ("return2string(%s) = '%s'", asString().c_str (), result.c_str ());

    return result;
}


/**
 * Check for matching types
 * @return 0 for exact matches
 *	   >0 for propagated matches
 *	   <0 for no matches
 */

int
TypeCode::matchtype (const TypeCode & type) const
{
    int ret = 0;
    const char *given = asString().c_str();
    const char *expected = type.asString().c_str();
loop:
    y2debug ("matchtype (expect %s, have %s)", expected, given);
    while (*expected == *given)
    {
	if (*expected == 0)
	{
	    break;
	}
	expected++;
	given++;
    }

    switch (*expected)
    {
	case 'a':
	case 'A':
	{
	    ret = (*given != 0) ? 0 : -1;
	}
	break;
	case 0:
	{
	    // should not happen? exp: "L", giv: "Ls"
	    // what if exp: "i", giv: "i|ii"?
	    ret = (*given == 0) ? 0 : -1;
	}
	break;
	default:
	{
	    switch (*given)
	    {
		case 'a':
		{
		    ret = -1;		// but any doesn't propagate
		}
		break;
		case 'C':		// match return type of code
		{
		    // FIXME really? make optional?
		    // how about "R"?
		    given++;
		    goto loop;
		}
		break;
		case 'v':		// nil matches all
		{
		    ret = 0;
		}
		break;
		case 'i':		// int -> float
		{
		    ret = (*expected == 'f') ? 2 : -1;
		}
		break;
		case 'f':		// float -> int
		{
		    ret = (*expected == 'i') ? 1 : -1;
		}
		break;
		case '_':		// locale -> string
		{
		    ret = (*expected == 's') ? 1 : -1;
		}
		break;
		default:
		{
		    ret = -1;
		}
		break;
	    }
	}
	break;
    }
y2debug ("-> %d", ret);
    return ret;
}


/**
 * Finds a type that can hold both given types
 * This should be the narrowest such type - TODO
 * Mallocs the result
 */
TypeCode
TypeCode::commontype (const TypeCode & type) const
{
    if (matchtype (type) >= 0)
    {
	return asString();
    }
    else if (!isAny()
	     && type.matchtype (asString()) >= 0)
    {
	return type;
    }
    return "a";
}


/*
 * check parameter matches
 */

bool
TypeCode::matchParameters (const char * &dtype, const char * &atype)
{
    y2debug ("declared: '%s', actual '%s'", dtype, atype);

    if (*dtype == 'w')			// wildcard matches all
    {
	while (*atype)		// advance over remaining types (wildcard matches everything)
	{
	    ++atype;
	}
	++dtype;		// advance over just processed type spec
	return true;
    }
    else if (*atype != *dtype)		// type specs do not match
    {
	if (*atype == 0 || *dtype == 0)
	{
	    return false;
	}
	else if ((*dtype == 'a')
		 || (*dtype == 'A'))
	{
	    ++dtype;
	    atype = typenext (atype);
	    return true;
	}
	else if (*atype == 'v')		// any type allows 'nil'
	{
	    dtype = typenext (dtype);
	    ++atype;
	    return true;
	}
	else
	{
	    return false;
	}
    }
    else if (*atype != 0)		// *dtype is equal, type specs match
    {
	return matchParameters (++dtype, ++atype);
    }
    else
    {
	return true; //end of both types
    }
}

// EOF
