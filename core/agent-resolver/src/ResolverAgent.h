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
#include <Y2.h>

/**
 * @short SCR Agent for Resolver commands
 */
class ResolverAgent : public SCRAgent
{
private:
    string file_name;

public:
    ResolverAgent ();
    ~ResolverAgent ();

    /**
     * Read()
     */
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull(), const YCPValue& optarg = YCPNull());

    /**
     * Writes data.
     */
    YCPBoolean Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPList Dir(const YCPPath& path);

    /**
     * Other commands
     */
    YCPValue otherCommand(const YCPTerm& term);
};

#endif /* ResolverAgent_h */
