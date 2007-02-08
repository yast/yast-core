// -*- c++ -*-

/*
 *  Authors:	Arvin Schnell <arvin@suse.de>
 *		Klaus Kaempf <kkaempf@suse.de>
 *		Stanislav Visnovsky <visnov@suse.cz>
 *  Maintainer: Arvin Schnell <arvin@suse.de>
 */


#ifndef ScriptingAgent_h
#define ScriptingAgent_h

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
     * Constructor. Also scans for scr-files.
     */
    ScriptingAgent ();

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
     * Reads data. Destroy the result after use.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    virtual YCPValue Read (const YCPPath &path, const YCPValue &arg = YCPNull (), const YCPValue &opt = YCPNull ());

    /**
     * Writes data. Destroy the result after use.
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
     * Handle the commands 'UnregisterAgent',
     * 'UnregisterAllAgents', 'MountAgent', 'MountAllAgents',
     * 'UnmountAgent' and 'UnmountAllAgents'.
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

    /**
     * Type and list of subagents
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
     */
    SubAgents::iterator findByPath (const YCPPath &path);

    /**
     * Parses the given directory and all its subdirectories for
     * SCR configuration files and evaluates them with the SCR
     * interpreter.
     */
    void parseConfigFiles (const string &directory);

    /**
     * Parses a single SCR configuration file and evaluates them with the SCR
     * interpreter.
     */
    void parseSingleConfigFile (const string &file);

};


#endif // ScriptingAgent_h
