/*
 * HalAgent.h
 *
 * An agent for some hal commands.
 *
 * Authors:	Arvin Schnell <aschnell@suse.de>
 */

#ifndef HalAgent_h
#define HalAgent_h


#include <libhal.h>

#include <ycp/YCPValue.h>
#include <scr/SCRAgent.h>


/**
 * @short SCR Agent for some hal commands.
 */
class HalAgent : public SCRAgent
{

public:

    HalAgent();
    ~HalAgent();

    /**
     * Read data
     */
    virtual YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull(),
			  const YCPValue& opt = YCPNull ());

    /**
     * Write data
     */
    virtual YCPBoolean Write(const YCPPath& path, const YCPValue& value,
			     const YCPValue& arg = YCPNull());

    /**
     * Execute a command
     */
    virtual YCPValue Execute(const YCPPath& path, const YCPValue& value = YCPNull(),
			     const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees
     */
    virtual YCPList Dir(const YCPPath& path) { return YCPList(); }

private:

    LibHalContext* hal_ctx;

    bool initialised;

    bool acquire_global_interface_lock(const string& interface, bool exclusive);
    bool release_global_interface_lock(const string& interface);

};


#endif // HalAgent_h
