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

   File:	YCPBuiltinMisc.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

$Id$
/-*/

#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "ycp/YCPBuiltinMisc.h"
#include "ycp/YCPBoolean.h"
#include "ycp/YCPInteger.h"
#include "ycp/YCPVoid.h"
#include "ycp/YCPString.h"
#include "ycp/YCPCode.h"
#include "ycp/StaticDeclaration.h"

#include "ycp/y2log.h"

extern StaticDeclaration static_declarations;


static YCPValue
CheckIP (const YCPString & ip)
{
    /**
     * @builtin checkIP (string ip) -> boolean
     * Check syntax of an IP address. (Syntax must be
     * nnn.nnn.nnn.nnn, where each nnn element is from <0, 255> range).
     *
     * Examples: <pre>
     * checkIP ("127.0.0.1") -> true
     * checkIP ("277.0.3.2") -> false
     * checkIP ("127.0.3.") -> false
     * </pre>
     */

    struct in_addr inp;
    return YCPBoolean (inet_aton (ip->value_cstr (), &inp) != 0);
}


static YCPValue
Time ()
{
    /**
     * @builtin time () -> integer
     * Return the number of seconds since 1.1.1970.
     */

    return YCPInteger (time (0));
}


static YCPValue
Sleep (const YCPInteger & ms)
{
    /**
     * @builtin sleep (integer ms) -> void
     * Sleeps a number of milliseconds.
     */

    usleep (ms->value () * 1000);
    return YCPVoid ();
}


static YCPValue
Random (const YCPInteger & max)
{
    /**
     * @builtin random (integer max) -> integer
     * Random number generator.
     * Returns integer in the interval <0,max).
     */

    // <1,10> 1+(int) (10.0*rand()/(RAND_MAX+1.0));
    int ret = (int) (max->value () * rand () / (RAND_MAX + 1.0));
    return YCPInteger (ret);
}


static YCPValue
Srandom1 ()
{
    /**
     * @builtin srandom () -> integer
     * Initialize random number generator with current date and
     * time and returns the seed.
     */

    int ret = time (0);
    srand (ret);
    return YCPInteger (ret);
}


static YCPValue
Srandom2 (const YCPInteger & seed)
{
    /**
     * @builtin srandom (integer seed) -> void
     * Initialize random number generator.
     */

    srand (seed->value ());
    return YCPVoid ();
}


static YCPValue
Eval (const YCPValue & v)
{
    /**
     * @builtin eval (any v) -> any
     * Evaluate a YCP value. See also the builtin ``, which is
     * kind of the counterpart to eval.
     *
     * Examples: <pre>
     * eval (``(1+2)) -> 3
     * { term a = ``add(); a = add(a, [1]); a = add(a, 4); return eval(a); } -> [1,4]
     * </pre>
     */

    if (!v->isCode())
    {
	return v;
    }
    return v->asCode()->evaluate();
}


static YCPValue
s_sformat (const YCPString &format, const YCPList &argv)
{
    /**
     * @builtin sformat (string form, any par1, any par2, ...) -> string
     * form is a string that may contains placeholders %1, %2, ...
     * Each placeholder is substituted with the argument converted
     * to string whose number is after the %. Only 1-9 are allowed
     * by now. The percentage sign is donated with %%.
     *
     * Example: <pre>
     * sformat ("%2 is greater %% than %1", 3, "five") -> "five is greater % than 3"
     * </pre>
     */

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
		if (argv->size () <= num)
		{
		    y2warning ("Illegal argument number %%%d in formatstring '%s'",
			       num, format->asString ()->value ().c_str ());
		}
		else if (argv->value (num).isNull())
		{
		    result += "<NULL>";
		}
		else if (argv->value (num)->isString ())
		{
		    result += argv->value (num)->asString ()->value ();
		}
		else
		{
		    result += argv->value (num)->toString ();
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


// FIXME


static YCPValue
Y2Log (loglevel_t level, const YCPString & format, const YCPList & args)
{
    YCPValue arg = s_sformat (format, args);
    if (arg.isNull () || !arg->isString ())
    {
	return YCPNull ();
    }

    extern ExecutionEnvironment ee;

    ycp2log (level, ee.filename().c_str(), ee.linenumber(), arg->asString ()->value ().c_str ());
    return YCPVoid ();
}


static YCPValue
Y2Debug (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2debug (string format, ...) -> void
     * Log a message to the y2log. Arguments are same as for sformat() builtin.
     * The y2log component is "YCP", so you can control these messages the
     * same way as other y2log messages.
     *
     * Example: <pre>
     * y2debug ("%1 is smaller than %2", 7, "13");
     * </pre>
     */

    return Y2Log (LOG_DEBUG, format, args);
}

static YCPValue
Y2Milestone (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2milestone (string format, ...) -> void
     */

    return Y2Log (LOG_MILESTONE, format, args);
}


static YCPValue
Y2Warning (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2warning (string format, ...) -> void
     */

    return Y2Log (LOG_WARNING, format, args);
}


static YCPValue
Y2Error (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2error (string format, ...) -> void
     */

    return Y2Log (LOG_ERROR, format, args);
}


static YCPValue
Y2Security (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2security (string format, ...) -> void
     */

    return Y2Log (LOG_SECURITY, format, args);
}


static YCPValue
Y2Internal (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2internal (string format, ...) -> void
     */

    return Y2Log (LOG_INTERNAL, format, args);
}


// TODO: copy docs and implementatin from head
static YCPValue
Y2FDebug (const YCPInteger & frame, const YCPString & format, const YCPList & args)
{
    return Y2Debug (format, args);
}

static YCPValue
Y2FMilestone (const YCPInteger & frame, const YCPString & format, const YCPList & args)
{
    return Y2Milestone (format, args);
}

static YCPValue
Y2FWarning (const YCPInteger & frame, const YCPString & format, const YCPList & args)
{
    return Y2Warning (format, args);
}

static YCPValue
Y2FError (const YCPInteger & frame, const YCPString & format, const YCPList & args)
{
    return Y2Error (format, args);
}

static YCPValue
Y2FSecurity (const YCPInteger & frame, const YCPString & format, const YCPList & args)
{
    return Y2Security (format, args);
}

static YCPValue
Y2FInternal (const YCPInteger & frame, const YCPString & format, const YCPList & args)
{
    return Y2Internal (format, args);
}


YCPBuiltinMisc::YCPBuiltinMisc ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "checkIP",	"b|s", 0,		(void *)CheckIP, 0 },
	{ "time",	"i|",  0,		(void *)Time, 0 },
	{ "sleep",	"v|i", 0,		(void *)Sleep, 0 },
	{ "random",	"i|i", 0,		(void *)Random, 0 },
	{ "srandom",	"i|",  0,		(void *)Srandom1, 0 },
	{ "srandom",	"v|i", 0,		(void *)Srandom2, 0 },
	{ "eval",	"A|A", DECL_NIL,	(void *)Eval, 0 },
	{ "sformat",	"s|sw", DECL_NIL|DECL_WILD, (void *)s_sformat, 0 },
	// ordinary logging
	{ "y2debug",	"v|sw", DECL_NIL|DECL_WILD, (void *)Y2Debug, 0 },
	{ "y2milestone","v|sw", DECL_NIL|DECL_WILD, (void *)Y2Milestone, 0 },
	{ "y2warning",	"v|sw", DECL_NIL|DECL_WILD, (void *)Y2Warning, 0 },
	{ "y2error",	"v|sw", DECL_NIL|DECL_WILD, (void *)Y2Error, 0 },
	{ "y2security", "v|sw", DECL_NIL|DECL_WILD, (void *)Y2Security, 0 },
	{ "y2internal", "v|sw", DECL_NIL|DECL_WILD, (void *)Y2Internal, 0 },
	// logging with a different call frame
	{ "y2debug",	"v|isw", DECL_NIL|DECL_WILD, (void *)Y2FDebug, 0 },
	{ "y2milestone","v|isw", DECL_NIL|DECL_WILD, (void *)Y2FMilestone, 0 },
	{ "y2warning",	"v|isw", DECL_NIL|DECL_WILD, (void *)Y2FWarning, 0 },
	{ "y2error",	"v|isw", DECL_NIL|DECL_WILD, (void *)Y2FError, 0 },
	{ "y2security", "v|isw", DECL_NIL|DECL_WILD, (void *)Y2FSecurity, 0 },
	{ "y2internal", "v|isw", DECL_NIL|DECL_WILD, (void *)Y2FInternal, 0 },
	{ 0, 0, 0, 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinMisc", declarations);
}
