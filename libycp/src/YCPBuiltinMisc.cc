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
#include "ycp/ExecutionEnvironment.h"

extern StaticDeclaration static_declarations;


static YCPInteger
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
     
    if (ms.isNull ())
	return YCPNull ();

    usleep (ms->value () * 1000);
    return YCPVoid ();
}


static YCPInteger
Random (const YCPInteger & max)
{
    /**
     * @builtin random (integer max) -> integer
     * Random number generator.
     * Returns integer in the interval <0,max).
     */

    if (max.isNull ())
	return YCPNull ();

    // <1,10> 1+(int) (10.0*rand()/(RAND_MAX+1.0));
    int ret = (int) (max->value () * rand () / (RAND_MAX + 1.0));
    return YCPInteger (ret);
}


static YCPInteger
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

    if (seed.isNull ())
    {
	ycp2error ("Cannot initialize random generator using 'nil'");
	return YCPNull ();
    }

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

    if (v.isNull ())
	return YCPNull ();

    if (!v->isCode())
    {
	return v;
    }
    return v->asCode()->evaluate();
}


static YCPString
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
     
    if (format.isNull ())
	return YCPNull ();
	
    if (argv.isNull ())
    {
	return format;
    }

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
		    y2warning ("Illegal argument number %%%d (max %d) in formatstring '%s'",
			       num+1, argv->size (), format->asString ()->value ().c_str ());
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

static YCPValue
Y2Log (loglevel_t level, const YCPString & format, const YCPList & args)
{
    YCPValue arg = s_sformat (format, args);
    if (arg.isNull () || !arg->isString ())
    {
	return YCPNull ();
    }

    extern ExecutionEnvironment ee;

    // The "" is a function name. TODO.  It will be useful but may
    // break ycp testsuites. Maybe it's the right time to do it.
    ycp2log (level, ee.filename().c_str(), ee.linenumber(), "", "%s", arg->asString ()->value ().c_str ());
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
	{ "time",	"integer ()",			(void *)Time		},
	{ "sleep",	"void (integer)",		(void *)Sleep		},
	{ "random",	"integer (integer)",		(void *)Random		},
	{ "srandom",	"integer ()",			(void *)Srandom1	},
	{ "srandom",	"void (integer)",		(void *)Srandom2	},
	{ "eval",	"flex (block <flex>)",		(void *)Eval,		DECL_NIL|DECL_FLEX },
	{ "eval",	"any (const any)",		(void *)Eval,		DECL_NIL|DECL_FLEX },
	{ "sformat",	"string (string, ...)",		(void *)s_sformat,	DECL_NIL|DECL_WILD },
	// ordinary logging
	{ "y2debug",	"void (string, ...)",		(void *)Y2Debug,	DECL_NIL|DECL_WILD },
	{ "y2milestone","void (string, ...)",		(void *)Y2Milestone,	DECL_NIL|DECL_WILD },
	{ "y2warning",	"void (string, ...)",		(void *)Y2Warning,	DECL_NIL|DECL_WILD },
	{ "y2error",	"void (string, ...)",		(void *)Y2Error,	DECL_NIL|DECL_WILD },
	{ "y2security", "void (string, ...)",		(void *)Y2Security,	DECL_NIL|DECL_WILD },
	{ "y2internal", "void (string, ...)",		(void *)Y2Internal,	DECL_NIL|DECL_WILD },
	// logging with a different call frame
	{ "y2debug",	"void (integer, string, ...)",	(void *)Y2FDebug,	DECL_NIL|DECL_WILD },
	{ "y2milestone","void (integer, string, ...)",	(void *)Y2FMilestone,	DECL_NIL|DECL_WILD },
	{ "y2warning",	"void (integer, string, ...)",	(void *)Y2FWarning,	DECL_NIL|DECL_WILD },
	{ "y2error",	"void (integer, string, ...)",	(void *)Y2FError,	DECL_NIL|DECL_WILD },
	{ "y2security", "void (integer, string, ...)",	(void *)Y2FSecurity,	DECL_NIL|DECL_WILD },
	{ "y2internal", "void (integer, string, ...)",	(void *)Y2FInternal,	DECL_NIL|DECL_WILD },
	{ 0 }
    };

    static_declarations.registerDeclarations ("YCPBuiltinMisc", declarations);
}
