/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	TypeStatics.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

   static member functions

$Id$
/-*/

#include "ycp/y2log.h"
#include "ycp/Type.h"

#include <ctype.h>

bool Type::m_nocheck = false;

//----------------------------------------------------------------

void
Type::setNocheck (bool nocheck)
{
    m_nocheck = nocheck;
}


constTypePtr
Type::vt2type (enum YCPValueType vt)
{
    // FIXME locale???
    switch (vt)
    {
	case YT_VOID:		return Type::Void;
	case YT_BOOLEAN:	return Type::Boolean;
	case YT_INTEGER:	return Type::Integer;
	case YT_FLOAT:		return Type::Float;
	case YT_STRING :	return Type::String;
	case YT_BYTEBLOCK:	return Type::Byteblock;
	case YT_PATH:		return Type::Path;
	case YT_SYMBOL:		return Type::Symbol;
	case YT_LIST:		return Type::List();
	case YT_TERM:		return Type::Term;
	case YT_MAP:		return Type::Map();
	case YT_CODE:		return Type::Block();
	case YT_ENTRY:		return Type::Variable();
	default:
		return Type::Unspec;
    }

    return Type::Error;
}

/**
 * signature parser, get next token
 * >= 0 -> tkind
 * -1 -> const
 * -100  (-x) -> NFlex
 * '&' -> reference
 *  '<'
 *  '>'
 *  ','
 *  '('
 *  ')'
 */
int
Type::nextToken (const char **signature)
{
//    const char *signature_copy = *signature;

    while (isspace (**signature)) (*signature)++;

    int k = UnspecT;

    switch (**signature)
    {
	case 0:
	break;
	case 'a':	// any
	case 'A':
	{
	    if (strncasecmp (*signature, "any", 3) == 0)
	    {
		*signature += 3;
		k = AnyT;
	    }
	}
	break;
	case 'b':	// boolean, byteblock, block
	case 'B':
	{
	    if (strncasecmp (*signature, "boolean", 7) == 0)
	    {
		*signature += 7;
		k = BooleanT;
	    }
	    else if (strncasecmp (*signature, "byteblock", 9) == 0)
	    {
		*signature += 9;
		k = ByteblockT;
	    }
	    else if (strncasecmp (*signature, "block", 5) == 0)
	    {
		*signature += 5;
		k = BlockT;
	    }
	}
	break;
	case 'c':	// const
	case 'C':
	{
	    if (strncasecmp (*signature, "const", 5) == 0)
	    {
		*signature += 5;
		k = -1;
	    }
	}
	break;
	case 'f':	// float, flex
	case 'F':
	{
	    if (strncasecmp (*signature, "float", 5) == 0)
	    {
		*signature += 5;
		k = FloatT;
	    }
	    else if (strncasecmp (*signature, "flex", 4) == 0)
	    {
		*signature += 4;
		k = 0;
		while (isdigit (**signature))
		{
		    k *= 10;
		    k += (**signature - '0');
		    *signature += 1;
		}
		if (k == 0)
		{
		    k = FlexT;
		}
		else
		{
		    k = -100 - k;
		}
	    }
	}
	break;
	case 'i':	// integer
	case 'I':
	{
	    if (strncasecmp (*signature, "integer", 7) == 0)
	    {
		*signature += 7;
		k = IntegerT;
	    }
	}
	break;
	case 'l':	// list, locale
	case 'L':
	{
	    if (strncasecmp (*signature, "list", 4) == 0)
	    {
		*signature += 4;
		k = ListT;
	    }
	    else if (strncasecmp (*signature, "locale", 6) == 0)
	    {
		*signature += 6;
		k = LocaleT;
	    }
	}
	break;
	case 'm':	// map
	case 'M':
	{
	    if (strncasecmp (*signature, "map", 3) == 0)
	    {
		*signature += 3;
		k = MapT;
	    }
	}
	break;
	case 'p':	// path
	case 'P':
	{
	    if (strncasecmp (*signature, "path", 4) == 0)
	    {
		*signature += 4;
		k = PathT;
	    }
	}
	break;
	case 's':	// static, string, symbol
	case 'S':
	{
	    if (strncasecmp (*signature, "string", 6) == 0)
	    {
		*signature += 6;
		k = StringT;
	    }
	    else if (strncasecmp (*signature, "symbol", 6) == 0)
	    {
		*signature += 6;
		k = SymbolT;
	    }
	}
	break;
	case 't':	// term, tuple
	case 'T':
	{
	    if (strncasecmp (*signature, "term", 4) == 0)
	    {
		*signature += 4;
		k = TermT;
	    }
	    else if (strncasecmp (*signature, "tuple", 5) == 0)
	    {
		*signature += 5;
		k = TupleT;
	    }
	}
	break;
	case 'v':	// void, variable
	case 'V':
	{
	    if (strncasecmp (*signature, "void", 4) == 0)
	    {
		*signature += 4;
		k = VoidT;
	    }
	    else if (strncasecmp (*signature, "variable", 8) == 0)
	    {
		*signature += 8;
		k = VariableT;
	    }
	}
	break;
	case '.':	// wildcard
	{
	    if (strncasecmp (*signature, "...", 3) == 0)
	    {
		*signature += 3;
		k = WildcardT;
	    }
	}
	break;
	case '&':
	case '<':
	case '>':
	case ',':
	case '(':
	case ')':
	{
	    k = **signature;
	    (*signature)++;
	}
	break;
	default:
	{
	    k = ErrorT;
	}
	break;
    }

    while (isspace (**signature)) (*signature)++;

//    y2debug ("nextToken (%s) = %d [%s]", signature_copy, k, *signature);
    return k;
}

//----------------------------------------------------------------

/**
 * Construct from a string literal type code
 * @param s eg. "list <string>"
 */

TypePtr
Type::fromSignature (const char ** signature)
{
    if ((signature == 0)
	|| (*signature == 0))
    {
	return 0;
    }
    if (**signature == 0)
    {
	return TypePtr (new Type(UnspecT));
    }

    TypePtr t = 0;

    bool as_const = false;

    const char *signature_copy = *signature;
    const char *signature_start = *signature;

//    y2debug ("Type::fromSignature (\"%s\")\n", *signature);

    int k = nextToken (signature);

    if (k == -1)		// const
    {
//	y2debug ("Const !");
	as_const = true;
	signature_copy = *signature;
	k = nextToken (signature);
    }

    char next = 0;

    switch (k)
    {
	case AnyT:		t = TypePtr (new Type (AnyT)); break;
	case BooleanT:		t = TypePtr (new Type (BooleanT)); break;
	case ByteblockT:	t = TypePtr (new Type (ByteblockT)); break;
	case ErrorT:		t = TypePtr (new Type (ErrorT)); break;
	case FlexT:		t = FlexTypePtr (new FlexType ()); break;
	case FloatT:		t = TypePtr (new Type (FloatT)); break;
	case IntegerT:		t = TypePtr (new Type (IntegerT)); break;
	case LocaleT:		t = TypePtr (new Type (LocaleT)); break;
	case PathT:		t = TypePtr (new Type (PathT)); break;
	case StringT:		t = TypePtr (new Type (StringT)); break;
	case SymbolT:		t = TypePtr (new Type (SymbolT)); break;
	case TermT:		t = TypePtr (new Type (TermT)); break;
	case VoidT:		t = TypePtr (new Type (VoidT)); break;
	case WildcardT:		t = TypePtr (new Type (WildcardT)); break;

	// codes
	case VariableT:		k = VariableT; next = '<'; break;
	case '(':		k = FunctionT; break;

	// constructors
	case BlockT:		k = BlockT; next = '<'; break;
	case ListT:		k = ListT; next = '<'; break;
	case MapT:		k = MapT; next = '<'; break;
	case TupleT:		k = TupleT; next = '<'; break;
	default:
	    if (k < -100)
	    {
		t = NFlexTypePtr (new NFlexType (-(k+100)));
	    }
	    else
	    {
		y2error ("Builtin signature code %d [%s] not handled\n", **signature, signature_start);
	    }
	break;
    }

    signature_copy = *signature;

    if (next != 0)
    {
	if (nextToken (signature) != next)
	{
	    y2error ("Expecting '%c' at '%s' [Complete signature is '%s']\n", next, signature_copy, signature_start);
	    return 0;
	}
    }

    if (t == 0)			// no base type yet
    {
//	y2debug ("k %d, signature '%s'", k, *signature);
	if (k == UnspecT)	// no nothing
	{
	    y2error ("Unknown type at '%s' [Complete signature is '%s']!\n", signature_copy, signature_start);
	    return 0;
	}

	signature_copy = *signature;
	TypePtr t1 = fromSignature (signature);

	if (t1 == 0)
	{
	    y2error ("Unknown type at '%s' [Complete signature is '%s']!\n", signature_copy, signature_start);
	    return 0;
	}
//	y2debug ("t1 '%s', k %d, signature '%s'", t1->toString().c_str(), k, *signature);

	switch (k)
	{
	    case VariableT: t = VariableTypePtr (new VariableType (t1)); next = '>'; break;
	    case BlockT:    t = BlockTypePtr (new BlockType (t1)); next = '>'; break;
	    case ListT:	    t = ListTypePtr (new ListType (t1)); next = '>'; break;
	    case MapT:
	    {
		signature_copy = *signature;

		if (nextToken (signature) != ',')
		{
		    y2error ("Expected ',' at '%s' [Complete signature is '%s']\n", signature_copy, signature_start);
		    return 0;
		}

		TypePtr t2 = fromSignature (signature);
		if (t2 == 0)
		{
		    y2debug ("Unknown type at '%s' [Complete signature is '%s']!\n", signature_copy, signature_start);
		    return 0;
		}

		t = MapTypePtr (new MapType (t1, t2));
		next = '>'; 
	    }
	    break;
	    case TupleT:    t = TupleTypePtr (new TupleType (t1)); next = 0; break;
	    case FunctionT: t = FunctionTypePtr (new FunctionType (t1)); next = 0; break;
	    default:
		y2error ("Post-Kind '%d'[%c] not handled\n", k, isprint (k) ? k : '?');
		return 0;
	    break;
	}

	if (next != 0)
	{
	    if (nextToken (signature) != next)
	    {
		y2error ("Expected '%c' at '%s' [Complete signature is '%s']\n", next, signature_copy, signature_start);
		return 0;
	    }
	}
    }

    if (t == 0)
    {
	return 0;
    }

//    y2debug ("t '%s', signature '%s', k %d", t->toString().c_str(), *signature, k);

    if (as_const) t->asConst();

    signature_copy = *signature;
    if (**signature == '&')
    {
	t->asReference();
	do { (*signature)++; } while (isspace (**signature));
    }

    // check for function, it's postfix !

    if (**signature == '(')
    {
//	y2debug ("function!");
	FunctionTypePtr f (new FunctionType (t));

	do
	{
	    do { (*signature)++; } while (isspace (**signature));
	    if (**signature == ')')
	    {
		break;
	    }
	    signature_copy = *signature;
	    TypePtr t1 = fromSignature (signature);
	    if (t1 == 0)
	    {
		*signature = signature_copy;
		break;
	    }

	    f->concat (t1);
	}
	while (**signature == ',');

	if (**signature != ')')
	{
	    y2error ("Expected ')' at '%s' [Complete signature is '%s']\n", signature_copy, signature_start);
	    return 0;
	}

	(*signature)++;
	t = f;
    }
    else if (k == TupleT)
    {
	TupleTypePtr tt = t;
//	y2debug ("tuple! '%s', signature '%s'", tt->toString().c_str(), *signature);
	while (**signature == ',')
	{
	    (*signature)++;
	    signature_copy = *signature;
	    TypePtr t1 = fromSignature (signature);
	    if (t1 == 0)
	    {
		y2error ("Unknown type at '%s' [Complete signature is '%s']!\n", signature_copy, signature_start);
		return 0;
	    }

	    tt->concat (t1);
	}
	if (**signature != '>')
	{
	    y2error ("Expected '>' at '%s' [Complete signature is '%s']\n", signature_copy, signature_start);
	    return 0;
	}
	t = tt;
    }

//    y2debug ("Type::fromSignature Done: '%s' -> '%s' [%s]\n", signature_start, t->toString().c_str(), *signature);

    return t;
}



//----------------------------------------------------------------

/**
 * determine actual type if declared type contains flex type
 * Returns actual - unchanged or fixed
 * @param symbol type of a symbol parameter from YEBuiltin, else isUnspec
 */
FunctionTypePtr
Type::determineFlexType (constFunctionTypePtr actual, constFunctionTypePtr declared, constTypePtr symboltype)
{
    y2debug ("determineFlexType (actual %s, declared %s, symboltype %s)", actual->toString().c_str(), declared->toString().c_str(), symboltype->toString().c_str());

    // if builtin decl returns 'flex', the parameter deduces the return type

    FunctionTypePtr result;
    y2debug ("ANY type check (declared '%s', actual '%s'!", declared->toString().c_str(), actual->toString().c_str());

    if (declared->parameterCount() <= 0)
    {
	ycp2error ("declared->parameterCount() <= 0");
	return Type::Error->clone();
    }

    unsigned int flexnumber = 0;

    constTypePtr flextype;
    do
    {
	flextype = declared->matchFlex (actual, flexnumber);

	y2debug ("flextype %d:'%s'", flexnumber, flextype == 0 ? "NONE" : flextype->toString().c_str());

	if (flextype == 0)
	{
	    result = declared->clone();
	}
	else
	{
	    // exchange <flex> with the correct type
	    result = declared->unflex (flextype, flexnumber);
	    declared = result;
	}
    }
    while ((flexnumber++ == 0) || (flextype != 0));
#if 0
    if (actual->match (result) != 0)
    {
	ycp2error ("'%s' does not match '%s'", actual->toString().c_str(), result->toString().c_str());
	result = Type::Error->clone();
    }
#endif
    y2debug ("determineFlexType returns '%s'", result->toString().c_str());
    return result;
}


// EOF
