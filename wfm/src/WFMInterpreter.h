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

   File:	WFMInterpreter.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef WFMInterpreter_h
#define WFMInterpreter_h

#include <stdio.h>
#include <set>

using std::set;

#include <ycp/YCPInterpreter.h>
#include <y2/Y2Component.h>
#include "WFMSubAgent.h"
#include "PkgModule.h"

class PerlModule;


#define MAX_CLIENT_NAME_LEN 160

class WFMInterpreter : public YCPInterpreter
{
    /**
     * Handle to my own component (Y2WFMComponent)
     */
    Y2Component *wfm_component;

    /**
     * Handle to the user interface
     */
    Y2Component *user_interface;

    /**
     * Type and list of SCR instances.
     */
    typedef vector <WFMSubAgent*> WFMSubAgents;
    WFMSubAgents scrs;

    /**
     * Finds a SCR instance to a given handle.
     */
    WFMSubAgents::iterator find_handle (int);

    /**
     * Handle count.
     */
    int handle_cnt;

    /**
     * Handle of default SCR instance.
     */
    int default_handle;

    /**
     * The local system agent.
     */
    WFMSubAgent local;

    /**
     * The name of the module that is realized by this wfm.
     */
    string modulename;

    /**
     * Arguments of the module that is realized through
     * the wfm. The script has access to it via the builtin
     * args(). The symbol of the term itself is the module name.
     */
    YCPList argumentlist;

    /**
     * Pointer to PkgModule class to handle "Pkg::<function>(...)" calls
     */
    PkgModule *pkgmodule;

    /**
     * Pointer to PerlModule class to handle "Perl::<function>(...)" calls
     */
    PerlModule *perlmodule;

    /**
     * This overridden function does only evaluate YCPB_NLOCALE, otherwise
     * calls YCPInterpreter::evaluateBuiltinBuiltin.
     */
    YCPValue evaluateBuiltinBuiltin (builtin_t code, const YCPList& args);

public:
    /**
     * Creates a new WFMInterpreter.
     * @param my_component A pointer to my own Y2Component, used
     *  by CallModule for "Y2Component::callModule"
     * @param user_interface A handle to the user interface.
     * @param argterm The arguments for the module that is realized
     * with this WFM.
     * @param parent Pointer to the parent interpreter if this is
     * a subinterpreter.
     */
    WFMInterpreter (Y2Component *my_component,
		    Y2Component *user_interface,
		    string modulename,
		    string fullname, const YCPList& arglist);

    /**
     * cleanup
     */
    ~WFMInterpreter();

    /**
     * Name of interpreter, returns "wfm".
     */
    string interpreter_name () const;

protected:

    YCPValue evaluateInstantiatedTerm(const YCPTerm& term);
    YCPValue callback(const YCPValue& value);
    YCPValue evaluateUI(const YCPValue& value);
    YCPValue evaluateWFM(const YCPValue& value);
    YCPValue evaluateSCR(const YCPValue& value);

    YCPValue setModuleName (const string& modulename);
    YCPValue setTextdomain (const string& textdomain);
    string getTextdomain (void);
    YCPValue includeFile (const string& filename);
    YCPValue importModule (const string& modulename);

private:

    /**
     * Get the language from the environment.
     */
    const char* get_env_lang () const;

    /**
     * translator data
     */
    string 	    currentTextdomain;
    string 	    currentLanguage;
    mutable bool    textdomainOrLanguageHasChanged;

    /**
     * system encoding
     */
    string 	    systemEncoding;

   /**
     * environment encoding
     */
    string	    environmentEncoding;

    /**
     * Implements the builtin UI
     */
    YCPValue evaluateWFM_UI(const YCPTerm& t) const;

    /**
     * Implements the builtin SCR
     */
    YCPValue evaluateWFM_SCR(const YCPTerm& t);

    /**
     * Opens a new SCR instance. Returns the handle.
     */
    YCPValue evaluateSCROpen (const YCPTerm &);

    /**
     * Closes a SCR instance. Invalidates the handle.
     */
    YCPValue evaluateSCRClose (const YCPTerm &);

    /**
     * Return the name of an SCR instance.
     */
    YCPValue evaluateSCRGetName (const YCPTerm &);

    /**
     * Sets the default SCR instance.
     */
    YCPValue evaluateSCRSetDefault (const YCPTerm &);

    /**
     * Returns the default SCR instance.
     */
    YCPValue evaluateSCRGetDefault (const YCPTerm &);

    /**
     * Implements the builtin CallModule
     */
    YCPValue evaluateCallModule(const YCPTerm&);

    /**
     * Implements the builtin CallFunction
     */
    YCPValue evaluateCallFunction(const YCPTerm&);

    /**
     * Implements the builtin CallFile
     */
    YCPValue evaluateCallFile(const YCPTerm&);

    /**
     * Implements the builtin Read (path, arg).
     */
    YCPValue evaluateRead (const YCPTerm&);

    /**
     * Implements the builtin Write (path, value, arg).
     */
    YCPValue evaluateWrite (const YCPTerm&);

    /**
     * Implements the builtin Execute (path, value, arg).
     */
    YCPValue evaluateExecute (const YCPTerm&);

    /**
     * Implements the builtin Dir (path).
     */
    YCPValue evaluateDir (const YCPTerm&);

    /**
     * Implements the builtins Args() and Args(integer)
     */
    YCPValue evaluateArgs(const YCPTerm&) const;

    /**
     * switch to new gettext environment if currentLanguage
     * or currentTextdomain has changed (textdomainOrLanguageHasChanged == true)
     */
    void changeToModuleLanguage() const;

    /**
     * Implements the translation functionality
     */
    YCPValue evaluateLocale(const YCPLocale&);

    /**
     * Implements the builtin SetLanguage
     */
    YCPValue evaluateSetLanguage(const YCPTerm&);

    /**
     * Implements the builtin GetLanguage
     */
    YCPValue evaluateGetLanguage(const YCPTerm&);

    /**
     * Implements the builtin GetEncoding
     */
    YCPValue evaluateGetEncoding(const YCPTerm&);

    /**
     * Implements the builtin GetEnvironmentEncoding
     */
    YCPValue evaluateGetEnvironmentEncoding(const YCPTerm&);

    /**
     * Sends a YCP value to a server component and gets the answer.
     * If the value is a quoted symbol or a quoted term, then
     * it is unquoted before sending.
     */
    YCPValue sendUnquoted(Y2Component *server, const YCPValue& value) const;

    /**
     * Returns the name of the current module
     */
    string moduleName() const;

    /**
     * Returns the argument list, i.e. the arguments with that this
     * module has been started. It is stored in the parent of all WFMInterpreters.
     */
    YCPList argList() const;

    /**
     * set of currently known (loaded) modules
     */
    typedef set <string> stringSet;
    stringSet known_modules;
};


#endif // WFMInterpreter_h
