/* Y2Logger.h
 *
 * Macros for logging the output.
 *
 * Just #include "Y2Logger.h" and use Y2_DEBUG, Y2_WARNING or Y2_ERROR
 * 
 * If you write Y2_DEBUG("Hello: %d",7) on line 13 in the file Source.cc,
 * the debug output will look like this:
 *   [...]:Source.cc[13] Hello: 7
 *
 * Additionally you can #define Y2_COMPONENT before inclusion and its name
 * will be put just before the Source.cc.
 * And don't forget set (end export the Y2DEBUG shell variable!
 *
 * Also define Y2_DEBUG_YES if you want to get the debugging output!
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 */

#ifndef Y2Logger_h
#define Y2Logger_h

#define y2log_component "ag_modules"
#include <ycp/y2log.h>

/*
 * Log the error and return ...
 */
#define Y2_RETURN_FALSE(format,args...) \
  do { y2error(format,##args); return false; } while(0)

#define Y2_RETURN_VOID(format,args...) \
  do { y2error(format,##args); return YCPVoid(); } while(0)

#define Y2_RETURN_STR(format,args...) \
  do { y2error(format,##args); return ""; } while(0)

#define Y2_RETURN_YCP_FALSE(format,args...) \
  do { y2error(format,##args); return YCPBoolean(false); } while(0)

#endif /* Y2Logger_h */
