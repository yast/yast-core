/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:	YCPBuiltinString.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
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
#include <iostream>
#include <string>

using std::string;
#include "ycp/md5.h"

#include "ycp/YCPBuiltinString.h"
#include "ycp/YCPString.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPError.h"
#include "ycp/YCPPath.h"
#include "ycp/YCPBoolean.h"
#include "ycp/y2log.h"

#include "ycp/StaticDeclaration.h"


extern StaticDeclaration static_declarations;


static YCPValue
s_size (const YCPString &s)
{
    /**
     * @builtin size (string s) -> integer
     * Returns the number of characters of the string <tt>s</tt>
     */

    return YCPInteger (s->value ().length ());
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

    return YCPString (s1->value () + p2->toString ());
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

    string ss = s->value ();
    string::size_type start = i1->value ();

    if ((start < 0)
	|| (start > ss.size ()))
    {
	return YCPError ("Substring index out of range", YCPString (""));
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

    string ss = s->value ();
    string::size_type start = i1->value ();
    string::size_type length = i2->value ();

    if (start > ss.size ())
	return YCPError ("Substring index out of range", YCPString (""));

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

    string ss = s->value ();
    for (unsigned i = 0; i < ss.size (); i++)
	ss[i] = tolower (ss[i]);
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

    string ss = s->value ();
    string include = i->value ();
    string::size_type pos = 0;

    while (pos = ss.find_first_not_of (include, pos), pos != string::npos)
	ss.erase (pos, ss.find_first_of (include, pos) - pos);

    return YCPString (ss);
}


static char salt_chars[] = {
    '.', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z'
};


class salt_init
{
public:
    salt_init () { srand (time (0)); }
    operator const char *() {
	salt[0] = salt_chars[rand () % sizeof (salt_chars)];
	salt[1] = salt_chars[rand () % sizeof (salt_chars)];
	return salt;
    }

private:
    char salt[2];
};


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

    static struct salt_init salt;

    string unencrypted = s->value ();
    string encrypted = crypt (unencrypted.c_str (), salt);
    return YCPString (encrypted);
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
     * crypt ("readable") -> "WEgvferaeaFy6"
     * </pre>
     */

    static struct salt_init salt;

    string unencrypted = s->value ();
    string encrypted = crypt_md5 (unencrypted.c_str (), salt);
    return YCPString (encrypted);
}


static YCPValue
s_mergestring (const YCPList &l, const YCPString &s)
{
    /**
     * @builtin mergestring (list (string) l, string c) -> string
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

    string ss1 = s1->value ();
    string::size_type pos = ss1.find_last_of (s2->value ());

    if (pos == string::npos)
	return YCPVoid ();		// not found
    else
	return YCPInteger (pos);	// found
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
	"\\0", "\\1", "\\2", "\\3", "\\4",
	"\\5", "\\6", "\\7", "\\8", "\\9"
    };

    string input_str (input);
    string result_str (result);

    for (unsigned int i = 1; i <= compiled.re_nsub && i < SUB_MAX; i++)
    {
	reg_ret.match_str[i] = matchptr[i].rm_so >= 0 ?
	    input_str.substr (matchptr[i].rm_so, matchptr[i].rm_eo) : "";
	reg_ret.match_nb = i;

	string::size_type col = result_str.find (index[i]);
	while (col != string::npos)
	{
	    result_str.replace (col, 2, reg_ret.match_str[i]);
	    col = result_str.find (index[i], col + 1);
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
     * Examples: <pre>
     * regexpmatch ("aaabbb", ".*ab.*") -> true
     * regexpmatch ("aaabbb", ".*ba.*") -> false
     * </pre>
     */

    const char *input = i->value ().c_str ();
    const char *pattern = p->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, "");

    if (result.error)
	return YCPError (string ("Error in regexpmatch ") + input + " " +
			 pattern + ": " + result.error_str);

    return YCPBoolean (result.solved);
}


static YCPValue
s_regexpsub (const YCPString &i, const YCPString &p, const YCPString &m)
{
    /**
     * @builtin regexpsub (string input, string pattern, string match) -> string
     *
     * Examples: <pre>
     * regexpsub ("aaabbb", "(.*ab).*", "s_\\1_e") -> "s_aaab_e"
     * regexpsub ("aaabbb", "(.*ba).*", "s_\\1_e") -> nil
     * </pre>
     */

    const char *input = i->value ().c_str ();
    const char *pattern = p->value ().c_str ();
    const char *match = m->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, match);

    if (result.error)
	return YCPError (string ("Error in regexp ") + input + " " + pattern +
			 " " + match + ": " + result.error_str);

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
     * !Attention pattern have to include parenthesize "(" ")"
     * If you need no parenthesize, use regexp_match
     *
     * If the pattern does not not match, the list ist empty.
     * Otherwise the list contains then matchted subexpressions for each pair
     * of parenthesize in pattern.
     *
     * If pattern does not contain a valid pattern, nil is returned.
     * In the include "common_functions, there are some convinience function,
     * like tokenX or regexp_error
     *
     * Examples: <pre>
     * list e = regexptokenize ("aaabbBb", "(.*[A-Z]).*")
     *
     * // e ==  [ "aaabbB" ]
     *
     * list h = regexptokenize ("aaabbb", "(.*ab)(.*)");
     *
     * // h == [ "aaab", "bb" ]
     *
     * include "wizard/common_functions.ycp"
     * token1 (h)         -> "aaab"
     * token2 (h)         -> "bb"
     *   ...
     * token9 (h)         -> ""
     * regexp_matched (h) -> true
     * regexp_error (h)   -> false
     *
     * list h = regexptokenize ("aaabbb", "(.*ba).*");
     *
     * // h == []
     *
     * token1 (h)         ->  ""
     * token2 (h)         ->  ""
     *   ...
     * token9 (h)         ->  ""
     * regexp_matched (h) ->  false
     * regexp_error (h)   ->  false
     *
     * list h = regexptokenize ("aaabbb", "(.*ba).*(");
     *
     * // h == nil
     *
     * token1 (h)         ->  ""
     * token2 (h)         ->  ""
     *   ...
     * token9 (h)         ->  ""
     * regexp_matched (h) ->  nil
     * regexp_error (h)   ->  true
     * </pre>
     */

    const char *input = i->value ().c_str ();
    const char *pattern = p->value ().c_str ();

    Reg_Ret result = solve_regular_expression (input, pattern, "");

    if (result.error)
	return YCPError (string ("Error in regexp ") + input + " " + pattern +
			 ": " + result.error_str);

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


YCPBuiltinString::YCPBuiltinString ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "+",		   "s|ss", 0, (void *)s_plus1, 0 },
	{ "+",		   "s|si", 0, (void *)s_plus2, 0 },
	{ "+",		   "s|sp", 0, (void *)s_plus3, 0 },
	{ "issubstring",   "b|ss", 0, (void *)s_issubstring, 0 },
	{ "tostring",	   "s|a",  0, (void *)s_tostring, 0 },
	{ "tohexstring",   "s|i",  0, (void *)s_tohexstring, 0 },
	{ "size",	   "i|s",  0, (void *)s_size, 0 },
	{ "find",	   "i|ss", 0, (void *)s_find, 0 },
	{ "tolower",	   "s|s",  0, (void *)s_tolower, 0 },
	{ "toascii",	   "s|s",  0, (void *)s_toascii, 0 },
	{ "deletechars",   "s|ss", 0, (void *)s_removechars, 0 },
	{ "filterchars",   "s|ss", 0, (void *)s_filterchars, 0 },
	{ "findfirstnotof","i|ss", 0, (void *)s_findfirstnotof, 0 },
	{ "findfirstof",   "i|ss", 0, (void *)s_findfirstof, 0 },
	{ "findlastof",	   "i|ss", 0, (void *)s_findlastof, 0 },
	{ "substring",	   "s|si", 0, (void *)s_substring1, 0 },
	{ "substring",	   "s|sii",0, (void *)s_substring2, 0 },
	{ "mergestring",   "s|Lss",0, (void *)s_mergestring, 0 },
	{ "crypt",	   "s|s",  0, (void *)s_crypt, 0 },
	{ "cryptmd5",	   "s|s",  0, (void *)s_cryptmd5, 0 },
	{ "regexpmatch",   "b|ss", 0, (void *)s_regexpmatch, 0 },
	{ "regexpsub",	   "s|sss",0, (void *)s_regexpsub, 0 },
	{ "regexptokenize","Ls|ss",0, (void *)s_regexptokenize, 0 },
	{ 0, 0, 0, 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinString", declarations);
}
