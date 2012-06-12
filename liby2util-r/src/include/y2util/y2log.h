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

// Implements y2_logger
void y2_logger_function (loglevel_t level, const string& component, const char *file,
		const int line, const char *func, const char *format, ...)
    __attribute__ ((format (printf, 6, 7)));
// The knights of Blanik only show up when nothing else can help, and so will
// the messages logged here. fate#302166
void y2_logger_blanik   (loglevel_t level, const string& component, const char *file,
		const int line, const char *func, const char *format, ...)
    __attribute__ ((format (printf, 6, 7)));

// Same as above, but with va_list
void y2_vlogger_function (loglevel_t level, const string& component, const char *file,
		 const int line, const char *func, const char *format, va_list ap);
void y2_vlogger_blanik   (loglevel_t level, const string& component, const char *file,
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
    else if (should_be_buffered ())					\
	y2_logger_blanik (level,comp,file,line,function,format,##args); \
} while (0)

#define y2_vlogger(level,comp,file,line,function,format,args)		\
do {									\
    if (should_be_logged (level, comp))					\
	y2_vlogger_function (level,comp,file,line,function,format,args);\
    else if (should_be_buffered ())					\
	y2_vlogger_blanik (level,comp,file,line,function,format,args);  \
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
 * Should we bother evaluating the arguments to the logging function?
 */
bool should_be_logged (int loglevel, const string& componentname);

/**
 * Should we bother evaluating the arguments to the buffering function?
 */
bool should_be_buffered ();

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

// stores a few strings. can append one. can return all. old are forgotten.
class LogTail {
public:
    typedef string Data;
    LogTail (size_t max_size = 42);
    ~LogTail ();
    void push_back (const Data &);

    // consumer returns true to continue iterating
    typedef bool (* Consumer) (const Data &);
    void for_each (Consumer c);
private:
    class Impl;
    Impl *m_impl;
};

// the instance used for last resort logging
extern LogTail blanik;

#endif /* _y2log_h */
