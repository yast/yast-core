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

   File:	Y2WFMComponent.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Mathias Kettner <kettner@suse.de>

/-*/

#ifndef Y2WFMComponent_h
#define Y2WFMComponent_h

#include <y2/Y2Component.h>
#include <WFMSubAgent.h>

class PkgModule;

#define MAX_CLIENT_NAME_LEN 160

class Y2WFMComponent : public Y2Component
{

public:
    /**
     * Creates a new WFM component
     */
    Y2WFMComponent();

    /**
     * Cleans up
     */
    ~Y2WFMComponent();

    /**
     * Returns "wfm";
     */
    virtual string name() const;

    /**
     * Executes the YCP script.
     */
    virtual YCPValue doActualWork(const YCPList& arglist, Y2Component *displayserver);

    /**
     * callback entry point
     *   usually calls back into Y2WFMInterpreter::evaluate
     *   We're not using a pointer here because the evaluate() slot
     *   already exists in the Y2Component class
     */
    virtual YCPValue evaluate(const YCPValue& command);
    
    static Y2WFMComponent* instance() { return current_wfm; }

    YCPInteger SCROpen (const YCPString& name, const YCPBoolean &check_version);
    void SCRClose (const YCPInteger& handle);
    YCPString SCRGetName (const YCPInteger &handle);
    void SCRSetDefault (const YCPInteger &handle);
    YCPInteger SCRGetDefault () const;
    YCPValue GetClientName(const YCPInteger& filedescriptor);
    YCPValue Args (const YCPInteger& index = YCPNull ()) const;
    YCPString GetLanguage () const;
    YCPString GetEncoding () const;
    YCPString SetLanguage (const YCPString& language, const YCPString& encoding = YCPNull ());
    YCPValue Read (const YCPPath &path, const YCPValue& arg);
    YCPValue Write (const YCPPath &path, const YCPValue& arg1, const YCPValue& arg2 = YCPNull ());
    YCPValue Execute (const YCPPath &path, const YCPValue& arg1);
    YCPValue CallFunction (const YCPString& client, const YCPList& args = YCPList ());
    
    virtual Y2Namespace* import (const char* name_space, const char* timestamp = NULL);
private:
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
     * Get the language from the environment.
     */
    const char* get_env_lang () const;

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

    string          currentLanguage;
    string          currentEncoding;

    static Y2WFMComponent* current_wfm;
};


#endif // Y2WFMComponent_h
