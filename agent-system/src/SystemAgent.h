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
    virtual YCPValue Read (const YCPPath& path, const YCPValue& arg = YCPNull(), const YCPValue& opt = YCPNull ());

    /**
     * Write data
     */
    virtual YCPBoolean Write (const YCPPath& path, const YCPValue& value,
		    const YCPValue& arg = YCPNull());

    /**
     * Execute a command
     */
    virtual YCPValue Execute (const YCPPath& path, const YCPValue& value = YCPNull(),
		      const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees
     */
    virtual YCPList Dir (const YCPPath& path) { return YCPList (); }

private:

    string tempdir;

};


#endif // SystemAgent_h
