/*
 * DummyAgent.h
 *
 * A dummy agent, only for testing purposes
 *
 * Author: Klaus Kaempf <kkaempf@suse.de>
 *         Michal Svec <msvec@suse.cz>
 *         Petr Blahos <pblahos@suse.cz>
 *         Gabriele Strattner <gs@suse.de>
 *
 * $Id$
 *
 */

#ifndef DummyAgent_h
#define DummyAgent_h

#include <scr/SCRAgent.h>
#include <Y2.h>

/**
 * @short SCR Agent for testing
 */

class DummyAgent : public SCRAgent
{
private:
   /**
    * data map from agent initialization
    */
   YCPList readList;
   YCPList writeList;
   YCPList execList;

   /**
    * default value if path has no match in dataMap
    */
   YCPValue defaultValue;
   YCPMap   defaultMap;

   /**
    * counts of previously done operations
    */
   int readCalls;
   int writeCalls;
   int execCalls;

   YCPValue checkPath (const YCPPath& path, const YCPMap& map, const YCPValue& defaultVal);

public:
    DummyAgent ();

    /**
     * Reads data. Destroy the result after use.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull(), const YCPValue& opt = YCPNull());

    /**
     * Writes data. Destroy the result after use.
     * @return Value defined in DataMap command. If no value is defined in DataMap, YCPBoolean(true) is returned.
     */
    YCPBoolean Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Execute a program. Destroy the result after use.
     * @return Value defined in DataMap command. If no value is defined in DataMap, YCPBoolean(0) is returned.
     */
    YCPValue Execute(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPList Dir(const YCPPath& path);

    /**
     * Evaluates the DataMap() command
     */
    YCPValue otherCommand(const YCPTerm& term);
};


#endif // DummyAgent_h
