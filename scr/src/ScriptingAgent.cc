

/*
 *  ScriptingAgent.cc
 *
 *  Basic SCR agent handling stuff
 *
 *  Authors:	Arvin Schnell <arvin@suse.de>
 *		Klaus Kaempf <kkaempf@suse.de>
 *		Stanislav Visnovsky <visnov@suse.cz>
 *  Maintainer:	Arvin Schnell <arvin@suse.de>
 *
 *  $Id$
 */


#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <algorithm>

#include <ycp/y2log.h>
#include <ycp/pathsearch.h>
#include <y2/Y2ComponentBroker.h>
#include "ScriptingAgent.h"


ScriptingAgent::ScriptingAgent ()
{
    RegisterNewAgents ();
}


ScriptingAgent::ScriptingAgent (const string& file)
{
    y2debug( "Scripting agent using only SCR %s", file.c_str () );
    
    parseSingleConfigFile (file);
}


ScriptingAgent::~ScriptingAgent ()
{
    UnregisterAllAgents ();
}


void
ScriptingAgent::parseConfigFiles (const string &directory)
{
    y2debug ("Y2SCRComponent::parseConfigFiles (%s)", directory.c_str ());

    DIR *dir = opendir (directory.c_str ());
    if (!dir)
    {
	y2debug ("Can't open directory %s for reading: %m", directory.c_str ());
	return;
    }

    struct dirent *entry;
    while ((entry = readdir (dir)))
    {
	if (!strcmp (entry->d_name, ".") || !(strcmp (entry->d_name, "..")))
	    continue;

	// Read only *.scr files. For example TRANS.TBL makes problems
	if (strlen (entry->d_name) <= 4 ||
	    strcmp (entry->d_name + strlen (entry->d_name) - 4, ".scr"))
	    continue;

	const string filename = directory + "/" + entry->d_name;
	
	parseSingleConfigFile (filename);
    }

    closedir (dir);
}


void
ScriptingAgent::parseSingleConfigFile (const string &filename)
{
    struct stat st;
    if (stat (filename.c_str (), &st) != 0)
    {
	y2debug ("Can't read dir entry file %s: %m", filename.c_str ());
	return;
    }

    if (!S_ISREG (st.st_mode) && !S_ISLNK (st.st_mode))
	return;

    FILE *file = fopen (filename.c_str (), "r");
    if (!file)
    {
	y2debug ("Can't open %s for reading: %m", filename.c_str ());
	return;
    }

    const int size = 250;
    char line[size];

    while (fgets (line, size, file))
    {
	// delete last char (newline)
	const int l = strlen (line);
	if (l > 0)
	    line[l - 1] = '\0';

	if (line[0] == '.')
	{
	    YCPPath path (line);
	    SubAgents::iterator agent = findByPath (path);
	    if (agent != agents.end ())
	    {
    		y2debug ("Ignoring re-registration of path '%s'", path->toString ().c_str ());
	    }
	    else
	    {
		RegisterAgent (path, YCPString (filename));
	    }
	    break;
	}
    }

    fclose (file);
}


YCPValue
ScriptingAgent::Read (const YCPPath &path, const YCPValue &arg, const YCPValue &opt)
{
    y2debug( "This is ScriptingAgent(%p)::Read", this );
    y2debug( "opt: %s", opt.isNull() ? "null" : opt->toString().c_str ());
    YCPValue v = executeSubagentCommand ("Read", path, arg, opt);
    if (v.isNull())
    {
	ycp2error ("SCR::Read() failed");
    }
    return v;
}


YCPBoolean
ScriptingAgent::Write (const YCPPath &path, const YCPValue &value,
		       const YCPValue &arg)
{
    YCPValue v = executeSubagentCommand ("Write", path, value, arg);
    if (v.isNull())
    {
	ycp2error ("SCR::Write() failed");
	return YCPNull ();
    }
    if (!v->isBoolean ())
    {
	ycp2error ("SCR::Write() did not return a boolean");
	return YCPNull ();
    }
    return v->asBoolean ();
}


YCPList
ScriptingAgent::Dir (const YCPPath &path)
{
    YCPValue v = executeSubagentCommand ("Dir", path);
    if (v.isNull())
    {
	ycp2error ("SCR::Dir() failed");
	return YCPNull ();
    }
    if (!v->isList ())
    {
	ycp2error ("SCR::Dir() did not return a list");
	return YCPNull ();
    }
    return v->asList ();
}


YCPValue
ScriptingAgent::Execute (const YCPPath &path, const YCPValue &value,
			 const YCPValue &arg)
{
    y2debug( "This is ScriptingAgent::Execute" );
    YCPValue v =  executeSubagentCommand ("Execute", path, value, arg);
    if (v.isNull())
    {
	ycp2error ("SCR::Execute() failed");
    }
    return v;
}

YCPMap
ScriptingAgent::Error (const YCPPath &path)
{
    YCPValue v = executeSubagentCommand ("Error", path);
    if (v.isNull())
    {
	ycp2error ("SCR::Error() failed");
	return YCPNull ();
    }
    if (!v->isMap ())
    {
	ycp2error ("SCR::Error() did not return a map");
	return YCPNull ();
    }
    return v->asMap ();
}

YCPValue
ScriptingAgent::otherCommand (const YCPTerm &term)
{
    // FIXME: convert to proper builtins
    const string sym = term->name ();

    if (sym == "MountAgent"
	     && term->size () == 1
	     && term->value (0)->isPath ())
    {
	return MountAgent (term->value (0)->asPath ());
    }
    else if (sym == "MountAllAgents"
	     && term->size () == 0)
    {
	return MountAllAgents ();
    }
    else if (sym == "UnmountAllAgents"
	     && term->size () == 0)
    {
	return UnmountAllAgents ();
    }
    else if (sym == "YaST2Version" || sym == "SuSEVersion")
    {
	// SuSEVersion is the older name (for historic reasons)
	// yes: ignore all arguments
	return YCPString (SUSEVERSION);
    }

    return YCPNull ();
}


YCPBoolean
ScriptingAgent::RegisterAgent (const YCPPath &path, const YCPValue &value)
{
    SubAgents::iterator agent = findByPath (path);
    if (agent == agents.end ())
    {
	y2debug ("Path '%s' registered", path->toString ().c_str ());
    }
    else
    {
	ycp2warning ("", 0, "Path '%s' newly registered", path->toString ().c_str ());
	delete *agent;
	agents.erase (agent);
    }

    agents.insert (std::lower_bound (agents.begin (), agents.end (), path),
		   new SCRSubAgent (path, value));

#if 0
    // only use for testing! insecure!
    FILE* fout = fopen ("/tmp/scr-agents.txt", "w");
    for (SubAgents::const_iterator agent = agents.begin ();
	 agent != agents.end (); ++agent)
    {
	fprintf (fout, "%ld %s\n", (*agent)->get_path ()->length (),
		 (*agent)->get_path ()->toString ().c_str ());
    }
    fclose (fout);
#endif

    return YCPBoolean (true);
}


YCPBoolean
ScriptingAgent::UnregisterAgent (const YCPPath &path)
{
    SubAgents::iterator agent = findByPath (path);
    if (agent == agents.end ())
    {
	y2debug ("Path '%s' is not registered", path->toString ().c_str ());
	return YCPBoolean (false);
    }

    y2debug ("Path '%s' unregistered", path->toString ().c_str ());
    delete *agent;
    agents.erase (agent);
    return YCPBoolean (true);
}


YCPBoolean
ScriptingAgent::UnregisterAllAgents ()
{
    for (SubAgents::iterator agent = agents.begin (); agent != agents.end ();
	 ++agent)
    {
	y2debug ("Path '%s' unregistered",
		 (*agent)->get_path ()->toString ().c_str ());
        delete *agent;
    }
    agents.clear ();
    return YCPBoolean (true);
}


YCPValue
ScriptingAgent::MountAgent (const YCPPath &path)
{
    SubAgents::iterator agent = findByPath (path);
    if (agent == agents.end ())
    {
	return YCPError ("Path '" + path->toString() + "' not registered",
			 YCPBoolean (false));
    }
    return (*agent)->mount (this);
}


YCPValue
ScriptingAgent::MountAllAgents ()
{
    YCPValue v = YCPNull();
    for (SubAgents::const_iterator agent = agents.begin ();
         agent != agents.end (); agent++)
    {
	v = (*agent)->mount (this);
	if (v.isNull ())
	{
	    return v;
	}
    }
    return YCPBoolean (true);
}


YCPBoolean
ScriptingAgent::UnmountAgent (const YCPPath &path)
{
    SubAgents::iterator agent = findByPath (path);
    if (agent == agents.end () || (*agent)->get_comp () == 0)
    {
	return YCPBoolean (false);
    }
    (*agent)->unmount ();
    return YCPBoolean (true);
}


YCPValue
ScriptingAgent::UnmountAllAgents ()
{
    for (SubAgents::const_iterator agent = agents.begin ();
	 agent != agents.end (); agent++)
    {
	(*agent)->unmount ();
    }
    return YCPBoolean (true);
}

YCPBoolean
ScriptingAgent::RegisterNewAgents ()
{
    for (int level = 0; level < Y2PathSearch::numberOfComponentLevels ();
	 level++)
    {
	string dir = Y2PathSearch::searchPath (Y2PathSearch::GENERIC, level) + "/scrconf";
	y2debug( "Scripting agent searching SCRs in %s", dir.c_str() );
	parseConfigFiles (dir);
    }
    return YCPBoolean (true);
}

YCPValue
ScriptingAgent::executeSubagentCommand (const char *command,
					const YCPPath &path,
					const YCPValue &arg,
					const YCPValue &optpar)
{
    y2debug( "ScriptingAgent::executeSubagentCommand: %s", command );
    y2debug( "path: %s", path->toString ().c_str ());
    y2debug( "arg: %s", arg.isNull() ? "null" : arg->toString().c_str ());
    y2debug( "opt: %s", optpar.isNull() ? "null" : optpar->toString().c_str ());
    
    /**
     *  Find the agent where the agent's path and the given path have
     *  the longest match. Example:
     *
     *  agent net at .etc.network
     *  agent isdn at .etc.network.isdn
     *
     *  The command Read (.etc.network.isdn.line0) will call agent
     *  isdn with Read (.line0). The command Read (.etc.network)
     *  will call agent net with Read (.).
     */

    /*
     *  The list of agents is sorted. So we can simply take the first
     *  that does match.
     */

    SubAgents::const_iterator agent = agents.end ();

    // known bug: doesn't work with const_reverse_iterator
    for (SubAgents::reverse_iterator it = agents.rbegin ();
	 it != agents.rend (); ++it)
    {
	if ((*it)->get_path ()->isPrefixOf (path))
	{
	    agent = it.base () - 1;	// convert reverse_iterator to iterator
	    break;
	}
    }

    if (agent == agents.end ())
    {
	// Special case to have the possibility of Dir (.rc) or similar...
	if (strcmp (command, "Dir") != 0)
	{
	    return YCPError ("Couldn't find an agent to handle '" +
			     path->toString () + "'");
	}

	const long path_length = path->length ();
	YCPList dir_list = YCPList ();

	for (SubAgents::const_iterator it = agents.begin ();
	     it != agents.end (); ++it)
	{
	    YCPPath it_path = (*it)->get_path ();
	    if (path->isPrefixOf (it_path))
	    {
		YCPString str = YCPString (it_path->component_str (path_length));
		const int size = dir_list->size ();
		if (size == 0 || !dir_list->value (size - 1)->equal (str))
		    dir_list->add (str);
	    }
	}

	return dir_list;
    }

    (*agent)->mount (this);

    if (!(*agent)->get_comp ())
    {
	ycp2error ("Couldn't mount agent to handle '%s'", path->toString().c_str ());
	return YCPNull ();
    }
    
    YCPTerm commandterm (command);
    commandterm->add (path->at ((*agent)->get_path ()->length ())); // relative path

    if (!arg.isNull ())
	commandterm->add (arg);
    if (!optpar.isNull ())
	commandterm->add (optpar);

    return (*agent)->get_comp ()->evaluate (commandterm);
}


ScriptingAgent::SubAgents::iterator
ScriptingAgent::findByPath (const YCPPath &path)
{
    SubAgents::iterator agent = std::lower_bound (agents.begin (),
						  agents.end (),
						  path);

    if (agent != agents.end () && (*agent)->get_path ()->equal (path))
	return agent;

    return agents.end ();
}
