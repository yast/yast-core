/*
 * YaST2: Core system
 *
 * Description:
 *   YaST2 SCR: Ini agent implementation
 *
 * Authors:
 *   Petr Blahos <pblahos@suse.cz>
 *
 * $Id$
 */

#ifndef _IniAgent_h
#define _IniAgent_h

#include <Y2.h>
#include <scr/SCRAgent.h>

#include "IniParser.h"

/**
 * @short An interface class between YaST2 and Ini Agent
 */
class IniAgent : public SCRAgent
{
    private:
        /**
         * Agent private members
         */
	YCPTerm generateSysConfigTemplate (string fn);

	IniParser parser;
    public:
        /**
         * Default constructor.
         */
        IniAgent();
        /** 
         * Destructor.
         */
        virtual ~IniAgent();

        /**
         * Provides SCR Read ().
         * @param path Path that should be read.
         * @param arg Additional parameter.
         */
        virtual YCPValue Read(const YCPPath &path, const YCPValue& arg = YCPNull(), const YCPValue& optarg = YCPNull() );

        /**
         * Provides SCR Write ().
         */
        virtual YCPBoolean Write(const YCPPath &path, const YCPValue& value, const YCPValue& arg = YCPNull());

        /**
         * Provides SCR Write ().
         */
        virtual YCPList Dir(const YCPPath& path);

        /**
         * Used for mounting the agent.
         */    
        virtual YCPValue otherCommand(const YCPTerm& term);
};

#endif /* _IniAgent_h */
