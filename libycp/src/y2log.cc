/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 logging implementation
 *
 * Authors:
 *   Mathias Kettner <kettner@suse.de>
 *   Thomas Roelz <tom@suse.de>
 *   Michal Svec <msvec@suse.cz>
 *   Arvin Schnell <arvin@suse.de>
 *
 * $Id$
 */

#include <errno.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "YCPParser.h"
#include "YCPValue.h"
#include "toString.h"

#include "y2log.h"

/* Defines */

#define Y2LOG_CONF	"/.yast2/logconf.ycp"	/* Relative to $HOME */

#define Y2LOG_VAR_DEBUG	"Y2DEBUG"
#define Y2LOG_VAR_ALL	"Y2DEBUGALL"
#define Y2LOG_VAR_SIZE	"Y2MAXLOGSIZE"
#define Y2LOG_VAR_NUM	"Y2MAXLOGNUM"

static YCPValue	y2logconf = YCPVoid ();

static bool did_set_logname = false;
static bool did_read_logconf = false;


void
y2setLogfileName (string filename)
{
    did_set_logname = true;

    Y2Logging::setLogfileName (filename.c_str ());
}


std::string
y2getLogfileName ()
{
    return Y2Logging::getLogfileName ();
}


static void
read_logconf ()
{
    did_read_logconf = true;

    /* Read the logconf.ycp */
    struct passwd *pw = getpwuid (geteuid ());
    if (pw)
    {
	string logconfname = string (pw->pw_dir) + Y2LOG_CONF;

	// We have to remember the errno. Otherwise a call of
	// y2error ("error: %m") can display a wrong message.
	int save_errno = errno;

	FILE *file = fopen (logconfname.c_str (), "r");
	if (file && (Y2Logging::loggingInitialized() < 1))
	{
	    YCPParser parser (file, logconfname.c_str ());
	    y2logconf = parser.parse ();
	    fclose (file);
	}

	errno = save_errno;
    }
    else
    {
	fprintf (Y2Logging::getY2LogStderr (), "Cannot read pwd entry for user "
		 "id %d. No logconf, using defaults.\n", geteuid ());
    }
}


bool
shouldBeLogged (int loglevel, string componentname)
{
    /* Everything should be logged */
    if (getenv(Y2LOG_VAR_ALL))
	return true;

    /* Prepare the logfile name. */
    if (!did_set_logname)
    {
	y2setLogfileName (""); /* The default */
    }

    /* Read log configuration. */
    if (!did_read_logconf)
    {
	read_logconf ();
    }

    /* Logconf is a list of entries. One entry is for one component */
    if (!y2logconf.isNull () && y2logconf->isList ())
    {
	YCPList l = y2logconf->asList();

	for ( int i = 0; i < l->size(); i++ )
	{
	    /* Entry is a tuple (list) of component name and list of enablings */
	    if( l->value(i)->isList() )
	    {
		YCPList entry = l->value(i)->asList();

		/* Check if it is valid. */
		if( entry->value(0)->isString() && entry->value(1)->isList() )
		{
		    if( componentname == entry->value(0)->asString()->value() )
		    {
			YCPList enabling = entry->value(1)->asList();
			if( loglevel < enabling->size() )
			{
			    if( enabling->value(loglevel)->isBoolean() )
				return enabling->value(loglevel)->asBoolean()->value();
			    else
				return true; /* Default is to turn on. */
			}
			else
			    return true; /* Default is to turn on. */
		    }
		}
	    }
	}
    }

    return loglevel != LOG_DEBUG || getenv (Y2LOG_VAR_DEBUG);
}
