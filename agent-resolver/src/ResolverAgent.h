/* ResolverAgent.h
 *
 * Classes for reading the resolv.conf configuration file.
 *
 * Author: Klaus Kaempf <kkaempf@suse.de>
 *         Daniel Vesely <dan@suse.cz>
 *         Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#ifndef ResolverAgent_h
#define ResolverAgent_h

#include <scr/SCRAgent.h>
#include <scr/SCRInterpreter.h>
#include <Y2.h>

/**
 * @short SCR Agent for Resolver commands
 */
class ResolverAgent : public SCRAgent
{
private:
    string file_name;

public:
    ResolverAgent (const string &fname);
    ~ResolverAgent ();

    /**
     * Read()
     */
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull());

    /**
     * Writes data.
     */
    YCPValue Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPValue Dir(const YCPPath& path);


};


class Y2ResolverComponent : public Y2Component
{
    ResolverAgent *agent;
    SCRInterpreter *interpreter;
public:
    Y2ResolverComponent() : agent(0), interpreter(0) {}

    ~Y2ResolverComponent() {
	delete agent;
    }

    string name() const { return "ag_resolver"; };
    YCPValue evaluate(const YCPValue& command);
};


class Y2CCResolver : public Y2ComponentCreator
{
public:
    Y2CCResolver() : Y2ComponentCreator(Y2ComponentBroker::BUILTIN) {};
    bool isServerCreator() const { return true; };
    Y2Component *create(const char *name) const;
};

#endif /* ResolverAgent_h */
