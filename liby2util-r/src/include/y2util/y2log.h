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
#include <stdio.h>

using std::string;

/* Logging levels */

enum loglevel_t {
    LOG_DEBUG = 0,	// debug message
    LOG_MILESTONE = 1,	// log great events, big steps
    LOG_WARNING = 2,	// warning in application level
    LOG_ERROR = 3,	// error in application level
    LOG_SECURITY = 4,	// security relevant problem or incident
    LOG_INTERNAL = 5	// internal bug. Please report to...
};

/* Logging functions */

void y2_logger_function (loglevel_t level, const char *component, const char *file,
		const int line, const char *func, const char *format, ...)
    __attribute__ ((format (printf, 6, 7)));

void y2_vlogger_function (loglevel_t level, const char *component, const char *file,
		 const int line, const char *func, const char *format, va_list ap);

void y2_logger_raw( const char* message );

/* Logging defines */

#ifdef y2log_subcomponent
#  define y2log_suffix "-" y2log_subcomponent
#else
#  define y2log_suffix
#endif

#ifdef y2log_component
#  define y2log_prefix y2log_component y2log_suffix
#else
#  ifdef Y2LOG
#    define y2log_prefix Y2LOG y2log_suffix
#  else
#    error neither y2log_component nor Y2LOG defined
#    define y2log_prefix ""
#  endif
#endif

#define y2_logger(level,comp,file,line,function,format,args...)		\
do {									\
    if (should_be_logged (level, comp))					\
	y2_logger_function (level,comp,file,line,function,format,##args);\
} while (0)

#define y2_vlogger(level,comp,file,line,function,format,args)		\
do {									\
    if (should_be_logged (level, comp))					\
	y2_vlogger_function (level,comp,file,line,function,format,args);\
} while (0)

/*
 * Caution: Don't use
 *     if (shouldbelogged(...) y2_logger(...)
 * above - this clashes with any
 *     if (...)
 *	  y2error(...)
 *     else
 * since the "else" branch always refers to the inner (!) "if"
 * - in this case, the "if" of this macro :-((
 */

#define y2logger(level, format, args...) \
    y2_logger(level,y2log_prefix,__FILE__,__LINE__,__FUNCTION__,format,##args)

#define y2vlogger(level, format, ap) \
    y2_vlogger(level,y2log_prefix,__FILE__,__LINE__,__FUNCTION__,format,ap)

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

#define y2lograw(message)		y2_logger_raw(message)

/**
 */
bool should_be_logged (int loglevel, string componentname);

/**
 * Set an alternate logfile name for @ref y2log. If this is not done by the
 * application the first call to y2log sets the logfile name as follows:
 * users: $HOME/.y2log
 * root: /var/log/YaST2/y2log
 *
 * The logname "-" is special: The log messages are written to stderr.
 */
void set_log_filename (string filename);
string get_log_filename();

/**
 * Read an alternate logconf file @ref y2log. If this is not done by the
 * application the first call to y2log sets the logconf file as follows:
 * users: $HOME/.yast2/log.conf
 * root: /etc/YaST2/log.conf
 */
void set_log_conf(string confname);

/**
 * Set (or reset) the simple mode
 */
void set_log_simple_mode(bool simple);

/**
 * enable or disable debug logging
 * @param on true for on
 */
void set_log_debug(bool on = true);

/**
 * whether debug logging is enabled
 */
bool get_log_debug();

#endif /* _y2log_h */
