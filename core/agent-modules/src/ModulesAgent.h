/* ModulesAgent.h -*- c++ -*-
 *
 * An agent for reading the modules.conf configuration file.
 *
 * Author: Michal Svec <msvec@suse.cz>
 *         Daniel Vesely <dan@suse.cz>
 *
 * $Id$
 *
 */

#ifndef ModulesAgent_h
#define ModulesAgent_h

#include <Y2.h>
#include <scr/SCRAgent.h>

class ModulesConf;

/**
 * @short An interface class between YaST2 and modules.conf
 */
class ModulesAgent : public SCRAgent {

private:
    ModulesConf *modules_conf;

public:
	/**
	 * Default constructor.
	 */
    ModulesAgent();
	/** 
	 * Destructor.
	 */
    virtual ~ModulesAgent();

	/**
	 * Provides SCR Read ().
	 * @param path Path that should be read.
	 * @param arg Additional parameter.
	 */
    virtual YCPValue Read(const YCPPath &path, const YCPValue& arg = YCPNull(), const YCPValue& optarg = YCPNull());

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

#endif /* ModulesAgent_h */
