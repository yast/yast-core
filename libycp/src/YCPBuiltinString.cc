/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|						     (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:	YCPBuiltinString.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE		// for crypt
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE		// for snprintf
#endif

#define ERR_MAX 80		// for regexp
#define SUB_MAX 10		// for regexp

#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <regex.h>
#include <libintl.h>
#include <iostream>
#include <string>

using std::string;

#include "ycp/YCPBuiltinString.h"
#include "ycp/YCPString.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPSymbol.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPVoid.h"
#include "ycp/y2log.h"
#include "y2string.h"
#include "y2crypt.h"

#include "ycp/StaticDeclaration.h"


extern StaticDeclaration static_declarations;


static YCPValue
s_size (const YCPString &s)
{
    /**
     * @builtin size (string s) -> integer
     * Returns the number of characters of the string <tt>s</tt>
     * Notice, that size(nil) -> nil
     */
     
    if (s.isNull ())
	return YCPNull ();

    // UTF-8 based length
    std::wstring out;
    utf82wchar(s->value (), &out);
    
    return YCPInteger (wcslen (out.c_str()));;
}


static YCPValue
s_plus1 (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin string s1 + string s2 -> string
     * Returns concatenation of <tt>s1</tt> and <tt>s2</tt>.
     *
     * Example: <pre>
     * "YaST" + "2" -> "YaST2"
     * </pre>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + s2->value_cstr ());
}


static YCPValue
s_plus2 (const YCPString &s1, const YCPInteger &i2)
{
    /**
     * @builtin string s1 + integer i2 -> string
     * Returns concatenation of <tt>s1</tt> and <tt>i2</tt> after
     * transforming <tt>i2</tt> to a string.
     *
     * Example: <pre>
     * "YaST" + 2 -> "YaST2"
     * </pre>
     */

    if (s1.isNull () || i2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + toString (i2->value ()));
}


static YCPValue
s_plus3 (const YCPString &s1, const YCPPath &p2)
{
    /**
     * @builtin string s1 + path p2 -> string
     * Returns concatenation of <tt>s1</tt> and <tt>p2</tt> after
     * transforming <tt>p2</tt> to a string.
     *
     * Example: <pre>
     * "YaST" + .two -> "YaST.two"
     * </pre>
     */

    if (s1.isNull () || p2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + p2->toString ());
}


static YCPValue
s_plus4 (const YCPString &s1, const YCPSymbol &s2)
{
    /**
     * @builtin string s1 + symbol s2 -> string
     * Returns concatenation of <tt>s1</tt> and <tt>s2</tt> after
     * transforming <tt>s2</tt> to a string AND stripping the leading
     * backquote.
     *
     * Example: <pre>
     * "YaST" + `two -> "YaSTtwo"
     * </pre>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + s2->symbol());
}


static YCPValue
s_issubstring (const YCPString &target, const YCPString &sub)
{
    /**
     * @builtin issubstring (string s, string substring) -> bool
     * Return true, if <tt>substring</tt> is a substring of <tt>s</tt>.
     *
     * Example: <pre>
     * issubstring ("some text", "tex") -> true
     * </pre>
     */

    if (target.isNull () || sub.isNull ())
	return YCPNull ();

    string s = target->value ();
    string substring = sub->value ();
	//y2milestone ("'%s' '%s' %p", s.c_str(), substring.c_str(), (void *)(s.find (substring)));
    return YCPBoolean (s.find (substring) != string::npos);
}


static YCPValue
s_tohexstring (const YCPInteger &i)
{
    /**
     * @builtin tohexstring (integer number) -> string
     * Converts a integer to a hexadecimal string.
     *
     * Example: <pre>
     * tohexstring (31) -> "0x1f"
     * </pre>
     */

    if (i.isNull ())
	return YCPNull ();

    char buffer[66];
    snprintf (buffer, 66, "0x%x", int (i->value ()));
    return YCPString (buffer);
}


static YCPValue
s_substring1 (const YCPString &s, const YCPInteger &i1)
{
    /**
     * @builtin substring (string s, integer start) -> string
     * Extract a substring of the string <tt>s</tt>, starting at
     * <tt>start</tt> after the first one.
     *
     * Examples: <pre>
     * substring ("some text", 5) -> "text"
     * substring ("some text", 42) -> ""
     * </pre>
     */

    if (s.isNull () || i1.isNull ())
	return YCPNull ();

    string ss = s->value ();
    string::size_type start = i1->value ();

    if ((start < 0)
	|| (start > ss.size ()))
    {
	ycp2error("Substring index out of range");
	return YCPString ("");
    }

    return YCPString (ss.substr (start, string::npos));
}


static YCPValue
s_substring2 (const YCPString &s, const YCPInteger &i1, const YCPInteger &i2)
{
    /**
     * @builtin substring (string s, integer start, integer length) -> string
     * Extract a substring of the string <tt>s</tt>, starting at
     * <tt>start</tt> after the first one with length of at most
     * <tt>length</tt>.
     *
     * Example: <pre>
     * substring ("some text", 5, 2) -> "te"
     * substring ("some text", 42, 2) -> ""
     * </pre>
     */

    if (s.isNull () || i1.isNull() || i2.isNull ())
	return YCPNull ();

    string ss = s->value ();
    string::size_type start = i1->value ();
    string::size_type length = i2->value ();

    if (start > ss.size ())
    {
	ycp2error ("Substring index out of range");
	return YCPString ("");
    }

    return YCPString (ss.substr (start, length));
}


static YCPValue
s_find (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin find (string s1, string s2) -> integer
     * Returns the first position in <tt>s1</tt> where the
     * string <tt>s2</tt> is contained in <tt>s1</tt>.
     *
     * Examples: <pre>
     * find ("abcdefghi", "efg") -> 4
     * find ("aaaaa", "z") -> -1
     * </pre>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    string ss1 = s1->value ();
    string::size_type pos = ss1.find (s2->value ());

    if (pos == string::npos)
	return YCPInteger (-1);		// not found
    else
	return YCPInteger (pos);	// found
}


static YCPValue
s_tolower (const YCPString &s)
{
    /**
     * @builtin tolower (string s) -> string
     * Returns a string that results from string <tt>s</tt> by
     * converting each character tolower.
     *
     * Example: <pre>
     * tolower ("aBcDeF") -> "abcdef"
     * </pre>
     */

    if (s.isNull ())
	return YCPNull ();

    string ss = s->value ();
    for (unsigned i = 0; i < ss.size (); i++)
	ss[i] = tolower (ss[i]);
    return YCPString (ss);
}


static YCPValue
s_toupper (const YCPString &s)
{
    /**
     * @builtin toupper (string s) -> string
     * Returns a string that results from string <tt>s</tt> by
     * converting each character toupper.
     *
     * Example: <pre>
     * tolower ("aBcDeF") -> "ABCDEF"
     * </pre>
     */

    if (s.isNull ())
	return YCPNull ();

    string ss = s->value ();
    for (unsigned i = 0; i < ss.size (); i++)
	ss[i] = toupper (ss[i]);
    return YCPString (ss);
}


static YCPValue
s_toascii (const YCPString &s)
{
    /**
     * @builtin toascii (string s) -> string
     * Returns a string that results from string <tt>s</tt> by
     * copying each character that is below 0x7F (127).
     *
     * Example: <pre>
     * toascii ("aÖBÄc") -> "aBc"
     * </pre>
     */

    if (s.isNull ())
	return YCPNull ();

    string ss = s->value ();
    unsigned int w = 0;
    for (unsigned int i = 0; i < ss.size (); i++)
	if (isascii (ss[i]))
	    ss[w++] = ss[i];
    return YCPString (ss.substr (0, w));
}


static YCPValue
s_removechars (const YCPString &s, YCPString &r)
{
    /**
     * @builtin deletechars (string s, string remove) -> string
     * Returns a string that results from string <tt>s</tt> by removing
     * all characters that occur in <tt>remove</tt>.
     *
     * Example: <pre>
     * deletechars ("aÖBÄc", "abcdefghijklmnopqrstuvwxyz") -> "ÖBÄ"
     * </pre>
     */

    if (s.isNull () || r.isNull ())
	return YCPNull ();

    string ss = s->value ();
    string include = r->value ();
    string::size_type pos = 0;

    while (pos = ss.find_first_of (include, pos), pos != string::npos)
    {
	ss.erase (pos, ss.find_first_not_of (include, pos) - pos);
    }
    return YCPString (ss);
}


static YCPValue
s_filterchars (const YCPString &s, const YCPString &i)
{
    /**
     * @builtin filterchars (string s, string include) -> string
     * Returns a string that results from string <tt>s</tt> by removing
     * all characters that do not occur in <tt>include</tt>.
     *
     * Example: <pre>
     * filterchars ("aÖBÄc", "abcdefghijklmnopqrstuvwxyz") -> "ac"
     * </pre>
     */

    if (s.isNull () || i.isNull ())
	return YCPNull ();

    string ss = s->value ();
    string include = i->value ();
    string::size_type pos = 0;

    while (pos = ss.find_first_not_of (include, pos), pos != string::npos)
	ss.erase (pos, ss.find_first_of (include, pos) - pos);

    return YCPString (ss);
}


static YCPValue
s_mergestring (const YCPList &l, const YCPString &s)
{
    /**
     * @builtin mergestring (list&lt;string&gt; l, string c) -> string
     * Merges a list of strings to a single list. Inserts <tt>c</tt>
     * between list elements (c may be empty). List elements which
     * are not of type strings are ignored.
     *
     * See also: splitstring
     *
     * Examples: <pre>
     * mergestring (["", "abc", "dev", "ghi"], "/") -> "/abc/dev/ghi"
     * mergestring (["abc", "dev", "ghi", ""], "/") -> "abc/dev/ghi/"
     * mergestring ([1, "a", 3], ".") -> "a"
     * mergestring ([], ".") -> ""
     * mergestring (["abc", "dev", "ghi"], "") -> "abcdevghi"
     * mergestring (["abc", "dev", "ghi"], "123") -> "abc123dev123ghi"
     * </pre>
     */
     
    if (l.isNull ())
	return YCPNull ();
	
    if (s.isNull ())
    {
	ycp2error ("Can't merge string using 'nil'");
	return YCPNull ();
    }

    string ret;

    if (l->isEmpty ())		// empty list -> empty result
	return YCPString (ret);

    string c = s->value ();

    // loop through list

    for (int i = 0; i < l->size (); i++)
    {
	YCPValue vv = l->value (i);
	if (!vv->isString ())	// skip non-string elements
	    continue;
	string vs = vv->asString ()->value ();

	if (i != 0)		// insert c *between* strings
	    ret += c;

	ret += vs;		// add string to result
    }

    return YCPString (ret);
}


static YCPValue
s_findfirstnotof (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin findfirstnotof (string s1, string s2) -> integer
     * Returns the position of the first character in <tt>s1</tt> that is
     * not contained in <tt>s2</tt>.
     *
     * Examples: <pre>
     * findfirstnotof ("abcdefghi", "abcefghi") -> 3
     * findfirstnotof ("aaaaa", "a") -> nil
     * </pre>
     */
     
    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    // this is needed on gcc 3.0. gcc 2.95 was wrong.
    if (s1->value ().empty ())
	return YCPInteger ((long long int) 0);

    string ss1 = s1->value ();
    string::size_type pos = ss1.find_first_not_of (s2->value ());

    if (pos == string::npos)
	return YCPVoid ();		// not found
    else
	return YCPInteger (pos);	// found
}


static YCPValue
s_findfirstof (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin findfirstof (string s1, string s2) -> integer
     * Returns the position of the first character in <tt>s1</tt> that is
     * contained in <tt>s2</tt>.
     *
     * Examples: <pre>
     * findfirstof ("abcdefghi", "cxdv") -> 2
     * findfirstof ("aaaaa", "z") -> nil
     * </pre>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    string ss1 = s1->value ();
    string::size_type pos = ss1.find_first_of (s2->value ());

    if (pos == string::npos)
	return YCPVoid ();		// not found
    else
	return YCPInteger (pos);	// found
}


static YCPValue
s_findlastof (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin findlastof (string s1, string s2) -> integer
     * Returns the position of the last character in <tt>s1</tt> that is
     * contained in <tt>s2</tt>.
     *
     * Examples: <pre>
     * findlastof ("abcdecfghi", "cxdv") -> 5
     * findlastof ("aaaaa", "z") -> nil
     * </pre>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    string ss1 = s1->value ();
    string::size_type pos = ss1.find_last_of (s2->value ());

    if (pos == string::npos)
	return YCPVoid ();		// not found
    else
	return YCPInteger (pos);	// found
}

static YCPValue
s_findlastnotof (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin findlastnotof( string s_1, string s_2 ) -> integer
     * Returns the position of the last character in <tt>s_1</tt> that is
     * NOT contained in <tt>s_2</tt>.
     *
     * Example <pre>
     * findlastnotof( "abcdefghi", "abcefghi" ) -> 3 ('d')
     * findlastnotof("aaaaa", "a") -> nil
     * </pre>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    string::size_type pos = s1->value ().find_last_not_of( s2->value () );

    if ( pos == string::npos ) return YCPVoid();    // not found
    else return YCPInteger( pos );                  // found
}


typedef struct REG_RET
{
    string result_str;
    string match_str[SUB_MAX];	// index 0 not used!!
    int match_nb;		// 0 - 9
    string error_str;
    bool error;
    bool solved;
} Reg_Ret;


/*
 * Universal regular expression solver.
 * It is used by all regexp* ycp builtins.
 */
Reg_Ret solve_regular_expression (const char *input, const char *pattern,
				  const char *result)
{
    int status;
    char error[ERR_MAX + 1];

    regex_t compiled;
    regmatch_t matchptr[SUB_MAX + 1];

    Reg_Ret reg_ret;
    reg_ret.match_nb = 0;
    reg_ret.error = true;
    reg_ret.error_str = "";

    status = regcomp (&compiled, pattern, REG_EXTENDED);
    if (status)
    {
	regerror (status, &compiled, error, ERR_MAX);
	reg_ret.error_str = string (error);
	return reg_ret;
    }

    if (compiled.re_nsub > SUB_MAX)
    {
	snprintf (error, ERR_MAX, "too much subexpresions: %d", compiled.re_nsub);
	reg_ret.error_str = string (error);
	regfree (&compiled);
	return reg_ret;
    }

    status = regexec (&compiled, input, compiled.re_nsub + 1, matchptr, 0);
    reg_ret.solved = !status;
    reg_ret.error = false;

    if (status)
    {
	regfree (&compiled);
	return reg_ret;
    }

    static const char *index[] = {
        NULL, /* not used */
        "\\1", "\\2", "\\3", "\\4",
        "\\5", "\\6", "\\7", "\\8", "\\9"
    };

    string input_str (input);
    string result_str (result);

    for (unsigned int i=0; (i <= compiled.re_nsub) && (i <= SUB_MAX); i++) {
        reg_ret.match_str[i] = matchptr[i].rm_so >= 0 ? input_str.substr(matchptr[i].rm_so, matchptr[i].rm_eo - matchptr[i].rm_so) : "";
        reg_ret.match_nb = i;

        string::size_type col = string::npos;
        if(index[i] != NULL) col = result_str.find(index[i]);
        while( col != string::npos ) {
            result_str.replace( col, 2, reg_ret.match_str[i]  );
            col = result_str.find(index[i], col + 1 );
        }
    }

    reg_ret.result_str = result_str;
    regfree (&compiled);
    return reg_ret;
}


static YCPValue
s_regexpmatch (const YCPString &i, const YCPString &p)
{
    /**
     * @builtin regexpmatch (string input, string pattern) -> boolean
     *
     * Searches a string for a POSIX Extended Regular Expression match.
     *
     * See regex(7).
     *
     * Example <pre>
     * regexpmatch ("aaabbbccc", "ab") -> true
     * regexpmatch ("aaabbbccc", "^ab") -> false
     * regexpmatch ("aaabbbccc", "ab+c") -> true
     * regexpmatch ("aaa(bbb)ccc", "\\(.*\\)") -> true     
     * </pre>
     */

    if (i.isNull () || p.isNull ())
	return YCPNull ();

    const char *input = i->value ().c_str ();
    const char *pattern = p->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, "");

    if (result.error)
    {
	ycp2error ("Error in regexpmatch %s %s: %s", input, pattern, result.error_str.c_str ());
	return YCPNull ();
    }

    return YCPBoolean (result.solved);
}

static YCPValue 
s_regexppos(const YCPString& inp, const YCPString& pat)
{
    /**
     * @builtin regexppos (string input, string pattern) -> [ pos, len ]
     * Returns a pair with position and length of the first match.
     * If no match is found it returns an empty list.
     *
     * See regex(7).
     *
     * Example <pre>
     * regexppos ("abcd012efgh345", "[0-9]+") -> [4, 3]
     * regexppos ("aaabbb", "[0-9]+") -> []
     * </pre>
     */

    if (inp.isNull () || pat.isNull ())
	return YCPNull ();

    const char *input   = inp->value ().c_str ();
    const char *pattern = pat->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, "");

    if (result.error) {
        ycp2error ("Error in regexp <%s> <%s>: %s", input, pattern, result.error_str.c_str ());
        return YCPNull ();
    }

    YCPList list;
    if (result.solved) {
        list->add ( YCPInteger(inp->value ().find (result.match_str[0])));
        list->add ( YCPInteger(result.match_str[0].length ()));
    }

    return list;
}


static YCPValue
s_regexpsub (const YCPString &i, const YCPString &p, const YCPString &m)
{
    /**
     * @builtin regexpsub (string input, string pattern, string output) -> string
     * Searches a string for a POSIX Extended Regular Expression match
     * and returns <i>output</i> with the matched subexpressions
     * substituted or <b>nil</b> if no match was found.
     *
     * See regex(7).
     *
     * Examples: <pre>
     * regexpsub ("aaabbb", "(.*ab)", "s_\\1_e") -> "s_aaab_e"
     * regexpsub ("aaabbb", "(.*ba)", "s_\\1_e") -> nil
     * </pre>
     */

    if (i.isNull () || p.isNull () || m.isNull ())
	return YCPNull ();

    const char *input = i->value ().c_str ();
    const char *pattern = p->value ().c_str ();
    const char *match = m->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, match);

    if (result.error)
    {
	ycp2error ("Error in regexp %s %s %s: %s", input, pattern, match, result.error_str.c_str ());
	return YCPNull ();
    }

    if (result.solved)
	return YCPString (result.result_str.c_str ());

    return YCPNull ();
}


static YCPValue
s_regexptokenize (const YCPString &i, const YCPString &p)
{
    /**
     * @builtin regexptokenize (string input, string pattern) -> list handle
     *
     * Searches a string for a POSIX Extended Regular Expression match
     * and returns a list of the matched subexpressions
     *
     * See regex(7).
     *
     * If the pattern does not match, the list is empty.
     * Otherwise the list contains then matchted subexpressions for each pair
     * of parenthesize in pattern.
     *
     * If the pattern is invalid, <b>nil</b> is returned.
     * In the include "common_functions, there are some convinience function,
     * like tokenX or regexp_error
     *
     * Examples: <pre>
     * list e = regexptokenize ("aaabbBb", "(.*[A-Z]).*");
     *
     * // e ==  [ "aaabbB" ]
     *
     * list h = regexptokenize ("aaabbb", "(.*ab)(.*)");
     *
     * // h == [ "aaab", "bb" ]
     *
     * list h = regexptokenize ("aaabbb", "(.*ba).*");
     *
     * // h == []
     *
     * list h = regexptokenize ("aaabbb", "(.*ba).*(");
     *
     * // h == nil
     * </pre>
     */
    // ")

    if (i.isNull () || p.isNull ())
	return YCPNull ();

    const char *input = i->value ().c_str ();
    const char *pattern = p->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, "");

    if (result.error)
    {
	ycp2error ("Error in regexp %s %s: %s", input, pattern, result.error_str.c_str ());
	return YCPNull ();
    }

    YCPList list;

    if (result.solved)
    {
	for (int i = 1; i <= result.match_nb; i++)
	{
	    list->add (YCPString (result.match_str[i]));
	}
    }
    return list;
}


static YCPValue
s_tostring (const YCPValue &v)
{
    /**
     * @builtin tostring (any value) -> string
     * Converts a value to a string.
     *
     */

    if (v.isNull())
    {
	return v;
    }
    if (v->valuetype() == YT_STRING)
    {
	return v->asString();
    }
    return YCPString(v->toString());
}


static YCPValue
s_timestring (const YCPString &fmt, const YCPInteger &time, const YCPBoolean &utc_flag)
{
   /**
     * @builtin timestring (string format, integer time, boolean utc) -> string
     * Combination of standard libc functions gmtime or localtime and strftime.
     *
     * Example <pre>
     * timestring ("%F %T %Z", time (), false) -> "2004-08-24 14:55:05 CEST"
     * </pre>
     */

    if (fmt.isNull () || time.isNull () || utc_flag.isNull ())
	return YCPNull ();

    string format = fmt->value();
    time_t t = time->value();
    bool utc = utc_flag->value();

    struct tm* tm = utc ? gmtime (&t) : localtime (&t);

    const size_t size = 100;
    char buffer[size];
    strftime (buffer, size, format.c_str (), tm);

    return YCPString (buffer);
}

static YCPValue
s_crypt (const YCPString &s)
{
    /**
     * @builtin crypt (string unencrypted) -> string
     * Encrypt the string <tt>unencrypted</tt> using the standard
     * password encryption provided by the system.
     *
     * Example: <pre>
     * crypt ("readable") -> "Y2PEyAiaeaFy6"
     * </pre>
     */

    if (s.isNull ())
	return YCPNull ();

    string unencrypted = s->value();
    string encrypted;

    if (crypt_pass (unencrypted, CRYPT, &encrypted))
        return YCPString (encrypted);
    else
    {
	ycp2error ("Encryption using crypt failed");
        return YCPNull ();
    }
}


static YCPValue
s_cryptmd5 (const YCPString &s)
{
    /**
     * @builtin cryptmd5 (string unencrypted) -> string
     * Encrypt the string <tt>unencrypted</tt> using md5
     * password encryption.
     *
     * Example: <pre>
     * cryptmd5 ("readable") -> "$1$BBtzrzzz$zc2vEB7XnA3Iq7pOgDsxD0"
     * </pre>
     */

    if (s.isNull ())
	return YCPNull ();

    string unencrypted = s->value();
    string encrypted;

    if (crypt_pass (unencrypted, MD5, &encrypted))
        return YCPString (encrypted);
    else
    {
	ycp2error ("Encryption using MD5 failed");
        return YCPNull ();
    }
}


static YCPValue 
s_cryptbigcrypt(const YCPString& original)
{
    /**
     * @builtin cryptbigcrypt (string unencrypted) -> string
     * Encrypt the string <tt>unencrypted</tt> using bigcrypt
     * password encryption. The password is not truncated.
     *
     * Example <pre>
     * cryptbigcrypt ("readable") -> "d4brTQmcVbtNg"
     * </pre>
     */

    if (original.isNull ())
	return YCPNull ();

    string unencrypted = original->value();
    string encrypted;

    if (crypt_pass (unencrypted, BIGCRYPT, &encrypted))
        return YCPString (encrypted);
    else
    {
	ycp2error ("Encryption using bigcrypt failed");
        return YCPNull ();
    }
}


static YCPValue 
s_cryptblowfish(const YCPString& original)
{
    /**
     * @builtin cryptblowfish (string unencrypted) -> string
     * Encrypt the string <tt>unencrypted</tt> using blowfish
     * password encryption. The password is not truncated.
     *
     * Example <pre>
     * cryptblowfish ("readable") -> "$2a$05$B3lAUExB.Bqpy8Pq0TpZt.s7EydrmxJRuhOZR04YG01ptwOUR147C"
     * </pre>
     */

    if (original.isNull ())
	return YCPNull ();

    string unencrypted = original->value();
    string encrypted;

    if (crypt_pass (unencrypted, BLOWFISH, &encrypted))
        return YCPString (encrypted);
    else
    {
	ycp2error ("Encryption using blowfish failed");
        return YCPNull ();
    }
}


static YCPValue
s_dgettext (const YCPString& domain, const YCPString& text)
{
    /**
     * @builtin _(string text) -> string
     * Translates the text using the current textdomain. 
     *
     * Example <pre>
     * _("File") -> "Soubor"
     * </pre>
     */

    /**
     * @builtin dgettext (string textdomain, string text) -> string
     * Translates the text using the given text domain into
     * the current language.
     *
     * This is a special case builtin not intended for general use.
     * See _() instead.
     *
     * Example <pre>
     * dgettext ("base", "No") -> "Nie"
     * </pre>
     */

    if (domain.isNull () || domain->isVoid ()) 
    {
	return YCPNull ();
    }

    // initialize text domain if not done so
    string dom = domain->value ();    
    YLocale::ensureBindDomain (dom);
    
    if (text.isNull () || text->isVoid ())
    {
	return YCPNull ();
    }
    
    return YCPString (dgettext (dom.c_str(), text->value().c_str()));
}


static YCPValue
s_dngettext (const YCPString& domain, const YCPString& singular, const YCPString& plural, const YCPInteger& count)
{
    /**
     * @builtin _(string singular, string plural, integer value) -> string
     * Translates the text using a locale-aware plural form handling and the 
     * current textdomain. The chosen form of the translation depends 
     * on the <tt>value</tt>.
     *
     * Example <pre>
     * _("%1 File", "%1 Files", 2) -> "%1 soubory"
     * </pre>
     */

    /**
     * @builtin dngettext (string textdomain, string singular, string plural, integer value) -> string
     * Translates the text using a locale-aware plural form handling using
     * the given textdomain.
     * The chosen form of the translation depend on the <tt>value</tt>.
     *
     * This is a special case builtin not intended for general use.
     * See _() instead.
     *
     * Example <pre>
     * dngettext ("base", "%1 File", "%1 Files", 2) -> "%1 soubory"
     * </pre>
     */

    if (domain.isNull () || domain->isVoid ()) 
    {
	return YCPNull ();
    }

    // initialize text domain if not done so
    string dom = domain->value ();    
    YLocale::ensureBindDomain (dom);
    
    if (singular.isNull () || singular->isVoid ()
	|| plural.isNull () || plural->isVoid ()
	|| count.isNull () || count->isVoid ())
    {
	return YCPNull ();
    }
    
    return YCPString (dngettext (domain->value().c_str(), singular->value().c_str(), plural->value().c_str(), count->value()));
}


YCPBuiltinString::YCPBuiltinString ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "+",		   "string (string, string)",		(void *)s_plus1 },
	{ "+",		   "string (string, integer)",		(void *)s_plus2 },
	{ "+",		   "string (string, path)",		(void *)s_plus3 },
	{ "+",		   "string (string, symbol)",		(void *)s_plus4 },
	{ "issubstring",   "boolean (string, string)",		(void *)s_issubstring },
	{ "tostring",	   "string (any)",			(void *)s_tostring },
	{ "tohexstring",   "string (integer)",			(void *)s_tohexstring },
	{ "size",	   "integer (string)",			(void *)s_size },
	{ "find",	   "integer (string, string)",		(void *)s_find },
	{ "tolower",	   "string (string)",			(void *)s_tolower },
	{ "toupper",	   "string (string)",			(void *)s_toupper },
	{ "toascii",	   "string (string)",			(void *)s_toascii },
	{ "deletechars",   "string (string, string)",		(void *)s_removechars },
	{ "filterchars",   "string (string, string)",		(void *)s_filterchars },
	{ "findfirstnotof","integer (string, string)",		(void *)s_findfirstnotof },
	{ "findfirstof",   "integer (string, string)",		(void *)s_findfirstof },
	{ "findlastof",	   "integer (string, string)",		(void *)s_findlastof },
	{ "findlastnotof", "integer (string, string)",		(void *)s_findlastnotof },
	{ "substring",	   "string (string, integer)",		(void *)s_substring1 },
	{ "substring",	   "string (string, integer, integer)",	(void *)s_substring2 },
	{ "timestring",	   "string (string, integer, boolean)",	(void *)s_timestring },
	{ "mergestring",   "string (const list <string>, string)", (void *)s_mergestring },
	{ "crypt",	   "string (string)",			(void *)s_crypt },
	{ "cryptmd5",	   "string (string)",			(void *)s_cryptmd5 },
	{ "cryptbigcrypt", "string (string)",			(void *)s_cryptbigcrypt },
	{ "cryptblowfish", "string (string)",			(void *)s_cryptblowfish },
	{ "regexpmatch",   "boolean (string, string)",		(void *)s_regexpmatch },
	{ "regexppos",	   "list<integer> (string, string)",	(void *)s_regexppos },
	{ "regexpsub",	   "string (string, string, string)",	(void *)s_regexpsub },
	{ "regexptokenize","list <string> (string, string)",	(void *)s_regexptokenize },
	{ "dgettext",	   "string (string, string)",		(void *)s_dgettext},
	{ "dngettext",	   "string (string, string, string, integer)",	(void *)s_dngettext},
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinString", declarations);
}
