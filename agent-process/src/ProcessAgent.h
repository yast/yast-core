/* ProcessAgent.h
 *
 * ------------------------------------------------------------------------------
 * Copyright (c) 2008 Novell, Inc. All Rights Reserved.
 *
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, contact Novell, Inc.
 *
 * To contact Novell about this file by physical or electronic mail, you may find
 * current contact information at www.novell.com.
 * ------------------------------------------------------------------------------
 *
 * An agent for managing multiple processes.
 *
 * Authors: Ladislav Slezák <lslezak@novell.com>
 *
 * $Id: ProcessAgent.h 27914 2006-02-13 14:32:08Z locilka $
 */

#ifndef _ProcessAgent_h
#define _ProcessAgent_h

#include <Y2.h>
#include <scr/SCRAgent.h>

#include <map>

class Process;

/**
 * @short An interface class between YaST2 and Process Agent
 */
class ProcessAgent : public SCRAgent
{
private:
    /**
     * Agent private variables
     */

    // typedef of internal data representation
    typedef map<pid_t, Process*> ProcessContainer;

    ProcessContainer _processes;

private:

    YCPValue ProcessOutput(std::string &output);

public:
    /**
     * Default constructor.
     */
    ProcessAgent();

    /**
     * Destructor.
     */
    virtual ~ProcessAgent();

    /**
     * Provides SCR Read ().
     * @param path Path that should be read.
     * @param arg Additional parameter.
     */
    virtual YCPValue Read(const YCPPath &path,
			  const YCPValue& arg = YCPNull(),
                          const YCPValue& opt = YCPNull());

    /**
     * Provides SCR Write ().
     */
    virtual YCPBoolean Write(const YCPPath &path,
			   const YCPValue& value,
			   const YCPValue& arg = YCPNull());

    /**
     * Provides SCR Execute ().
     */
    virtual YCPValue Execute(const YCPPath &path,
			     const YCPValue& value = YCPNull(),
			     const YCPValue& arg = YCPNull());

    /**
     * Provides SCR Dir ().
     */
    virtual YCPList Dir(const YCPPath& path);

    /**
     * Used for mounting the agent.
     */
    virtual YCPValue otherCommand(const YCPTerm& term);
};

#endif /* _ProcessAgent_h */
