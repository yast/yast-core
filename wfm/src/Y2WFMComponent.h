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

   Author:	Stanislav Visnovsky <visnov@suse.cz>
   Maintainer:	Stanislav Visnovsky <visnov@suse.cz>

/-*/

#ifndef Y2WFMComponent_h
#define Y2WFMComponent_h

#include <y2/Y2Component.h>

#include <ycp/YCPInteger.h>
#include <ycp/YCPList.h>
#include <ycp/YCPString.h>

#include "WFMSubAgent.h"

#define MAX_CLIENT_NAME_LEN 160

class Y2SystemNamespace;

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

    static Y2WFMComponent* instance();

    YCPInteger SCROpen (const YCPString& name, const YCPBoolean &check_version);
    void SCRClose (const YCPInteger& handle);
    YCPString SCRGetName (const YCPInteger &handle);
    void SCRSetDefault (const YCPInteger &handle);
    YCPInteger SCRGetDefault () const;
    YCPValue Args (const YCPInteger& index = YCPNull ()) const;
    YCPList  SetArgs (const YCPList& new_args);
    YCPString GetLanguage () const;
    YCPString GetEncoding () const;
    YCPString SetLanguage (const YCPString& language, const YCPString& encoding = YCPNull ());
    YCPValue Read (const YCPPath &path, const YCPValue& arg);
    YCPValue Write (const YCPPath &path, const YCPValue& arg1, const YCPValue& arg2 = YCPNull ());
    YCPValue Execute (const YCPPath &path, const YCPValue& arg1);
    YCPValue CallFunction (const YCPString& client, const YCPList& args = YCPList ());
    YCPString GetEnvironmentEncoding ();
    YCPBoolean ClientExists (const YCPString& client);

    virtual Y2Namespace* import (const char* name_space);

   /**
     * Setups this script component.
     * @param the name of the component that is realized be the script.
     * @param script the script. This component clones it, so you can
     * destroy the script after the constructor call.
     */
    void setupComponent (string client_name, string fullname,
                       const YCPValue& script);

private:

    bool createDefaultSCR ();

    /**
     * Type and list of SCR instances.
     */
    typedef vector <WFMSubAgent*> WFMSubAgents;
    typedef vector <Y2SystemNamespace*> SystemNamespaces;
    
    WFMSubAgents scrs;
    SystemNamespaces system_namespaces;

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

    string          currentLanguage;
    string          currentEncoding;

    /**
     * system encoding
     */
    string          systemEncoding;

   /**
     * environment encoding
     */
    string          environmentEncoding;


    static Y2WFMComponent* current_wfm;
    
       /**
     * The script that implements the component.
     */
    YCPValue script;

    /**
     * The name of the client that is implemented by the script.
     */
    string client_name;

    /**
     * The fullname of the script file.
     */
    string fullname;
};


#endif // Y2WFMComponent_h
