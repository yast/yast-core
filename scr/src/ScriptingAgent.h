// -*- c++ -*-

/*
 *  Authors:	Arvin Schnell <arvin@suse.de>
 *		Klaus Kaempf <kkaempf@suse.de>
 *  Maintainer: Arvin Schnell <arvin@suse.de>
 */


#ifndef ScriptingAgent_h
#define ScriptingAgent_h

#include <y2/Y2Component.h>
#include <scr/SCRAgent.h>
#include "SCRSubAgent.h"


class ScriptingAgent : public SCRAgent
{

public:

    /**
     * Constructor. Also scans for scr-files.
     */
    ScriptingAgent ();

    /**
     * Destructor. Also deletes subagents.
     */
    ~ScriptingAgent ();

    /**
     * Reads data. Destroy the result after use.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    YCPValue Read (const YCPPath &path, const YCPValue &arg = YCPNull ());

    /**
     * Writes data. Destroy the result after use.
     */
    YCPValue Write (const YCPPath &path, const YCPValue &value,
		    const YCPValue &arg = YCPNull ());

    /**
     * Get a list of all subtrees.
     */
    YCPValue Dir (const YCPPath &path);

    /**
     * Executes a command.
     */
    YCPValue Execute (const YCPPath &path, const YCPValue &value =
		      YCPNull (), const YCPValue &arg = YCPNull ());

    /**
     * Handle the commands 'RegisterAgent', 'UnregisterAgent',
     * 'UnregisterAllAgents', 'MountAgent', 'MountAllAgents',
     * 'UnmountAgent' and 'UnmountAllAgents'.
     */
    YCPValue otherCommand (const YCPTerm &term);

    /**
     * Register a agent, that is specify the scr path and the filename for
     * it's definition or the term with the definition. The preferred way
     * is to specify the filename.
     */
    YCPValue RegisterAgent (const YCPPath &path, const YCPValue &value);

private:

    /**
     * Type and list of subagents
     */
    typedef vector<SCRSubAgent*> SubAgents;
    SubAgents agents;

    /**
     * Unregister a agent.
     */
    YCPValue UnregisterAgent (const YCPPath &path);

    /**
     * Unregister all agents.
     */
    YCPValue UnregisterAllAgents ();

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
     * Unmount the agent handling path.
     */
    YCPValue UnmountAgent (const YCPPath &path);

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

};


#endif // ScriptingAgent_h
