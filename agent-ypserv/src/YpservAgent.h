/*							-*- C++ -*-
 * YpservAgent.h
 *
 * An agent for finding NIS servers
 *
 * Authors: Martin Vidner <mvidner@suse.cz>
 *
 * $Id$
 */

#ifndef YpservAgent_h
#define YpservAgent_h


#include <ycp/YCPValue.h>
#include <scr/SCRAgent.h>
//#include <scr/SCRInterpreter.h>


/**
 * @short SCR Agent for ypserv commands
 */
class YpservAgent : public SCRAgent
{

public:
    /**
     * Read data
     */
    virtual YCPValue Read (const YCPPath& path, const YCPValue& arg = YCPNull());

    /**
     * Write data
     */
    virtual YCPValue Write (const YCPPath& path, const YCPValue& value,
			    const YCPValue& arg = YCPNull());

    /**
     * Execute a command
     */
    virtual YCPValue Execute (const YCPPath& path,
			      const YCPValue& value = YCPNull(),
			      const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees
     */
    virtual YCPValue Dir (const YCPPath& path);
};


#endif // YpservAgent_h
