/*
 * SystemAgent.h
 *
 * An agent for handling commands on the system
 *
 * Authors: Klaus Kaempf <kkaempf@suse.de>
 *          Michal Svec <msvec@suse.cz>
 *          Petr Blahos <pblahos@suse.cz>
 *
 * $Id$
 */

#ifndef SystemAgent_h
#define SystemAgent_h


#include <ycp/YCPValue.h>
#include <scr/SCRAgent.h>
#include <scr/SCRInterpreter.h>


/**
 * @short SCR Agent for system commands
 */
class SystemAgent : public SCRAgent
{

public:

    SystemAgent ();
    ~SystemAgent ();

    /**
     * Read data
     */
    YCPValue Read (const YCPPath& path, const YCPValue& arg = YCPNull());

    /**
     * Write data
     */
    YCPValue Write (const YCPPath& path, const YCPValue& value,
		    const YCPValue& arg = YCPNull());

    /**
     * Execute a command
     */
    YCPValue Execute (const YCPPath& path, const YCPValue& value = YCPNull(),
		      const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees
     */
    YCPValue Dir (const YCPPath& path) { return YCPList (); }

private:

    string tempdir;

};


#endif // SystemAgent_h
