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

   File:	WFMInterpreter.cc

   Authors:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
		Arvin Schnell <arvin@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <resolv.h>
#include <netdb.h>
#include <dirent.h>
#include <sys/stat.h>
#include <locale.h>
#include <langinfo.h>
#include <libintl.h>
#include <algorithm>

#include <config.h>
#include <ycp/y2log.h>
#include <ycp/YCPParser.h>
#include <y2/pathsearch.h>
#include <y2/Y2ComponentBroker.h>
#include <scr/SCRAgent.h>
#include <WFMInterpreter.h>


const char*
WFMInterpreter::get_env_lang () const
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


WFMInterpreter::WFMInterpreter (Y2Component *my_component,
				Y2Component *user_interface,
				string modulename, string fullname,
				const YCPList& argumentlist)
    : wfm_component (my_component),
      user_interface (user_interface),
      handle_cnt (1),
      local ("ag_system", -1),
      modulename (modulename),
      argumentlist (argumentlist),
      pkgmodule (0)
{
    current_file = fullname;
    y2debug ("new WFMInterpreter @ %p, my_component @ %p, user_interface @ %p (%s)", this, my_component, user_interface, current_file.c_str());

    scrs.push_back (new WFMSubAgent ("scr", 0));
    default_handle = 0;

    // pre-init language
    const char* lang = get_env_lang ();
    if (lang)
    {
	YCPList args;
	args->add (YCPString (lang));
	evaluateSetLanguage (YCPTerm (YCPSymbol("SetLanguage", false), args));
    }
}


WFMInterpreter::~WFMInterpreter ()
{
    y2debug ("destructing %p", this);

    for (WFMSubAgents::iterator it = scrs.begin (); it != scrs.end (); it++)
        delete *it;

    if (pkgmodule != 0)
	delete pkgmodule;
}


WFMInterpreter::WFMSubAgents::iterator
WFMInterpreter::find_handle (int handle)
{
    WFMSubAgents::iterator it = std::lower_bound (scrs.begin (), scrs.end (),
						  handle, wfmsubagent_less);

    if (it != scrs.end () && (*it)->get_handle () == handle)
	return it;

    return scrs.end ();
}


string
WFMInterpreter::interpreter_name () const
{
    return "WFM";	// must be upper case
}


YCPValue
WFMInterpreter::evaluateInstantiatedTerm (const YCPTerm& term)
{
    string sym = term->symbol()->symbol();

    y2debug ("WFMInterpreter::evaluateInstantiatedTerm (%s::%s)", term->name_space().c_str(), term->toString().c_str());

    if (term->name_space() == "Pkg")
    {
	if (pkgmodule == 0)
	{
	    pkgmodule = new PkgModule (this);
	    if (pkgmodule == 0)
	    {
		return YCPError ("Can't create PkgModule", YCPVoid());
	    }
	}
	return pkgmodule->evaluate (sym, term->args());
    }

    if	    (sym == "UI")		return evaluateWFM_UI (term);

    else if (sym == "SCR")		return evaluateWFM_SCR (term);
    else if (sym == "SCROpen")		return evaluateSCROpen (term);
    else if (sym == "SCRClose")		return evaluateSCRClose (term);

    else if (sym == "SCRGetName")	return evaluateSCRGetName (term);
    else if (sym == "SCRSetDefault")	return evaluateSCRSetDefault (term);
    else if (sym == "SCRGetDefault")	return evaluateSCRGetDefault (term);

    else if (sym == "Read")		return evaluateRead (term);
    else if (sym == "Write")		return evaluateWrite (term);
    else if (sym == "Execute")		return evaluateExecute (term);
    else if (sym == "Dir")		return evaluateDir (term);

    else if (sym == "SetLanguage")	return evaluateSetLanguage (term);
    else if (sym == "GetLanguage")	return evaluateGetLanguage (term);
    else if (sym == "GetEncoding")	return evaluateGetEncoding (term);

    else if (sym == "CallModule")	return evaluateCallModule (term);
    else if (sym == "CallFunction")	return evaluateCallFunction (term);
    else if (sym == "GetClientName")	return evaluateGetClientName (term);

    else if (sym == "Args")		return evaluateArgs (term);

    else return YCPNull();
}


YCPValue
WFMInterpreter::callback (const YCPValue& value)
{
    y2debug ("WFMInterpreter[%p]::callback(%s)\n", this, value->toString().c_str());
    if (value->isBuiltin())
    {
	YCPBuiltin b = value->asBuiltin();
	YCPValue v = b->value (0);

	if (b->builtin_code() == YCPB_UI)
	{
	    return sendUnquoted (user_interface, value);
	}
	else if (b->builtin_code() == YCPB_SCR)
	{
	    WFMSubAgents::iterator scr = find_handle (default_handle);
	    if (scr == scrs.end ())
		return YCPError ("Unknown SCR handle.\n");

	    if (!(*scr)->start ())
		return YCPVoid ();

	    return sendUnquoted ((*scr)->comp (), value);
	}
	else
	{
	    return evaluate (v);
	}
    }
    return YCPError ("WFM::callback not builtin\n", YCPNull());
}


YCPValue
WFMInterpreter::evaluateWFM (const YCPValue& value)
{
    y2debug ("WFMInterpreter[%p]::evaluateWFM (%s)\n", this, value->toString().c_str());
    if (value->isBuiltin())
    {
	YCPBuiltin b = value->asBuiltin();
	if (b->builtin_code() == YCPB_DEEPQUOTE)
	{
	    return evaluate (b->value(0));
	}
    }
    else if (value->isTerm() && value->asTerm()->isQuoted())
    {
	YCPTerm vt = value->asTerm();
	YCPTerm t(YCPSymbol(vt->symbol()->symbol(), false), vt->name_space());
	for (int i=0; i<vt->size(); i++)
	{
	    t->add(vt->value(i));
	}
	return evaluate (t);
    }
    return evaluate (value);
}


YCPValue
WFMInterpreter::evaluateUI (const YCPValue& value)
{
    y2debug ("WFMInterpreter[%p]::evaluateUI[%p] (%s)\n", this, user_interface, value->toString().c_str());
    if (value->isLocale())
    {
	return evaluateLocale (value->asLocale());
    }
    else if (value->isBuiltin())
    {
	YCPBuiltin b = value->asBuiltin();
	if (b->builtin_code() == YCPB_DEEPQUOTE)
	{
	    return sendUnquoted(user_interface, b->value(0));
	}
    }
    else if (value->isTerm())
    {
	YCPTerm vt = value->asTerm();
	YCPTerm t(YCPSymbol(vt->symbol()->symbol(), false), vt->name_space());
	for (int i = 0; i < vt->size (); i++)
	{
	    YCPValue v = evaluate (vt->value(i));
	    if (v.isNull())
	    {
		return YCPError ("UI parameter is NULL\n", YCPNull());
	    }
	    t->add(v);
	}
	return sendUnquoted(user_interface, t);
    }
    return sendUnquoted(user_interface, value);
}


YCPValue
WFMInterpreter::evaluateSCR (const YCPValue& value)
{
    y2debug ("WFMInterpreter[%p]::evaluateSCR (%s)\n", this, value->toString().c_str());

    WFMSubAgents::iterator scr = find_handle (default_handle);
    if (scr == scrs.end ())
	return YCPError ("Unknown SCR handle.\n");

    if (!(*scr)->start ())
	return YCPVoid ();

    if (value->isBuiltin())
    {
	YCPBuiltin b = value->asBuiltin();
	if (b->builtin_code() == YCPB_DEEPQUOTE)
	{
	    return sendUnquoted ((*scr)->comp (), b->value(0));
	}
    }
    else if (value->isTerm())
    {
	YCPTerm vt = value->asTerm ();
	YCPTerm t (YCPSymbol (vt->symbol ()->symbol (), false), vt->name_space());
	for (int i = 0; i < vt->size (); i++)
	{
	    YCPValue v = evaluate (vt->value (i));
	    if (v.isNull ())
	    {
		return YCPError ("SCR parameter is NULL\n", YCPNull ());
	    }
	    t->add (v);
	}
	return sendUnquoted ((*scr)->comp (), t);
    }
    return sendUnquoted ((*scr)->comp (), value);
}


// override setTextdomain() from YCPBasicInterpreter

YCPValue
WFMInterpreter::setTextdomain (const string& textdomain)
{
    ycp2debug (current_file.c_str(),current_line,"setTextdomain (%s)", textdomain.c_str());
    currentTextdomain = textdomain;
    textdomainOrLanguageHasChanged = true;
    return YCPNull();
}


// override getTextdomain() from YCPBasicInterpreter

string
WFMInterpreter::getTextdomain (void)
{
    return currentTextdomain;
}


YCPValue
WFMInterpreter::evaluateLocale (const YCPLocale& locale)
{
    if (textdomainOrLanguageHasChanged)
	changeToModuleLanguage();

    return locale->translate (currentTextdomain.c_str());
}


YCPValue
WFMInterpreter::evaluateBuiltinBuiltin (builtin_t code, const YCPList& args)
{
    if (code == YCPB_NLOCALE)
    {
	if (args->size() == 3 && args->value(0)->isString() &&
	    args->value(1)->isString() && args->value(2)->isInteger())
	{
	    YCPLocale locale = YCPLocale (args->value(0)->asString(),
					  args->value(1)->asString(),
					  args->value(2)->asInteger());
	    return locale->translate (currentTextdomain.c_str());
	}
    }

    return YCPInterpreter::evaluateBuiltinBuiltin (code, args);
}


void
WFMInterpreter::changeToModuleLanguage () const
{
    // change module and language
    setlocale (LC_ALL, currentLanguage.c_str());
    setlocale (LC_NUMERIC, "C");	// but always format numbers with "."

    bindtextdomain (currentTextdomain.c_str(), LOCALEDIR);

    bind_textdomain_codeset (currentTextdomain.c_str(), currentEncoding.c_str());

    /* Change language. see info:gettext: */
    setenv ("LANGUAGE", currentLanguage.c_str(), 1);
    setenv ("LC_MESSAGES", currentLanguage.c_str(), 1);
    setenv ("LANG", currentLanguage.c_str(), 1);

    /* Make change known.  */
    {
	extern int _nl_msg_cat_cntr;
	++_nl_msg_cat_cntr;
    }

    y2debug ("language '%s', encoding '%s', textdomain '%s'",
	     currentLanguage.c_str(), currentEncoding.c_str(),
	     currentTextdomain.c_str());

    textdomainOrLanguageHasChanged = false;
    return;
}


YCPValue
WFMInterpreter::includeFile (const string& filename)
{
    /**
     * @builtin include "file" -> void
     *
     * @example include "file.inc"
     *
     * In the example, WFM looks for the file YAST2HOME/include/file.inc and
     * executes it.
     * If the filename starts with "./", the include path is relative
     * to the current directory.
     *
     * textdomain is handled in libycp/YCPBlock.cc (include statement)
     *
     */

    string fullname;

    const char *cfilename = filename.c_str();
    y2debug ("WFMInterpreter::includeFile (%s)", cfilename);

    int fd = -1;
    if (cfilename[0] == '.' && cfilename[1] == '/')
    {
	fullname = string (cfilename+2);
	fd = open (cfilename+2, O_RDONLY);
    }
    else
    {
	fullname = Y2PathSearch::findy2 ("include/" + filename);
	if (!fullname.empty ())
	{
	    fd = open (fullname.c_str (), O_RDONLY, 0644);
	}
    }

    if (fd < 0)
    {
	return YCPError ("Can't find YCP include component " + filename + ": " + strerror (errno));
    }

    // save file/line info

    string old_file = current_file;
    int old_line = current_line;

    current_file = fullname;
    current_line = 1;

    YCPParser parser (fd, fullname.c_str());
    parser.setBuffered(); // Read from file. Buffering is always possible here
    YCPValue ycpscript = parser.parse();
    close (fd);

    YCPValue result = YCPNull();

    if (ycpscript.isNull())
    {
	result = YCPError ("Invalid YCP include " + filename);
    }
    else
    {
	ycp2debug(old_file.c_str(),old_line, "Including YCP file %s", fullname.c_str());

	result = evaluate (ycpscript);
    }

    // restore file/line info

    current_file = old_file;
    current_line = old_line;

    return result;
}


YCPValue
WFMInterpreter::importModule (const string& modulename)
{
    /**
     * @builtin import "file" -> void
     *
     * @example import "module"
     *
     * In the example, WFM looks for the file YAST2HOME/modules/module.ycp and
     * loads it.
     * If the filename starts with "./", the include path is relative
     * to the current directory.
     *
     * textdomain is handled in libycp/YCPBlock.cc (import statement)
     *
     * Returns void if module already loaded.
     * Returns true if module loaded now.
     * Returns NULL if module unknown.
     *
     * See <a href="../libycp/modules.html">detailed module documentation.</a>
     */

    string fullname;
    const char *cmodulename = modulename.c_str();

    if (known_modules.find (modulename) != known_modules.end())
    {
	return YCPVoid();
    }

    y2debug ("WFMInterpreter::importModule (%s)", cmodulename);

    int fd = -1;

    //
    // check for leading "./"
    //

    const char *modpart = 0;
    if (cmodulename[0] == '.' && cmodulename[1] == '/')
    {
	modpart = strrchr (cmodulename, '/') + 1;
	if (known_modules.find (modpart) != known_modules.end())
	    return YCPVoid();

	fullname = string (cmodulename+2);
	fd = open (cmodulename+2, O_RDONLY);
	if (fd == -1)
	{
	    fullname = fullname + ".ycp";
	    cmodulename = fullname.c_str();
	    fd = open (cmodulename, O_RDONLY);
	}
    }
    else
    {
	fullname = Y2PathSearch::findy2 ("modules/" + modulename);
	if (fullname.empty())
	{
	    fullname = Y2PathSearch::findy2 ("modules/" + modulename + ".ycp");
	}
	if (!fullname.empty ())
	{
	    fd = open (fullname.c_str (), O_RDONLY, 0644);
	}
    }

    if (fd < 0)
    {
	return YCPError ("Can't find YCP module " + modulename + ": " + strerror (errno), YCPNull());
    }

    // save module/line info

    string old_file = current_file;
    int old_line = current_line;

    current_file = fullname;
    current_line = 1;

    YCPParser parser (fd, fullname.c_str());
    parser.setBuffered(); // Read from file. Buffering is always possible here
    YCPValue ycpscript = parser.parse();
    close (fd);

    YCPValue result = YCPNull();

    if (ycpscript.isNull())
    {
	result = YCPError ("Invalid YCP module " + modulename, YCPNull());
    }
    else
    {
	ycp2milestone (old_file.c_str(),old_line, "Loading YCP module %s", modulename.c_str());

	result = evaluate (ycpscript);

	// if (result.isNull()) ...

	known_modules.insert (modpart ? modpart : modulename);
    }

    // restore file/line info

    current_file = old_file;
    current_line = old_line;

    return YCPBoolean (true);
}


YCPValue
WFMInterpreter::evaluateWFM_UI (const YCPTerm& term) const
{
    /**
     * @builtin UI:: -> any
     * @builtin UI(any v) -> any
     * Send a command to the user interface. The value v is sent
     * to and evaluated by the user interface. The term UI(...)
     * itself evaluates to the value the is returned by the user
     * interface. Please note, that the argument expression that
     * you write into brackets of UI is prior to sending also
     * evaluated by the WFM, just like the arguments of any
     * term that is evaluated. So if you write <tt>UI(17 + 3)</tt>
     * the UI will actually get the value <tt>20</tt>. Or if you
     * write <tt>UI(ShowDialog(d))</tt>, the command <tt>ShowDialog</tt>
     * will be evaluated by the WFM, which in this case results in
     * an error message (WFM doesn't know anything about ShowDialog()).
     *
     * Use deep quoting to prohibit evaluation of a complete term.
     * If you write <tt>UI(``(17+3))</tt>
     * the UI will get the term 17+3 and evaluate that.
     *
     * Use the simple quote symbol, if you just want to send <i>one term</i>
     * to the UI and want that term evaluated by the UI, but the arguments
     * by the WFM. <tt>UI(`ShowDialog(d))</tt> first looks up the variable
     * <tt>d</tt> <i>in the WFM</i>, but the <tt>ShowDialog</tt> is evaluated
     * by the UI. <tt>UI(``ShowDialog(d))</tt> evaluates both the variable <tt>d</tt>
     * and the <tt>ShowDialog</tt> by the UI.
     *
     * The UI call removes the single quote from the outer most term before
     * sending the expression to the UI.
     *
     * @example UI(`QueryWidget(dlg, `Id("language")))
     * @example UI::QueryWidget(dlg, `Id("language"))
     */

    if (term->size() == 1)
    {
	// send fullname to ui interpreter
	YCPBlock block;
	YCPBuiltinStatement statement (0, YCPB_FULLNAME, YCPString (current_file));
	block->add (statement);
	sendUnquoted (user_interface, block);

	return sendUnquoted (user_interface, term->value(0));
    }

    return YCPNull();
}


YCPValue
WFMInterpreter::evaluateWFM_SCR (const YCPTerm &term)
{
    /**
     * @builtin SCR::<any v> -> any
     * Send a command to the SCR (system configuration repository).
     * The value v is sent
     * to and evaluated by the scr. The term SCR::...
     * itself evaluates to the value the is returned by the user
     * interface.
     */

    if (term->size() == 1)
    {
	WFMSubAgents::iterator scr = find_handle (default_handle);
	if (scr == scrs.end ())
	    return YCPError ("Unknown SCR handle.\n");

	if (!(*scr)->start ())
	    return YCPVoid ();

	// send fullname to scr interpreter
	YCPBlock block;
	YCPBuiltinStatement statement (0, YCPB_FULLNAME, YCPString (current_file));
	block->add (statement);
	sendUnquoted ((*scr)->comp (), block);

	return sendUnquoted ((*scr)->comp (), term->value(0));
    }

    return YCPNull();
}


YCPValue
WFMInterpreter::evaluateSCROpen (const YCPTerm& term)
{
    /**
     * @builtin SCROpen (string name, bool check_version) -> integer
     * Create a new scr instance. The name must be a valid y2component name
     * (e.g. "scr", "chroot=/mnt:scr"). The component is created immediately.
     * The parameter check_version determined whether the SuSE Version should
     * be checked. On error a negative value is returned.
     */

    if (term->size () == 2 && term->value (0)->isString () &&
	term->value (1)->isBoolean ())
    {
	string name = term->value (0)->asString ()->value ();
	bool check_version = term->value (1)->asBoolean ()->value ();

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

    return YCPError ("Bad args for SCROpen()");
}


YCPValue
WFMInterpreter::evaluateSCRClose (const YCPTerm& term)
{
    /**
     * @builtin SCRClose (integer handle) -> void
     * Close a scr instance.
     */

    if (term->size () == 1 && term->value (0)->isInteger ())
    {
	int handle = term->value (0)->asInteger ()->value ();
	WFMSubAgents::iterator it = find_handle (handle);
	delete *it;
	scrs.erase (it);
	return YCPVoid ();
    }

    return YCPError ("No handle given to SCRClose");
}


YCPValue
WFMInterpreter::evaluateSCRGetName (const YCPTerm &term)
{
    /**
     * @builtin SCRGetName (integer handle) -> string
     * Get the name of a scr instance.
     */

    if (term->size () == 1 && term->value (0)->isInteger ())
    {
	int handle = term->value (0)->asInteger ()->value ();
	WFMSubAgents::iterator it = find_handle (handle);
	return YCPString (it != scrs.end () ? (*it)->get_name () : "");
    }

    return YCPError ("No handle for SCRGetName()");
}


YCPValue
WFMInterpreter::evaluateSCRSetDefault (const YCPTerm &term)
{
    /**
     * @builtin SCRSetDefault (integer handle) -> void
     * Set's the default scr instance.
     */

    if (term->size () == 1 && term->value (0)->isInteger ())
    {
	default_handle = term->value (0)->asInteger ()->value ();
	return YCPVoid ();
    }

    return YCPError ("No handle for SCRSetDefault()");
}


YCPValue
WFMInterpreter::evaluateSCRGetDefault (const YCPTerm &term)
{
    /**
     * @builtin SCRGetDefault () -> void
     * Get's the default scr instance.
     */

    if (term->size () == 0)
    {
	return YCPInteger (default_handle);
    }

    return YCPError ("No handle fro SCRGetDefault()");
}


YCPValue
WFMInterpreter::evaluateCallModule (const YCPTerm& term)
{
    /**
     * @builtin CallModule(string module, list arguments) -> any
     * Call a submodule. The submodule will take over the interaction
     * with the userinterface. The <tt>module</tt> specifies the name
     * of the module to call and <tt>arguments</tt> are the arguments
     * to the module call. The term's symbol is "CallModule".
     * The function evaluates the value returned by the module called.
     *
     * @example CallModule("inst_lilo", [true, false, user_settings])
     */

    if ((term->size() == 2 && term->value(0)->isString() && term->value(1)->isList()))
    {
	// Create component

	string modulename = term->value(0)->asString()->value();

	// save textdomain and file/line around CallModule()

	const string old_textdomain = getTextdomain ();
	string old_file = current_file;
	int old_line = current_line;

	ycp2milestone (old_file.c_str(), old_line,
		       "Calling YCP module %s (%s)", modulename.c_str(),
		       term->value(1)->asList()->toString().c_str());

	YCPValue result = wfm_component->callModule (modulename, term->value(1)->asList(),
						     user_interface);
	if (result.isNull())
	{
	    result = YCPError ("Can't find module " + modulename, YCPNull());
	}
	else
	{
	    y2debug ("... (%s)", result->toString().c_str());
	}

	// restore all

	current_line = old_line;
	current_file = old_file;
	setTextdomain (old_textdomain);

	return result;
    }
    else return YCPError("Bad parameters for CallModule()");
}


YCPValue
WFMInterpreter::evaluateCallFunction (const YCPTerm& term)
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

    YCPTerm argterm("", false);

    if (term->size() == 1)
    {
	if (term->value(0)->isTerm())
	{
	    argterm = term->value(0)->asTerm();
	}
	else if (term->value(0)->isString())
	{
	    argterm = YCPTerm (term->value(0)->asString()->value(), true);
	}
	else
	{
	    return YCPError ("Argument not string or term");
	}
    }
    else if (term->size() == 2
	     && term->value(0)->isString()
	     && term->value(1)->isList())
    {
	argterm = YCPTerm (YCPSymbol (term->value(0)->asString()->value(), true), term->value(1)->asList());
    }
    else
    {
	return YCPError("Bad parameters for CallFunction()");
    }

    string new_modulename = argterm->symbol()->symbol();
    string filename = "clients/" + new_modulename + ".ycp";
    string fullname = Y2PathSearch::findy2 (filename);

    int fd = -1;
    if (!fullname.empty ())
    {
	fd = open (fullname.c_str (), O_RDONLY, 0644);
    }

    if (fd < 0)
    {
	return YCPError ("Can't find YCP client component " + filename + ": " + strerror (errno));
    }

    YCPValue result = YCPVoid();

    string old_file = current_file;
    int old_line = current_line;

    current_file = fullname;
    current_line = 1;

    YCPParser parser(fd, fullname.c_str());
    parser.setBuffered(); // Read from file. Buffering is always possible here
    YCPValue ycpscript = parser.parse();
    close(fd);

    if (ycpscript.isNull())
    {
	    result = YCPError ("Invalid YCP component " + fullname);
    }
    else
    {
	ycp2milestone (old_file.c_str(), old_line,
		       "Calling YCP function module %s", filename.c_str());

	// save textdomain around CallFunction()

	string old_textdomain = getTextdomain ();

	// I just evaluate the value ycpscript. But I have to make
	// sure, that it has access to it's arguments given in argterm.
	// If temporarily overwrite the member variable argumentlist.

	YCPList old_argumentlist = this->argumentlist;
	this->argumentlist = argterm->args(); // YYYYYYYYYYY switch CallFunction from term to list

	result = evaluate(ycpscript);
	argumentlist = old_argumentlist;
	setTextdomain (old_textdomain);
    }

    current_file = old_file;
    current_line = old_line;

    return result;
}

YCPValue WFMInterpreter::evaluateGetClientName(const YCPTerm& term)
{
    /**
     * @builtin GetClientName(integer filedescriptor) -> string
     * This builtin read the clientname from a pipe
     */

    if (term->size() == 1 && term->value(0)->isInteger())
    {
	YCPList arglist = argList();
	long long index = term->value(0)->asInteger()->value();
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

    return YCPError ("Bad args for GetClientName()");
}



YCPValue
WFMInterpreter::sendUnquoted (Y2Component *server, const YCPValue& value) const
{
    if (value->isSymbol() && value->asSymbol()->isQuoted())
    {
	return server->evaluate(YCPSymbol(value->asSymbol()->symbol(), false));
    }
    else if (value->isTerm() && value->asTerm()->isQuoted())
    {
	YCPTerm qt = value->asTerm();
	YCPTerm t(YCPSymbol(qt->symbol()->symbol(), false));
	for (int i=0; i<qt->size(); i++)
	{
	    t->add(qt->value(i));
	}
	return server->evaluate(t);
    }

    return server->evaluate(value);
}


YCPValue
WFMInterpreter::evaluateRead (const YCPTerm &term)
{
    /**
     * @builtin Read (path, [any]) -> any
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
	return YCPVoid ();

    if (term->size () >= 1 && term->value (0)->isPath ())
    {
	YCPPath path = term->value (0)->asPath ();

	// strip leading .local from path

	if (path->length () < 2)
	    return YCPError ("Too short path for WFM::"+term->toString());

	if (path->component_str (0) != "local")
	    return YCPError ("Path not '.local' in WFM::"+term->toString());

	path = path->at (1);


	switch (term->size ())
	{
	    case 1:
		return local.agent ()->Read (path);

	    case 2:
		return local.agent ()->Read (path, term->value (1));
	}
    }

    return YCPError ("Bad arguments for WFM::" + term->toString());
}


YCPValue
WFMInterpreter::evaluateWrite (const YCPTerm &term)
{
    /**
     * @builtin Write (path, any, [any]) -> boolean
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
	return YCPVoid ();

    if (term->size () >= 1 && term->value (0)->isPath ())
    {
	YCPPath path = term->value (0)->asPath ();

	// strip leading .local from path

	if (path->length () < 2)
	    return YCPError ("Too short path for WFM::"+term->toString());

	if (path->component_str (0) != "local")
	    return YCPError ("Path not '.local' in WFM::"+term->toString());

	path = path->at (1);

	switch (term->size ())
	{
	    case 2:
		return local.agent ()->Write (path, term->value (1));

	    case 3:
		return local.agent ()->Write (path, term->value (1), term->value (2));
	}
    }

    return YCPError ("Bad arguments for WFM::" + term->toString());
}


YCPValue
WFMInterpreter::evaluateExecute (const YCPTerm &term)
{
    /**
     * @builtin Execute (path, [any, [any]]) -> any
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
	return YCPVoid ();

    if (term->size () >= 1 && term->value (0)->isPath ())
    {
	YCPPath path = term->value (0)->asPath ();

	// strip leading .local from path

	if (path->length () < 2)
	    return YCPError ("Too short path for WFM::"+term->toString());

	if (path->component_str (0) != "local")
	    return YCPError ("Path not '.local' in WFM::"+term->toString());

	path = path->at (1);

	switch (term->size ())
	{
	    case 1:
		return local.agent ()->Execute (path);

	    case 2:
		return local.agent ()->Execute (path, term->value (1));

	    case 3:
		return local.agent ()->Execute (path, term->value (1), term->value (2));
	}
    }

    return YCPError ("Bad arguments for WFM::" + term->toString());
}


YCPValue
WFMInterpreter::evaluateDir (const YCPTerm &term)
{
    /**
     * @builtin Dir (path) -> any
     * Special interface to the system agent. Not for general use.
     */

    if (!local.start ())
	return YCPVoid ();

    if (term->size () >= 1 && term->value (0)->isPath ())
    {
	YCPPath path = term->value (0)->asPath ();

	// strip leading .local from path

	if (path->length () < 2)
	    return YCPError ("Too short path for WFM::"+term->toString());

	if (path->component_str (0) != "local")
	    return YCPError ("Path not '.local' in WFM::"+term->toString());

	path = path->at (1);

	switch (term->size ())
	{
	    case 1:
		return local.agent ()->Dir (path);
	}
    }

    return YCPError ("Bad arguments for WFM::" + term->toString());
}


YCPValue
WFMInterpreter::evaluateArgs (const YCPTerm &term) const
{
    /**
     * @builtin Args() -> list
     * Returns the arguments with which the module was called.
     * It is a list whose
     * arguments are the module's arguments. If the module
     * was called with <tt>CallModule("my_mod",&nbsp;[17,&nbsp;true])</tt>,
     * &nbsp;<tt>Args()</tt> will return <tt>[ 17, true ]</tt>.
     */
    if (term->size() == 0)
    {
	return argList();
    }
    /**
     * @builtin Args(integer n) -> any
     * Returns the n'th of the arguments with that the module was called.
     * If the module
     * was called with <tt>CallModule("my_mod",&nbsp;[17,&nbsp;true])</tt>,
     * &nbsp;<tt>Args(0)</tt> call will return <tt>17</tt>.
     */
    else if (term->size() == 1 && term->value(0)->isInteger())
    {
	YCPList arglist = argList();
	long long index = term->value(0)->asInteger()->value();
	if (index < 0)
	{
	    return YCPError ("Invalid negative index to Args(). Only values >= 0 are allowed");
	}
	else if (index < arglist->size())
	{
	    return arglist->value(index);
	}
	else
	{
	    return YCPError ("Index to Args() larger than number of available arguments.");
	}
    }

    return YCPNull();
}


YCPList
WFMInterpreter::argList () const
{
    return argumentlist;
}


string
WFMInterpreter::moduleName () const
{
    return modulename;
}


YCPValue
WFMInterpreter::setModuleName (const string& modulename)
{
    ycp2debug (current_file.c_str(), current_line, "setModuleName (%s)",
	       modulename.c_str());
    return YCPNull();
}


YCPValue
WFMInterpreter::evaluateGetLanguage (const YCPTerm& term)
{
    /**
     * @builtin GetLanguage() -> string
     * Returns the current language code (without modifiers !)
     */
    return YCPString(currentLanguage);
}


YCPValue
WFMInterpreter::evaluateGetEncoding (const YCPTerm& term)
{
    /**
     * @builtin GetEncoding() -> string
     * Returns the current encoding code
     */
    return YCPString(currentEncoding);
}


YCPValue
WFMInterpreter::evaluateSetLanguage (const YCPTerm& term)
{
    /**
     * @builtin SetLanguage("de_DE" [, encoding]) -> void
     * Selects the language for translate()
     * @example SetLanguage("de_DE", "ISO-8859-1") -> nil
     * if the encoding isn't specified, it's set to nl_langinfo (CODESET)
     */

    if (term->size() > 0 && term->value(0)->isString())
    {
	currentLanguage = term->value(0)->asString()->value();

	if (term->size() > 1 && term->value(1)->isString())
	{
	    currentEncoding = term->value(1)->asString()->value();
	}
	else
	{
	    setlocale (LC_ALL, currentLanguage.c_str());	// prepare for nl_langinfo
	    currentEncoding = nl_langinfo (CODESET);
	    if (currentEncoding.empty())
	    {
		y2warning ("nl_langinfo returns empty encoding for %s", currentLanguage.c_str());
		currentEncoding = "UTF-8";	// default encoding
	    }
	}

	textdomainOrLanguageHasChanged = true;

	ycp2debug (current_file.c_str (), current_line,
		       "WFM SetLanguage(\"%s\"), Encoding(\"%s\")",
		       currentLanguage.c_str(), currentEncoding.c_str());
	return YCPVoid();
    }

    return YCPError ("Bad WFM::SetLanguage ("+term->toString()+")", YCPNull());
}
