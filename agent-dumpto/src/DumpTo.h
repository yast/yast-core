/**

 DumpTo.h

 Purpose:	class definition for DumpTo
 Creator:	kkaempf@suse.de
 Maintainer:	kkaempf@suse.de

 */

// -*- c++ -*-

#ifndef DumpTo_h
#define DumpTo_h

#include <stdio.h>
#include <sys/types.h>

#include <scr/SCRAgent.h>
#include <scr/SCRInterpreter.h>


/**
 * @short SCR Agent for dumping YCP values to files
 * in a human readable format
 */
class DumpTo : public SCRAgent
{
public:
    /**
     * Creates a new DumpTo.
     */
    DumpTo();

    /**
     * Cleans up
     */
    ~DumpTo ();

    /**
     * Reads data. Destroy the result after use.
     * @param path specifies the file to be read
     */
    YCPValue Read(const YCPPath& path, const YCPValue& arg = YCPNull()) { return YCPVoid (); };

    /**
     * Writes data. Destroy the result after use.
     * @param path specifies the file to be written
     * @param value specifies the value dumped to the file
     * returns YCPBooleanRep (true) on success
     */
    YCPValue Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg = YCPNull());

    /**
     * Get a list of all subtrees.
     */
    YCPValue Dir(const YCPPath& path);

    // ----------------------------------------

private:
    bool do_debug;

    /**
     * recursively dump value to file
     */
    int dumpValue (FILE *f, int level, const YCPValue& value);

    /**
     * open file according to path
     */
    FILE *openFile (const YCPPath& path, bool writing);

    /**
     * indent output by level
     */
    void indentOutput (FILE *f, int level);
};


#endif // DumpTo_h
