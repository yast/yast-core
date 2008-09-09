/* y2changes.h
 *
 * YaST2: Core system
 *
 * YaST2 user-level logging implementation
 *
 * Authors: Mathias Kettner <kettner@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *	    Stanislav Visnovsky <visnov@suse.cz>
 */

#ifndef _y2changes_h
#define _y2changes_h

#include <string>
#include <stdio.h>

using std::string;

/* Logging functions */

enum logcategory_t {
    CHANGES_ITEM = 0,		//	system view, typically execution of a file
    CHANGES_NOTE = 1,		//	additional information for the user, e.g. start of a module
};

// Implements y2changes_function
void y2changes_function (logcategory_t category, const char *format, ...) __attribute__ ((format (printf, 2, 3)));

#define y2useritem(format, args...)	y2changes_function(CHANGES_ITEM, format,##args)
#define y2usernote(format, args...)	y2changes_function(CHANGES_NOTE, format,##args)

#endif /* _y2changes_h */
