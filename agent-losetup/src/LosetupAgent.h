// -*- c++ -*-

#ifndef LosetupAgent_h
#define LosetupAgent_h

#include <scr/SCRAgent.h>
#include <scr/SCRInterpreter.h>
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
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull());

    /**
     * Creates a loop device with the annotated encryption.
     */
    YCPValue Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPValue Dir(const YCPPath& path);
};

class Y2LosetupComponent : public Y2Component
{
    LosetupAgent *agent;
    SCRInterpreter *interpreter;
public:
    Y2LosetupComponent() : agent(0), interpreter(0) {}

    ~Y2LosetupComponent() {
	if (interpreter) {
	    delete agent;
	    delete interpreter;
	}
    }

    string name() const { return "ag_losetup"; };
    YCPValue evaluate(const YCPValue& command);
};


class Y2CCLosetup : public Y2ComponentCreator
{
public:
    Y2CCLosetup() : Y2ComponentCreator(Y2ComponentBroker::BUILTIN) {};
    bool isServerCreator() const { return true; };
    Y2Component *create(const char *name) const;
};

#endif // LosetupAgent_h
