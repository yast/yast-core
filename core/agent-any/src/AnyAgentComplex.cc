/**
 *
 *  AnyAgentComplex.cc
 *
 *  Purpose:	complex expression data parse for AnyAgent
 *
 *  Creator:	kkaempf@suse.de
 *  Maintainer:	kkaempf@suse.de
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <stack>

#include "AnyAgent.h"
#include <ycp/y2log.h>


#define KEY4FILLUP "`FILLUP"


// current match of a Choice()

static YCPValue currentMatch = YCPNull ();


// lineNumber
// return number of YCPListRep element returned by getLine ()

int
AnyAgent::lineNumber () const
{
    return line_number;
}


// getLine
// return next valid data from YCPListRep (YCPStringRep)
//
// discard comment lines

char const *
AnyAgent::getLine ()
{
    static bool got_anything = false;

    string s;
    char const *cs;

    if (alldata.isNull ())
	return 0;

    line_number++;

    if (line_number >= alldata->size ())	// eof
    {
	y2debug ("getLine eof %d of %d", line_number, alldata->size ());
	if (!got_anything)
	{
	    got_anything = true;
	    return "";
	}
	got_anything = false;
	return 0;
    }

    s = alldata->value (line_number)->asString ()->value ();

    if ((mComment.find_first_of (s[0]) != string::npos)	// skip comment lines
	&&(!isFillup))
    {
	return getLine ();
    }

    y2debug ("getLine (%s)", s.c_str ());
    cs = alldata->value (line_number)->asString ()->value ().c_str ();
    if (*cs == 0)
	return getLine ();
    got_anything = true;
    return cs;
}


// putLine
//

const string
AnyAgent::putLine (const string s)
{
    return "";
}


// parseChoice
//   parse one of many

YCPValue
AnyAgent::parseChoice (char const *&line, const YCPList & syntax, bool optional)
{
    if ((line == 0) || syntax.isNull ())
	return YCPNull ();

    y2debug ("parseChoice ('%s',[%s])", line, syntax->toString ().c_str ());

    for (int i = 0; i < syntax->size (); i++)
    {
	YCPValue v = syntax->value (i);
	if (v.isNull ())
	{
	    y2error ("Bad element in Choice()");
	    return YCPNull ();
	}
	if (!v->isList ())
	{
	    y2error ("Choice element must be list");
	    return YCPNull ();
	}

	YCPList element = v->asList ();
	if ((element->size () <= 0) || (element->size () > 2))
	{
	    y2error ("Choice element list must have 1 or 2 entries");
	    return YCPNull ();
	}

	YCPValue match = element->value (0);

	// get optional action

	YCPValue action = (element->size () > 1) ? element->value (1) : YCPNull ();

	// force match
	y2debug ("choice (%d)", i);

	char const *try_line = line;

	currentMatch = parseData (try_line, match, false);

	// have match
	if (!currentMatch.isNull ())
	{
	    line = try_line;
	    y2debug ("choice (%d) match", i);
	    if (!action.isNull ())
		return parseData (line, action, optional);
	    else
		return currentMatch;
	}
    }

    return YCPNull ();
}


// unparseChoice
//

const string
AnyAgent::unparseChoice (const YCPList & syntax, const YCPValue & value)
{
    return "";
}


// parseSequence
//   parse all of many

YCPValue
AnyAgent::parseSequence (char const *&line, const YCPList & syntax,
			 bool optional)
{
    YCPValue element = YCPNull ();
    if (syntax.isNull ())
	return element;

    y2debug ("parseSequence ('%s',[%s])", line, syntax->toString ().c_str ());

    char const *lstart = line;

    for (int i = 0; i < syntax->size (); i++)
    {
	element = syntax->value (i);

	element = parseData (line, element, optional);

	if (element.isNull ())
	{
	    lstart = 0;
	    break;
	}
    }
    if (lstart != 0)
	return YCPString (string (lstart, line - lstart));
    return element;
}


// unparseSequence
//

const string
AnyAgent::unparseSequence (const YCPList & syntax, const YCPValue & value)
{
    return "";
}


// parseList
//

YCPValue
AnyAgent::parseList (char const *&line, const YCPList & syntax, bool optional)
{
    if (syntax.isNull ())
	return YCPNull ();

    y2debug ("parseList ('%s',[%s])", line, syntax->toString ().c_str ());

    YCPList list;

    for (;;)
    {
	// value of line
	YCPValue vl = parseData (line, syntax->value (0), optional);
	optional = false;
	if (vl.isNull ())
	    break;
	if (!vl->isVoid ())
	    list->add (vl);
	y2debug ("vl (%s)", vl->toString ().c_str ());
	// value of separator/string
	YCPValue vs = parseData (line, syntax->value (1), false);
	if (vs.isNull ())
	    break;
    }

    y2debug ("list (%s)", list->toString ().c_str ());
    return list;
}


// unparseList
//

const string
AnyAgent::unparseList (const YCPList & syntax, const YCPValue & value)
{
    if (syntax.isNull () || value.isNull ())
	return "";

    y2debug ("unparseList ('%s',%s)", syntax->toString ().c_str (),
	     value->toString ().c_str ());

    if (!value->isList ())
    {
	y2error ("unparseList: value has wrong type");
	return "";
    }
    YCPList list = value->asList ();

    string s;
    int lsize = list->size ();

    y2debug ("unparseList (%d: '%s')", lsize, list->toString ().c_str ());

    if (lsize > 0)
    {
	for (int i = 0; i < lsize; i++)
	{
	    const string data = unparseData (syntax->value (0), list->value (i));
	    if (data.empty ())
		break;
	    string cont;
	    if (i < lsize - 1)
	    {
		const YCPValue & element = syntax->value (1);
		cont = unparseData (element, YCPNull ());
		if (cont.empty ())
		{
		    if (element->valuetype () != YT_TERM)
			break;
		    else
		    {
			YCPTerm term = element->asTerm ();
			if ((term.isNull ())
			    || ((term->name () != "Skip")
				&& (term->name () !=
				    "Fillup")))
			{
			    break;
			}
		    }
		}
	    }
	    else		// no continuation after last list element
		cont = "";

	    s += data;
	    s += cont;
	}
    }

    return s;
}


// parseTuple
//

YCPValue
AnyAgent::parseTuple (char const *&line, const YCPList & syntax, bool optional)
{
    if (syntax.isNull ())
	return YCPNull ();

    if (line == 0)
	return YCPNull ();

    y2debug ("parseTuple (%s,%s)", line, syntax->toString ().c_str ());
    YCPMap map;

    tupleContinue = false;

    for (int i = 0; i < syntax->size (); i++)
    {
	YCPValue element_syntax = syntax->value (i);

	if (parseData (line, element_syntax, optional).isNull ())
	{
	    if (!optional)
		return YCPNull ();
	    break;
	}

	y2debug ("tuple name %ld, value %ld", (long) tupleName.size (),
		 (long) tupleValue.size ());

	if (tupleName.size () > 0 && tupleName.top () != "" &&
	    tupleValue.size () > 0 && !tupleValue.top ().isNull () &&
	    (!tupleValue.top ()->isVoid ()))
	{
	    y2debug ("map add %s:%s", tupleName.top ().c_str (),
		     tupleValue.top ()->toString ().c_str ());
	    map->add (YCPString (tupleName.top ()), tupleValue.top ());
	    tupleName.top () = "";
	    tupleValue.top () = YCPNull ();
	}

	if (tupleContinue && (i == (syntax->size () - 1)))
	{
	    tupleContinue = false;
	    i = -1;		// restart at 0
	    optional = true;	// it's optional from now on
	}

    }

    y2debug ("tuple (%s)", map->toString ().c_str ());
    return map;
}


// unparseTuple
//

const string
AnyAgent::unparseTuple (const YCPList & syntax, const YCPValue & value)
{
    if (syntax.isNull () || value.isNull ())
	return "";

    y2debug ("unparseTuple ('%s',%s)", syntax->toString ().c_str (),
	     value->toString ().c_str ());
    if (!value->isMap ())
    {
	y2error ("unparseTuple: value has wrong type");
	return "";
    }

    int ssize = syntax->size ();
    string s;

    // if Fillup allowed and defined, initialize s with it

    if (isFillup)
    {
	YCPMap map = value->asMap ();
	YCPValue fillup = map->value (YCPString (KEY4FILLUP));
	if ((!fillup.isNull ()) && fillup->isString ())
	    s += fillup->asString ()->value ();
    }

    for (int i = 0; i < ssize; i++)
    {
	const YCPValue & element = syntax->value (i);
	const string data = unparseData (element, value);
	if (data.empty ())
	{
	    y2debug ("unparseTuple fail ? ('%s',%s)",
		     element->toString ().c_str (), value->toString ().c_str ());
	    if (element->valuetype () == YT_TERM)
	    {
		YCPTerm term = element->asTerm ();
		if (!(term.isNull ()) && 
		    ((term->name () == "Skip") ||
		     (term->name () == "Fillup")))
		    continue;
	    }
	    break;
	}
	s += data;
    }

    return s;
}


// parseData
// toplevel parsing function

YCPValue
AnyAgent::parseData (char const *&line, const YCPValue & syntax, bool optional)
{
    if ((line == 0) || (*line == 0))
	line = getLine ();
    if (line == 0)
	return YCPNull ();
    if (syntax.isNull ())
	return YCPNull ();

    y2debug ("parseData %s('%s',[%s])", (optional ? "?" : "!"), line,
	     syntax->toString ().c_str ());

    switch (syntax->valuetype ())
    {
	case YT_TERM: {

	    YCPTerm term = syntax->asTerm ();
	    const string s = term->name ();
	    y2debug ("YT_TERM (%s)", s.c_str ());

	    // Optional

	    if (s == "Optional" && term->size () > 0)
	    {
		YCPValue ov = parseData (line, term->value (0), true);
		if (ov.isNull ())
		    ov = YCPVoid ();
		return ov;
	    }

	    // Continue

	    else if (s == "Continue" && term->size () > 0)
	    {
		YCPValue tv = parseData (line, term->value (0), false);
		if (!tv.isNull ())
		    tupleContinue = true;
		return tv;
	    }

	    // Choice

	    else if (s == "Choice" && term->size () > 0)
	    {
		return parseChoice (line, term->args (), optional);
	    }

	    // Sequence

	    else if (s == "Sequence" && term->size () > 0)
	    {
		return parseSequence (line, term->args (), optional);
	    }

	    // List

	    else if (s == "List" && term->size () == 2)
	    {
		return parseList (line, term->args (), optional);
	    }

	    // Tuple

	    else if (s == "Tuple" && term->size () > 0)
	    {
		tupleName.push ("");
		tupleValue.push (YCPNull ());
		YCPValue tv = parseTuple (line, term->args (), optional);
		tupleName.pop ();
		tupleValue.pop ();
		return tv;
	    }

	    // Var

	    else if (s == "Var" && term->size () > 0)
	    {
		for (int i = 0; i < term->size (); i++)
		{
		    if (parseData (line, term->value (i), optional).isNull ())
			break;
		}
		return YCPVoid ();
	    }

	    // Name

	    else if (s == "Name" && term->size () == 1)
	    {
		if (!mReadOnly)
		{
		    y2error ("'Name' not allowed for writable agents");
		    return YCPNull ();
		}
		YCPValue tn = parseData (line, term->value (0), false);
		if (!tn.isNull () && tupleName.size () > 0)
		{
		    if (tn->isString ())
			tupleName.top () = tn->asString ()->value ();
		    else
			tupleName.top () = tn->toString ();
		    y2debug ("Name: %s", tn->toString ().c_str ());
		}
		return tn;
	    }

	    // Value

	    else if (s == "Value" && term->size () == 1)
	    {
		YCPValue tv = parseData (line, term->value (0), false);
		if (!tv.isNull () && tupleValue.size () > 0)
		    tupleValue.top () = tv;
		// y2debug ("Value: %p", tv);
		return tv;
	    }

	    // Fillup

	    else if (isFillup && (s == "Fillup") &&	// fillup allowed and Fillup () found
		     (term->size () == 0) && (tupleName.size () > 0))
	    {		// inside Tuple ()
		string fillup;
		while ((line != 0)
		       && (mComment.find_first_of (line[0]) != string::npos))
		{
		    fillup = fillup + line;
		    line = getLine ();
		}
		tupleName.top () = KEY4FILLUP;
		tupleValue.top () = YCPString (fillup);
		return tupleValue.top ();
	    }

	    // Skip

	    else if (s == "Skip")
	    {
		return YCPVoid ();
	    }

	    // Match

	    else if (s == "Match")
	    {
		return currentMatch;
	    }

	    // Separator

	    else if (s == "Separator" && term->size () == 1)
	    {
		return parseSeparator (line, term->value (0)->asString ()->value ().c_str (),
				       optional);
	    }

	    // Whitespace

	    else if (s == "Whitespace")
	    {
		return parseSeparator (line, " \t", optional);
	    }

	    // String

	    else if (s == "String" && term->size () > 0)
	    {
		if (term->size () == 1)
		    return parseString (line, term->value (0)->asString ()->value ().c_str (),
					0, optional);
		if (term->size () == 2)
		    return parseString (line, term->value (0)->asString ()->value ().c_str (),
					term->value (1)->asString ()->value ().c_str (),
					optional);
	    }

	    // Or

	    else if (s == "Or" && term->size () > 0)
	    {
		const char *ltry = line;
		bool lopt = false;
		for (int i = 0; i < term->size (); i++)
		{
		    if (i == term->size () - 1)	// pass optional on last try
			lopt = optional;
		    YCPValue vtry = parseData (ltry, term->value (i), lopt);
		    if (!vtry.isNull ())
		    {
			y2debug ("Or () success");
			line = ltry;
			return vtry;
		    }
		}
		y2debug ("Or () failed");
		return YCPVoid ();
	    }

	    // Number

	    else if (s == "Number")
	    {
		return parseNumber (line, optional);
	    }

	    // Hexval

	    else if (s == "Hexval")
	    {
		return parseHexval (line, optional);
	    }

	    // Boolean

	    else if (s == "Boolean")
	    {
		return parseBoolean (line, optional);
	    }

	    // Float

	    else if (s == "Float")
	    {
		return parseFloat (line, optional);
	    }

	    // Ip4Number

	    else if (s == "Ip4Number")
	    {
		return parseIp4Number (line, optional);
	    }

	    // Hostname

	    else if (s == "Hostname")
	    {
		return parseHostname (line, optional);
	    }

	    // Username

	    else if (s == "Username")
	    {
		return parseUsername (line, optional);
	    }

	    // <name>

	    else if (islower (s[0]) && tupleName.size () > 0 && (term->size () == 1))
	    {
		tupleName.top () = s;
		tupleValue.top () = parseData (line, term->value (0), optional);
		return tupleValue.top ();
	    }

	    else
	    {
		y2error ("parseData: unknown term '%s'", s.c_str ());
		return YCPVoid ();
	    }
	} break;

	case YT_STRING:
	    y2debug ("YT_STRING");
	    return parseVerbose (line, syntax->asString ()->value ().c_str (),
				 optional);
	    break;

	default:
	    y2error ("parseData: unknown syntax %s", syntax->toString ().c_str ());
	    break;
    }

    return YCPNull ();
}


// unparseData
//
// value.isNull () is allowed !!
//

const string
AnyAgent::unparseData (const YCPValue & syntax, const YCPValue & value)
{
    if (syntax.isNull ())
	return "";

    y2debug ("unparseData ('%s',%s)", value.isNull () ? "" : value->toString ().c_str (),
	     syntax->toString ().c_str ());

    switch (syntax->valuetype ())
    {
	case YT_TERM: {

	    YCPTerm term = syntax->asTerm ();
	    if (term.isNull ())
	    {
		y2error ("YT_TERM no term");
	    }
	    string s = term->name ();
	    y2debug ("YT_TERM (%s)", s.c_str ());

	    // Optional

	    if (s == "Optional" && term->size () > 0)
	    {
		return unparseData (term->value (0), value);
	    }

	    // Continue

	    else if (s == "Continue" && term->size () > 0)
	    {
		if (tupleContinue)
		    return unparseData (term->value (0), value);
	    }

	    // Choice

	    else if (s == "Choice" && term->size () > 0)
	    {
		return unparseChoice (term->args (), value);
	    }

	    // Sequence

	    else if (s == "Sequence" && term->size () > 0)
	    {
		tupleName.push ("");
		tupleValue.push (YCPNull ());
		const string s = unparseSequence (term->args (), value);
		tupleName.pop ();
		tupleValue.pop ();
		return s;
	    }

	    // List

	    else if (s == "List" && term->size () == 2)
	    {
		return unparseList (term->args (), value);
	    }

	    // Tuple

	    else if (s == "Tuple" && term->size () > 0)
	    {
		tupleName.push ("");
		tupleValue.push (YCPNull ());
		const string s = unparseTuple (term->args (), value);
		tupleName.pop ();
		tupleValue.pop ();
		return s;
	    }

	    // Var

	    else if (s == "Var" && term->size () > 0)
	    {
		for (int i = 0; i < term->size (); i++)
		{
		    const string vdata = unparseData (term->value (i), value);
		    if (vdata.empty ())
			break;
		}
		return "";
	    }

	    // Name

	    else if (s == "Name" && term->size () == 1)
	    {
		y2error ("unparse Name ()");
		return "";
	    }

	    // Value

	    else if (s == "Value" && term->size () == 1)
	    {
		const string s = unparseData (term->value (0), value);
#if 0
		if (tv && tupleValue.size () > 0)
		    tupleValue.top () = tv;
#endif
		return s;
	    }

	    // Separator

	    else if (s == "Separator" && term->size () == 1)
	    {
		return unparseSeparator (term->value (0));
	    }

	    // Whitespace

	    else if (s == "Whitespace")
	    {
		return unparseSeparator (YCPString (" \t"));
	    }

	    // Skip (is handled correctly within unparseTuple()

	    else if (s == "Skip")
	    {
		return "";
	    }

	    // Fillup (is handled correctly within unparseTuple()

	    else if (s == "Fillup")
	    {
		return "";
	    }

	    // String

	    else if (s == "String" && term->size () > 0)
	    {
		if (term->size () == 1)
		    return unparseString (term->value (0), YCPNull (), value);
		if (term->size () == 2)
		    return unparseString (term->value (0), term->value (1),
					  value);
	    }

	    // Or

	    else if (s == "Or" && term->size () == 2)
	    {
#if 1
		const YCPValue vbackup = value;
		const string vdata = unparseData (term->value (0), value);
		if (vdata != "")
		    return vdata;
		else
		    return unparseData (term->value (1), vbackup);
#endif
	    }

	    // Number

	    else if (s == "Number")
	    {
		return unparseNumber (value);
	    }

	    // Boolean

	    else if (s == "Boolean")
	    {
		return unparseBoolean (value);
	    }

	    // Float

	    else if (s == "Float")
	    {
		return unparseFloat (value);
	    }

	    // Ip4Number

	    else if (s == "Ip4Number")
	    {
		return unparseIp4Number (value);
	    }

	    // Hostname

	    else if (s == "Hostname")
	    {
		return unparseHostname (value);
	    }

	    // Username

	    else if (s == "Username")
	    {
		return unparseUsername (value);
	    }

	    // <name> (<syntax>)

	    else if (islower (s[0]) && (term->size () == 1))
	    {
		if (!value->isMap ())
		    y2error ("request for element '%s' but value not map",
			     s.c_str ());
		else
		{
		    YCPValue v = value->asMap ()->value (YCPString (s));
		    if (v.isNull ())
		    {
			y2error ("No value for key '%s' in map", s.c_str ());
			return "";
		    }
		    return unparseData (term->value (0), v);
		}
	    }

	    else
	    {
		y2error ("unparseData: unknown term '%s'", s.c_str ());
		return "";
	    }
	} break;

	case YT_STRING:
	    y2debug ("YT_STRING");
	    return unparseVerbose (syntax);
	    break;

	default:
	    y2error ("unparseData: unknown syntax %s", syntax->toString ().c_str ());
	    break;
    }

    return "";
}

