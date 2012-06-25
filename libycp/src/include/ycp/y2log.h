/* y2log.h
 *
 * YaST2: Core system
 *
 * YaST2 logging implementation
 *
 * Authors: Mathias Kettner <kettner@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *
 */

#ifndef _y2log_ycp_h
#define _y2log_ycp_h

#include <y2util/y2log.h>
#include "ExecutionEnvironment.h"

/* YCP Errors */

#define y2scanner(file,line,format,args...) \
    y2_logger(LOG_ERROR,"Scanner",file,line,"",format,##args)
#define syn2error(file,line,format,args...) \
    y2_logger(LOG_ERROR,"Parser",file,line,"",format,##args)
#define syn2warning(file,line,format,args...) \
    y2_logger(LOG_WARNING,"Parser",file,line,"",format,##args)
#define sem2error(file,line,format,args...) \
    y2_logger(LOG_ERROR,"Runtime",file,line,"",format,##args)

#define ycp2log(level,file,line,func,format,args...) \
    y2_logger(level,"YCP",file,line,func,format,##args)

#define y2ycp(level,file,line,format,args...) \
    y2_logger(level,"Interpreter",file,line,"",format,##args)

#define ycp2debug(file,line,format,args...) \
    y2ycp(LOG_DEBUG,file,line,format,##args)
#define ycp2milestone(file,line,format,args...) \
    y2ycp(LOG_MILESTONE,file,line,format,##args)
#define ycp2warning(file,line,format,args...) \
    y2ycp(LOG_WARNING,file,line,format,##args)
#define ycp2error(format,args...) 		\
    do {					\
	y2ycp(LOG_ERROR, YaST::ee.filename().c_str(), YaST::ee.linenumber(), format, ##args); \
    } while (0)
#define ycp2security(file,line,format,args...) \
    y2ycp(LOG_SECURITY,file,line,format,##args)
#define ycp2internal(file,line,format,args...) \
    y2ycp(LOG_INTERNAL,file,line,format,##args)

// logging cleanup
#define ycp_log(level,format,args...) 		\
    do {					\
	y2_logger(level, Y2LOG, YaST::ee.filename().c_str(), YaST::ee.linenumber(), "", format, ##args); \
    } while (0)


#define ycperror(format,args...) 		\
    ycp_log(LOG_ERROR, format, ##args)

#define ycpwarning(format,args...) 		\
    ycp_log(LOG_WARNING, format, ##args)

#define ycpdebug(format,args...) 		\
    ycp_log(LOG_DEBUG, format, ##args)

#define ycpinternal(format,args...) 		\
    ycp_log(LOG_INTERNAL, format, ##args)

#define ycpmilestone(format,args...) 		\
    ycp_log(LOG_MILESTONE, format, ##args)

/// c++ interface for logging
class Logger {
public:
    virtual ~Logger() {}
    virtual void error(string error) = 0;
    virtual void warning(string warning) = 0;
};

#endif /* _y2log_ycp_h */
