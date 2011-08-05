/*---------------------------------------------------------------------\
|                                                                      |  
|                      __   __    ____ _____ ____                      |  
|                      \ \ / /_ _/ ___|_   _|___ \                     |  
|                       \ V / _` \___ \ | |   __) |                    |  
|                        | | (_| |___) || |  / __/                     |  
|                        |_|\__,_|____/ |_| |_____|                    |  
|                                                                      |  
|                               core system                            | 
|                                                        (C) SuSE GmbH |  
\----------------------------------------------------------------------/ 

   File:       Debugger.h

   Author:     Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef Debugger_h
#define Debugger_h

#include <stdio.h>
#include <list>
#include <string>
#include <y2/SymbolEntry.h>

class Y2Namespace;

/**
 * @short Debugger singleton to keep debugging-related status
 */
class Debugger
{
public:
    typedef enum {
	c_unknown,
	c_next, 
	c_step,
	c_continue,
	c_print,
	c_backtrace,
	c_breakpoint,
	c_removebreakpoint,
	c_setvalue
    } command_t;

private:
    int m_socket, m_ns;
    FILE *m_descriptor;
    command_t m_last_command;
    std::string m_outputstash;
    
    bool m_tracing;
    
    typedef struct {
	Y2Namespace* ns;
	bool tracing;
    } stackitem_t;
    
    std::list<stackitem_t> m_blockstack;
    
    bool m_remote;

public:

    Debugger ();

    ~Debugger ();

    /**
     * Initialize the socket and reset the communication
     */
    bool initialize (bool remote);
    bool initializeRemote ();
    bool initializeLocal ();

    /**
     * @short Read the input from controlling socket and act accordingly.
     *
     * For actions needed to be done in context of YCP code being run,
     * return the information to the caller.
     */
    bool processInput (command_t &command, std::list<std::string> &arguments);
    
    bool sendOutput (std::string output );
    
    // save the text for the next output sending
    void stashOutput (std::string output );
    
    command_t lastCommand () const { return m_last_command; }
    
    // sets the last command to be c_step, enables tracing of the next block to be
    // entered
    void setTracing ();
    void setTracing (bool enable);

    void setBreakpoint (std::list<std::string> &arguments);
    void removeBreakpoint (std::list<std::string> &arguments);
    void generateBacktrace ();
    void printVariable (std::string variable_name);
    void setVariable (std::string arg);
    
    void enableTracing (Y2Namespace* block, bool enable);
    bool tracing (Y2Namespace* block) const;
    bool tracing () const;
    
    void pushBlock (Y2Namespace* block, bool tracing);
    void popBlock ();
    
private:
    SymbolEntryPtr findSymbol (std::string name);
};

#endif	// Debugger_h
