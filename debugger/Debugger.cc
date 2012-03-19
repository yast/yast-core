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

   File:	Debugger.cc

   Basic infrastructure for YCP debugger

   Author:	Stanislav Visnovsky <visnov@suse.de>
   Maintainer:	Stanislav Visnovsky <visnov@suse.de>

/-*/

#include <y2/Y2ComponentBroker.h>
#include <y2/Y2Component.h>
#include <y2/Y2Namespace.h>
#include <ycp/y2log.h>
#include <ycp/ExecutionEnvironment.h>
#include <ycp/YBlock.h>
#include <ycp/YBreakpoint.h>
#include <ycp/Parser.h>
#include <y2util/stringutil.h>
#include "Debugger.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#define ADDRESS     "/tmp/yast.socket"
#define PORT	    16384

extern ExecutionEnvironment ee;

Debugger::Debugger () : 
  m_socket (-1),
  m_descriptor (NULL),
  m_outputstash (""),
  m_tracing (false),
  m_remote (false)
{}

Debugger::~Debugger ()
{
    // close the controll socket
    if (m_socket) 
    {
	close (m_socket);
	if (m_remote) 
	    unlink (ADDRESS);
    }
    m_socket = -1;
}

bool Debugger::initialize(bool remote)
{
    m_remote = remote;
    return remote ? initializeRemote() : initializeLocal();
}

bool Debugger::initializeLocal()
{
  socklen_t fromlen;
  int ns, len;
  struct sockaddr_un saun, fsaun;

  // FIXME: possible leak
  m_socket = -1;
  m_descriptor = NULL;

  if ((m_socket = socket (AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
      return false;
    }

  saun.sun_family = AF_UNIX;
  strcpy (saun.sun_path, ADDRESS);

  int res = access (ADDRESS, F_OK);
  if ( res == 0 || errno != ENOENT )
  {
    y2security ("Cannot create debugger socket: %s", res == 0 ? "File exists" : strerror (errno) );
    return false;
  }

  len = sizeof (saun.sun_family) + strlen (saun.sun_path);

  if (bind (m_socket,(struct sockaddr *) &saun, len) < 0)
  {
    return false;
  }

  // wait for connection
  if (listen (m_socket, 1) < 0)
  {
    y2error ("Debugger: listen failed");
    return false;
  }
  fromlen = 108;

  if ((ns = accept (m_socket, (struct sockaddr *) &fsaun, &fromlen)) < 0)
  {
    y2error ("Debugger: accept failed");
    return false;
  }

  m_descriptor = fdopen (ns, "r");
  m_last_command = c_step;	// step to enable debugging from the start
  m_ns = ns;

  return true;
}

bool Debugger::initializeRemote()
{
  socklen_t fromlen;
  int ns;
  struct sockaddr_in sain, fsain;

  // FIXME: possible leak
  m_socket = -1;
  m_descriptor = NULL;

  if ((m_socket = socket (PF_INET, SOCK_STREAM, 0)) < 0)
    {
      return false;
    }

  sain.sin_family = PF_UNIX;
  sain.sin_addr.s_addr=htonl(INADDR_ANY);
  sain.sin_port=htons(PORT);

  if (bind (m_socket,(struct sockaddr *) &sain, sizeof (sain)) < 0)
  {
    y2error ("Debugger: bind failed");
    return false;
  }

  // wait for connection
  if (listen (m_socket, 1) < 0)
  {
    y2error ("Debugger: listen failed");
    return false;
  }

  fromlen = sizeof(fsain);

  if ((ns = accept (m_socket, (struct sockaddr *) &fsain, &fromlen)) < 0)
  {
    y2error ("Debugger: accept failed");
    return false;
  }

  m_descriptor = fdopen (ns, "r");
  m_last_command = c_step;	// step to enable debugging from the start
  m_ns = ns;

  return true;
}

void Debugger::setTracing()
{
    m_last_command = c_step;
    m_tracing = true;
}

void Debugger::setBreakpoint (std::list<std::string> &args)
{
    SymbolEntryPtr sentry = findSymbol (args.front ());

    if (sentry)
    {
	if( !sentry->isFunction ())
	    sendOutput ("Identifier is not a function");
	else
	{
	    YFunctionPtr fnc = ((YFunctionPtr)((YSymbolEntryPtr)sentry)->code());
	
	    // set the breakpoint wrapper
	    if (fnc->definition ()->kind () == YCode::yiBreakpoint)
	    {
		((YBreakpointPtr)fnc->definition ())->setEnabled (true);
		sendOutput ("Breakpoint enabled");
		return;
	    }
	
	    YBreakpointPtr bp = new YBreakpoint (fnc->definition(), args.front () );
	
	    fnc->setDefinition (bp);
	    std::string result = "Set breakpoint to " + sentry->toString();
	    sendOutput (result);
	}
    }
    else
	sendOutput ("Identifier not found");
}

void Debugger::removeBreakpoint (std::list<std::string> &args)
{
    SymbolEntryPtr sentry = findSymbol (args.front ());

    if (sentry)
    {
	if( !sentry->isFunction ())
	    sendOutput ("Identifier is not a function");
	else
	{
	    YFunctionPtr fnc = ((YFunctionPtr)((YSymbolEntryPtr)sentry)->code());
	
	    if (fnc->definition ()->kind () == YCode::yiBreakpoint)
	    {
		// disable the breakpoint wrapper
		((YBreakpointPtr)fnc->definition ())->setEnabled (false);
		sendOutput ("Breakpoint disabled");
		return;
	    }
	    sendOutput ("Breakpoint not found");
	}
    }
    else
	sendOutput ("Identifier not found");
}

void Debugger::generateBacktrace ()
{
    std::string result = "Call stack:";
    ExecutionEnvironment::CallStack stack = ee.callstack();
    ExecutionEnvironment::CallStack::const_reverse_iterator it = stack.rbegin();
    while (it != stack.rend())
    {
        result = result 
    	  + "\n"
    	  + ( (*it)->filename ) 
          + ":" 
          + stringutil::numstring((*it)->linenumber) 
          + ": "
          + ((*it)->function->entry()->toString());

        int paramcount = ((constFunctionTypePtr)((*it)->function->entry()->type()))->parameterCount();
        if( paramcount > 0 )
		result += " with paramters: ";
        for( int i = 0; i < paramcount ; i++ )
        {
          string param = (*it)->params[0]->toString();
          if (param.length() > 80)
          {
              param = "<too long>";
          }
          result += param;
          if (i < paramcount - 1)
          {
            result += ", ";
          }
        }
        ++it;
    };

    sendOutput (result);
}

SymbolEntryPtr Debugger::findSymbol (std::string arg)
{
    std::vector<std::string> words;
    SymbolEntryPtr sentry = NULL;

    stringutil::split(arg, words, ":");

    if (words.size () > 1)
    {
	// name contains namespace, handle here
	std::string ns_name = words[0];
	std::string name = words[1];
	Y2Component* c = Y2ComponentBroker::getNamespaceComponent (ns_name.c_str());
	if( c )
	{
	    if (c->name () == "wfm")
	    {
		Y2Namespace* ns = c->import (ns_name.c_str());
		if (ns) 
		    // this returns NULL in case the name was not found
		    return ns->lookupSymbol (name.c_str());
	    }
	    else
		return NULL; // this is not YCP symbol
	}
    }
    else 
    {
	// try parameters
	ExecutionEnvironment::CallStack stack = ee.callstack();
	    
	if( stack.size() > 0 )
	{
	    ExecutionEnvironment::CallStack::const_reverse_iterator it = stack.rbegin();
	    
    	    YSymbolEntryPtr ysentry = (YSymbolEntryPtr)((*it)->function->entry());

	    // this returns NULL in case the name was not found
	    sentry = ((YFunctionPtr)ysentry->code())->declaration ()->lookupSymbol (words[0].c_str());

	    if (sentry)
		return sentry;
	}
	    
	// try block stack
	for( std::list<stackitem_t>::iterator blk = m_blockstack.begin(); blk != m_blockstack.end (); blk++)
	{
	    sentry = blk->ns->lookupSymbol (words[0].c_str());
	    if (sentry)
		return sentry;
	}
    }

    // not found
    return NULL;
}

void Debugger::printVariable (std::string arg)
{
    SymbolEntryPtr sentry = findSymbol (arg);

    if (sentry)
    {
	if( !sentry->isVariable ())
	    sendOutput ("Identifier is not a variable");
	else
	{
	    if( sentry->value().isNull() )
		sendOutput ("nil");
	    else
		sendOutput (sentry->value()->toString());
	}
    }
    else
	sendOutput ("Identifier not found");
}

void Debugger::setVariable (std::string assign)
{
    // first, split by '='
    std::vector<std::string> words;
    stringutil::split(assign, words, "=");
    
    if( words.size() != 2 )
    {
	sendOutput ("Set variable format is <name>=<constant>");
	return;
    }

    SymbolEntryPtr sentry = findSymbol (words.front ());

    if (sentry)
    {
	if( !sentry->isVariable ())
	    sendOutput ("Identifier is not a variable");
	else
	{
	    // set the new value
	    Parser parser (words[1].c_str()); // set parser to value

	    YCodePtr pc = parser.parse ();
	    if (!pc )
	    {
		sendOutput ("Cannot parse new value");
	    }
	    else
	    {
		sentry->setValue (pc->evaluate (true));   // set the new value
		sendOutput ( std::string(sentry->name ()) + " = " + sentry->value ()->toString () );
	    }
	}
    }
    else
	sendOutput ("Identifier not found");
}

bool Debugger::processInput (command_t &command, std::list<std::string> &arguments)
{
    char c;
    std::string s;
    std::list<std::string> args;

    // FIXME: use flex
    if (m_descriptor == NULL)
	return false;
	
    // First, send the current context
    YStatementPtr statement = ee.statement ();
    
after_internal:
    if (statement)
	sendOutput (ee.filename() + ":" + stringutil::numstring(ee.linenumber()) + " >>> " + statement->toString ());
    else
	sendOutput ("no code");
    
    // clean up for next command
    s = "";
    args.clear();

    while ((c = fgetc (m_descriptor)) != EOF)
    {
	if (c == '\n')
	{
	    break;
	}
	s += c;
    }
    
    if (s.empty ())
    {
	y2error ("Communication with debugging UI closed");
	close (m_socket);
	
	if (m_remote)
	    unlink (ADDRESS);

	m_socket = -1;
	m_descriptor = NULL;
	return false;
    }
    
    command = c_unknown;
    // FIXME: I said flex!
    if (s == "c")
    {
	command = Debugger::c_continue;
    }
    else if (s == "n")
    {
	command = Debugger::c_next;
    }
    else if (s == "s")
    {
	command = Debugger::c_step;
    }
    else if (s == "bt")
    {
	command = Debugger::c_backtrace;
    }
    else if (s[0] == 'v')
    {
	command = Debugger::c_setvalue;
	args.push_back(s.substr(2));
    }
    else if ( s[0] == 'b' )
    {
	command = Debugger::c_breakpoint;
	args.push_back(s.substr(2));
    }
    else if ( s[0] == 'r' && s[1] == 'b')
    {
	command = Debugger::c_removebreakpoint;
	args.push_back(s.substr(3));
    }
    else if (s[0] == 'p')
    {
	command = Debugger::c_print;
	args.push_back(s.substr(2));
    }
    
    if (command == Debugger::c_print)
    {
	printVariable (args.front () );
	goto after_internal;
    }
    
    if (command == Debugger::c_breakpoint)
    {
	setBreakpoint (args);
	goto after_internal;
    }

    if (command == Debugger::c_removebreakpoint)
    {
	removeBreakpoint (args);
	goto after_internal;
    }

    if (command == Debugger::c_backtrace)
    {
	generateBacktrace ();
	goto after_internal;
    }
    
    if (command == Debugger::c_setvalue)
    {
	setVariable (args.front () );
	goto after_internal;
    }
    
    arguments = args;
    m_last_command = command;

    return true;
}

bool Debugger::sendOutput( std::string output )
{
    output = m_outputstash + output;
    y2debug ("Sending out output %s", output.c_str() );
    if( output.empty() )
	output = " ";
    output = output + "\n<EOF>\n";
    send (m_ns, output.c_str (), strlen( output.c_str ()), 0);
    
    // clean up the output stash, it's sent already
    m_outputstash = "";
    return true;
}

void Debugger::stashOutput( std::string output )
{
    m_outputstash += output;
}

void Debugger::enableTracing (Y2Namespace* block, bool enable)
 {
    for( std::list<stackitem_t>::iterator it = m_blockstack.begin (); it != m_blockstack.end (); it++)
    {
	if ( it->ns == block )
	    it->tracing = enable;
	return;
    }
}

bool Debugger::tracing (Y2Namespace* block) const
{
    for( std::list<stackitem_t>::const_iterator it = m_blockstack.begin (); it != m_blockstack.end (); it++)
    {
	if ( it->ns == block )
	    return it->tracing;
    }
    return false;
}

void Debugger::pushBlock (Y2Namespace* block, bool tracing)
{
    stackitem_t si;
    si.ns = block;
    si.tracing = tracing;
    
    m_blockstack.push_front (si);
}
 
void Debugger::popBlock ()
{
    m_blockstack.pop_front ();
}

bool Debugger::tracing () const
{
    return m_tracing;
}

void Debugger::setTracing (bool enable)
{
    m_tracing = enable;
}

