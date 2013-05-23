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
   Summary:     WFM Builtins

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#include <libintl.h>
#include <langinfo.h>
#include <locale.h>
#include <errno.h>

#include <algorithm>

#include <y2/Y2ComponentBroker.h>
#include <y2/Y2ProgramComponent.h>

#include <ycp/y2log.h>
#include <ycp/pathsearch.h>
#include <ycp/ExecutionEnvironment.h>
#include <ycp/Parser.h>
#include <ycp/Bytecode.h>
#include <ycp/YBlock.h>
#include <ycp/YBreakpoint.h>
#include <scr/SCRAgent.h>
#include <scr/SCR.h>

#include "Y2WFMComponent.h"
#include "Y2SystemNamespace.h"


Y2WFMComponent* Y2WFMComponent::current_wfm = 0;

Y2WFMComponent::Y2WFMComponent ():
      handle_cnt (1),
      local ("ag_system", -1),
      modulename (""),
      argumentlist (YCPList()),
script (YCPNull()),
      client_name (""),
      fullname ("")
{
    y2debug ("Initialized Y2WFMComponent instance");

    // pre-init language
    const char* lang = get_env_lang ();
    if (lang)
    {
        YCPList args;
        args->add (YCPString (lang));

        // set locale according to the language setting
        setlocale (LC_ALL, "");
        // get encoding of the environment where yast is started
        environmentEncoding = nl_langinfo (CODESET);

        SetLanguage (YCPString (lang));
    }

    // initialize SCR and builtins, so it is enough to create the instance of
    // WFM to get builtins work - used by perl bindings (#37338)

    createDefaultSCR ();

    if (scrs.empty ())
    {
	// problems with creation of default SCR in constructor
	return;
    }

    if (! current_wfm)
    {
	// if there is wfm already, it will handle the WFM builtins!
	current_wfm = this;
    }
}


Y2WFMComponent::~Y2WFMComponent ()
{
    // delete all SCRs
    for (WFMSubAgents::iterator it = scrs.begin (); it != scrs.end (); it++)
        delete *it;

    if (current_wfm == this)
    {
	current_wfm = NULL;
    }
}


string
Y2WFMComponent::name () const
{
    return "wfm";
}


bool Y2WFMComponent::createDefaultSCR ()
{
    // first, initialize builtin declarations if needed
    if (! SCR::registered)
    {
	SCR scr;
    }

    // create default scr
    WFMSubAgent* scr = new WFMSubAgent ("scr", 0);

    if (!scr->start ()) {
	return false;
    }

    scrs.push_back (scr);
    default_handle = 0;
    scr->agent()->setAsCurrentSCR();

    return true;
}

YCPValue
Y2WFMComponent::doActualWork (const YCPList& arglist, Y2Component *displayserver)
{
    y2debug( "Starting evaluation" );
    
    bool debugger = false;
    YCPList client_arglist = arglist;
    
    // hack: look only at the last entry, if it's debugger or not
    if (arglist->size () > 0)
    {
	YCPValue last = arglist->value (arglist->size ()-1);
	if (last->isSymbol () && last->asSymbol()->symbol() == "debugger")
	{
	    y2debug ("Enabling debugger");
	    debugger = true;

	    // remove the flag from the arguments
	    client_arglist->remove (arglist->size ()-1);
	}
    }

    // Prepare the arguments. It has the form [script, [clientargs...]]
    YCPList wfm_arglist;
    wfm_arglist->add(script);
    wfm_arglist->add(YCPString(name()));
    wfm_arglist->add (YCPString (fullname));
    wfm_arglist->add(client_arglist);

    // store the old arguments and module name to preserve reentrancy
    YCPList old_arguments = argumentlist;
    string old_modulename = modulename;

    // wfm always gets three arguments:
    // 0: any script:        script to execute
    // 1: string modulename: name of the module to realize
    // 2: list arglist:      arguments for that module

    YCPValue script = YCPVoid();
    modulename = "unknown";
    string current_file = "unknown";
    YCPList  args_for_the_script;

    if (wfm_arglist->size() != 4
	|| !wfm_arglist->value(1)->isString()
	|| !wfm_arglist->value(2)->isString()
	|| !wfm_arglist->value(3)->isList())
    {
	y2error ("Incorrect arguments %s", wfm_arglist->toString().c_str());
    }
    else
    {
	script		    = wfm_arglist->value(0);
	modulename	    = wfm_arglist->value(1)->asString()->value();
	current_file	    = wfm_arglist->value(2)->asString()->value();
	argumentlist	    = wfm_arglist->value(3)->asList();
    }

    y2debug ("Script is: %s", script->toString().c_str());

    y2debug ("Y2WFMComponent @ %p, displayserver @ %p", this, displayserver);
    
    if (debugger)
	script = YCPCode ((YCodePtr)new YBreakpoint (script->asCode ()->code (), "code start"));

    YCPValue v = script->asCode ()->evaluate ();

    y2debug( "Evaluation finished" );

    // restore the old arguments and module name to preserve reentrancy
    argumentlist = old_arguments;
    modulename = old_modulename;

    return v;
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
    static const char* names[] = { "LANGUAGE", "LC_ALL", "LC_MESSAGES", "LANG" };

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
    /**
     * @builtin SCROpen
     * @short Create a new scr instance.
     *
     * @description
     *
     * Creates a new scr instance. The name must be a valid y2component name
     * (e.g. "scr", "chroot=/mnt:scr"). The component is created immediately.
     * The parameter check_version determines whether the SuSE Version should
     * be checked. On error a negative value is returned.
     *
     * @param string name a valid y2component name
     * @param boolean check_version determines whether the SuSE Version should be
     * checked
     * @return integer On error a negative value is returned.
     */

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
    /**
     * @builtin SCRClose
     * @short Closes a scr instance.
     * @param integer handle  SCR handle
     * @return void
     */

    int handle = h->value ();
    WFMSubAgents::iterator it = find_handle (handle);

    if (it == scrs.end ())
    {
	// invalid handle
	ycperror ("Trying to close undefined handle '%d'", handle);
	return;
    }

    // if it was the default one, try to handle the situation gracefully
    if (handle == default_handle)
    {
	default_handle = -1;
	ycpmilestone ("There is no default SCR set now");
	// SCR::instance () is set to NULL by the SCRAgent destructor

	for ( SystemNamespaces::iterator ns = system_namespaces.begin ();
		ns != system_namespaces.end (); ns ++ )
	{
	    y2milestone ("Redirecting system namespace '%s' to local"
		, (*ns)->name ().c_str ());

	    (*ns)->useLocal ();
	}

    }

    WFMSubAgent* agent = (*it);

    scrs.erase (it);
    delete agent;

    ycpmilestone ("SCR handle %d closed", handle);
}


YCPString
Y2WFMComponent::SCRGetName (const YCPInteger &h)
{
    /**
     * @builtin SCRGetName
     * @short Get the name of a scr instance.
     * @param integer handle SCR handle
     * @return string Name
     */

    int handle = h->value ();
    WFMSubAgents::iterator it = find_handle (handle);
    return YCPString (it != scrs.end () ? (*it)->get_name () : "");
}


void
Y2WFMComponent::SCRSetDefault (const YCPInteger &handle)
{
    /**
     * @builtin SCRSetDefault
     * @short Sets the default scr instance.
     * @param integer handle SCR handle
     * @return void
     */

    y2milestone ("Setting default SCR: %s", handle->toString ().c_str ());

    default_handle = handle->value ();
    WFMSubAgents::iterator it = find_handle (default_handle);
    if (it != scrs.end ())
    {
	if ((*it)->agent ())
	{
	    (*it)->agent ()->setAsCurrentSCR ();

	    bool remote = (*it)->comp ()->remote ();

	    for ( SystemNamespaces::iterator ns = system_namespaces.begin ();
		    ns != system_namespaces.end (); ns ++ )
	    {
		y2milestone ("Redirecting system namespace '%s' via '%s'"
		    , (*ns)->name ().c_str (), (*it)->comp ()->name ().c_str ());

		if (remote)
		    (*ns)->useRemote (dynamic_cast<Y2ProgramComponent*>((*it)->comp ()));
		else
		    (*ns)->useLocal ();
	    }
	}
    }
}


YCPInteger
Y2WFMComponent::SCRGetDefault () const
{
    /**
     * @builtin SCRGetDefault
     * @short Gets the default scr instance.
     * @return integer Default SCR handle
     */

    return YCPInteger (default_handle);
}


YCPValue
Y2WFMComponent::Args (const YCPInteger& i) const
{
    /**
     * @builtin Args
     * @short Returns the arguments with which the module was called.
     * @description
     * The result is a list whose
     * arguments are the module's arguments. If the module
     * was called with <tt>CallFunction("my_mod", [17,true])</tt>,
     * <tt>Args()</tt> will return <tt>[ 17, true ]</tt>.
     *
     * @return list List of arguments
     */

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
    /**
     * @builtin GetLanguage
     * @short Returns the current language code (without modifiers !)
     * @return string Language
     */

    return YCPString(currentLanguage);
}


YCPString
Y2WFMComponent::GetEncoding () const
{
    /**
     * @builtin GetEncoding
     * @short Returns the current encoding code
     * @return string Encoding
     */

    return YCPString(systemEncoding);
}


YCPString
Y2WFMComponent::GetEnvironmentEncoding ()
{
    /**
     * @builtin GetEnvironmentEncoding
     * @short Returns the encoding code of the environment where YaST is started
     * @return string encoding code of the environment
     */
    return YCPString(environmentEncoding);
}


YCPString
Y2WFMComponent::SetLanguage (const YCPString& language, const YCPString& encoding)
{
    /**
     * @builtin SetLanguage
     * @short Selects the language for translate()
     * @param string language
     * @optarg string encoding
     * @usage SetLanguage("de_DE", "UTF-8") -> ""
     * @usage SetLanguage("de_DE@euro") -> "ISO-8859-15"
     *
     * @description
     * The "<proposed encoding>" is the output of 'nl_langinfo (CODESET)'
     * and only given if SetLanguage() is called with a single argument.
     *
     * @return string proposed encoding
     * have fun
     *
     */

    string proposedEncoding;

    currentLanguage = language->value ();

    if (! encoding.isNull ())
    {
	systemEncoding = encoding->value();
	y2milestone( "SET encoding to: %s", systemEncoding.c_str() );
    }
    else
    {
	// get current LC_CTYPE
	string locale = setlocale( LC_CTYPE, NULL );

        // prepare for nl_langinfo (set LC_CTYPE)
	setlocale ( LC_CTYPE, currentLanguage.c_str());

	// get the proposed encoding from nl_langinfo()
	proposedEncoding = nl_langinfo (CODESET);
	if (proposedEncoding.empty())
	{
	    y2warning ("nl_langinfo returns empty encoding for %s", currentLanguage.c_str());
	}
	else
	{
	    systemEncoding = proposedEncoding;
	}

	y2milestone ( "GET encoding for %s:  %s", currentLanguage.c_str(), proposedEncoding.c_str() );

	// reset LC_CTYPE !!! (for ncurses)
	setlocale( LC_CTYPE, locale.c_str() );
    }

    setlocale (LC_NUMERIC, "C");       // always format numbers with "."

    /* Change language. see info:gettext: */
    setenv ("LANGUAGE", currentLanguage.c_str(), 1);

    /* Make change known.  */
    {
        extern int _nl_msg_cat_cntr;
        ++_nl_msg_cat_cntr;
    }

    // FIXME: should be ycp2debug
    y2debug ( "WFM SetLanguage(\"%s\"), Encoding(\"%s\")",
	       currentLanguage.c_str(), systemEncoding.c_str());
    return YCPString (proposedEncoding);
}


YCPValue
Y2WFMComponent::Read (const YCPPath &path, const YCPValue& arg)
{
    /**
     * @builtin Read
     * @short Special interface to the system agent. Not for general use.
     * @param path path Path
     * @optarg any options
     * @return any
     * @description
     * WFM::Read (.local.foo, ... ) works like SCR::Read (.target.foo, ...)
     * but in the inst-sys rather than on the system being installed.
     * It only works for paths starting with .local
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
     * @builtin Write
     * @short Special interface to the system agent. Not for general use.
     * @param path path Path
     * @optarg any options
     * @return boolean
     * @description
     * WFM::Write (.local.foo, ... ) works like SCR::Write (.target.foo, ...)
     * but in the inst-sys rather than on the system being installed.
     * It only works for paths starting with .local
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
     * @builtin Execute
     * @short Special interface to the system agent. Not for general use.
     * @param path path Path
     * @optarg any options
     * @return any
     * @description
     * WFM::Execute (.local.foo, ... ) works like SCR::Execute (.target.foo, ...)
     * but in the inst-sys rather than on the system being installed.
     * It only works for paths starting with .local
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


YCPBoolean
Y2WFMComponent::ClientExists (const YCPString& client)
{
    /**
     * @builtin ClientExists
     * @short Checks whether a YCP client exists
     * @param string name client name
     *
     * @description
     * This is similar to 'call' or 'CallFunction' but client is only
     * checked for existence and not executed. If client exists
     * 'true' is returned, otherwise 'false'.
     *
     * @usage ClientExists ("inst_mouse") -> true
     * @usage ClientExists ("missing_client") -> false
     * @return boolean whether client exists
     */

    string new_modulename = client->value ();

    if (client.isNull() || new_modulename == "")
    {
	y2error ("Client name not defined %s", new_modulename.c_str());
	return YCPBoolean (false);
    }

    // try loading a client via Y2ComponentBroker
    Y2Component* client_comp = Y2ComponentBroker::createClient (new_modulename.c_str ());
    if (client_comp)
    {
	return YCPBoolean (true);
    }
    else
    {
	return YCPBoolean (false);
    }
}

YCPList
Y2WFMComponent::SetArgs (const YCPList &new_args)
{
  YCPList result = argumentlist;
  argumentlist = new_args;
  return result;
}

YCPValue
Y2WFMComponent::CallFunction (const YCPString& client, const YCPList& args)
{
    /**
     * @builtin call
     * @short Executes a YCP client or a Y2 client component.
     * @param string name client name
     * @param list arguments list of arguments
     *
     * @description
     * This implies * that the called YCP code has full access to
     * all module status in the currently running YaST.
     *
     * The modulename is temporarily changed to the name of the
     * called script or a component.
     *
     * In the example, WFM looks for the file YAST2HOME/clients/inst_mouse.ycp
     * and executes it. If the client is not found, a Y2 client component
     * is tried to be created.
     *
     * @usage call ("inst_mouse", [true, false]) -> ....
     * @return any
     */

    string new_modulename = client->value ();

    // try loading a client via Y2ComponentBroker
    Y2Component* client_comp = Y2ComponentBroker::createClient (new_modulename.c_str ());
    if (client_comp)
    {
	string filename = YaST::ee.filename();
	int linenumber = YaST::ee.linenumber();
	ycp2milestone (filename.c_str(), linenumber,
		       "Calling YaST client %s", new_modulename.c_str ());
	ycp2debug (filename.c_str(), linenumber,
		       "(arguments: %s)", args->toString ().c_str ());
	YCPValue result = client_comp->doActualWork (args, NULL);
	YaST::ee.setFilename(filename);
	YaST::ee.setLinenumber(linenumber);
	ycp2milestone (filename.c_str(), linenumber, "Called YaST client returned.");
	// some clients return plaintext secrets #248300
	ycp2debug (filename.c_str(), linenumber,
		       "Called YaST client returned: %s", result.isNull () ? "nil" : result->toString ().c_str ());
	return result;

    }
    else
    {
	// no help
	ycp2error ("Can't find YCP client component %s: %s", new_modulename.c_str(), strerror (errno));
	return YCPNull ();
    }
}


Y2Namespace *
Y2WFMComponent::import (const char* name_space)
{
    // System:: namespace access
    if (strstr (name_space, "System::") == name_space)
    {
        y2milestone("import System namespace %s", name_space);
        // check if namespace is not already imported

      	for ( SystemNamespaces::iterator ns = system_namespaces.begin ();
          		ns != system_namespaces.end (); ns ++ )
        {
            if ((*ns)->name() == name_space)
            {
                y2milestone("Namespace %s already imported", name_space);
                return *ns;
            }
        }

        char* subsys = const_cast<char*>(name_space) + 8; // skip the prefix
        Y2Component* local_comp = Y2ComponentBroker::getNamespaceComponent (subsys);

	if (local_comp == 0)
	    return 0;

	Y2Namespace* local_ns = local_comp->import (subsys);

        if (local_ns != 0)
        {
            // there is a local component, use it for the wrapper

            Y2SystemNamespace* ns = new Y2SystemNamespace (local_ns);

            system_namespaces.push_back (ns);
            
            y2milestone("Namespace %s properly imported", ns->name().c_str());
            return ns;
        }
        return 0;
    }

    y2milestone ("Y2WFMComponent::import (%s)", name_space);

    // create the namespace
    // maybe this failed, but it does not mean any problems, block () will simply return 0
    YBlockPtr block = Bytecode::readModule (name_space);
    if (block == 0)
    {
	return 0;
    }
    return block->nameSpace();
}

void Y2WFMComponent::setupComponent (string cn, string fn,
				      const YCPValue& sc)
{
    script = sc;
    client_name = cn;
    fullname = fn;
}

Y2WFMComponent* Y2WFMComponent::instance()
{
    if (! current_wfm)
    {
	current_wfm = new Y2WFMComponent();
    }

    return current_wfm;
}
