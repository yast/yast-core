/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       SCRAgent.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

#ifndef SCRAgent_h
#define SCRAgent_h


#include <YCP.h>
#include <ycp/y2log.h>

/**
 * @short SuSE Configuration Repository Agent
 *
 * An SCRAgent is an information agent. It handles a subtree of
 * the whole SRC data tree of the system. You can look upon
 * it as a database that is similar to a filesystem. Data
 * is grouped in a tree. But type of the data being stored
 * are not files but YCP values.
 */

class SCRAgent
{
public:
    /**
     * Initializes the base class.
     */
    SCRAgent ();

    /**
     * Cleans up. Furthermore baseclass must have a virtual destructor.
     */
    virtual ~SCRAgent ();

    /**
     * Reads data. Destroy the result after use.
     * @param path Specifies what part of the subtree should
     * be read. The path is specified _relatively_ to Root()!
     */
    virtual YCPValue Read (const YCPPath& path, const YCPValue& arg = YCPNull(), const YCPValue& opt = YCPNull()) = 0;

    /**
     * Writes data. Destroy the result after use.
     */
    virtual YCPBoolean Write (const YCPPath& path, const YCPValue& value,
			    const YCPValue& arg = YCPNull()) = 0;

    /**
     * Get a list of all subtrees.
     */
    virtual YCPList Dir (const YCPPath& path) = 0;

    /**
     * Execute a command
     */
    virtual YCPValue Execute (const YCPPath& path, const YCPValue& value = YCPNull(),
			      const YCPValue& arg = YCPNull()) {
	ycp2error( "Unimplemented Execute called for path %s", path-> toString ().c_str () );
	return YCPNull ();
    }

    /**
     * Register an agent
     */
    virtual YCPBoolean RegisterAgent (const YCPPath& path, const YCPValue& value) {
	ycp2error( "Unimplemented RegisterAgent called for path %s", path-> toString ().c_str () );
	return YCPBoolean( false );
    }

    /**
     * Unregister an agent
     */
    virtual YCPBoolean UnregisterAgent (const YCPPath& path) {
	ycp2error( "Unimplemented UnregisterAgent called for path %s", path-> toString ().c_str () );
	return YCPBoolean( false );
    }

    /**
     * Unregister all agents
     */
    virtual YCPBoolean UnregisterAllAgents () {
	ycp2error( "Unimplemented UnregisterAllAgents called" );
	return YCPBoolean( false );
    }

    /**
     * Unregister an agent
     */
    virtual YCPBoolean RegisterAgent (const YCPPath& path) {
	return RegisterAgent (path, YCPNull ());
    }

    /**
     * Unmount an agent
     */
    virtual YCPBoolean UnmountAgent (const YCPPath& path) {
	return YCPBoolean( false );
    }

    /**
     * Execute other commands. Return 0 if the command is
     * not defined in your Agent.
     */
    virtual YCPValue otherCommand (const YCPTerm& term);

    /**
     * A pointer to the SCRAgent (which normally is the ScriptingAgent)
     * that created this SCRAgent. It can be used to call other SCRAgents
     * directly from C++. You must check if it is not 0.
     */
    SCRAgent *mainscragent;

    /**
     * Reads the scr config file and returns the term. It skips all lines
     * upto (including) the first starting with a ".", which is the path
     * where the agant gets mounted (by the ScriptingAgent).
     */
    static YCPValue readconf (const char *filename);
    
    static SCRAgent* instance();
    
    void setAsCurrentSCR() {
	current_scr = this;
    }
    
private:
    static SCRAgent* current_scr;
};


#endif // SCRAgent_h
