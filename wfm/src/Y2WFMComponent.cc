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

   File:	Y2WFMComponent.cc

   Author:	Mathias Kettner <kettner@suse.de>
		Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include <libintl.h>
#include <locale.h>

#include <y2/Y2ComponentBroker.h>

#include <ycp/y2log.h>
#include <ycp/pathsearch.h>
#include <ycp/ExecutionEnvironment.h>
#include <ycp/Parser.h>
#include <ycp/Bytecode.h>
#include <ycp/YBlock.h>
#include <scr/SCRAgent.h>

#include "Y2WFMComponent.h"

extern ExecutionEnvironment ee;

Y2WFMComponent* Y2WFMComponent::current_wfm = 0;

Y2WFMComponent::Y2WFMComponent ():
      handle_cnt (1),
      local ("ag_system", -1),
      modulename (""),
      argumentlist (YCPList()),
      pkgmodule (0)
{
    y2debug ("Initialized Y2WFMComponent instance");
    // pre-init language
    const char* lang = get_env_lang ();
    if (lang)
    {
	SetLanguage( YCPString(lang) );
    }
}


Y2WFMComponent::~Y2WFMComponent ()
{
    // delete all SCRs
    for (WFMSubAgents::iterator it = scrs.begin (); it != scrs.end (); it++)
        delete *it;
}


string
Y2WFMComponent::name () const
{
    return "wfm";
}


YCPValue
Y2WFMComponent::doActualWork (const YCPList& arglist, Y2Component *displayserver)
{
    y2debug( "Starting evaluation" );
    
    // wfm always gets three arguments:
    // 0: any script:        script to execute
    // 1: string modulename: name of the module to realize
    // 2: list arglist:      arguments for that module

    YCPValue script = YCPVoid();
    modulename = "unknown";
    string current_file = "unknown";
    YCPList  args_for_the_script;

    if (arglist->size() != 4
	|| !arglist->value(1)->isString()
	|| !arglist->value(2)->isString()
	|| !arglist->value(3)->isList())
    {
	y2error ("Incorrect arguments %s", arglist->toString().c_str());
    }
    else
    {
	script		    = arglist->value(0);
	modulename	    = arglist->value(1)->asString()->value();
	current_file	    = arglist->value(2)->asString()->value();
	argumentlist	    = arglist->value(3)->asList();
    }

    y2debug ("Y2WFMComponent @ %p, displayserver @ %p", this, displayserver);
    
    // create default scr
    WFMSubAgent* scr = new WFMSubAgent ("scr", 0);
    scrs.push_back (scr);
    default_handle = 0;
    
    if (!scr->start ()) {
	return YCPVoid ();
    }
    
    scr->agent()->setAsCurrentSCR();

    // preserve stack ordering of WFMs - callmodule
    Y2WFMComponent* old_wfm = current_wfm;
    current_wfm = this;


    YCPValue v = script->asCode ()->evaluate ();
    
    current_wfm = old_wfm;

    y2debug( "Evaluation finished" );

    return v;
}


YCPValue
Y2WFMComponent::evaluate (const YCPValue& command)
{
    return YCPError("No interpreter for WFM callback available");
}

Y2WFMComponent::WFMSubAgents::iterator
Y2WFMComponent::find_handle (int handle)
{
    WFMSubAgents::iterator it = std::lower_bound (scrs.begin (), scrs.end (),
                                                  handle, wfmsubagent_less);

    if (it != scrs.end () && (*it)->get_handle () == handle)
        return it;

    return scrs.end ();
}


const char*
Y2WFMComponent::get_env_lang () const
{
    static char* names[] = { "LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG" };

    for (size_t i = 0; i < sizeof (names)/sizeof (names[0]); i++)
    {
	const char* tmp = getenv (names[i]);
	if (tmp)
	    return tmp;
    }

    return 0;
}


/********************************** builtins ******************************/

YCPInteger
Y2WFMComponent::SCROpen (const YCPString& scrname, const YCPBoolean &checkversion)
{
    string name = scrname->value ();
    bool check_version = checkversion->value ();

    int handle = handle_cnt++;

    WFMSubAgent* agent = new WFMSubAgent (name, handle);

    int error;
    if (!agent->start_and_check (check_version, &error))
    {
	y2error ("SCROpen '%s' failed: %d", name.c_str (), error);
	delete agent;
	return YCPInteger (error);
    }

    scrs.push_back (agent);
    return YCPInteger (handle);
}


void
Y2WFMComponent::SCRClose (const YCPInteger& h)
{
    int handle = h->value ();
    WFMSubAgents::iterator it = find_handle (handle);
    delete *it;
    scrs.erase (it);
}


YCPString
Y2WFMComponent::SCRGetName (const YCPInteger &h)
{
    int handle = h->value ();
    WFMSubAgents::iterator it = find_handle (handle);
    return YCPString (it != scrs.end () ? (*it)->get_name () : "");
}


void
Y2WFMComponent::SCRSetDefault (const YCPInteger &handle)
{
    default_handle = handle->value ();
    WFMSubAgents::iterator it = find_handle (default_handle);
    if (it != scrs.end ())
    {
	if ((*it)->agent ())
	    (*it)->agent ()->setAsCurrentSCR ();
    }
}


YCPInteger
Y2WFMComponent::SCRGetDefault () const
{
    return YCPInteger (default_handle);
}

YCPValue 
Y2WFMComponent::GetClientName(const YCPInteger& filedescriptor)
{
    YCPList arglist = argumentlist;
    long long index = filedescriptor->value ();
    if (index < 0)
    {
	return YCPError ("Invalid negative index to Args(). Only values >= 0 are allowed");
    }
    else
    {
	int   clnamefd;
	int   readgood, red;
	char  readstr[MAX_CLIENT_NAME_LEN+2] = "";

	// char debug_buffer[100];
	// sprintf( debug_buffer, "NB. %lld", index);
	// y2debug( debug_buffer);

	clnamefd = index;

	readgood = 0;
	do {
	    red = read( clnamefd, readstr + readgood, MAX_CLIENT_NAME_LEN - 1 - readgood);

	    // sprintf( debug_buffer, "READ %d -- %s", red, readstr);
	    // y2error( debug_buffer);

	    if (red == 0)
		break;
	    if (red < 0)
	    {
		YCPError(" Can't read secret from fd\n");
		y2error( " Can't read secret from fd\n");
		readgood = -1;
		break;
	    }
	    readgood += red;

	} while (readgood < MAX_CLIENT_NAME_LEN - 1);

	if ( readgood > 0 )
	{
	    readstr[readgood] = 0;

	    // sprintf( debug_buffer, "READ %s", readstr);
	    // y2debug( debug_buffer);
	    y2debug ("Client: %s", readstr);
	    return YCPString( readstr );
	}
	else
	{
	    y2error( "Can't read from pipe. readgood <= 0");
	    return YCPNull();
	}
    }
}

YCPValue
Y2WFMComponent::Args (const YCPInteger& i) const
{
    if (i.isNull ())
    {
	return argumentlist;
    }
    else
    {
	YCPList arglist = argumentlist;
	long long index = i->value ();
	if (index < 0)
	{
	    return YCPError ("Invalid negative index to Args(). Only values >= 0 are allowed");
	}
	else if (index < arglist->size())
	{
	    return arglist->value (index);
	}
	else
	{
	    return YCPError ("Index to Args() larger than number of available arguments.");
	}
    }

    return YCPNull();
}


YCPString
Y2WFMComponent::GetLanguage () const
{
    return YCPString(currentLanguage);
}


YCPString
Y2WFMComponent::GetEncoding () const
{
    return YCPString(currentEncoding);
}


YCPString
Y2WFMComponent::SetLanguage (const YCPString& language, const YCPString& encoding)
{
    string proposedEncoding;

    currentLanguage = language-> value();
    if (encoding.isNull ()) {
	setlocale (LC_ALL, currentLanguage.c_str ());	// prepare for nl_langinfo
	proposedEncoding = nl_langinfo (CODESET);
	if (proposedEncoding.empty())
	{
	    y2warning ("nl_langinfo returns empty encoding for %s", currentLanguage.c_str());
	}
	y2milestone ("LC_ALL = '%s', proposedEncoding %s", currentLanguage.c_str(), proposedEncoding.c_str());
	currentEncoding = "UTF-8";				// default encoding
    }
    else 
    {
	currentEncoding = encoding->value ();
    }
    
    setlocale (LC_NUMERIC, "C");        // always format numbers with "."

    bindtextdomain ("base", LOCALEDIR);
    
    y2debug ("Codeset will be %s", currentEncoding.c_str());

    bind_textdomain_codeset ("base", currentEncoding.c_str());

    /* Change language. see info:gettext: */
    setenv ("LANGUAGE", currentLanguage.c_str(), 1);
    setenv ("LC_MESSAGES", currentLanguage.c_str(), 1);

    /* Make change known.  */
    {
        extern int _nl_msg_cat_cntr;
        ++_nl_msg_cat_cntr;
    }

    y2debug ("language '%s', text encoding '%s', textdomain '%s'",
             currentLanguage.c_str(), currentEncoding.c_str(),
             "base");

    return YCPString (proposedEncoding);
}

YCPValue Y2WFMComponent::Read (const YCPPath &path, const YCPValue& arg)
{
    /**
     * @builtin Read (path, [any]) -> any
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
        return YCPVoid ();

    // strip leading .local from path

    if (path->length () < 2)
        return YCPError ("Too short path for WFM::Read");

    if (path->component_str (0) != "local")
        return YCPError ("Path not '.local' in WFM::Read");

    YCPPath p = path->at (1);

    if (arg.isNull ())
    {
        return local.agent ()->Read (p);
    }
    else 
    {
        return local.agent ()->Read (p, arg);
    }
}

YCPValue
Y2WFMComponent::Write (const YCPPath &path, const YCPValue &arg1, const YCPValue &arg2)
{
    /**
     * @builtin Write (path, any, [any]) -> boolean
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
        return YCPVoid ();

    // strip leading .local from path

    if (path->length () < 2)
        return YCPError ("Too short path for WFM::Write");

    if (path->component_str (0) != "local")
        return YCPError ("Path not '.local' in WFM::Write");

    YCPPath p = path->at (1);

    return local.agent ()->Write (p, arg1, arg2);
}

YCPValue
Y2WFMComponent::Execute (const YCPPath &path, const YCPValue &arg1)
{
    /**
     * @builtin Execute (path, any) -> any
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
        return YCPVoid ();

    // strip leading .local from path

    if (path->length () < 2)
        return YCPError ("Too short path for WFM::Execute");

    if (path->component_str (0) != "local")
        return YCPError ("Path not '.local' in WFM::Execute");

    YCPPath p = path->at (1);

    return local.agent ()->Execute (p, arg1);
}

YCPValue
Y2WFMComponent::CallFunction (const YCPString& client, const YCPList& args)
{
    /**
     * @builtin CallFunction(string name, list arguments) -> any
     * This is much like CallModule, but it differs in the way how the
     * module is called. The module call does not make use of the Y2 component
     * system. CallFunction simply looks for a YCP script residing in
     * YAST2HOME or one of the other directories in the Y2 search path.
     * You can't call arbitrary client components, just YCP scripts.
     * The YCP script then is not executed in an own workflow manager,
     * but is just called as a function.
     *
     * This implies, that the called YCP code has full access to
     * all symbol definitions of the YCP code is was called from!
     * A new scope is opened for each block of the YCP code. Since most
     * YCP scripts are implemented as blocks (starting with {) // keep emacs happy }
     * so any symbol declarations inside the called code are dropped
     * when the code is left.
     *
     * What CallFunction has in common with CallModule is that
     * the modulename is temporarily changed to the name of the
     * called function. This has an impact on the translator, that
     * always needs to know, which module as currently being executed.
     *
     * @example CallFunction ("inst_mouse", [true, false]) -> ....
     *
     * In the example, WFM looks for the file YAST2HOME/clients/inst_mouse.ycp
     * and executes it.
     */

    string new_modulename = client->value ();
    string filename = "clients/" + new_modulename + ".ycp";
    string fullname = Y2PathSearch::findy2 (filename);
    
    int fd = -1;
    if (!fullname.empty ())
    {
	fd = open (fullname.c_str (), O_RDONLY, 0644);
    }

    if (fd < 0)
    {
	// try loading a client via Y2ComponentBroker
	Y2Component* client = Y2ComponentBroker::createClient (new_modulename.c_str ());
	if (client)
	{
	    ycp2milestone (ee.filename ().c_str(), ee.linenumber (),
		       "Calling YaST client %s", new_modulename.c_str ());
	    YCPValue result = client->doActualWork (args, NULL);
	    
	    ycp2milestone (ee.filename ().c_str(), ee.linenumber (),
		       "Called YaST client returned: %s", result.isNull () ? "nil" : result->toString ().c_str ());
	    return result;

	}
	else
	{
	    // no help
	    return YCPError ("Can't find YCP client component " + filename + ": " + strerror (errno));
	}
    }

    YCPValue result = YCPVoid();

    Parser parser(fd, fullname.c_str());
    parser.setBuffered(); // Read from file. Buffering is always possible here
    YCode* ycpscript = parser.parse();
    close(fd);

    if (! ycpscript)
    {
	    result = YCPError ("Invalid YCP component " + fullname);
    }
    else
    {
	ycp2milestone (ee.filename ().c_str(), ee.linenumber (),
		       "Calling YCP function module %s", filename.c_str());

	// I just evaluate the value ycpscript. But I have to make
	// sure, that it has access to it's arguments given in argterm.
	// If temporarily overwrite the member variable argumentlist.

	YCPList old_argumentlist = this->argumentlist;
	this->argumentlist = args; // YYYYYYYYYYY switch CallFunction from term to list

	result = ycpscript->evaluate();
	argumentlist = old_argumentlist;
    }

    return result;
}

Y2Namespace* Y2WFMComponent::import (const char* name_space, const char* timestamp)
{
    // create the namespace
    // maybe this failed, but it does not mean any problems, block () will simply return 0
    y2debug ("Timestamp requested: %s", timestamp);
    Y2Namespace *ns = Bytecode::readModule (name_space, timestamp != NULL ? timestamp : "" );
    
    return ns;
}
