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

   File:       YCPBuiltinMisc.cc

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#include <unistd.h>	// for usleep()

#include "YCPInterpreter.h"
#include "../config.h"

#include <sys/stat.h>
#include <netinet/in.h>

extern "C" {
    int inet_aton(const char *cp, struct in_addr *inp);
}


YCPValue evaluateRemove(YCPInterpreter *interpreter, const YCPList& args)
{
  /**
   * @builtin remove(list|term l, integer i) -> any
   * Remove the i'th value from a list or a term. The first value has the index 0.
   * The call remove([1,2,3], 1) thus returns [1,3]. Returns nil if the
   * index is invalid.
   *
   * Example <pre>
   * remove([1,2], 0) -> [2]
   * remove(`fun(3,7), 0) -> [3,7];
   * remove(`fun(3,7), 1) -> `fun(7);
   * </pre>
   *
   * @builtin remove(map m, any k) -> map
   * Remove the key k and a corresponding value from a map m.
   * Returns nil if the key k is not found in m.
   *
   * Example <pre>
   * remove($[1:2], 0) -> $[]
   * remove ($[1:2, 3:4], 1) -> $[3:4]
   * </pre>
   */

    if (args->value(0)->isMap())
    {
	YCPMap map = YCPMap();
	map = args->value(0)->asMap()->shallowCopy();
	YCPValue key = args->value(1);
	if(map->value(key).isNull())
	{
	    interpreter->reportError(LOG_ERROR, "Key %s not found", key->toString().c_str());
	    return YCPVoid();
	}
	map->remove(key);
	return map;
    }

  else if (args->value(1)->isInteger()) {

    long idx = args->value(1)->asInteger()->value();

    YCPList list = YCPList();

    if (args->value(0)->isList())
      list = args->value(0)->asList()->shallowCopy();
    else if (args->value(0)->isTerm()) {
      list = args->value(0)->asTerm()->args()->shallowCopy();
      if(!idx)
        return list;
      else
        idx--;
    }
    else {
        interpreter->reportError(LOG_ERROR, "Wrong argument for remove()");
        return YCPVoid();
    }

    if (idx < 0 || idx >= list->size()) {
      interpreter->reportError(LOG_ERROR, "Index %ld for remove() out of range", idx);
      return YCPVoid();
    }
    else {
      list->remove(idx);
      if(args->value(0)->isList())
        return list;
      else {
        YCPTerm term = YCPTerm(args->value(0)->asTerm()->symbol(),list);
        return term;
      }
    }
  }

  return YCPNull();
}


YCPValue evaluateSelect(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin select(list|term l, integer i, any default = nil) -> any
     * Gets the i'th value of a list or a term. The first value has the index 0.
     * The call select([1,2,3], 1) thus returns 2.
     * Returns default if the index is invalid.
     *
     * Example <pre>
     * select([1,2], 0) -> 1
     * select(`hirn(true, false), 1) -> false
     * select(`hirn(true, false), 33, true) -> true
     * </pre>
     */

    if (args->value(1)->isInteger())
    {
	long idx = args->value(1)->asInteger()->value();

	YCPList list = YCPList();

	if (args->value(0)->isList())
	{
	    list = args->value(0)->asList();
	}
	else if (args->value(0)->isTerm())
	{
	    list = args->value(0)->asTerm()->args();
	}
	else
	{
	    interpreter->reportError(LOG_ERROR, "Wrong argument for select()");
	    return YCPVoid();
	}

	if (idx < 0 || idx >= list->size())
	{
	    if (args->size() == 3)
	    {
		return args->value(2);
	    }
	    else
	    {
		interpreter->reportError(LOG_ERROR, "Index %ld for select() out of range", idx);
		return YCPVoid();
	    }
	}
	else return list->value(idx);
    }
    else return YCPNull();
}


YCPValue evaluateTime (YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin time () -> integer
     * Return the seconds since 1.1.1970
     */
    if (args->size () == 0)
    {
	return YCPInteger (time (NULL));
    }
    else return YCPNull ();
}


YCPValue evaluateSleep(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin sleep(integer ms) -> void
     * Sleeps a number of milliseconds.
     */
    if (args->size() == 1 && args->value(0)->isInteger())
    {
	usleep(args->value(0)->asInteger()->value() * 1000);
	return YCPVoid();
    }
    else return YCPNull();
}


YCPValue evaluateRandom(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin random(integer max) -> integer
     * Random number generator.
     * Returns integer in the interval <0,max).
     */
    if (args->size() == 1 && args->value(0)->isInteger())
    {
	// <1,10> 1+(int) (10.0*rand()/(RAND_MAX+1.0));
	int ret = (int) (args->value(0)->asInteger()->value()*rand()/(RAND_MAX+1.0));
	return YCPInteger(ret);
    }
    else return YCPNull();
}


YCPValue evaluateSrandom(YCPInterpreter *interpreter, const YCPList& args)
{
    /**
     * @builtin srandom(integer seed) -> void
     * @builtin srandom() -> integer
     * Initialize random number generator.
     * If no argument is given, initialize the generator
     * with current date and time and return the seed.
     */
    if (args->size() == 1 && args->value(0)->isInteger())
    {
	srand(args->value(0)->asInteger()->value());
	return YCPVoid();
    }
    else if (args->size() == 0)
    {
	int ret = time(0);
	srand(ret);
	return YCPInteger(ret);
    }
    else return YCPNull();
}


/**
 * @builtin y2debug(string format, ...) -> void
 * @builtin y2milestone(string format, ...) -> void
 * @builtin y2warning(string format, ...) -> void
 * @builtin y2error(string format, ...) -> void
 * @builtin y2security(string format, ...) -> void
 * @builtin y2internal(string format, ...) -> void
 * Log a message to the y2log at different log levels.
 * Arguments are same as for sformat() builtin.
 * The y2log component is "YCP", so you can control these messages
 * same way as other y2log messages.
 *
 * Example <pre>
 * y2error("%1 is smaller than %2", 7, "13");
 * </pre>
 *
 * Normally, the file and line number of the calling YCP code is
 * included in the log message.  But if the first parameter is a
 * positive integer n, the location of its n-th calling function is
 * used.
 *
 * Example <pre>
 * define void Aargh() ``{
 *   y2error (1, "Aargh!");
 * }
 *
 * define divide (float a, float b) ``{
 *   if (b == 0) { Aargh(); }
 *   ...
 * }
 * </pre>
 */
YCPValue evaluateY2TraceLog (loglevel_t level, YCPInterpreter *interpreter, YCPList args);

YCPValue evaluateY2log(loglevel_t level, YCPInterpreter *interpreter, const YCPList &args)
{
  if (args->size () > 0 && args->value (0)->isInteger ())
    return evaluateY2TraceLog (level, interpreter, args);

  YCPValue arg = evaluateSformat(interpreter,args);
  if(arg.isNull() || !arg->isString())
    return YCPNull();

  int line = interpreter->current_line;
  const char *file = interpreter->current_file.c_str();
  const char *func = interpreter->current_func.c_str();
  const char *message = arg->asString()->value().c_str();

  ycp2log(level,file,line,func,"%s",message);
  return YCPVoid();
}

YCPValue evaluateY2TraceLog (loglevel_t level, YCPInterpreter *interpreter, YCPList args)
{
  // positive: pretend one of our callers invoked the log
  // negative: print backtrace
  int frame = args->value (0)->asInteger ()->value ();
  const char *message = "";
  string message_s;
  if (args->size () == 1)
  {
      message = "frame 0";
  }
  else
  {
      args->remove (0);
      YCPValue arg = evaluateSformat (interpreter,args);
      if (arg.isNull () || !arg->isString ())
      {
	  return YCPNull ();
      }
      message_s = arg->asString ()->value ();
      message = message_s.c_str ();
      // if we did not use message_s, message would point to a
      // temporary string that is destroyed at the end of the
      // enclosing block, that is, here
  }

  if (frame <= 0)
  {
      int line = interpreter->current_line;
      const char *file = interpreter->current_file.c_str();
      const char *func = interpreter->current_func.c_str();
      ycp2log(level,file,line,func,"%s",message);
  }

  if (frame != 0)
  {
      typedef vector<YCPInterpreter::CallFrame>::reverse_iterator ri;
      ri i = interpreter->backtrace.rbegin ();
      ri e = interpreter->backtrace.rend ();
      int f = 1;
      while (i != e)
      {
	  int line = i->line;
	  const char *file = i->file.c_str ();
	  const char *func = i->func.c_str ();

	  if (frame > 0)
	  {
	      if (f == frame)
	      {
		  ycp2log(level,file,line,func,"%s",message);
		  break;
	      }
	  }
	  else
	  {
	      ycp2log(level,file,line,func,"frame %d",f);
	  }
	  ++f;
	  ++i;
      }
  }
  return YCPVoid ();
}

YCPValue evaluateY2Debug(YCPInterpreter *interpreter, const YCPList& args) {
  return evaluateY2log(LOG_DEBUG,interpreter,args);
}

YCPValue evaluateY2Milestone(YCPInterpreter *interpreter, const YCPList& args) {
  return evaluateY2log(LOG_MILESTONE,interpreter,args);
}

YCPValue evaluateY2Warning(YCPInterpreter *interpreter, const YCPList& args) {
  return evaluateY2log(LOG_WARNING,interpreter,args);
}

YCPValue evaluateY2Error(YCPInterpreter *interpreter, const YCPList& args) {
  return evaluateY2log(LOG_ERROR,interpreter,args);
}

YCPValue evaluateY2Security(YCPInterpreter *interpreter, const YCPList& args) {
  return evaluateY2log(LOG_SECURITY,interpreter,args);
}

YCPValue evaluateY2Internal(YCPInterpreter *interpreter, const YCPList& args) {
  return evaluateY2log(LOG_INTERNAL,interpreter,args);
}


YCPValue evaluateBuiltinOp (YCPInterpreter *interpreter, builtin_t code, const YCPList& args)
{
    switch (code)
    {
	case YCPB_ADD:
	{
	    if (args->size() == 2)
		return args->value(0)->asBuiltin()->functionalAdd (args->value(1), false);
	}
	break;
	default:
	    break;
    }
    ycp2warning (interpreter->current_file.c_str(), interpreter->current_line, "evaluateBuiltinOp unknown code %d", code);
    return YCPNull();
}


