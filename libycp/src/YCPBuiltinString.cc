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

   File:       YCPBuiltinString.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/


#ifndef _GNU_SOURCE
#define _GNU_SOURCE		// for asprintf
#endif

#define ERR_MAX 80              // for regexp
#define SUB_MAX 10              // for regexp

#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <regex.h>
#include <string>

using std::string;

#include "YCPInterpreter.h"
#include "toString.h"
#include "y2log.h"
#include "y2crypt.h"

////////////////////////////////////////////////////////////////////////////////
//                             typedefs                                       //
////////////////////////////////////////////////////////////////////////////////

struct Reg_Ret
{
    string  result_str;
    string  match_str[SUB_MAX+1];  // index 0 not used!!
    int     match_nb;       // 0 - 9
    string  error_str;
    bool    error;
    bool    solved;
};


////////////////////////////////////////////////////////////////////////////////
//                         local prototypes                                   //
////////////////////////////////////////////////////////////////////////////////

Reg_Ret solve_regular_expression( const char        *test_str,
				  const char        *pattern,
				  const char        *result);




////////////////////////////////////////////////////////////////////////////////
//                            BUILTINS                                        //
////////////////////////////////////////////////////////////////////////////////


YCPValue evaluateStringOp(YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    string arg1 = args->value(0)->asString()->value();

    if (code == YCPB_SIZE) {
	/**
	 * @builtin size(string s) -> integer
	 * Returns the number of characters of the string <tt>s</tt>
	 */
	return YCPInteger(arg1.length());
    }
    else if (code != YCPB_PLUS)
    {
	return YCPError("Undefined string operation", YCPNull());
    }

    /**
     * @builtin +(string a, string|integer|path|symbol b) -> string
     * Concatenates <tt>a</tt> and (the string representation of) <tt>b</tt>
     * Example <pre>
     * "(" + 123 + ")" -> "(123)"
     * </pre>
     */
    if (args->size() < 2)
	return YCPError ("wrong number of arguments for string addition");

    YCPValue arg2 = args->value(1);

    switch (arg2->valuetype()) {
	case YT_STRING:
	    return YCPString (arg1 + arg2->asString()->value_cstr());
	break;
	case YT_INTEGER:
	    return YCPString(arg1 + toString(arg2->asInteger()->value()));
	break;
	case YT_PATH:
	    return YCPString(arg1 + arg2->asPath()->toString());
	break;
	case YT_SYMBOL:
	    return YCPString(arg1 + arg2->asSymbol()->symbol());
	break;
	default:
	break;
    }
    return YCPError ("cant add this type to string", YCPNull());
}


YCPValue evaluateIsSubString(YCPInterpreter *interpreter, const YCPList& args)
{
    if ( args->size() == 2 &&
	 args->value(0)->isString() && args->value(1)->isString() )
    {
	/**
	 * @builtin issubstring(string s, string substring) -> bool
	 * true, if <tt>substring</tt> is a substring of <tt>s</tt>
	 *
	 * Example <pre>
	 * issubstring("some text", "tex") -> true
	 * </pre>
	 */

	string s         = args->value(0)->asString()->value();
	string substring = args->value(1)->asString()->value();

	return YCPBoolean( string::npos != s.find( substring ) );
    }
    return YCPError ("bad arguments for issubstring", YCPNull());
}


YCPValue evaluateSformat(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin sformat(string form, any par1, any par2, ...) -> string
     * form is a string that may contains placeholders %1, %2, ...
     * Each placeholder is substituted with the argument converted
     * to string whose number is after the %. Only 1-9 are allowed
     * by now. The procent sign is donated with %%.
     *
     * Example <pre>
     * sformat("%2 is greater %% than %1", 3, "five") -> "five is greater % than 3"
     * </pre>
     */

    if (args->size() <= 0 || args->value(0).isNull() || !args->value(0)->isString())
    {
	return YCPError ("First argument to sformat is not a string");
    }

    string format = args->value(0)->asString()->value();
    string result = "";
    const char *read = format.c_str();
    while (*read) {
	if (*read == '%') {
	    read++;
	    if (*read == '%') result += "%";
	    else if (*read >= '1' && *read <= '9')
	    {
		int num = *read-'0';
		if (args->size() <= num)
		{
		    y2error ("Illegal argument number %%%d in formatstring %s",
			       num, args->value(0)->asString()->toString().c_str());
		}
		else if (args->value(num)->isString())
		    result += args->value(num)->asString()->value();
		else
		    result += args->value(num)->toString();
	    }
	    else
	    {
		y2error ("%% in formatstring %s missing a number",
			   args->value(0)->asString()->toString().c_str());
	    }
	    read++;
	}
	else result += *read++;
    }

    return YCPString(result);
}


YCPValue evaluateToString(YCPInterpreter *interpreter, const YCPList& args)
{
    if (args->size() == 2 && args->value(0)->isFloat() &&
	args->value(1)->isInteger()) {

	    /**
	     * @builtin tostring(float f, integer precision) -> string
	     * Converts a floating point number to a string, using the
	     * specified precision.
	     *
	     * Example <pre>
	     * tostring(0.12345, 4) -> 0.1235
	     * </pre>
	     */
	YCPFloat value = args->value(0)->asFloat();
	YCPInteger precision = args->value(1)->asInteger();
	char *buffer;

	asprintf (&buffer, "%.*f", int (precision->value()), value->value());
	YCPValue ret = YCPString (buffer);
	free (buffer);
	return ret;
    }
    return YCPError ("bad arguments for toString()", YCPNull());
}


YCPValue evaluateToHexString(YCPInterpreter *interpreter, const YCPList& args)
{
    if (args->size() == 1 && args->value(0)->isInteger()) {

	    /**
	     * @builtin tohexstring(integer number) -> string
	     * Converts a integer to a hexadecimal string
	     *
	     * Example <pre>
	     * tohexstring(31) -> "0x1f"
	     * </pre>
	     */
	YCPInteger value = args->value(0)->asInteger();
	char *buffer;

	asprintf (&buffer, "0x%x", int (value->value()));
	YCPValue ret = YCPString (buffer);
	free (buffer);
	return ret;
    }
    else return YCPNull();
}



YCPValue evaluateSubString(YCPInterpreter *interpreter, const YCPList& args)
{
y2debug ("evaluateSubString (%s)\n", args->toString().c_str());
    if ((args->size() == 2 || args->size() == 3) &&
	args->value(0)->isString() && args->value(1)->isInteger() &&
	(args->size() == 2 || args->value(2)->isInteger())) {
	/**
	 * @builtin substring(string s, integer start, integer length) -> string
	 * Extract a substring of the string <tt>s</tt>, starting at
	 * <tt>start</tt> after the first one with length of at most <tt>length</tt>.
	 *
	 * Example <pre>
	 * substring("some text", 5, 2) -> "te"
	 * </pre>
	 */

	/**
	 * @builtin substring(string s, integer start) -> string
	 * Extract a substring of the string <tt>s</tt>, starting at
	 * <tt>start</tt> after the first one.
	 *
	 * Example <pre>
	 * substring("some text", 5) -> "text"
	 * </pre>
	 */

	string s = args->value(0)->asString()->value();
	string::size_type start = args->value(1)->asInteger()->value();
	string::size_type length = args->size() == 3 ? args->value(2)->asInteger()->value() : string::npos;
	if (start > s.size()) {
	    return YCPError ("Substring index out of range", YCPString(""));
	}

	return YCPString(s.substr(start, length));
    }
    return YCPError ("bad arguments for substring()", YCPNull());
}


YCPValue evaluateToLower (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin tolower (string s) -> string
     * Returns a string that results from string<tt>s</tt> by
     * converting each character tolower.
     *
     * Example <pre>
     * tolower ("aBcDeF") -> "abcdef"
     * </pre>
     */

    if (args->size () == 1 && args->value (0)->isString ())
    {
	string s = args->value (0)->asString ()->value ();
	for (unsigned i = 0; i < s.size (); i++)
	{
	    s[i] = tolower (s[i]);
	}

	return YCPString (s);
    }
    else return YCPNull ();
}


YCPValue evaluateToUpper (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin toupper (string s) -> string
     * Returns a string that results from string<tt>s</tt> by
     * converting each character toupper.
     *
     * Example <pre>
     * tolower ("aBcDeF") -> "ABCDEF"
     * </pre>
     */

    if (args->size () == 1 && args->value (0)->isString ())
    {
	string s = args->value (0)->asString ()->value ();
	for (unsigned i = 0; i < s.size (); i++)
	{
	    s[i] = toupper (s[i]);
	}

	return YCPString (s);
    }
    else return YCPNull ();
}


YCPValue evaluateToASCII(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin toascii( string s ) -> string
    * Returns a string that results from string <tt>s</tt> by
    * copying each character that is below 0x7F (127).
    *
    * Example <pre>
    * toascii( "aÖBÄc" ) -> "aBc"
    * </pre>
    */

   if ( args->size() == 1 && args->value(0)->isString() )
   {
      string s = args->value(0)->asString()->value();
      unsigned w = 0;
      for ( unsigned i = 0; i < s.size(); i++ )
      {
	 if (isascii(s[i])) s[w++] = s[i];
      }

      return YCPString( s.substr(0,w) );
   }
   else return YCPNull();
}


YCPValue evaluateRemoveChars(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin deletechars(string s, string remove) -> string
    * Returns a string that results from string <tt>s</tt> by removing all
    * characters that occur in <tt>remove</tt>
    *
    * Example <pre>
    * deletechars("aÖBÄc", "abcdefghijklmnopqrstuvwxyz") -> "ÖBÄ"
    * </pre>
    */

   if (args->size() == 2 && args->value(0)->isString() &&
       args->value(1)->isString())
   {
      string s = args->value(0)->asString()->value();
      string include = args->value(1)->asString()->value();
      string::size_type pos = 0;
      while (pos = s.find_first_of(include, pos), pos != string::npos)
      {
	  s.erase(pos, s.find_first_not_of(include, pos) - pos);
      }

      return YCPString(s);
   }
   else return YCPNull();
}


YCPValue evaluateFilterChars(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin filterchars(string s, string include) -> string
    * Returns a string that results from string <tt>s</tt> by removing all
    * characters that do not occur in <tt>include</tt>
    *
    * Example <pre>
    * filterchars("aÖBÄc", "abcdefghijklmnopqrstuvwxyz") -> "ac"
    * </pre>
    */

   if (args->size() == 2 && args->value(0)->isString() &&
       args->value(1)->isString())
   {
      string s = args->value(0)->asString()->value();
      string include = args->value(1)->asString()->value();
      string::size_type pos = 0;
      while (pos = s.find_first_not_of(include, pos), pos != string::npos)
      {
	  s.erase(pos, s.find_first_of(include, pos) - pos);
      }

      return YCPString(s);
   }
   return YCPError ("bad arguments for filterchars()", YCPNull());
}


YCPValue evaluateCrypt(YCPInterpreter *interpreter, const YCPList &args)
{
    /**
     * @builtin crypt (string unencrypted) -> string
     * Encrypt the string <tt>unencrypted</tt> using the standard
     * password encryption. The password is not truncated.
     *
     * Example <pre>
     * crypt ("readable") -> "nvot7qjTCJfYU"
     * </pre>
     */

    if (args->size() == 1 && args->value(0)->isString())
    {
	string unencrypted = args->value(0)->asString()->value();
	string encrypted;

	if (crypt_pass (unencrypted, CRYPT, &encrypted))
	    return YCPString (encrypted);
	else
	    return YCPError ("Encryption using crypt failed");
    }

    return YCPNull();
}


YCPValue evaluateCryptMD5(YCPInterpreter *interpreter, const YCPList &args)
{
    /**
     * @builtin cryptmd5 (string unencrypted) -> string
     * Encrypt the string <tt>unencrypted</tt> using md5
     * password encryption. The password is not truncated.
     *
     * Example <pre>
     * cryptmd5 ("readable") -> "$1$kyzzbyzz$VhfZH1puuxPL7.0.W75SX."
     * </pre>
     */

    if (args->size() == 1 && args->value(0)->isString())
    {
	string unencrypted = args->value(0)->asString()->value();
	string encrypted;

	if (crypt_pass (unencrypted, MD5, &encrypted))
	    return YCPString (encrypted);
	else
	    return YCPError ("Encryption using md5 failed");
    }

    return YCPNull();
}


YCPValue evaluateCryptBigcrypt(YCPInterpreter *interpreter, const YCPList &args)
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

    if (args->size() == 1 && args->value(0)->isString())
    {
	string unencrypted = args->value(0)->asString()->value();
	string encrypted;

	if (crypt_pass (unencrypted, BIGCRYPT, &encrypted))
	    return YCPString (encrypted);
	else
	    return YCPError ("Encryption using bigcrypt failed");
    }

    return YCPNull();
}


YCPValue evaluateCryptBlowfish(YCPInterpreter *interpreter, const YCPList &args)
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

    if (args->size() == 1 && args->value(0)->isString())
    {
	string unencrypted = args->value(0)->asString()->value();
	string encrypted;

	if (crypt_pass (unencrypted, BLOWFISH, &encrypted))
	    return YCPString (encrypted);
	else
	    return YCPError ("Encryption using blowfish failed");
    }

    return YCPNull();
}


YCPValue evaluateMergeString(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin mergestring(list (string) l, string c) -> string
    * Merges a list of strings to a single list. Inserts c between
    * list elements (c may be empty).
    * List elements which are not of type strings are ignored.
    *
    * see also: splitstring
    *
    * Example <pre>
    * mergestring(["", "abc", "dev", "ghi" ], "/") -> "/abc/dev/ghi"
    * mergestring(["abc", "dev", "ghi", "" ], "/") -> "abc/dev/ghi/"
    * mergestring([ 1, "a", 3], ".") -> "a"
    * mergestring([ ], ".") -> ""
    * mergestring(["abc", "dev", "ghi" ], "") -> "abcdevghi"
    * mergestring(["abc", "dev", "ghi" ], "123") -> "abc123dev123ghi"
    * </pre>
    */

   if (args->size() == 2 && args->value(0)->isList() &&
       args->value(1)->isString())
   {
      string s;

      YCPList l = args->value(0)->asList();
      if (l->isEmpty())				// empty list -> empty result
	return YCPString(s);

      string c = args->value(1)->asString()->value();

      int i;
      bool started = false;

      // loop through list

      for (i = 0; i < l->size(); i++) {
	YCPValue vv = l->value(i);
	if (!vv->isString())		// skip non-string elements
	  continue;
	string vs = vv->asString()->value();

	if (started)			// insert c *between* strings
	  s += c;

	s += vs;			// add string to result
	started = true;			// mark as between
      }

      return YCPString(s);
   }
   return YCPError ("bad arguments for mergestring()", YCPNull());
}


YCPValue evaluateFindFirstNotOf(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin findfirstnotof( string s_1, string s_2 ) -> integer
     * Returns the position of the first character in <tt>s_1</tt> that is
     * NOT contained in <tt>s_2</tt>.
     *
     * Example <pre>
     * findfirstnotof( "abcdefghi", "abcefghi" ) -> 3 ('d')
     * findfirstnotof("aaaaa", "a") -> nil
     * </pre>
     */

    if ( args->size() == 2 && args->value(0)->isString() && args->value(1)->isString() )
    {
	// this is needed on gcc 3.0. gcc 2.95 was wrong.
	if (args->value(1)->asString()->value().empty ())
	    return YCPInteger ((long long int) 0);

	string s = args->value(0)->asString()->value();
	string::size_type pos = s.find_first_not_of( args->value(1)->asString()->value() );

	if ( pos == s.npos ) return YCPVoid();	// not found
	else return YCPInteger( pos );		// found
    }
    return YCPError ("bad arguments for findfirstnotof()", YCPNull());
}


YCPValue evaluateFindFirstOf(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin findfirstof( string s_1, string s_2 ) -> integer
    * Returns the position of the first character in <tt>s_1</tt> that is
    * contained in <tt>s_2</tt>.
    *
    * Example <pre>
    * findfirstof( "abcdefghi", "cxdv" ) -> 2 ('c')
    * findfirstof("aaaaa", "z") -> nil
    * </pre>
    */

   if ( args->size() == 2 && args->value(0)->isString() && args->value(1)->isString() )
   {
      string s = args->value(0)->asString()->value();
      string::size_type pos = s.find_first_of( args->value(1)->asString()->value() );

      if ( pos == s.npos ) return YCPVoid();	// not found
      else return YCPInteger( pos );		// found
   }
    return YCPError ("bad arguments for findfirstof()", YCPNull());
}



YCPValue evaluateFindLastOf(YCPInterpreter *interpreter, const YCPList& args)
{
   /**
    * @builtin findlastof( string s_1, string s_2 ) -> integer
    * Returns the position of the last character in <tt>s_1</tt> that is
    * contained in <tt>s_2</tt>.
    *
    * Example <pre>
    * findlastof( "abcdecfghi", "cxdv" ) -> 5 ('c')
    * findlastof("aaaaa", "z") -> nil
    * </pre>
    */

   if ( args->size() == 2 && args->value(0)->isString() && args->value(1)->isString() )
   {
      string s = args->value(0)->asString()->value();
      string::size_type pos = s.find_last_of( args->value(1)->asString()->value() );

      if ( pos == s.npos ) return YCPVoid();	// not found
      else return YCPInteger( pos );		// found
   }
    return YCPError ("bad arguments for findlastof()", YCPNull());
}


/**
 * @builtin regexpmatch( string input, string pattern ) -> boolean
 *
 * Example <pre>
 * regexpmatch( "aaabbb", ".*ab.*" ) -> true
 * regexpmatch( "aaabbb", ".*ba.*" ) -> false
 * </pre>
 */
YCPValue evaluateRegexpMatch(YCPInterpreter *interpreter, const YCPList& args)
{
    if( args->size() != 2 || !args->value(0)->isString() || !args->value(1)->isString() )
	return YCPError("Wrong Arguments to regexpmatch( string input, string pattern )");

    const char *input   = args->value(0)->asString()->value().c_str();
    const char *pattern = args->value(1)->asString()->value().c_str();

    Reg_Ret result = solve_regular_expression( input, pattern, "" );

    if(result.error) {
	y2error("Error in regexp <%s> <%s>: %s", input, pattern, result.error_str.c_str());
	return YCPNull();
    }

    return YCPBoolean(result.solved);
}


/**
 * @builtin regexpsub( string input, string pattern, string match ) -> string
 *
 * Example <pre>
 * regexpsub( "aaabbb", "(.*ab).*",  "s_\\1_e" ) -> "s_aaab_e"
 * regexpsub( "aaabbb", "(.*ba).*",  "s_\\1_e" ) -> nil
 * </pre>
 *
 */
YCPValue evaluateRegexpSub(YCPInterpreter *interpreter, const YCPList& args)
{
    if (args->size() != 3 || !args->value(0)->isString() ||
	!args->value(1)->isString() || !args->value(2)->isString())
    {
	return YCPError ("Wrong Arguments to regexpsub (string input, "
			 "string pattern, string match)");
    }

    const char *input   = args->value(0)->asString()->value().c_str();
    const char *pattern = args->value(1)->asString()->value().c_str();
    const char *match   = args->value(2)->asString()->value().c_str();

    Reg_Ret result = solve_regular_expression (input, pattern, match);

    if (result.error)
    {
	y2error ("Error in regexp <%s> <%s> <%s>: %s", input, pattern,
		 match, result.error_str.c_str());
    }

    if (result.solved)
    {
	return YCPString (result.result_str.c_str());
    }
    else
    {
	return YCPVoid ();
    }
}


/**
 * @builtin regexptokenize( string input, string pattern ) -> list handle
 *
 * !Attention pattern have to include parenthesize "(" ")"
 * If you need no parenthesize, use regexp_match
 *
 * If the pattern does not not match, the list ist empty.
 * Otherwise the list contains then matchted subexpressions for each pair
 * of parenthesize in pattern.
 *
 * If pattern does not contain a valid pattern, nil is returned.
 * In the include "common_functions, there are some convinience function, like tokenX
 * or regexp_error
 *
 * Example <pre>
 * list e = regexptokenize( "aaabbBb", "(.*[A-Z]).*")
 *
 * // e ==  [ "aaabbB" ]
 *
 *
 * list h = regexptokenize( "aaabbb", "(.*ab)(.*)");
 *
 * // h == [ "aaab", "bb" ]
 *
 * include "wizard/common_functions.ycp"
 * token1(h)         -> "aaab"
 * token2(h)         -> "bb"
 *   ...
 * token9(h)         -> ""
 * regexp_matched(h) -> true
 * regexp_error(h)   -> false
 *
 * list h = regexptokenize( "aaabbb", "(.*ba).*");
 *
 * // h == []
 *
 * token1(h)         ->  ""
 * token2(h)         ->  ""
 *   ...
 * token9(h)         ->  ""
 * regexp_matched(h) ->  false
 * regexp_error(h)   ->  false
 *
 *
 * list h = regexptokenize( "aaabbb", "(.*ba).*(");
 *
 * // h == nil
 *
 * token1(h)         ->  ""
 * token2(h)         ->  ""
 *   ...
 * token9(h)         ->  ""
 * regexp_matched(h) ->  nil
 * regexp_error(h)   ->  true
 * </pre>
 */
YCPValue evaluateRegexpTokenize(YCPInterpreter *interpreter, const YCPList& args)
{
    if( args->size() != 2 || !args->value(0)->isString() || !args->value(1)->isString() )
	return YCPError("Wrong Arguments to regexpmatch( string input, string pattern )");

    const char *input   = args->value(0)->asString()->value().c_str();
    const char *pattern = args->value(1)->asString()->value().c_str();

    Reg_Ret result = solve_regular_expression( input, pattern, "" );

    if(result.error) {
	y2error("Error in regexp <%s> <%s>: %s", input, pattern, result.error_str.c_str());
	return YCPNull();
    }

    YCPList list;
    if(result.solved)
	for(int i=1; i<=result.match_nb; i++)
	    list->add( YCPString( result.match_str[i] ));

    return list;
}


/*
 * Universal regular expression solver.
 * It is used by all regex* ycp builtins.
 */
Reg_Ret solve_regular_expression( const char        *input,
				  const char        *pattern,
				  const char        *result)
{
    int status;
    char error[ERR_MAX+1];

    regex_t compiled;
    regmatch_t matchptr[SUB_MAX+1];

    Reg_Ret reg_ret;
    reg_ret.match_nb = 0;
    reg_ret.error = true;
    reg_ret.error_str = "";

    status = regcomp (&compiled, pattern, REG_EXTENDED);
    if(status) {
	regerror(status, &compiled, error, ERR_MAX);
	reg_ret.error_str = string(error);
	return reg_ret;
    }

    if(compiled.re_nsub > SUB_MAX) {
	snprintf(error, ERR_MAX, "too much subexpresions: %zd", compiled.re_nsub);
	reg_ret.error_str = string(error);
	regfree(&compiled);
	return reg_ret;
    }

    status = regexec(&compiled, input, compiled.re_nsub+1, matchptr, 0);
    reg_ret.solved = !status;
    reg_ret.error = false;

    if(status) {
	regfree(&compiled);
	return reg_ret;
    }

    static const char *index[SUB_MAX+1] = {
        NULL, /* not used */
	"\\0", "\\1", "\\2", "\\3", "\\4",
	"\\5", "\\6", "\\7", "\\8", "\\9"
    };

    string input_str(input);
    string result_str(result);

    for(unsigned int i=1; (i <= compiled.re_nsub) && (i <= SUB_MAX); i++) {
	reg_ret.match_str[i] = matchptr[i].rm_so >= 0 ? input_str.substr(matchptr[i].rm_so, matchptr[i].rm_eo - matchptr[i].rm_so) : "";
	reg_ret.match_nb = i;

	string::size_type col = result_str.find(index[i]);
	while( col != string::npos ) {
	    result_str.replace( col, 2, reg_ret.match_str[i]  );
	    col = result_str.find(index[i], col + 1 );
	}
    }

    reg_ret.result_str = result_str;
    regfree (&compiled);
    return reg_ret;
}
