/* y2log.h
 *
 * YaST2: Core system
 *
 * YaST2 logging implementation
 *
 * Authors: Mathias Kettner <kettner@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#ifndef _y2log_h
#define _y2log_h

#include <string>
#include <y2util/y2logging.h>

using std::string;

/* Logging defines */

#ifndef y2log_component
#  ifdef Y2LOG
#    define y2log_component Y2LOG
#  else
#    error neither y2log_component nor Y2LOG defined
#  endif
#endif

#define y2_logger_func(level,comp,file,line,function,format,args...)	\
do {									\
    if (shouldBeLogged (level, comp))					\
	Y2Logging::y2_logger (level,comp,file,line,function,format,##args);	\
} while (0)

/*
 * Caution: Don't use
 *     if (shouldBeLogged(...) y2_logger(...)
 * above - this clashes with any
 *     if (...)
 *	  y2error(...)
 *     else
 * since the "else" branch always refers to the inner (!) "if"
 * - in this case, the "if" of this macro :-((
 */


#define y2logger(level, format, args...) \
    y2_logger_func(level,y2log_component,__FILE__,__LINE__,__FUNCTION__,format,##args)

#define y2vlogger(level, format, ap) \
    y2_logger_func(level,y2log_component,__FILE__,__LINE__,__FUNCTION__,format,ap)

#ifdef WITHOUT_Y2DEBUG
#  define y2debug(format, args...)
#else
#  define y2debug(format, args...)	y2logger(LOG_DEBUG,format,##args)
#endif

#define y2milestone(format, args...)	y2logger(LOG_MILESTONE,format,##args)
#define y2warning(format, args...)	y2logger(LOG_WARNING,format,##args)
#define y2error(format, args...)	y2logger(LOG_ERROR,format,##args)
#define y2security(format, args...)	y2logger(LOG_SECURITY,format,##args)
#define y2internal(format, args...)	y2logger(LOG_INTERNAL,format,##args)

/* YCP Errors */

#define y2scanner(file,line,format,args...) \
    y2_logger_func(LOG_ERROR,"Scanner",file,line,"",format,##args)
#define syn2error(file,line,format,args...) \
    y2_logger_func(LOG_ERROR,"Parser",file,line,"",format,##args)
#define sem2error(file,line,format,args...) \
    y2_logger_func(LOG_ERROR,"Runtime",file,line,"",format,##args)

#define ycp2log(level,file,line,func,format,args...) \
    y2_logger_func(level,"YCP",file,line,func,format,##args)

#define y2ycp(level,file,line,format,args...) \
    y2_logger_func(level,"Interpreter",file,line,"",format,##args)

#define ycp2debug(file,line,format,args...) \
    y2ycp(LOG_DEBUG,file,line,format,##args)
#define ycp2milestone(file,line,format,args...) \
    y2ycp(LOG_MILESTONE,file,line,format,##args)
#define ycp2warning(file,line,format,args...) \
    y2ycp(LOG_WARNING,file,line,format,##args)
#define ycp2error(file,line,format,args...) \
    y2ycp(LOG_ERROR,file,line,format,##args)
#define ycp2security(file,line,format,args...) \
    y2ycp(LOG_SECURITY,file,line,format,##args)
#define ycp2internal(file,line,format,args...) \
    y2ycp(LOG_INTERNAL,file,line,format,##args)


bool shouldBeLogged (int loglevel, string componentname);

/**
 * Set an alternate logfile name for @ref y2log. If this is not done by the
 * application the first call to y2log sets the logfile name as follows:
 * users: $HOME/.y2log
 * root: /var/log/YaST2/y2log
 * The logname "-" is special: The log messages are written to stderr.
 */
void y2setLogfileName (std::string filename);
std::string y2getLogfileName();

#endif /* _y2log_h */
