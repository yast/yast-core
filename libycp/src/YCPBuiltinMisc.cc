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
   Summary:     Miscellaneous YCP Builtins

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

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

// "is" is actually not a built-in, it is a special language form
// because its second argument is a type, not a value. but people
// expect it to be documented among built-ins.
// (it is implemented in class YEIs in YExpression.cc)
/**
 * @builtin is
 * @short Checks whether a value is of a certain type
 * @param any value a value whose type is checked
 * @param type type type to check
 * @return boolean
 * @usage
 * any ui = UI::UserInput();
 * if (is (ui, string)) {
 *     foo ("Hello, " + (string) ui);
 * }
 * else if (is (ui, symbol)) {
 *     bar ((symbol) ui);
 * }
 */

static YCPInteger
Time ()
{
    /**
     * @builtin time
     * @short Return the number of seconds since 1.1.1970.
     * @return integer
     * @usage time() -> 1111207439
     */

    return YCPInteger (time (0));
}


static YCPValue
Sleep (const YCPInteger & ms)
{
    /**
     * @builtin sleep 
     * @short Sleeps a number of milliseconds.
     * @param integer MILLISECONDS Time in milliseconds
     * @return void
     * @usage sleep(3000) -> sleeps 3 sec.
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
     * @builtin random
     * @short Random number generator.
     * @description
     * Returns a random integer in the interval [0,MAX).
     * <tt>srandom</tt> must be activated to get really random
     * numbers.
     *
     * @param integer MAX
     * @return integer Returns integer in the interval [0,MAX).
     * @usage random(100) -> 82
     * @usage random(100) -> 36
     */

    if (max.isNull ())
	return YCPNull ();

    // see NOTES in man 3 rand,
    // <1,10> 1+(int) (10.0*rand()/(RAND_MAX+1.0));
    int ret = (int) (max->value () * rand () / (RAND_MAX + 1.0));
    return YCPInteger (ret);
}


static YCPInteger
Srandom1 ()
{
    /**
     * @builtin srandom
     * @short Initialize random number generator
     * @description
     * Initialize random number generator with current date and
     * time and returns the seed.
     *
     * @return integer
     * @id srandom-time
     * @usage srandom()
     */

    int ret = time (0);
    srand (ret);
    return YCPInteger (ret);
}


static YCPValue
Srandom2 (const YCPInteger & seed)
{
    /**
     * @builtin srandom
     * @short Initialize random number generator.
     * @param integer SEED
     * @return void
     * @id srandom-integer
     * @usage srandom(3355)
     */

    if (seed.isNull ())
    {
	ycp2error ("Cannot initialize random generator using 'nil'");
	return YCPNull ();
    }

    srand (seed->value ());
    return YCPVoid ();
}


static YCPBoolean
Setenv2 (const YCPString & name, const YCPString & value, const YCPBoolean & overwrite)
{
    /**
     * @builtin setenv
     * @short Change or add an environment variable
     * @description
     * The setenv() function adds the variable to the
     * environment with the value. If variable exist
     * the value is changed.
     *
     * @param string variable
     * @param string value
     * @param boolean overwrite
     * @return boolean
     * @id setenv-choose
     * @usage setenv("PATH", "/home/user", true)
     */
    //3rd argument (1) means that value will be overwrite if it exist
    int ret = setenv(name->value().c_str(), value->value().c_str(), (overwrite->value() ? 1:0) ); 
    if (ret == 0) {
        return YCPBoolean(true);
    } else { 
        ycp2error ("[Setenv1] failed %s",strerror(errno));
        return YCPBoolean(false);
    }
}


static YCPBoolean
Setenv1 (const YCPString & name, const YCPString & value)
{
    /**
     * @builtin setenv
     * @short Change or add an environment variable
     * @description
     * The setenv() function adds the variable to the
     * environment with the value. If variable exist
     * the value is changed.
     *
     * @param string variable
     * @param string value
     * @return boolean
     * @id setenv-always
     * @usage setenv("PATH", "/home/user")
     */
    //3rd argument (1) means that value will be overwrite if it exist

    return Setenv2(name, value, YCPBoolean(true));

}


static YCPString
Getenv (const YCPString & name)
{
     /**
     * @builtin getenv
     * @short Change or add an environment variable
     * @description
     * The getenv(variable) function returns the value of variable from
     * environment. If variable doesn't exist
     * the value is NULL.
     *
     * @param string name
     * @return string value
     * @usage getenv ("USER") -> "root"
     * @usage getenv ("LC_CTYPE") -> "en_US.UTF-8"
     */
 
    char *value = getenv(name->value().c_str());
    if (value) { 
        string ret (value);        
        return YCPString(ret);
    } else {
        return YCPNull();
    }
}


static YCPValue
Eval (const YCPValue & v)
{
    /**
     * @builtin eval
     * @short Evaluate a YCP value.
     * @description
     * See also the builtin ``, which is kind of the counterpart to eval.
     *
     * @usage eval (``(1+2)) -> 3
     */

    if (v.isNull ())
    {
	return YCPNull ();
    }
    if (!v->isCode())
    {
	return v;
    }
    return v->asCode()->evaluate();
}


static YCPString
s_sformat (const YCPValue &format, const YCPValue &_argv)
{
    /**
     * @builtin  sformat
     * @short Format a String
     * @description
     * FORM is a string that may contains placeholders %1, %2, ...
     * Each placeholder is substituted with the argument converted
     * to string whose number is after the %. Only 1-9 are allowed
     * by now. The percentage sign is donated with %%.
     *
     * @param string FORM
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return string
     * @usage sformat ("%2 is greater %% than %1", 3, "five") -> "five is greater % than 3"
     */
     
    if (format.isNull ()
	|| !format->isString())
    {
	return YCPNull ();
    }	
    if (_argv.isNull ()
	|| !_argv->isList())
    {
	return format->asString();
    }
    YCPList argv = _argv->asList();

    const char *read = format->asString()->value ().c_str ();

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
     * @builtin y2debug
     * @short Log a message to the y2log.
     *
     * @description
     * Arguments are same as for sformat() builtin.
     * The y2log component is "YCP", so you can control these messages the
     * same way as other y2log messages.
     *
     * @param string FORMAT
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return void
     * @see sformat
     *
     * @usage y2debug ("%1 is smaller than %2", 7, "13");
     */

    return Y2Log (LOG_DEBUG, format, args);
}

static YCPValue
Y2Milestone (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2milestone
     * @short Log a milestone to the y2log.
     *
     * @param string FORMAT
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return void
     * @see sformat
     *
     * @usage y2milestone("%1 - Humans detected!", "2038-02-12") -> "2038-02-12 - Humans detected!"
     */

    return Y2Log (LOG_MILESTONE, format, args);
}


static YCPValue
Y2Warning (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2warning
     * @short Log a warning to the y2log.
     *
     * @param string FORMAT
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return void
     * @see sformat
     *
     * @usage y2warning ("Breakers don't work!") -> "Breakers don't work!"
     * @usage y2warning ("%1 %2 packets have been lost", 12, "UDP") -> "12 UDP packets have been lost"
     */

    return Y2Log (LOG_WARNING, format, args);
}


static YCPValue
Y2Error (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2error
     * @short Log an error to the y2log.
     *
     * @param string FORMAT
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return void
     * @see sformat
     *
     * @usage y2error ("Invalid format of IPv4 '%1'.", "333.10.20.1") -> "Invalid format of IPv4 '333.10.20.1'"
     */

    return Y2Log (LOG_ERROR, format, args);
}


static YCPValue
Y2Security (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2security
     * @short Log a security message to the y2log.
     *
     * @param string FORMAT
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return void
     * @see sformat
     *
     * @usage y2security ("Users on vacations: %1", ["josh", "joe", "pete"]) -> "Users on vacations: ["josh", "joe", "pete"]"
     */

    return Y2Log (LOG_SECURITY, format, args);
}


static YCPValue
Y2Internal (const YCPString & format, const YCPList & args)
{
    /**
     * @builtin y2internal
     * @short Log an internal message to the y2log.
     *
     * @param string FORMAT
     * @param any PAR1
     * @param any PAR2
     * @param any ...
     * @return void
     * @see sformat
     *
     * @usage y2internal("This is a robbery!") -> "This is a robbery!"
     */

    return Y2Log (LOG_INTERNAL, format, args);
}


// TODO: copy docs and implementatin from head
static YCPValue
Y2FDebug (const YCPInteger & f, const YCPString & format, const YCPList & args)
{

  // FIXME: positive: pretend one of our callers invoked the log
  // negative: print backtrace
    extern ExecutionEnvironment ee;

    int frame = f->value ();
  
    Y2Debug (format, args);

    if (frame < 0)
    {
	ee.backtrace (LOG_DEBUG, 0);
    }

    return YCPVoid ();
}

static YCPValue
Y2FMilestone (const YCPInteger & f, const YCPString & format, const YCPList & args)
{
  // FIXME: positive: pretend one of our callers invoked the log
  // negative: print backtrace
    extern ExecutionEnvironment ee;

    int frame = f->value ();
  
    Y2Milestone (format, args);

    if (frame < 0)
    {
	ee.backtrace (LOG_MILESTONE, 0);
    }

    return YCPVoid ();
}

static YCPValue
Y2FWarning (const YCPInteger & f, const YCPString & format, const YCPList & args)
{
  // FIXME: positive: pretend one of our callers invoked the log
  // negative: print backtrace
    extern ExecutionEnvironment ee;

    int frame = f->value ();
  
    Y2Warning (format, args);

    if (frame < 0)
    {
	ee.backtrace (LOG_WARNING, 0);
    }

    return YCPVoid ();
}

static YCPValue
Y2FError (const YCPInteger & f, const YCPString & format, const YCPList & args)
{
  // FIXME: positive: pretend one of our callers invoked the log
  // negative: print backtrace
    extern ExecutionEnvironment ee;

    int frame = f->value ();
  
    Y2Error (format, args);

    if (frame < 0)
    {
	ee.backtrace (LOG_ERROR, 0);
    }

    return YCPVoid ();
}

static YCPValue
Y2FSecurity (const YCPInteger & f, const YCPString & format, const YCPList & args)
{
  // FIXME: positive: pretend one of our callers invoked the log
  // negative: print backtrace
    extern ExecutionEnvironment ee;

    int frame = f->value ();
  
    Y2Security (format, args);

    if (frame < 0)
    {
	ee.backtrace (LOG_SECURITY, 0);
    }

    return YCPVoid ();
}

static YCPValue
Y2FInternal (const YCPInteger & f, const YCPString & format, const YCPList & args)
{
  // FIXME: positive: pretend one of our callers invoked the log
  // negative: print backtrace
    extern ExecutionEnvironment ee;

    int frame = f->value ();
  
    Y2Internal (format, args);

    if (frame < 0)
    {
	ee.backtrace (LOG_INTERNAL, 0);
    }

    return YCPVoid ();
}


YCPBuiltinMisc::YCPBuiltinMisc ()
{
    // must be static, registerDeclarations saves a pointer to it!
    static declaration_t declarations[] = {
	{ "time",	"integer ()",			(void *)Time		},
	{ "sleep",	"void (integer)",		(void *)Sleep		},
	{ "random",	"integer (integer)",		(void *)Random		},
        { "setenv",	"boolean (string,string)", 	(void *)Setenv1		},
	{ "setenv",	"boolean (string,string,boolean)", (void *)Setenv2	}, 
	{ "getenv",	"string (string)",		(void *)Getenv		},
	{ "srandom",	"integer ()",			(void *)Srandom1	},
	{ "srandom",	"void (integer)",		(void *)Srandom2	},
	{ "eval",	"flex (block <flex>)",		(void *)Eval,		DECL_NIL|DECL_FLEX },
	{ "eval",	"flex (const flex)",		(void *)Eval,		DECL_NIL|DECL_FLEX },
	{ "sformat",	"string (string, ...)",		(void *)s_sformat,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
	// ordinary logging
	{ "y2debug",	"void (string, ...)",		(void *)Y2Debug,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
	{ "y2milestone","void (string, ...)",		(void *)Y2Milestone,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
	{ "y2warning",	"void (string, ...)",		(void *)Y2Warning,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
	{ "y2error",	"void (string, ...)",		(void *)Y2Error,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
	{ "y2security", "void (string, ...)",		(void *)Y2Security,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
	{ "y2internal", "void (string, ...)",		(void *)Y2Internal,	DECL_NIL|DECL_WILD|DECL_FORMATTED },
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
