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

   File:	sformat.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include "y2log.h"
#include "sformat.h"


YCPString sformat (const YCPString &format, const YCPList &args)
{
    const char *read = format->value ().c_str ();
    string result = "";

    while (*read)
    {
	if (*read == '%')
	{
	    read++;
	    if (*read == '%')
	    {
		result += "%";
	    }
	    else if (*read >= '1' && *read <= '9')
	    {
		int num = *read - '0' - 1;
		if (args->size () <= num)
		{
		    y2warning ("Illegal argument number %%%d in formatstring %s",
			       num, format->asString ()->value ().c_str ());
		}
		else if (args->value (num)->isString ())
		{
		    result += args->value (num)->asString ()->value ();
		}
		else
		{
		    result += args->value (num)->toString ();
		}
	    }
	    else
	    {
		y2warning ("%% in formatstring %s missing a number",
			   format->asString ()->value ().c_str ());
	    }
	    read++;
	}
	else
	{
	    result += *read++;
	}
    }

    return YCPString (result);
}
