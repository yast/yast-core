/**
 *
 *  AnyAgentBasic.cc
 *
 *  Purpose:	basic type handling for AnyAgent
 *
 *  Creator:	kkaempf@suse.de
 *  Maintainer:	kkaempf@suse.de
 *
 */

#include <stdio.h>
#include <stdlib.h>
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


// parse IP4 number
// nnn.nnn.nnn.nnn

YCPValue
AnyAgent::parseIp4Number (char const *&lptr, bool optional)
{
    long long num = 0LL;
    int dotcount = 0;

    while (dotcount < 4)
    {
	if (!isdigit (lptr[0]))
	{
	    y2error ("parseIp4Number not starting with digit");
	    return YCPVoid ();
	}
	num <<= 8;
	int i = atoi (lptr);
	if ((i < 0) || (i > 255))
	{
	    y2error ("parseIp4Number bad value %d", i);
	    return YCPVoid ();
	}
	num += i;
	while (isdigit (lptr[0]))
	    lptr++;
	if (dotcount < 3)
	{
	    if (lptr[0] != '.')
	    {
		y2error ("parseIp4Number no dot, %d", dotcount);
		return YCPVoid ();
	    }
	    lptr++;
	}
	dotcount++;
    }

    return YCPInteger (num);
}


// unparseIp4Number
//

const string
AnyAgent::unparseIp4Number (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isInteger ()))
	return "";

    char s[61];			// nnn.nnn.nnn.nnn\0
    long i = value->asInteger ()->value () & 0xffffffffL;
    snprintf (s, 61, "%ld.%ld.%ld.%ld", (i >> 24) & 0xff, (i >> 16) & 0xff,
	      (i >> 8) & 0xff, i & 0xff);

    y2debug ("unparseIp4Number (%s,%s)", value->toString ().c_str (), s);
    return string (s);
}


// parse boolean
//

YCPValue
AnyAgent::parseBoolean (char const *&lptr, bool optional)
{
    if (strncmp (lptr, "yes", 3) == 0)
    {
	lptr += 3;
	return YCPBoolean (true);
    }
    else if (strncmp (lptr, "no", 2) == 0)
    {
	lptr += 2;
	return YCPBoolean (false);
    }
    if (!optional)
	y2error ("*** not a bool");
    return YCPNull ();
}


// unparseBoolean
//

const string
AnyAgent::unparseBoolean (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isBoolean ()))
	return "";

    y2debug ("unparseBoolean (%s)", value->toString ().c_str ());
    return value->asBoolean ()->value () ? "yes" : "no";
}


// parse integer number
//

YCPValue
AnyAgent::parseNumber (char const *&lptr, bool optional)
{
    long long num = 0LL;

    if (!isdigit (lptr[0]))
    {
	if (!optional)
	    y2error ("*** not a number");
	return YCPNull ();
    }
    num = atoll (lptr);
    while (isdigit (lptr[0]))
	lptr++;

    return YCPInteger (num);
}


// unparseNumber
//

const string
AnyAgent::unparseNumber (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isInteger ()))
	return "";

    // (64bit signed max) -> 9223372036854775807L
    char s[32];
    sprintf (s, "%Ld", value->asInteger ()->value ());

    y2debug ("unparseNumber (%s, %s)", value->toString ().c_str (), s);
    return string (s);
}


// parse hex value
//

YCPValue
AnyAgent::parseHexval (char const *&lptr, bool optional)
{
    long long num = 0LL;

    if (!isxdigit (lptr[0]))
    {
	if (!optional)
	    y2error ("*** not a hex value");
	return YCPNull ();
    }
    // allow 0x as start
    if ((lptr[0] == '0') && ((lptr[1] == 'x') || (lptr[1] == 'X')))
	lptr += 2;

    while (isxdigit (lptr[0]))
    {
	char x = lptr[0];
	x -= '0';
	if (x > 10)
	{
	    x -= 'A' - '0' - 10;
	    if (x > 16)
	    {
		x -= ('a' - 'A');
		if (x > 16)
		    break;
	    }
	}

	num <<= 4;
	num += x;
	lptr++;
    }

    return YCPInteger (num);
}


// unparseHexval
//

const string
AnyAgent::unparseHexval (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isInteger ()))
	return "";

    // 64bit -> 16 digits
    char s[32];
    sprintf (s, "%Lx", value->asInteger ()->value ());

    y2debug ("unparseHexval (%s, %s)", value->toString ().c_str (), s);
    return string (s);
}


// parse string from matching set
//

YCPValue
AnyAgent::parseString (char const *&lptr, const char *set, const char *stripped,
		       bool optional)
{
    char const *start = lptr;

    // increment lptr according to match

    if (*set == '^')
    {
	set++;
	while ((*lptr != 0) && (strchr (set, *lptr) == 0))
	    lptr++;
    }
    else
    {
	while ((*lptr != 0) && (strchr (set, *lptr) != 0))
	    lptr++;
    }
    if ((start == lptr) && (*set != 0))
	return YCPNull ();

    // adjust start and end for stripping

    char const *end = lptr;

    if (stripped != 0)
    {
	while ((start < end) && (strchr (stripped, *start) != 0))
	    start++;
	while ((start < end) && (strchr (stripped, *(end - 1)) != 0))
	    end--;
    }

    string s (start, end - start);
    return YCPString (s);
}


// unparseString
//

const string
AnyAgent::unparseString (const YCPValue & syntax, const YCPValue & stripped,
			 const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isString ()) ||
	(syntax.isNull ()) || (!syntax->isString ()))
	return "";

    const char *v = value->asString ()->value ().c_str ();
    const char *set = syntax->asString ()->value ().c_str ();

    string s;

    if (*v != 0)
    {
	s.reserve (strlen (v));
	if (*set == '^')
	{
	    set++;
	    while (*v != 0 && strchr (set, *v) == 0)
		s += *v++;
	}
	else
	{
	    while (*v != 0 && strchr (set, *v) != 0)
		s += *v++;
	}
    }

    y2debug ("unparseString (%s[%s],%s)", value->toString ().c_str (), set,
	     s.c_str ());
    return s;
}


// parse float number
// ddd.ddd

YCPValue
AnyAgent::parseFloat (char const *&lptr, bool optional)
{
    const char * start = lptr;
    y2debug ("parseFloat (%s)", lptr);

    if (!isdigit (*lptr) && (*lptr != '.'))
	return YCPNull ();

    while (isdigit (*lptr))
	lptr++;
    if (*lptr == '.')
	lptr++;
    while (isdigit (*lptr))
	lptr++;

    float value;
    sscanf (start, "%f", &value);
    return YCPFloat ((double) value);
}


// unparseFloat
//

const string
AnyAgent::unparseFloat (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isFloat ()))
	return "";

    char s[64];
    snprintf (s, 64, "%f", value->asFloat ()->value ());

    y2debug ("unparseFloat (%s,%s)", value->toString ().c_str (), s);
    return string (s);
}


// parse hostname
//

YCPValue
AnyAgent::parseHostname (char const *&lptr, bool optional)
{
    const char * start = lptr;

    if (!isalpha (*lptr))
    {
	if (!optional)
	    y2error ("*** bad hostname");
	return YCPNull ();
    }

    while (*lptr)
    {
	if (!isalnum (*lptr))
	{
	    int stop = 0;
	    switch (*lptr)
	    {
		case '_':
		    break;
		case '.':
		    if (!isalpha (*(lptr + 1)))
			stop = 1;
		    break;
		default:
		    stop = 1;
		    break;
	    }
	    if (stop)
		break;
	}
	lptr++;
    }

    string s (start, lptr - start);
    return YCPString (s);
}


// unparseHostname
//

const string
AnyAgent::unparseHostname (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isString ()))
	return "";

    const char *v = value->asString ()->value ().c_str ();
    string s;

    if (isalpha (*v))
    {
	s.reserve (strlen (v));
	while (isalnum (*v) || (*v == '_') ||
	       ((*v == '.') && (isalpha (*(v + 1)))))
	    s.append (1, *v++);
    }

    y2debug ("unparseHostname (%s,%s)", value->toString ().c_str (), s.c_str ());
    return s;
}


// parse username
//

YCPValue
AnyAgent::parseUsername (char const *&lptr, bool optional)
{
    const char * start = lptr;

    if (!isalpha (*lptr))
    {
	if (!optional)
	    y2error ("*** bad username");
	return YCPNull ();
    }

    while (*lptr)
    {
	if (!isalnum (*lptr))
	    break;
	lptr++;
    }

    string s (start, lptr - start);
    return YCPString (s);
}


// unparseUsername
//

const string
AnyAgent::unparseUsername (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isString ()))
	return "";

    const char *v = value->asString ()->value ().c_str ();
    string s;

    if (isalpha (*v))
    {
	s.reserve (strlen (v));
	while (isalnum (*v))
	    s += *v++;
    }

    y2debug ("unparseUsername (%s,%s)", value->toString ().c_str (), s.c_str ());
    return s;
}


YCPValue
AnyAgent::parseVerbose (char const *&lptr, const char *match, bool optional)
{
    const int n = strlen (match);

    if (strncmp (lptr, match, n) == 0)
    {
	const char * start = lptr;
	lptr += n;
	return YCPString (string (start, n));
    }

    if (optional)
	return YCPVoid ();

    return YCPNull ();
}


// unparseVerbose
//

const string
AnyAgent::unparseVerbose (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isString ()))
	return "";

    const char *v = value->asString ()->value ().c_str ();
    string s = string (v);

    y2debug ("unparseVerbose (%s,%s)", value->toString ().c_str (), s.c_str ());
    return s;
}


// parse separator
//

YCPValue
AnyAgent::parseSeparator (char const *&lptr, const char *match, bool optional)
{
    char const *start = lptr;

    while ((*lptr != 0) && (strchr (match, *lptr) != 0))
	lptr++;

    return (optional || (lptr > start)) ?
	YCPString (string (start, lptr - start)) : YCPNull ();
}


// unparseSeparator
//

const string
AnyAgent::unparseSeparator (const YCPValue & value)
{
    if ((value.isNull ()) || (!value->isString ()))
	return "";

    const char *v = value->asString ()->value ().c_str ();

    if (*v == 0)
	return "";

    y2debug ("unparseSeparator (%c)", *v);
    return string (1, *v);
}

