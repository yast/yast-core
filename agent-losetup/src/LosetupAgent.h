// -*- c++ -*-

#ifndef LosetupAgent_h
#define LosetupAgent_h

#include <scr/SCRAgent.h>
#include <Y2.h>

/**
 * @short SCR Agent for access to loop setup
 */

class LosetupAgent : public SCRAgent
{
public:
    LosetupAgent();

    /**
     * Not implemented yet
     */
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull(), const YCPValue& opt = YCPNull());

    /**
     * Creates a loop device with the annotated encryption.
     */
    YCPBoolean Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPList Dir(const YCPPath& path);
};


#endif // LosetupAgent_h
