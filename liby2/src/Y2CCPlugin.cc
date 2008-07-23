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

   File:	Y2CCPlugin.cc

   Author:	Arvin Schnell <arvin@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/


#include <sys/stat.h>
#include <unistd.h>

#include <ycp/y2log.h>
#include <ycp/pathsearch.h>
#include "Y2CCPlugin.h"
#include "Y2PluginComponent.h"


Y2CCPlugin::Y2CCPlugin (bool creates_servers)
    : Y2ComponentCreator (Y2ComponentBroker::PLUGIN),
      creates_servers (creates_servers)
{
    make_rxs ();
}


bool
Y2CCPlugin::isServerCreator () const
{
    return creates_servers;
}


void
Y2CCPlugin::make_rxs () const
{
    extern int _nl_msg_cat_cntr;
    my_nl_msg_cat_cntr = _nl_msg_cat_cntr;

    // from Y2CCSerial (and must be the exactly the same)
    regcomp (&rxs1, "serial(\\(9600\\|19200\\|38400\\|57600\\|115200\\))"
	     ":\\(/.*\\)", 0);

    // from Y2CCRemote (and must be the exactly the same)
    regcomp (&rxr1, "\\(telnet\\|.sh\\|rlogin\\)://\\([[:alpha:]][[:alnum:]_]*\\):"
	     "\\([^@]\\+\\)@\\([[:alnum:]._]\\+\\)/\\(.*\\)", 0);
    regcomp (&rxr2, "\\(telnet\\|.sh\\|rlogin\\)://\\([[:alpha:]][[:alnum:]_]*\\)@"
	     "\\([[:alnum:]._]\\+\\)/\\(.*\\)", 0);
    regcomp (&rxr3, "\\(su\\|sudo\\)://\\([[:alpha:]][[:alnum:]_]*\\)/\\(.*\\)", 0);
}


void
Y2CCPlugin::free_rxs () const
{
    regfree (&rxs1);
    regfree (&rxr1);
    regfree (&rxr2);
    regfree (&rxr3);
}


Y2Component*
Y2CCPlugin::createInLevel (const char* name, int level, int current_level) const
{
    if (strlen (name) > 0 && name[0] == '/')
	return 0;

    // Some components need special care, because their name and the name
    // of their plugin differ.
    
    // remote plugin can't handle "remote" component
    if (strcmp (name, "remote") == 0)
	return 0;

    extern int _nl_msg_cat_cntr;
    if (_nl_msg_cat_cntr != my_nl_msg_cat_cntr)
    {
	free_rxs ();
	make_rxs ();
    }

    const char* tmp1 = name;	// for the filename
    const char* tmp2 = name;	// for the creator name

    if (strcmp (name, "stderr") == 0)
	tmp1 = tmp2 = "stdio";

    if (regexec (&rxs1, name, 0, 0, 0) == 0)
	tmp1 = tmp2 = "serial";

    if (regexec (&rxr1, name, 0, 0, 0) == 0 ||
	regexec (&rxr2, name, 0, 0, 0) == 0 ||
	regexec (&rxr3, name, 0, 0, 0) == 0)
	tmp1 = tmp2 = "remote";

    // Look for the plugin and create component.

    string filename = Y2PathSearch::findy2plugin (tmp1, level);
    if (filename.empty ())
	return 0;

    return new Y2PluginComponent (creates_servers, filename, tmp2, name, level);
}


Y2Component*
Y2CCPlugin::provideNamespace(const char* name_space)
{
    y2debug ("Y2PluginComponent tries to locate namespace '%s'", name_space);
    Y2PluginComponent* comp = NULL;
    // try to load the library
    for (int level = 0; level < Y2PathSearch::numberOfComponentLevels ();
         level++)
    {
	string filename = Y2PathSearch::findy2plugin (name_space, level);
	if (filename.empty ())
	    continue;

	y2debug ("Trying file '%s'", filename.c_str ());
    
	comp = new Y2PluginComponent (filename, name_space, name_space, name_space);
	if (comp) break;
    }

    // return the internal component, that should provide the namespace
    return comp ? comp->component () : NULL;
}


Y2CCPlugin g_y2ccplugin0 (true);
Y2CCPlugin g_y2ccplugin1 (false);
