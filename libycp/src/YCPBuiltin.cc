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

   File:       YCPBuiltin.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * YCPBuiltin data type
 *
 */

#include "y2log.h"
#include "ycp/YCPBuiltin.h"
#include "ycp/YCPTerm.h"		// for YCPB_BRACKET
#include "ycp/YCPString.h"		// for YCPB_TEXTDOMAIN


// YCPBuiltinRep
YCPBuiltinRep::YCPBuiltinRep(const builtin_t b)
    : code(b)
{
}


YCPBuiltinRep::YCPBuiltinRep(const builtin_t b, const YCPValue& v)
    : code(b)
{
    l = YCPList();
    l->add (v);
}


YCPBuiltinRep::YCPBuiltinRep(const builtin_t b, const YCPList& l)
    : code(b)
    , l(l)
{
}


// get symbol of builtin
builtin_t YCPBuiltinRep::builtin_code() const
{
    return code;
}


// get list of builtin
YCPList YCPBuiltinRep::args() const
{
    return l;
}


//compare builtins
YCPOrder YCPBuiltinRep::compare(const YCPBuiltin& t) const
{
    YCPOrder order = YO_EQUAL;

    if (code < t->code)
	order = YO_LESS;
    else if (code > t->code)
	order = YO_GREATER;

    if ( order == YO_EQUAL ) order = l->compare( t->l );

    return order;
}


// get the builtin as string
string YCPBuiltinRep::toString() const
{
    struct b2s_t { char *s; int count; };

    /* array of string representations of builtin codes
     * count determines the number of arguments
     *	-1:	unknown
     *	0:	any number, string is prefix to (<args>)
     *	1:	prefix operator
     *	2:	infix operator
     */

    static const b2s_t b2s[] =
    {
	{ "**unknown**", 0 }, 	/* 0 YCPB_UNKNOWN, */
	{ "``", 1 }, 		/* 1 YCPB_DEEPQUOTE */
	{ "// localdomain", 1 },/* 2 YCPB_LOCALDOMAIN */
	{ "is", 0 }, 		/* 3 YCPB_IS */
	{ "+", 2 }, 		/* 4 YCPB_PLUS */
	{ "-", 2 }, 		/* 5 YCPB_MINUS */
	{ "*", 2 }, 		/* 6 YCPB_MULT */
	{ "/", 2 }, 		/* 7 YCPB_DIV */
	{ "%", 2 }, 		/* 8 YCPB_MOD */
	{ "-", 1 }, 		/* 9 YCPB_NEG */
	{ "&", 2 }, 		/* 10 YCPB_AND */
	{ "|", 2 }, 		/* 11 YCPB_OR */
	{ "~", 1 }, 		/* 12 YCPB_BNOT */
	{ "&&", 2 }, 		/* 13 YCPB_LOGAND */
	{ "||", 2 },	 	/* 14 YCPB_LOGOR */
	{ "!", 1 }, 		/* 15 YCPB_NOT */
	{ "==", 2 },	 	/* 16 YCPB_EQ */
	{ "<", 2 }, 		/* 17 YCPB_ST */
	{ ">", 2 }, 		/* 18 YCPB_GT */
	{ "<=", 2 },	 	/* 19 YCPB_SE */
	{ ">=", 2 },	 	/* 20 YCPB_GE */
	{ "!=", 2 }, 		/* 21 YCPB_NEQ */
	{ "define ", 2 },	/* 22 YCPB_LOCALDEFINE */
	{ "", 3 }, 		/* 23 YCPB_LOCALDECLARE */
	{ "global define ", 2 }, /* 24 YCPB_GLOBALDEFINE */
	{ "global ", 3 },	/* 25 YCPB_GLOBALDECLARE */
	{ "size", 0},		/* 26 YCPB_SIZE */
	{ "lookup", 0},		/* 27 YCPB_LOOKUP */
	{ "select", 0},		/* 28 YCPB_SELECT */
	{ "remove", 0},		/* 29 YCPB_REMOVE */
	{ "foreach", 0},	/* 30 YCPB_FOREACH */
	{ "eval", 0},		/* 31 YCPB_EVAL */
	{ "symbolof", 0},	/* 32 YCPB_SYMBOLOF */
	{ "_callback", 1 },	/* 33 YCPB_CALLBACK */
	{ "_dump_scope", 0 },	/* 34 YCPB_DUMPSCOPE */
	{ "_dump_meminfo", 0 }, /* 35 YCPB_MEMINFO */
	{ "include", 1 }, 	/* 36 YCPB_INCLUDE */
	{ "module", 1 }, 	/* 37 YCPB_MODULE */
	{ "import", 1 }, 	/* 38 YCPB_IMPORT */
	{ "export", 1 }, 	/* 39 YCPB_EXPORT */
	{ "textdomain", 1 }, 	/* 40 YCPB_TEXTDOMAIN */
	{ "undefine", 1 }, 	/* 41 YCPB_UNDEFINE */
	{ "_fullname", 1 }, 	/* 42 YCPB_FULLNAME */
	{ "isnil", 1 }, 	/* 43 YCPB_ISNIL */
	{ "triple", 3 }, 	/* 44 YCPB_TRIPLE */
	{ "union", 0 }, 	/* 45 YCPB_UNION */
	{ "add", 0 },	 	/* 46 YCPB_ADD */
	{ "change", 0 },	/* 47 YCPB_CHANGE */
	{ "merge", 0 },		/* 48 YCPB_MERGE */
	{ "nisnil", 1 }, 	/* 49 YCPB_NISNIL */
	{ "WFM::", 1 },		/* 50 YCPB_WFM */
	{ "SCR::", 1 },		/* 51 YCPB_SCR */
	{ "UI::", 1 },		/* 52 YCPB_UI */
	{ "<<", 2 },		/* 53 YCPB_LEFT */
	{ ">>", 2 },		/* 54 YCPB_RIGHT */
	{ "nlocale", 3 },	/* 55 YCPB_NLOCALE */
	{ "[]", 101 },		/* 56 YCPB_BRACKET */
	{ "[]=", 102 },		/* 57 YCPB_BASSIGN */
	{ "//gettextdomain", 0 },/* 58 YCPB_TEXTDOMAIN */
	{ "sort", 0 },		/* 59 YCPB_SORT */
	{ "lsort", 0 },		/* 60 YCPB_LSORT */
	{ "", 2 } 		/* 61 YCPB_ASSIGN */
    };

    if (code > YCPB_ASSIGN)
    {
	y2error ("Bad code !\n");
	return "**BAD[??]**";
    }

    const b2s_t *b2sp = &b2s[(int)code];
    string sb2sp = string (b2sp->s);

    if (b2sp->count == 0)
    {
	return sb2sp
		+ " ("
		+ l->commaList()
		+ ")";
    }
    else if ((b2sp->count < 100)
	     && (b2sp->count != l->size()))
    {
	y2error("Wrong number of arguments (%d) for builtin [%s:%d]\n", l->size(), b2sp->s, b2sp->count);
	return "**BAD[" + string(b2sp->s) + "]**";
    }

    if (b2sp->count == 1)		// unary, prefix
    {
	switch (code)
	{
	    case YCPB_WFM:
	    case YCPB_SCR:
	    {
		return sb2sp + value(0)->toString();
	    }
	    break;
	    case YCPB_UI:
	    {
		if (value(0)->isBuiltin()
		    && value(0)->asBuiltin()->builtin_code() == YCPB_GLOBALDEFINE)
		{
		    return "UI::{ " + value(0)->toString() + "}";
		}
		return sb2sp + value(0)->toString();
	    }
	    break;
	    case YCPB_DEEPQUOTE:
	    {
		switch (value(0)->valuetype())
		{
		    case YT_BLOCK:
			return sb2sp
			    + value(0)->toString();
		    break;
		    default:
			return sb2sp
			    + "(" + value(0)->toString() + ")";
		}
	    }
	    break;
	    case YCPB_TEXTDOMAIN:
	    {
		if (value(0)->isString()
		    && !(value(0)->asString()->value().empty()))
		    return sb2sp + " " + value(0)->toString() + ";";
		else
		    return sb2sp + " (" + value(0)->toString() + ")";
	    }
	    break;
	    case YCPB_LOCALDOMAIN:
	    case YCPB_MODULE:
	    case YCPB_IMPORT:
	    case YCPB_FULLNAME:
	    case YCPB_INCLUDE:
	    {
		return sb2sp + " "
		    + value(0)->toString() + ";";
	    }
	    break;
	    case YCPB_EXPORT:
	    case YCPB_UNDEFINE:
	    {
		return sb2sp + " "
		    + value(0)->asList()->commaList() + ";";
	    }
	    break;
	    case YCPB_ISNIL:
	    {
		return value(0)->toString() + " == nil";
	    }
	    break;
	    case YCPB_NISNIL:
	    {
		return value(0)->toString() + " != nil";
	    }
	    break;
	    default:
	    {
		return sb2sp
		    + " ("
		    + value(0)->toString()
		    + ")";
	    }
	    break;
	}
    }
    else if (b2sp->count == 2)   // binary builtin
    {
	switch (code)
	{
	    case YCPB_LOCALDEFINE:
	    {
		return sb2sp
		    + value(0)->toString()
		    + " "
		    + value(1)->toString();
	    }
	    break;
	    case YCPB_GLOBALDEFINE:
	    {
		return sb2sp
		    + value(0)->toString()
		    + " "
		    + value(1)->toString();
	    }
	    break;
	    case YCPB_ASSIGN:
	    {
		return value(0)->toString()
		    + " = ("
		    + value(1)->toString()
		    + ")";
	    }
	    break;
	    default:
	    {
		return "("
		    + value(0)->toString()
		    + " " + b2sp->s + " "
		    + value(1)->toString()
		    + ")";
	    }
	    break;
	}
    }
    else if (b2sp->count == 3)   // ternary builtin
    {
	switch (code)
	{
	    case YCPB_TRIPLE:
	    {
	        return value(0)->toString()
		    + " ? "
		    + value(1)->toString()
	            + " : "
	            + value(2)->toString();

	    }
	    break;
	    case YCPB_GLOBALDECLARE:
	    {
		return sb2sp
		    + value(0)->toString()
		    + " "
		    + value(1)->toString()
		    + " = "
		    + value(2)->toString();
	    }
	    case YCPB_LOCALDECLARE:
	    {
		return value(0)->toString()
		    + " "
		    + value(1)->toString()
		    + " = "
		    + value(2)->toString();
	    }

	    case YCPB_NLOCALE:
	    {
		return "_(" + value (0)->toString ()
		    + ", " + value (1)->toString ()
		    + ", " + value (2)->toString () + ")";
	    }

	    default:
	    {
		return "("
		    + value(0)->toString()
		    + " " + b2sp->s + " "
		    + value(1)->toString()
	            + " " + b2sp->s + " "
	            + value(2)->toString()
		    + ")";
	    }
	    break;
	}
    }
    else if (b2sp->count == 101)   // YCPB_BRACKET
    {
	string s = value(0)->toString();
	s += "[";
	s += value(1)->asList()->commaList();
	s += "]";
	if (l->size() > 2)		// default
	{
	    s += ":";
	    s += l->value(2)->toString();
	}
	return s;
    }
    else if (b2sp->count == 102)   // YCPB_BASSIGN
    {
	string s = value(0)->toString();
	s += "[";
	s += value(1)->asList()->commaList();
	s += "] = ";
	s += value(2)->toString();
	return s;
    }
    return "**UNKNOWN**";
}


// test if the builtin's list is empty
bool YCPBuiltinRep::isEmpty() const
{
  return l->isEmpty();
}


// get the size of the builtin's list
int YCPBuiltinRep::size() const
{
    return l->size();
}


// Reserves a number of elements in the builtin's list.
void YCPBuiltinRep::reserve (int size)
{
    l->reserve (size);
}


// get the n-th element of the builtin's list
YCPValue YCPBuiltinRep::value(int n) const
{
    return l->value(n);
}


// add an element to the builtin's list
void YCPBuiltinRep::add(const YCPValue& value)
{
    l->add(value);
}


YCPBuiltin YCPBuiltinRep::functionalAdd (const YCPValue& val, bool prepend) const
{
#warning TODO: implement this better. Avoid duplicating the list.
    YCPBuiltin newbuiltin (code);
    YCPList newlist;
    newlist->reserve (size() + 1);
    if (prepend) newlist->add (val);
    for (int i=0; i<size(); i++)
    {
	newlist->add (value(i));
    }
    if (!prepend) newlist->add (val);
    newbuiltin->l = newlist;
    return newbuiltin;
}


YCPValueType YCPBuiltinRep::valuetype() const
{
    return YT_BUILTIN;
}
