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
   Summary:     YCP String Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

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
s_isempty(const YCPString& s)
{
    /**
     * @builtin isempty 
     * @id isempty-string
     * @short Returns whether the string <tt>s</tt> is empty.
     * @param string s String
     * @return boolean Emptiness of string <tt>s</tt>
     *
     * @description
     * Notice that the string <tt>s</tt> must not be nil.
     *
     * @usage isempty("") -> true
     * @usage isempty("test") -> false 
     */

    return YCPBoolean(s->isEmpty());
}


static YCPValue
s_size (const YCPString &s)
{
    /**
     * @builtin size
     * @id size-string
     * @short Returns the number of characters of the string <tt>s</tt>
     * @param string s String
     * @return integer Size of string <tt>s</tt>
     *
     * @description
     * Notice, that size(nil) -> nil
     *
     * @usage size("size") -> 4
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
     * @operator string s1 + string s2 -> string
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
     * @operator string s1 + integer i2 -> string
     * @short String and integer Concatenation 
     *
     * @description
     * Returns concatenation of <tt>s1</tt> and <tt>i2</tt> after
     * transforming <tt>i2</tt> to a string.
     *
     * Example: 
     *
     * <code>
     * "YaST" + 2 -> "YaST2"
     * </code>
     */

    if (s1.isNull () || i2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + toString (i2->value ()));
}


static YCPValue
s_plus3 (const YCPString &s1, const YCPPath &p2)
{
    /**
     * @operator string s1 + path p2 -> string
     * @short String and path Concatenation
     * @description
     * Returns concatenation of <tt>s1</tt> and <tt>p2</tt> after
     * transforming <tt>p2</tt> to a string.
     *
     * Example: 
     * <code>
     * "YaST" + .two -> "YaST.two"
     * </code>
     */

    if (s1.isNull () || p2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + p2->toString ());
}


static YCPValue
s_plus4 (const YCPString &s1, const YCPSymbol &s2)
{
    /**
     * @operator string s1 + symbol s2 -> string
     * @short String and symbol Concatenation
     *
     * @description
     * Returns concatenation of <tt>s1</tt> and <tt>s2</tt> after
     * transforming <tt>s2</tt> to a string AND stripping the leading
     * backquote.
     *
     * Example: 
     * <code>
     * "YaST" + `two -> "YaSTtwo"
     * </code>
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    return YCPString (s1->value () + s2->symbol());
}


static YCPValue
s_issubstring (const YCPString &target, const YCPString &sub)
{
    /**
     * @builtin issubstring
     * @short searches for a specific string within another string
     * @param string s String to be searched
     * @param string substring Pattern to be searched for
     * @return boolean
     * @description 
     * Return true, if <tt>substring</tt> is a substring of <tt>s</tt>.
     *
     * @usage issubstring ("some text", "tex") -> true
     */

    if (target.isNull () || sub.isNull ())
	return YCPNull ();

    string s = target->value ();
    string substring = sub->value ();
	//y2milestone ("'%s' '%s' %p", s.c_str(), substring.c_str(), (void *)(s.find (substring)));
    return YCPBoolean (s.find (substring) != string::npos);
}


static YCPValue
s_tohexstring1 (const YCPInteger &i)
{
    /**
     * @builtin tohexstring
     * @id tohexstring-1
     * @short Converts an integer to a hexadecimal string.
     * @param integer number Number
     * @return string number in Hex
     *
     * @description
     *
     * @usage tohexstring (31) -> "0x1f"
     */

    if (i.isNull ())
	return YCPNull ();

    char buffer[64 + 3];
    snprintf (buffer, 64 + 3, "0x%llx", i->value ());
    return YCPString (buffer);
}


static YCPValue
s_tohexstring2 (const YCPInteger &i, const YCPInteger &w)
{
    /**
     * @builtin tohexstring
     * @id tohexstring-2
     * @short Converts an integer to a hexadecimal string.
     * @param integer number Number
     * @param integer width Width
     * @return string number in Hex
     *
     * @description
     *
     * @usage tohexstring (31, 1) -> "0x1f"
     * @usage tohexstring (31, 4) -> "0x001f"
     */

    if (i.isNull () || w.isNull())
	return YCPNull ();

    char buffer[64 + 3];
    snprintf (buffer, 64 + 3, "0x%0*llx", (int) w->value(), i->value ());
    return YCPString (buffer);
}


static YCPValue
s_substring1 (const YCPString &s, const YCPInteger &i1)
{
    /**
     * @builtin substring
     * @id substring-rest
     * @short Returns part of a string
     * @param string STRING Original String
     * @param integer OFFSET Start position
     * @optarg integer LENGTH Length of new string
     * @return string 
     * @description
     *
     * Returns the portion of <tt>STRING</tt>  specified by the <tt>OFFSET</tt>
     * and <tt>LENGHT</tt> parameters. <tt>OFFSET</tt> starts with 0.
     *
     * @usage substring ("some text", 5) -> "text"
     * @usage substring ("some text", 42) -> ""
     * @usage substring ("some text", 5, 2) -> "te"
     * @usage substring ("some text", 42, 2) -> ""
     * @usage substring("123456789", 2, 3) -> "345"
     */

    if (s.isNull () || i1.isNull ())
	return YCPNull ();

    string ss = s->value ();
    int start = i1->value ();

    if ((start < 0)
	|| ((size_t)start > ss.size ()))
    {
	ycp2error("Substring index out of range");
	return YCPString ("");
    }

    return YCPString (ss.substr ((string::size_type) start, string::npos));
}


static YCPValue
s_substring2 (const YCPString &s, const YCPInteger &i1, const YCPInteger &i2)
{
    /**
     * @builtin substring
     * @id substring-length
     * @short Extracts a substring
     *
     * @description
     * Extracts a substring of the string <tt>STRING</tt>, starting at
     * <tt>OFFSET</tt> after the first one with length of at most
     * <tt>LENGTH</tt>. <tt>OFFSET</tt> starts with 0.
     *
     * @param string STRING
     * @param integer OFFSET
     * @param integer LENGTH
     * @return string
     * @usage substring ("some text", 5, 2) -> "te"
     * @usage substring ("some text", 42, 2) -> ""
     * @usage substring("123456789", 2, 3) -> "345"
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
s_lsubstring1 (const YCPString &s, const YCPInteger &i1)
{
    /**
     * @builtin lsubstring
     * @id lsubstring-rest
     * @short Extracts a substring in UTF-8 encoded string
     *
     * @description
     * Extracts a substring of the string <tt>STRING</tt>, starting at
     * <tt>OFFSET</tt> after the first one with length of at most
     * <tt>LENGTH</tt>. <tt>OFFSET</tt> starts with 0. This method uses UTF-8 encoding.
     *
     * @param string STRING
     * @param integer OFFSET
     * @param integer LENGTH
     * @return string
     * @usage substring ("some text", 5) -> "text"
     * @usage substring ("some text", 42) -> ""
     */

    if (s.isNull () || i1.isNull())
	return YCPNull ();

    string lss = s->value ();
    wstring ss;
    
    if( ! utf82wchar( lss, &ss ) )
    {
	y2error( "Unable to recode string '%s' to UTF-8", lss.c_str() );
	return YCPNull ();
    }
    
    string::size_type start = i1->value ();

    if (start > ss.size ())
    {
	ycp2error ("Substring index out of range");
	return YCPString ("");
    }

    ss = ss.substr ((wstring::size_type) start, wstring::npos);
    
    if( !wchar2utf8( ss, &lss ) )
    {
	y2error( "Unable to recode result string '%ls' from UTF-8", ss.c_str() );
	return YCPNull ();
    }
    
    return YCPString(lss);
}


static YCPValue
s_lsubstring2 (const YCPString &s, const YCPInteger &i1, const YCPInteger &i2)
{
    /**
     * @builtin lsubstring
     * @id lsubstring-length
     * @short Extracts a substring in UTF-8 encoded string
     *
     * @description
     * Extracts a substring of the string <tt>STRING</tt>, starting at
     * <tt>OFFSET</tt> after the first one with length of at most
     * <tt>LENGTH</tt>. <tt>OFFSET</tt> starts with 0. This method uses UTF-8 encoding.
     *
     * @param string STRING
     * @param integer OFFSET
     * @param integer LENGTH
     * @return string
     * @usage lsubstring ("some text", 5, 2) -> "te"
     * @usage lsubstring ("some text", 42, 2) -> ""
     * @usage lsubstring("123456789", 2, 3) -> "345"
     */

    if (s.isNull () || i1.isNull() || i2.isNull ())
	return YCPNull ();

    string lss = s->value ();
    wstring ss;
    
    if( ! utf82wchar( lss, &ss ) )
    {
	y2error( "Unable to recode string '%s' to UTF-8", lss.c_str() );
	return YCPNull ();
    }
    
    string::size_type start = i1->value ();
    string::size_type length = i2->value ();

    if (start > ss.size ())
    {
	ycp2error ("Substring index out of range");
	return YCPString ("");
    }

    ss = ss.substr (start, length);
    
    if( !wchar2utf8( ss, &lss ) )
    {
	y2error( "Unable to recode result string '%ls' from UTF-8", ss.c_str() );
	return YCPNull ();
    }
    
    return YCPString(lss);
}


static YCPValue
s_search (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin search
     * @short Returns position of a substring
     * @param string STRING1 String
     * @param string STRING2 Substring
     * @return integer OFFSET
     * If substring is not found search returns `nil'.
     *
     * @description
     *
     * The <tt>search</tt> function searches string for the first occurency of
     * a specified substring (possibly a single character) and returns its
     * starting position.
     * 
     * Returns the first position in <tt>STRING1</tt> where the
     * string <tt>STRING2</tt> is contained in <tt>STRING1</tt>.
     * <tt>OFFSET</tt> starts with 0.
     *
     * @see findfirstof
     * @see findfirstnotof
     * @see find
     * @usage search ("abcdefghi", "efg") -> 4
     * @usage search ("aaaaa", "z") -> nil
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    string ss1 = s1->value ();
    string::size_type pos = ss1.find (s2->value ());

    if (pos == string::npos)
	return YCPVoid ();		// not found
    else
	return YCPInteger (pos);	// found
}


static YCPValue
s_find (const YCPString &s1, const YCPString &s2)
{
    /**
     * @builtin find
     * @id find-string
     * @short Returns position of a substring
     * @param string STRING1 String
     * @param string STRING2 Substring
     * @return integer OFFSET
     * If substring is not found find returns `-1'.
     *
     * @description
     *
     * The <tt>find</tt> function searches string for the first occurency of
     * a specified substring (possibly a single character) and returns its
     * starting position.
     * 
     * Returns the first position in <tt>STRING1</tt> where the
     * string <tt>STRING2</tt> is contained in <tt>STRING1</tt>.
     * <tt>OFFSET</tt> starts with 0.
     *
     * @deprecated Use search() instead
     * @see findfirstof
     * @see findfirstnotof
     * @see search
     * @usage find ("abcdefghi", "efg") -> 4
     * @usage find ("aaaaa", "z") -> -1
     */

    YCPValue ret = s_search (s1, s2);
    if (!ret.isNull () && ret->isVoid ())
	ret = YCPInteger (-1);

    return ret;
}


static YCPValue
s_tolower (const YCPString &s)
{
    /**
     * @builtin tolower
     * @short Makes a string lowercase
     * @param string s String
     * @return string String in lower case
     *
     * @description
     * Returns string with all alphabetic characters converted to lowercase.
     * Notice: national characters are left unchanged.
     *
     * @usage tolower ("aBcDeF") -> "abcdef"
     * @usage tolower ("ABCÁÄÖČ") -> "abcÁÄÖČ"
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
     * @builtin toupper
     * @short  Makes a string uppercase
     *
     * @description 
     * Returns string with all alphabetic characters converted to
     * uppercase.
     
     * @see toupper
     * @usage tolower ("aBcDeF") -> "ABCDEF"
     * @usage toupper ("abcáäöč") -> "ABCáäöč"
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
     * @builtin toascii 
     * @short Returns characters below 0x7F included in <tt>STRING</tt>
     * @param string STRING
     * @return string
     *
     * @description
     * Returns a string that results from string <tt>STRING</tt> by
     * copying each character that is below 0x7F (127).
     *
     * @usage toascii ("aBë") -> "aB"
     * @usage toascii ("123+-abcABCöëä") -> "123+-abcABC"
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
     * @builtin deletechars
     *
     * @short Removes all characters from a string
     * @param string STRING
     * @param string REMOVE Characters to be removed from <tt>STRING</tt>
     * @return string
     *
     * @description
     * Returns a string that results from string <tt>STRING</tt> by removing
     * all characters that occur in string <tt>REMOVE</tt>.
     *
     * @see filterchars
     * @usage deletechars ("a", "abcdefghijklmnopqrstuvwxyz") -> ""
     * @usage deletechars ("abc","cde") -> "ab"
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
     * @builtin filterchars
     * @short Filters characters out of a String
     * @param string STRING
     * @param string CHARS chars to be included
     * @return string
     *
     * @description
     * Returns a string that results from string <tt>STRING</tt> by removing
     * all characters that do not occur in <tt>CHARS</tt>.
     *
     * @see deletechars
     * @usage filterchars ("a", "abcdefghijklmnopqrstuvwxyz") -> "a"
     * @usage filterchars ("abc","cde") -> "c"
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
     * @builtin mergestring 
     * @short Joins list elements with a string
     * @param list<string> PIECES A List of strings
     * @param string GLUE
     * @return string 
     * @description
     *
     * Returns a string containing a string representation of all the list
     * elements in the same order, with the glue string between each element.
     * 
     * List elements which are not of type strings are ignored.
     *
     * @see splitstring
     *
     * @usage mergestring (["", "abc", "dev", "ghi"], "/") -> "/abc/dev/ghi"
     * @usage mergestring (["abc", "dev", "ghi", ""], "/") -> "abc/dev/ghi/"
     * @usage mergestring ([1, "a", 3], ".") -> "a"
     * @usage mergestring ([], ".") -> ""
     * @usage mergestring (["abc", "dev", "ghi"], "") -> "abcdevghi"
     * @usage mergestring (["abc", "dev", "ghi"], "123") -> "abc123dev123ghi"
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
     * @builtin findfirstnotof
     * @short Searches string for the first non matching chars
     * @param string STRING
     * @param string CHARS
     *
     * @description
     * The <tt>findfirstnotof</tt> function searches the first element of string that
     * doesn't match any character stored in chars and returns its position.
     *
     * @return integer the position of the first character in <tt>STRING</tt> that is
     * not contained in <tt>CHARS</tt>.
     *
     * @see findfirstof
     * @see find
     * @usage findfirstnotof ("abcdefghi", "abcefghi") -> 3
     * @usage findfirstnotof ("aaaaa", "a") -> nil
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
     * @builtin findfirstof
     * @short Finds position of the first matching characters in string
     * @param string STRING
     * @param string CHARS Characters to find
     *
     * @description
     * The <tt>findfirstof</tt> function searches string for the first match of any
     * character stored in chars and returns its position. 
     *
     * If no match is found findfirstof returns `nil'. 
     * 
     * @return integer the position of the first character in <tt>STRING</tt> that is
     * contained in <tt>CHARS</tt>.
     *
     * @see findfirstnotof
     * @see find
     * @usage findfirstof ("abcdefghi", "cxdv") -> 2
     * @usage findfirstof ("aaaaa", "z") -> nil
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
     * @builtin findlastof 
     * @short Searches string for the last match
     * @description
     * The `findlastof' function searches string for the last match of any
     * character stored in chars and returns its position.
     *
     * @param string STRING String
     * @param string CHARS Characters to find
     *
     * @return integer the position of the last character in <tt>STRING</tt> that is
     * contained in <tt>CHARS</tt>.
     *
     * @see findfirstof
     * @usage findlastof ("abcdecfghi", "cxdv") -> 5
     * @usage findlastof ("aaaaa", "z") -> nil
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
     * @builtin findlastnotof
     * @short Searches the last element of string that doesn't match
     * @param string STRING
     * @param string CHARS Characters
     * @description The `findlastnotof' function searches the last element of
     * string that doesn't match any character stored in chars and returns its
     * position.
     *
     * If no match is found the function returns `nil'.
     * 
     * @return integer The position of the last character in <tt>STRING</tt> that is
     * NOT contained in <tt>CHARS</tt>.
     *
     * @see findfirstnotof
     * @usage findlastnotof( "abcdefghi", "abcefghi" ) -> 3 ('d')
     * @usage findlastnotof("aaaaa", "a") -> nil
     */

    if (s1.isNull () || s2.isNull ())
	return YCPNull ();

    string::size_type pos = s1->value ().find_last_not_of( s2->value () );

    if ( pos == string::npos ) return YCPVoid();    // not found
    else return YCPInteger( pos );                  // found
}

/// (regexp builtins)
typedef struct REG_RET
{
    string result_str;		// for regexpsub
    string match_str[SUB_MAX];	// index 0 not used!!
    int match_nb;		// 0 - 9
    string error_str;		// from regerror
    bool error;
    bool solved;
} Reg_Ret;


/*
 * Universal regular expression solver.
 * It is used by all regexp* ycp builtins.
 * Replacement is done if result is not ""
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
	snprintf (error, ERR_MAX, "too many subexpresions: %zu", compiled.re_nsub);
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

    string input_str (input);

    for (unsigned int i=0; (i <= compiled.re_nsub) && (i <= SUB_MAX); i++) {
        reg_ret.match_str[i] = matchptr[i].rm_so >= 0 ? input_str.substr(matchptr[i].rm_so, matchptr[i].rm_eo - matchptr[i].rm_so) : "";
        reg_ret.match_nb = i;
    }


    string result_str;
    const char * done = result;	// text before 'done' has been dealt with
    const char * bspos = result;


    while (1) {
      bspos = strchr (bspos, '\\');
      if (bspos == NULL) // not found
	break;

      // STATE: \ seen
      ++bspos;

      if (*bspos >= '1' && *bspos <= '9') {
	// copy non-backslash text
	result_str.append (done, bspos - 1 - done);
	// copy replacement string
	result_str += reg_ret.match_str[*bspos - '0'];
	done = bspos = bspos + 1;
      }
    }
    // copy the rest
    result_str += done;
      
    reg_ret.result_str = result_str;
    regfree (&compiled);
    return reg_ret;
}


static YCPValue
s_regexpmatch (const YCPString &i, const YCPString &p)
{
    /**
     * @builtin regexpmatch
     *
     * @short Searches a string for a POSIX Extended Regular Expression match.
     * @param string INPUT
     * @param string PATTERN
     * @return boolean
     *
     * @see regex(7)
     *
     * @usage regexpmatch ("aaabbbccc", "ab") -> true
     * @usage regexpmatch ("aaabbbccc", "^ab") -> false
     * @usage regexpmatch ("aaabbbccc", "ab+c") -> true
     * @usage regexpmatch ("aaa(bbb)ccc", "\\(.*\\)") -> true     
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
     * @builtin regexppos 
     * @short  Returns a pair with position and length of the first match.
     * @param string INPUT
     * @param string PATTERN
     * @return list
     *
     * @description
     * If no match is found it returns an empty list.
     *
     * @see  regex(7).
     *
     * @usage regexppos ("abcd012efgh345", "[0-9]+") -> [4, 3]
     * @usage ("aaabbb", "[0-9]+") -> []
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
     * @builtin regexpsub
     * @short Regex Substitution
     * @param string INPUT
     * @param string PATTERN
     * @param string OUTPUT
     * @return string
     *
     * @description
     * Searches a string for a POSIX Extended Regular Expression match
     * and returns <i>OUTPUT</i> with the matched subexpressions
     * substituted or <b>nil</b> if no match was found.
     *
     * @see regex(7)
     *
     * @usage regexpsub ("aaabbb", "(.*ab)", "s_\\1_e") -> "s_aaab_e"
     * @usage regexpsub ("aaabbb", "(.*ba)", "s_\\1_e") -> nil
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
     * @builtin regexptokenize
     * @short Regex tokenize
     * @param string INPUT
     * @param string PATTERN
     * @return list
     *
     * @see regex(7).
     * @description
     * Searches a string for a POSIX Extended Regular Expression match
     * and returns a list of the matched subexpressions
     *
     * If the pattern does not match, the list is empty.
     * Otherwise the list contains then matchted subexpressions for each pair
     * of parenthesize in pattern.
     *
     * If the pattern is invalid, 'nil' is returned.
     *
     * @usage
     * Examples: 
     * // e ==  [ "aaabbB" ]
     * list e = regexptokenize ("aaabbBb", "(.*[A-Z]).*");
     *
     * // h == [ "aaab", "bb" ]
     * list h = regexptokenize ("aaabbb", "(.*ab)(.*)");
     *
     * // h == []
     * list h = regexptokenize ("aaabbb", "(.*ba).*");
     *
     * // h == nil
     * list h = regexptokenize ("aaabbb", "(.*ba).*(");
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
     * @builtin tostring 
     * @id tostring-any
     * @short Converts a value to a string.
     *
     * @param any VALUE
     * @return string
     *
     * @usage tostring(.path) -> ".path"
     * @usage tostring([1,2,3]) -> "[1, 2, 3]"
     * @usage tostring($[1:1,2:2,3:3]) -> "$[1:1, 2:2, 3:3]"
     * @usage tostring(`Empty()) -> "`Empty ()"
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
     * @builtin timestring
     * @short Returns time string
     * @description
     * Combination of standard libc functions gmtime or localtime and strftime.
     *
     * @param string FORMAT
     * @param integer TIME
     * @param boolean UTC
     * @return string
     *
     * @usage timestring ("%F %T %Z", time (), false) -> "2004-08-24 14:55:05 CEST"
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
     * @builtin crypt
     * @short Encrypts a string
     * @description
     * Encrypts the string <tt>UNENCRYPTED</tt> using the standard
     * password encryption provided by the system.
     * @param string UNENCRYPTED
     * @return string 
     *
     * @usage crypt ("readable") -> "Y2PEyAiaeaFy6"
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
     * @builtin cryptmd5
     * @short Encrypts a string using md5
     * @description
     * Encrypts the string <tt>UNENCRYPTED</tt> using MD5
     * password encryption.
     * @param string  UNENCRYPTED
     * @return string
     *
     * @usage cryptmd5 ("readable") -> "$1$BBtzrzzz$zc2vEB7XnA3Iq7pOgDsxD0"
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
     * @builtin cryptbigcrypt
     * @short Encrypts a string using bigcrypt
     * @description
     * Encrypts the string <tt>UNENCRYPTED</tt> using bigcrypt
     * password encryption. The password is not truncated.
     * @param string UNENCRYPTED
     * @return string
     *
     * @usage cryptbigcrypt ("readable") -> "d4brTQmcVbtNg"
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
     * @builtin cryptblowfish
     * @short Encrypts a string with blowfish
     * @description
     * Encrypts the string <tt>UNENCRYPTED</tt> using blowfish
     * password encryption. The password is not truncated.
     *
     * @param string UNENCRYPTED
     * @return string
     * @usage cryptblowfish ("readable") -> "$2a$05$B3lAUExB.Bqpy8Pq0TpZt.s7EydrmxJRuhOZR04YG01ptwOUR147C"
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
s_cryptsha256(const YCPString& original)
{
    /**
     * @builtin cryptsha256
     * @short Encrypts a string with sha256
     * @description
     * Encrypts the string <tt>UNENCRYPTED</tt> using sha256
     * password encryption. The password is not truncated.
     *
     * @param string UNENCRYPTED
     * @return string
     * @usage cryptsha256 ("readable") -> "$5$keev8D8I$kZdbw1WYM7XJtn4cpl1S3QtoKXnxIIFVSqwadMAGLE3"
     */

    if (original.isNull ())
	return YCPNull ();

    string unencrypted = original->value();
    string encrypted;

    if (crypt_pass (unencrypted, SHA256, &encrypted))
        return YCPString (encrypted);
    else
    {
	ycp2error ("Encryption using sha256 failed");
        return YCPNull ();
    }
}


static YCPValue
s_cryptsha512(const YCPString& original)
{
    /**
     * @builtin cryptsha512
     * @short Encrypts a string with sha512
     * @description
     * Encrypts the string <tt>UNENCRYPTED</tt> using sha512
     * password encryption. The password is not truncated.
     *
     * @param string UNENCRYPTED
     * @return string
     * @usage cryptsha512 ("readable") -> "$6$QskPAFTK$R40N1UI047Bg.nD96ZYSGnx71mgbBgb.UEtKuR8bGGxuzYgXjCTxKIQmqXrgftBzA20m2P9ayrUKQQ2pnWzm70"
     */

    if (original.isNull ())
	return YCPNull ();

    string unencrypted = original->value();
    string encrypted;

    if (crypt_pass (unencrypted, SHA512, &encrypted))
        return YCPString (encrypted);
    else
    {
	ycp2error ("Encryption using sha512 failed");
        return YCPNull ();
    }
}


static YCPValue
s_dgettext (const YCPString& domain, const YCPString& text)
{
    /**
     * @operator _(string text) -> string
     * Translates the text using the current textdomain. 
     *
     * Example <pre>
     * _("File") -> "Soubor"
     * </pre>
     */

    /**
     * @builtin dgettext
     * @short Translates the text using the given text domain
     * @description
     * Translates the text using the given text domain into
     * the current language.
     *
     * This is a special case builtin not intended for general use.
     * See _() instead.
     *
     * @param string textdomain
     * @param string text
     * @return string
     * @usage dgettext ("base", "No") -> "Nie"
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
     * @operator _(string singular, string plural, integer value) -> string
     * Translates the text using a locale-aware plural form handling and the 
     * current textdomain. The chosen form of the translation depends 
     * on the <tt>value</tt>.
     *
     * Example <pre>
     * _("%1 File", "%1 Files", 2) -> "%1 soubory"
     * </pre>
     */

    /**
     * @builtin dngettext
     * @short Translates the text using a locale-aware plural form handling
     * @description
     * Translates the text using a locale-aware plural form handling using
     * the given textdomain.
     *
     * The chosen form of the translation depend on the <tt>value</tt>.
     *
     * This is a special case builtin not intended for general use.
     * See _() instead.
     *
     * @param string textdomain
     * @param string singular
     * @param string plural
     * @param integer value
     * @return string
     *
     * @usage dngettext ("base", "%1 File", "%1 Files", 2) -> "%1 soubory"
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


static YCPValue
s_dpgettext (const YCPString& domain, const YCPString& domain_path, const YCPString& text)
{
    /**
     * @builtin dpgettext
     * @short Translates the text using the given text domain and path
     * @description
     * Translates the text using the given text domain into
     * the current language and path of localization .
     * Path of localization is same than dirname in function bindtextdomain()
     *
     * This is a special case builtin not intended for general use.
     * See _() instead.
     *
     * @param string textdomain
     * @param string dirname
     * @param string text
     * @return string
     * @usage dpgettext ("base", "/texdomain/path", "No") -> "Nie"
     */

    if ((domain.isNull () || domain->isVoid ()) ||
	(domain_path.isNull () || domain_path->isVoid ()) ||
	(text.isNull () || text->isVoid ())) 
    {
	return YCPNull ();
    }

    // initialize text domain if not done so
    string dom = domain->value ();
    string dom_path = domain_path->value ();

    // check if domain exist
    // it is important to bind domain back (LOCALEDIR) 
    // if it is already binded
    bool known_domain = false;
    if (YLocale::findDomain(dom))
	known_domain = true;

    YLocale::bindDomainDir (dom, dom_path);
    string result = dgettext (dom.c_str(), text->value().c_str());
    // if it is known domain bind it back to LOCALEDIR
    if (known_domain)
	YLocale::bindDomainDir (dom, LOCALEDIR);

    return YCPString (result);
}




YCPBuiltinString::YCPBuiltinString ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
#define ETC 0, NULL, constTypePtr(), NULL
#define ETCf   NULL, constTypePtr(), NULL
	{ "+",		   "string (string, string)",		(void *)s_plus1,                         ETC },
	{ "+",		   "string (string, integer)",		(void *)s_plus2,			 ETC },
	{ "+",		   "string (string, path)",		(void *)s_plus3,			 ETC },
	{ "+",		   "string (string, symbol)",		(void *)s_plus4,			 ETC },
	{ "issubstring",   "boolean (string, string)",		(void *)s_issubstring,			 ETC },
	{ "tostring",	   "string (any)",			(void *)s_tostring,			 ETC },
	{ "tohexstring",   "string (integer)",			(void *)s_tohexstring1,			 ETC },
	{ "tohexstring",   "string (integer, integer)",		(void *)s_tohexstring2,                  ETC },
	{ "isempty",	   "boolean (string)",			(void *)s_isempty,			 ETC },
	{ "size",	   "integer (string)",			(void *)s_size,				 ETC },
	{ "find",	   "integer (string, string)",		(void *)s_find,	DECL_DEPRECATED,        ETCf },
	{ "search",	   "integer (string, string)",		(void *)s_search,			 ETC },
	{ "tolower",	   "string (string)",			(void *)s_tolower,			 ETC },
	{ "toupper",	   "string (string)",			(void *)s_toupper,			 ETC },
	{ "toascii",	   "string (string)",			(void *)s_toascii,                       ETC },
	{ "deletechars",   "string (string, string)",		(void *)s_removechars,			 ETC },
	{ "filterchars",   "string (string, string)",		(void *)s_filterchars,			 ETC },
	{ "findfirstnotof","integer (string, string)",		(void *)s_findfirstnotof,		 ETC },
	{ "findfirstof",   "integer (string, string)",		(void *)s_findfirstof,			 ETC },
	{ "findlastof",	   "integer (string, string)",		(void *)s_findlastof,			 ETC },
	{ "findlastnotof", "integer (string, string)",		(void *)s_findlastnotof,		 ETC },
	{ "substring",	   "string (string, integer)",		(void *)s_substring1,                    ETC },
	{ "substring",	   "string (string, integer, integer)",	(void *)s_substring2,			 ETC },
	{ "timestring",	   "string (string, integer, boolean)",	(void *)s_timestring,			 ETC },
	{ "mergestring",   "string (const list <string>, string)", (void *)s_mergestring,		 ETC },
	{ "crypt",	   "string (string)",			(void *)s_crypt,			 ETC },
	{ "cryptmd5",	   "string (string)",			(void *)s_cryptmd5,			 ETC },
	{ "cryptbigcrypt", "string (string)",			(void *)s_cryptbigcrypt,		 ETC },
	{ "cryptblowfish", "string (string)",			(void *)s_cryptblowfish,                 ETC },
	{ "cryptsha256",   "string (string)",			(void *)s_cryptsha256,                   ETC },
	{ "cryptsha512",   "string (string)",			(void *)s_cryptsha512,                   ETC },
	{ "regexpmatch",   "boolean (string, string)",		(void *)s_regexpmatch,			 ETC },
	{ "regexppos",	   "list<integer> (string, string)",	(void *)s_regexppos,			 ETC },
	{ "regexpsub",	   "string (string, string, string)",	(void *)s_regexpsub,			 ETC },
	{ "regexptokenize","list <string> (string, string)",	(void *)s_regexptokenize,		 ETC },
	{ "dgettext",	   "string (string, string)",		(void *)s_dgettext,			 ETC },
	{ "dngettext",	   "string (string, string, string, integer)",	(void *)s_dngettext,		 ETC },
	{ "dpgettext",	   "string (string, string, string)",	(void *)s_dpgettext,                     ETC },
	{ "lsubstring",	   "string (string, integer)",		(void *)s_lsubstring1,                   ETC },
	{ "lsubstring",	   "string (string, integer, integer)",	(void *)s_lsubstring2,                   ETC },
	{ NULL, NULL, NULL, ETC }
#undef ETC
#undef ETCf
    };

    static_declarations.registerDeclarations ("YCPBuiltinString", declarations);
}
