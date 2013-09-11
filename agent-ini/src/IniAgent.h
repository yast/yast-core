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

          /**
           * Helper to lazy load root when operation happen on agent. It must be
           * placed everywhere where action with agent happen like Read, Write or
           * OtherCommand. It must not be called on destructor as it depends on
           * SCR root that can be already destructed.
           */
          void setLastRoot();

          /**
           * remember root used in last operation. It is needed during
           * destructor write, because original scr can be already destructed
           * leading to segfault
           * C string is owned by class and should be destructed with free as it
           * is allocated with strdup
           */
          char * last_root;
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

        virtual const char * root() const;
};

#endif /* _IniAgent_h */
