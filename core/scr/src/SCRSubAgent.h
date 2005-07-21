// -*- c++ -*-

/*
 *  Author:	Arvin Schnell <arvin@suse.de>
 *  Maintainer:	Arvin Schnell <arvin@suse.de>
 */


#ifndef SCRSubAgent_h
#define SCRSubAgent_h

#include <y2/Y2Component.h>
#include <scr/SCRAgent.h>


class SCRSubAgent
{

public:

    /**
     * Constructor.
     */
    SCRSubAgent (YCPPath, YCPValue);

    /**
     * Destructor. Does also unmount the subagent.
     */
    ~SCRSubAgent ();

    /**
     * Mount the subagent, that is create the component. Does nothing if
     * the subagent is already mounted.
     */
    YCPValue mount (SCRAgent *parent);

    /**
     * Unmount the subagent, that is delete the component. Does nothing if
     * the subagent is not mounted.
     */
    void unmount ();

    /**
     * Returns the path of the subagent.
     */
    YCPPath get_path () const { return my_path; }

    /**
     * Returns the component of the subagent. This does not call mount ().
     * Is 0 if mount () was not called of failed.
     */
    Y2Component * get_comp () const { return my_comp; }

    /**
     * Used for finding subagents.
     */
    friend int operator < (const SCRSubAgent *, const YCPPath &);

private:

    /**
     * The scr path.
     */
    YCPPath my_path;

    /**
     * The value is either a string with the filename of the definition
     * or the term of the definition.
     */
    YCPValue my_value;

    /**
     * The component. 0 means not created (mounted).
     */
    Y2Component *my_comp;

    SCRSubAgent (const SCRSubAgent &);		// disallow
    void operator = (const SCRSubAgent &);	// disallow

};


#endif // SCRSubAgent_h
