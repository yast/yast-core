/**
 *
 *  AnyAgent.cc
 *
 *  Purpose:	generalized agent handler for handling system files
 *		from /proc and /etc
 *
 *  Creator:	kkaempf@suse.de
 *  Maintainer:	kkaempf@suse.de
 *
 *  loads syntax description at startup to read and write
 *  system file
 *
 *  loads complete file (incl. comments) to internal YCPListRep and
 *  provides valid (i.e. non-comment) lines in another data
 *  structure defined by syntax.
 *
 *  Syntax description:
 *
 *  Header (<syntax>)
 *	Header to write at start of file, used only by Write ()
 *
 *  List (<syntax>, terminal)
 *	list of syntax, separated by terminal character
 *	represented as YCPListRep
 *
 *  Tuple (<name> (<syntax>), ...)		// fixed name
 *	tuple of syntax
 *	represented as YCPMapRep with <name>
 *
 *  Separator (<string>)
 *	separator characters
 *	used to separate data elements
 *
 *  Sequence (<syntax>,...)
 *	sequence of syntactical constructs
 *
 *  Choice ([<match>, <opt-action>], ...)
 *	<string>
 *	string constant, verbose match
 *
 *  Whitespace ()
 *	== Separator (" \t")
 *
 *  String (<string>)
 *	string match consisting of characters from <string>
 *	if <string> starts with ^, set of characters that do not match
 *	represented as YCPStringRep.
 *	if <optional-stripped> is given (as a string constant!), these
 *	characters are stripped from the match if they're found as leading
 *	or trailing characters.
 *	i.e. given the string " xxx xxx " matches String (" x") completely
 *	and results in " xxx xxx ". With String (" x", " "), leading and
 *	trailing blanks are stripped. So " xxx xxx " is still matched but
 *	the result is "xxx xxx".
 *	When writing such values, stripping is also performed. So writing
 *	" xxx xxx " results in the output of "xxx xxx"
 *
 *  Number ()
 *	integer
 *	represented as YCPIntegerRep
 *
 *  Hexval ()
 *	hexadecimal value
 *	represented as YCPIntegerRep
 *
 *  Float ()
 *	floating point
 *	represented as YCPFloatRep
 *
 *  Ip4Number ()
 *	IP4 address as nnn.nnn.nnn.nnn
 *	represented as YCPIntegerRep
 *
 *  Hostname ()
 *	hostname, with or without domain
 *	represented as YCPStringRep
 *
 *  Username ()
 *	username
 *	represented as YCPStringRep
 *
 *  Var (...Name (syntax) ... Value (syntax))
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <string>
#include <stack>
#include <iostream>
#include <fstream>

#include "AnyAgent.h"
#include <ycp/y2log.h>


AnyAgent::AnyAgent ()
    : description_read (false),
      mtime (0),
      cache (YCPNull ()),
      cchanged (true),
      alldata (YCPNull ()),
      achanged (true),
      mName (YCPNull ()),
      isFillup (false),
      mSyntax (YCPNull ()),
      mHeader (YCPNull ())
{
}


AnyAgent::~AnyAgent ()
{
}


YCPValue
AnyAgent::otherCommand (const YCPTerm & term)
{
    const string sym = term->name ();
    
    y2debug( "Received term in otherCommand: %s", term->toString().c_str() );

    if (sym == "Description" && term->size () >= 4)
    {
	if (description_read)
	{
	    y2warning ("Rereading Description. It's not intended this way!");
	}

	// extract File () or Run () or Local ()

	mType = MTYPE_NONE;
	YCPValue v = term->value (0);
	if (v->isTerm ())
	{
	    YCPTerm t = v->asTerm ();
	    const string s = t->name ();
	    if (s == "File" && t->size () > 0)
	    {
		mType = MTYPE_FILE;
		if (t->size () == 1 && t->value (0)->isString ())
		    mName = t->value (0)->asString ();
		else
		    mName = t;
	    }
	    else if (s == "Run" && t->size () > 0)
	    {
		mType = MTYPE_PROG;
		if (t->size () == 1 && t->value (0)->isString ())
		    mName = t->value (0)->asString ();
		else
		    mName = t;
	    }
	    else if (s == "Local" && t->size () > 0)
	    {
		mType = MTYPE_LOCAL;
		if (t->size () == 1 && t->value (0)->isString ())
		    mName = t->value (0)->asString ();
		else
		    mName = t;
	    }
	    else
	    {
		ycp2error ("Bad first arg of Description (): %s", s.c_str ());
		return YCPNull ();
	    }
	}

	if (mType == MTYPE_NONE)
	{
	    ycp2error ("First arg of Description () not recognized");
	    return YCPNull ();
	}

	// extract comment characters
	// or `Fillup ("<comment chars>")

	if (term->value (1)->isString ())
	{
	    mComment = term->value (1)->asString ()->value ();
	}
	else if (term->value (1)->isTerm ())
	{
	    YCPTerm fillterm = term->value (1)->asTerm ();
	    if ((fillterm->name () == "Fillup") &&
		(fillterm->size () == 1) && (fillterm->value (0)->isString ()))
	    {
		isFillup = true;
		mComment = fillterm->value (0)->asString ()->value ();
	    }
	    else
	    {
		ycp2error ("Second arg of Description() not Fillup(string)");
		return YCPNull ();
	    }
	}
	else
	{
	    ycp2error ("Second arg of Description() not string");
	    return YCPNull ();
	}

	// extract read-only flag

	mReadOnly = true;
	if (term->value (2)->isBoolean ())
	{
	    mReadOnly = term->value (2)->asBoolean ()->value ();
	    if (!mReadOnly && (mType == MTYPE_PROG))
	    {
		y2warning ("Run () must be read-only !");
		mReadOnly = true;
	    }
	}
	else
	    y2warning ("Third arg of Description () not boolean");

	// extract syntax description

	mSyntax = term->value (3);

	// extract optional header description

	if (term->size () > 4)
	{
	    mHeader = term->value (4);
	}
	else
	{
	    mHeader = YCPNull ();
	}

	description_read = true;

	return YCPVoid ();
    }

    return YCPNull ();
}


// ------------------------------------------------------------------

/**
 * Read
 *
 * read value from relative path
 *
 */

YCPValue
AnyAgent::Read (const YCPPath & path, const YCPValue& arg, const YCPValue& optarg)
{
    if (!description_read)
    {
	ycp2error ("Can't execute Read prior to reading Description.");
	return YCPVoid ();
    }

    y2debug ("Read (%s: %s, %s, %s)", path->toString ().c_str (),
	     mName->toString ().c_str (), mSyntax->toString ().c_str (),
	     isFillup ? "Fillup" : mComment.c_str ());

    YCPValue value = validateCache (YCPNull (), arg);

    if (value.isNull ())
    {
	ycp2error ("Read validate failed");
	return YCPNull ();
    }

    if (!path->isRoot ())
    {
	int len = path->length ();
	if (path->component_str (0) == "_")
	{

	    // ._ -> return raw data completely
	    //

	    if (len == 1)
		return alldata;

	    // ._.<num> -> return line <num> of raw data
	    //

	    if ((len == 2) && isdigit (path->component_str (1)[0]))
	    {
		int num = atoi (path->component_str (1).c_str ());
		if ((num < 0) || (num > alldata->size ()))
		{
		    y2error ("Bad index %d for ._.<num>", num);
		    return YCPVoid ();
		}
		return alldata->value (num);
	    }
	}
	else
	{
	    return readValueByPath (value, path);
	}

	return YCPVoid ();
    }

    return value;
}


/**
 * Write
 *
 * write value to relative path
 *
 * return YCPIntegerRep(0) if ok
 * or YCPVoidRep if error
 *
 *
 * Approach:
 *
 * descend path to get syntax
 *	(gives expected syntax of value)
 * unparse value according to syntax
 *	(gives string if value matches syntax, else error)
 * set new value to cache
 *
 */

YCPBoolean
AnyAgent::Write (const YCPPath & path, const YCPValue & value,
		 const YCPValue & arg)
{
    if (!description_read)
    {
	ycp2error("Can't execute Write prior to reading Description.");
	return YCPBoolean (false);
    }

    y2debug ("Write (%s:%s)", path->toString ().c_str (),
	     value->toString ().c_str ());

    if (mReadOnly)
    {
	ycp2error ("Write (%s) is read-only", path->toString ().c_str ());
	return YCPBoolean (false);
    }

    // fill cache, can't be program here

    // check path

    if (!path->isRoot ())
    {
	int len = path->length ();

	// ._   -> write raw data completely
	//

	if (path->component_str (0) == "_")
	{
	    if ((len == 1) && (value->isList ()))
	    {
		y2debug ("Write: replace _");
		YCPValue vc_result = validateCache (value->asList ());
		if (vc_result.isNull ())
		    return YCPBoolean (false);

		return YCPBoolean (true);
	    }

	    // ._.<num> -> write line <num> of raw data
	    //

	    if ((len == 2) && isdigit (path->component_str (1)[0]))
	    {
		if (!value->isString ())
		{
		    ycp2error ( "Bad value %s for path", path->toString ().c_str ());
		    return YCPBoolean (false);
		}

		YCPList newdata;

		int num = atoi (path->component_str (1).c_str ());
		y2debug ("Write: write line %d", num);
		if (num < 0)
		    return YCPBoolean (false);

		if (num == alldata->size ())
		{
		    y2debug ("Write: extending data");
		    for (int i = 0; i < alldata->size (); i++)
			newdata->add (alldata->value (i));
		    newdata->add (value);
		}
		else
		{
		    y2debug ("Write: replace line %d", num);

		    // replace by copying

		    for (int i = 0; i < alldata->size (); i++)
		    {
			if (i == num)
			    newdata->add (value);
			else
			    newdata->add (alldata->value (i));
		    }
		}

		YCPValue vc_result = validateCache (newdata);
		if (vc_result.isNull ())
		    return YCPBoolean (false);

		return YCPBoolean (true);
	    }

	    ycp2error ("Bad write path %s", path->toString ().c_str());
	    return YCPBoolean (false);
	}

	// path not root and not ._
	//

    }

    YCPValue syntax = findSyntax (mSyntax, path);

    if (syntax.isNull ())
    {
	ycp2error ("No syntax for path %s", path->toString ().c_str ());
	return YCPBoolean (false);
    }

    // convert value to string

    const string s = unparseData (syntax, value);

    if (s.empty ())
    {
	ycp2error ("Wrong value for path %s", path->toString ().c_str ());
	return YCPBoolean (false);
    }

    // place value into cache, write string ...

    y2debug ("Write[%s]", s.c_str ());
    {
	string sfname;
	const char *fname;

	if (mName->isTerm ())
	{
	    sfname = evalArg (arg);
	    if (sfname.empty ())
		return YCPBoolean (false);
	}
	else
	{
	    sfname = mName->asString ()->value ();
	}

	fname = sfname.c_str ();

	y2debug (" to %s", fname);

	std::ofstream dummy_file (fname);
	if (!dummy_file)
	{
	    ycp2error ("Can't open path %s", sfname.c_str ());
	    return YCPBoolean (false);
	}

	dummy_file << s.c_str () << std::endl;

	// destructor closes file
    }

    return YCPBoolean (true);
}


/**
   Dir

   show subtree
*/


YCPList
AnyAgent::Dir (const YCPPath & path)
{
    if (!description_read)
    {
	ycp2error ("Can't execute Dir prior to reading Description.");
	return YCPNull ();
    }

    YCPList l;
    l->add (mSyntax);
    return l;
}


const char *
AnyAgent::get_line (FILE * fp)
{
    const int parsebufsize = 8192;

    static char parsebuf[parsebufsize];
    if (fgets (parsebuf, parsebufsize, fp) != parsebuf)
    {
	if (ferror (fp) != 0)
	    y2error ("parseFile (): %s", strerror (errno));
	return 0;
    }

    return parsebuf;
}

// ------------------------------------------------------------------

/**
 * validateCache
 *
 * parse file according to mSyntax and
 * construct YCPValueRep
 *
 */

YCPValue
AnyAgent::validateCache (const YCPList & data, const YCPValue & arg)
{
    y2debug ("validateCache (%s)", !data.isNull () ? data->toString ().c_str () : "<nil>");

    if (data.isNull ())
    {				// check for read
	YCPValue filedata = readFile (arg);
	if (filedata.isNull () || !filedata->isList ())
	    return filedata;
	alldata = filedata->asList ();
    }
    else
    {				// check for write
	if (!cchanged)
	    return cache;
	alldata = data;
    }

    if (alldata.isNull ())
    {
	ycp2error ("validateCache oops alldata failed");
	return YCPBoolean (false);
    }

    cchanged = true;

    // now parse file according to mComment/isFillup and mSyntax

    line_number = -1;		// initialize
    const char *line = "";
    YCPValue value = parseData (line, mSyntax, false);

    if (!value.isNull ())
    {
	cache = value;
	cchanged = false;
    }

    return value;
}


string
AnyAgent::evalArg (const YCPValue & arg)
{
    if (arg.isNull () || !arg->isList ())
    {
	y2error ("bad argument for Read ()");
	return "";
    }

    string fullpath;
    YCPTerm t = mName->asTerm ();
    YCPList l = arg->asList ();

    // construct filename in fullpath
    // loop over Run() or File() argument list
    // take `arg() from arg

    for (int i = 0; i < t->size (); i++)
    {
	// string constant

	if (t->value (i)->isString ())
	{
	    fullpath += t->value (i)->asString ()->value ();
	}
	else if (t->value (i)->isTerm ())
	{
	    // `arg(n), check n for correct type and match for current arg

	    YCPTerm ta = t->value (i)->asTerm ();
	    if (ta->name () == "arg"
		&& ta->size () == 1 && ta->value (0)->isInteger ())
	    {
		int ti = ta->value (0)->asInteger ()->value ();
		if ((ti < 0) && (ti >= l->size ()))
		{
		    y2error ("Bad arg value %d for Read ()", ti);
		    return "";
		}
		if (!l->value (ti)->isString ())
		{
		    y2error ("Bad arg type for Read ()");
		    return "";
		}
		fullpath += l->value (ti)->asString ()->value ();
	    }
	    else
	    {
		y2error ("bad element in argument for Read (%s)",
			 t->value (i)->toString ().c_str ());
		return "";
	    }
	}
	else
	{
	    y2error ("bad element in argument for Read (%s)",
		     t->value (i)->toString ().c_str ());
	    return "";
	}
    }
    return fullpath;
}


/**
 * readFile
 *
 * read complete file to alldata
 *
 */

YCPValue
AnyAgent::readFile (const YCPValue & arg)
{
    struct stat buf;
    FILE *fp;
    string ss;

    if (mName->isTerm ())
    {
	ss = evalArg (arg);
	if (ss.empty ())
	    return YCPNull ();
    }
    else
    {
	ss = mName->asString ()->value ();
    }

    if (mType == MTYPE_PROG)
    {
	const char *s = ss.c_str ();
	y2debug ("readFile, run (%s)", s);

	// always invalidate cache
	buf.st_mtime = 0;

	const char *original_locale = getenv ("LC_ALL");
	if (setenv ("LC_ALL", "C", 1) < 0)
	    y2error ("Cannot reset locales;");

	fp = popen (s, "r");

	if (original_locale)
	{
	    if (setenv ("LC_ALL", original_locale, 1) < 0)
		y2error ("Cannot revert locales;");
	}
	else
	{
	    unsetenv ("LC_ALL");
	}

	if (fp == 0)
	{
	    ycp2error ("Can't run '%s': %d", ss.c_str (), errno);
	    return YCPNull ();
	}
    }
    else
    {
	const char *s = ss.c_str ();

	y2debug ("readFile, read (%s)", s);

	if (stat (s, &buf) != 0)
	{
	    mtime = 0;	// error case: reset mtime
	    if (errno == ENOENT)
	    {
		ycp2error ("File not found %s", s);
		return YCPList ();
	    }

	    ycp2error ("Can't stat '%s' :%d", s, errno);
	    return YCPNull ();
	}

	if (buf.st_mtime == mtime)
	    return alldata;

	// open file

	fp = fopen (s, "r");

	if (fp == 0)
	{
	    if (errno == EACCES)
	    {
		ycp2error ("Cant access %s", s);
		return YCPList ();
	    }

	    ycp2error ( "Error opening '%s': %d", s, errno);
	    return YCPNull ();
	}
    }

    // read complete file to alldata as YCPListRep (YCPStringRep)

    YCPList data;

    const char *line;
    while ((line = get_line (fp)) != 0)
    {
	data->add (YCPString (line));
    }

    if (mType == MTYPE_PROG)
	pclose (fp);
    else
	fclose (fp);

    mtime = buf.st_mtime;
    alldata = data;
    achanged = false;

    return alldata;
}


/**
 * writeFile
 *
 */

const string
AnyAgent::writeFile (const YCPValue & arg)
{
    if (!mHeader.isNull ())
	unparseData (mHeader, YCPNull ());
    return "";
}


/**
 * readValueByPath
 *
 * read sub-value denoted by path
 *
 * if path == .<num> && value->isList()
 *	return element <num> of list
 *
 * if path == .<name> && value->isMap()
 *	return element <name> of list
 */

YCPValue
AnyAgent::readValueByPath (const YCPValue & value, const YCPPath & path)
{
    YCPValue sub_value = value;
    for (int i = 0; i < path->length (); i++)
    {
	const char *s = path->component_str (i).c_str ();

	// .<num> -> return entry <num> of cached list data
	//

	if (isdigit ((unsigned char) s[0]))
	{
	    int num = atoi (s);

	    if (sub_value->isList ())
	    {
		if ((num < 0) || (num > sub_value->asList ()->size ()))
		{
		    y2error ("Bad index %d for ._.<num>", num);
		    return YCPVoid ();
		}
		sub_value = sub_value->asList ()->value (num);
	    }
	    else if (sub_value->isMap ())
	    {
		YCPValue v = sub_value->asMap ()->value (YCPString (s));
		if (v.isNull ())
		{
		    y2error ("Bad index %s for ._.<num>", s);
		    return YCPVoid ();
		}
		else
		    sub_value = v;
	    }
	}

	// .<name> -> return entry <name> of cached map data
	//

	else if (sub_value->isMap ())
	{
	    sub_value = sub_value->asMap ()->value (YCPString (s));
	}

	else
	{
	    y2error ("Read path element '%s' does not match value %s",
		     s, sub_value->toString ().c_str ());
	    sub_value = YCPVoid ();
	    break;
	}
    }

    return sub_value;
}


/** writeValueByPath
 *
 * write sub-value denoted by path to current
 *
 * return new current
 *
 * if path == .<num> && value->isList()
 *	return element <num> of list
 *
 * if path == .<name> && value->isMap()
 *	return element <name> of list
 */

YCPValue
AnyAgent::writeValueByPath (const YCPValue & current, const YCPPath & path,
			    const YCPValue & value)
{
    y2debug ("writeValueByPath ()");

    YCPValue run = current;
    for (int i = 0; i < path->length (); i++)
    {
	const char *s = path->component_str (i).c_str ();

	y2debug ("writeValueByPath (%s)", s);

	// .<num> -> return entry <num> of cached list data
	//

	if (isdigit (s[0]))
	{
	    int num = atoi (s);

	    if (run->isList ())
	    {
		if ((num < 0) || (num > run->asList ()->size ()))
		{
		    y2error ("Bad index %d for ._.<num>", num);
		    return YCPVoid ();
		}
		run = run->asList ()->value (num);
	    }
	    else if (run->isMap ())
	    {
		YCPValue v = run->asMap ()->value (YCPString (s));
		if (v.isNull ())
		{
		    y2error ("Bad index %s for ._.<num>", s);
		    return YCPVoid ();
		}
		else
		    run = v;
	    }
	}

	// .<name> -> return entry <name> of cached map data
	//

	else if (run->isMap ())
	{
	    run = run->asMap ()->value (YCPString (s));
	}

	else
	{
	    y2error ("Write path element '%s' does not match value %s",
		     s, run->toString ().c_str ());
	    run = YCPVoid ();
	    break;
	}
    }

    return run;
}


/**
 * findSyntax
 *
 * find syntax for path
 *
 */

YCPValue
AnyAgent::findSyntax (const YCPValue & syntax, const YCPPath & path)
{
    y2debug ("findSyntax ('%s':'%s')", syntax->toString ().c_str (),
	     path->toString ().c_str ());

    YCPValue cur_syntax = syntax;

    const int len = path->length ();
    if (len > 0)
    {
	for (int i = 0; i < len; i++)
	{
	    const string p = path->component_str (i);

	    if (cur_syntax.isNull ())
		break;

	    switch (cur_syntax->valuetype ())
	    {
		case YT_TERM: {
		    YCPTerm t = cur_syntax->asTerm ();
		    string s = t->name ();

		    // `tuple_name (<cur_syntax>)

		    if (s == p && t->size () > 0)
		    {
			cur_syntax = t->value (0);
		    }

		    else if (s == "Tuple" && t->size () > 0)
		    {
			if (isdigit (p[0]))
			{
			    int tnum = atoi (p.c_str ());
			    for (int j = 0; j < t->size (); j++)
			    {
				YCPValue v = t->value (j);
				if (v->isTerm ())
				{
				    YCPTerm vt = v->asTerm ();
				    if (islower (vt->name ()[0]) &&
					(vt->size () > 0))
				    {
					if (tnum == 0)
					{
					    cur_syntax = vt->value (0);
					    break;
					}
					tnum--;
				    }
				}
			    }
			}
			else
			{
			    for (int j = 0; j < t->size (); j++)
			    {
				YCPValue v = t->value (j);
				if (v->isTerm ())
				{
				    YCPTerm vt = v->asTerm ();
				    if (p == vt->name () && vt->size () > 0)
				    {
					cur_syntax = vt->value (0);
					break;
				    }
				}
			    }
			}
		    }

		    else if (s == "List" && t->size () > 0 && isdigit (p[0]))
		    {
			cur_syntax = t->value (0);
		    }

		    else
		    {
			y2error ("Can't find syntax for path '%s' in '%s'",
				 p.c_str (), cur_syntax->toString ().c_str ());
			cur_syntax = YCPNull ();
		    }
		}
		break;

		case YT_STRING:
		    break;

		default:
		    cur_syntax = YCPNull ();
		    break;
	    }
	}
    }

    y2debug ("found syntax (%s)", !cur_syntax.isNull () ?
	     cur_syntax->toString ().c_str () : "<nil>");

    return cur_syntax;
}

