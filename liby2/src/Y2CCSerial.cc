/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Y2CCSerial.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * Component Creator that creates serial components
 *
 * Author: Thomas Roelz <tom@suse.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <regex.h>

#include <ycp/y2log.h>
#include "Y2CCSerial.h"
#include "Y2SerialComponent.h"


Y2CCSerial::Y2CCSerial (bool creates_servers)
    : Y2ComponentCreator (Y2ComponentBroker::BUILTIN),
      creates_servers (creates_servers)
{
    make_rxs ();
}


bool Y2CCSerial::isServerCreator() const
{
    return creates_servers;
}


void
Y2CCSerial::make_rxs () const
{
    extern int _nl_msg_cat_cntr;
    my_nl_msg_cat_cntr = _nl_msg_cat_cntr;

    // serial(baud):serial_device
    regcomp(&rx1, "serial(\\(9600\\|19200\\|38400\\|57600\\|115200\\)):\\(/.*\\)", 0);
}


void
Y2CCSerial::free_rxs () const
{
    regfree (&rx1);
}


Y2Component *Y2CCSerial::create(const char *componentname) const
{
    extern int _nl_msg_cat_cntr;
    if (_nl_msg_cat_cntr != my_nl_msg_cat_cntr)
    {
	free_rxs ();
	make_rxs ();
    }

    regmatch_t matches[3];

    long baud = 0L;
    string device;

    if (0 == regexec(&rx1, componentname, 3, matches, 0))
    {
       baud  = strtol(string(componentname + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so).c_str(),
		      0,   // endptr
		      10); // base
       device = string(componentname + matches[2].rm_so, matches[2].rm_eo - matches[2].rm_so);
    }
    else
    {
       return 0;   // doesn't match
    }

    return new Y2SerialComponent(device, baud);
}


Y2Component*
Y2CCSerial::provideNamespace(const char*)
{
    y2debug ("Y2SerialComponent cannot import namespaces");
    return NULL;
}


Y2CCSerial g_y2ccserial0 (true);
Y2CCSerial g_y2ccserial1 (false);
