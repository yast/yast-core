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

   File:	YCPLocale.cc

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
/*
 * YCPLocale data type
 */


#include <libintl.h>

#include "y2log.h"
#include "YCPString.h"
#include "YCPLocale.h"


// YCPLocaleRep

YCPLocaleRep::YCPLocaleRep (const YCPString& msg1)
    : has_msg2 (false),
      msg1 (msg1),
      msg2 (""),
      num ((long long int) 0)
{
}


YCPLocaleRep::YCPLocaleRep (const YCPString& msg1, const YCPString& msg2,
			    const YCPInteger& num)
    : has_msg2 (true),
      msg1 (msg1),
      msg2 (msg2),
      num (num)
{
}


YCPString YCPLocaleRep::value () const
{
    return msg1;
}


YCPString YCPLocaleRep::translate (const char *textdomain) const
{
    const char* ret;

    if (!has_msg2) {
	const char* s = msg1->value_cstr ();
	ret = dgettext (textdomain, s);
	y2debug ("localize <%s> to <%s>", s, ret);
    } else {
	const char* s = msg1->value_cstr ();
	const char* p = msg2->value_cstr ();
	const int n = num->value ();
	ret = dngettext (textdomain, s, p, n);
	y2debug ("localize <%s, %s, %d> to <%s>", s, p, n, ret);
    }

    return YCPString (ret);
}


string YCPLocaleRep::toString () const
{
    if (!has_msg2)
	return string ("_(") + msg1->toString () + ")";
    else
	return string ("_(") + msg1->toString () + ", " +
	    msg2->toString () + ", " + num->toString () + ")";
}


YCPOrder YCPLocaleRep::compare (const YCPLocale& l, bool rl) const
{
    return msg1->compare (l->msg1, rl);
}


YCPValueType YCPLocaleRep::valuetype () const
{
    return YT_LOCALE;
}
