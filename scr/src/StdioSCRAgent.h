// -*- c++ -*-

/*
 *  Authors:	Stanislav Visnovsky <visnov@suse.cz>
 *  Maintainer: Arvin Schnell <arvin@suse.de>
 */


#ifndef StdioSCRAgent_h
#define StdioSCRAgent_h

#include <y2/Y2Component.h>
#include <scr/SCRAgent.h>


/**
 * This agent is a proxy, which transforms a direct builtin call into 
 * Y2Component::evaluate () call.
 */
class StdioSCRAgent : public SCRAgent
{

public:

    StdioSCRAgent (Y2Component* handler) : m_handler (handler) {}

    ~StdioSCRAgent () {}

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

private:
    Y2Component* m_handler;
};


#endif // StdioSCRAgent_h
