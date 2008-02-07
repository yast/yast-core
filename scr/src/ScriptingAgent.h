// -*- c++ -*-

/*
 *  Authors:	Arvin Schnell <arvin@suse.de>
 *		Klaus Kaempf <kkaempf@suse.de>
 *		Stanislav Visnovsky <visnov@suse.cz>
 *  Maintainer: Arvin Schnell <arvin@suse.de>
 */


#ifndef ScriptingAgent_h
#define ScriptingAgent_h

#include <time.h>
#include <y2/Y2Component.h>
#include <scr/SCRAgent.h>
#include "SCRSubAgent.h"

/**
 * The main agant that dispatches calls to other agents.
 */
class ScriptingAgent : public SCRAgent
{

public:

    /**
     * Constructor.
     */
    ScriptingAgent ();

    // used only in agent-ini/testsuite...?
    // TODO try to eliminate it
    /**
     * Constructor. Load only a single SCR.
     * 
     * @param file	SCR configuration file to be registered.
     */
    ScriptingAgent (const string& file);

    /**
     * Destructor. Also deletes subagents.
     */
    ~ScriptingAgent ();

    /**
     * Reads data.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    virtual YCPValue Read (const YCPPath &path, const YCPValue &arg = YCPNull (), const YCPValue &opt = YCPNull ());

    /**
     * Writes data.
     */
    virtual YCPBoolean Write (const YCPPath &path, const YCPValue &value,
		    const YCPValue &arg = YCPNull ());

    /**
     * Get a list of all subtrees.
     */
    virtual YCPList Dir (const YCPPath &path);

    /**
     * Executes a command.
     */
    virtual YCPValue Execute (const YCPPath &path, const YCPValue &value =
		      YCPNull (), const YCPValue &arg = YCPNull ());

    /**
     * Get a detailed error description if a previous command failed
     */
    virtual YCPMap Error (const YCPPath &path);

    /**
     * Handle the commands
     * MountAgent, MountAllAgents, UnmountAllAgents,
     * YaST2Version, SuSEVersion.
     * Formerly also
     * 'UnregisterAgent', 'UnregisterAllAgents',
     * 'UnmountAgent' which are now builtins.
     */
    YCPValue otherCommand (const YCPTerm &term);

    /**
     * Register a agent, that is specify the scr path and the filename for
     * it's definition or the term with the definition. The preferred way
     * is to specify the filename.
     */
    virtual YCPBoolean RegisterAgent (const YCPPath &path, const YCPValue &value);

    /**
     * Unregister a agent.
     */
    virtual YCPBoolean UnregisterAgent (const YCPPath &path);

    /**
     * Unregister all agents.
     */
    virtual YCPBoolean UnregisterAllAgents ();

    /**
     * Unmount the agent handling path.
     */
    virtual YCPBoolean UnmountAgent (const YCPPath &path);

private:

    // once we have to do a sweep (read all scr files because of
    // a Dir or we were not lucky with a path patch), set this flag so
    // that we do not unnecessarily sweep again
    bool done_sweep;

    // FIXME rethink the caching
    struct RegistrationDir {
	string name;
	time_t last_changed; //!< st_mtime of the dir
    };

    /**
     * Where to look for *.scr files, in order of preference
     */
    list<RegistrationDir> registration_dirs;

    /**
     * Populate registration_dirs
     */
    void InitRegDirs ();

    /**
     * Type and list of subagents
     * The vector is sorted by path
     */
    typedef vector<SCRSubAgent*> SubAgents;
    SubAgents agents;


    /**
     * Mount the agent handling path. This function is called
     * automatically when the agent is used.
     */
    YCPValue MountAgent (const YCPPath &path);

    /**
     * Mount all agents.
     */
    YCPValue MountAllAgents ();

    /**
     * Unmount all agents.
     */
    YCPValue UnmountAllAgents ();

    /**
     * Read all registration files.
     */
    void Sweep ();

    /**
     * Register new agents. (bnc#245508#c16)
     * Rescan the scrconf registration directories and register any
     * agents at new(!) paths. Agents, even new ones, on paths that
     * are registered already, will not be replaced.  This means that
     * .oes.specific.agent will start to work but something like
     * adding
     * /usr/local/etc/sysconfig to .sysconfig.network would not.
     */
    YCPBoolean RegisterNewAgents ();

    /**
     * For .foo.bar.baz, register foo.bar.baz.scr, or foo.bar.scr, or foo.scr.
     * BTW we can register an unrelated path because this is just a heuristic.
     */
    void tryRegister (const YCPPath &path);

    /**
     * Iterate thru @ref agents
     * @return end if not found
     */
    SubAgents::const_iterator findSubagent (const YCPPath &path);

    /**
     * Find it in @ref agents, registering if necessary, sweeping if necessary
     * @see tryRegister
     * @see Sweep
     */
    SubAgents::const_iterator findAndRegisterSubagent (const YCPPath &path);

    /**
     * If a SCR::Dir falls inside our tree, we have to provide a listing
     */
    YCPList dirSubagents (const YCPPath &path);

    /**
     * Calls a subagent to execute a Read, Write, Dir or other command
     * @param command the command like "Read", "Dir", ..
     * @param path All commands take a path as first parameter. Here you
     * give an absolute path, for example .etc.liloconf.global. When calling
     * the agent, this path will be made relative to the agents root. E.g. If the
     * agent's root path would be .etc.liloconf and the command "Read",
     * the resulting command would be Read (.global).
     * @param optional value to be given as second parameter to the call
     */
    YCPValue executeSubagentCommand (const char *command,
				     const YCPPath &path,
				     const YCPValue &arg = YCPNull (),
				     const YCPValue &optpar = YCPNull ());

    /**
     * Find agent exactly matching path. Returns agents.end () if the path
     * isn't covered by any agent.
     * Does not try to register.
     */
    SubAgents::iterator findByPath (const YCPPath &path);

    /**
     * Parses all SCR configuration files in the given directory,
     * registers the agents.
     * (If a SCR path is already registered, keep the old one.)
     */
    void parseConfigFiles (const string &directory);

    /**
     * Parses a single SCR configuration file,  registers the agent.
     * (If the SCR path is already registered, keep the old one.)
     */
    void parseSingleConfigFile (const string &file);

};


#endif // ScriptingAgent_h
