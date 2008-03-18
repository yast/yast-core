/* ProcessAgent.cc
 *
 * ------------------------------------------------------------------------------
 * Copyright (c) 2008 Novell, Inc. All Rights Reserved.
 * 
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of version 2 of the GNU General Public License as published by the
 * Free Software Foundation.
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
 * Implementation of the Process agent
 *
 * Authors: Ladislav Slezák <lslezak@novell.com>
 *
 * $Id: ProcessAgent.cc 27914 2006-02-13 14:32:08Z locilka $
 */

#include "ProcessAgent.h"
#include "Process.h"

#include <y2util/ExternalProgram.h>

/**
 * Constructor
 */
ProcessAgent::ProcessAgent() : SCRAgent()
{
}

/**
 * Destructor - release all processes
 */
ProcessAgent::~ProcessAgent()
{
    // release created objects
    for (ProcessContainer::iterator it = _processes.begin(); it != _processes.end(); it++)
    {
	delete it->second;
    }

    // remove invalid pointers
    _processes.clear();
}

/**
 * Dir - return list of managed processes
 */
YCPList ProcessAgent::Dir(const YCPPath& path)
{
    /**
     * @builtin Dir(.process) -> list<integer>
     * Return list od processes (IDs) managed by the process agent.
     *
     * @example Dir(.process) -> [ 23568, 28896 ]
     */
    // return all processes
    if (path->isRoot())
    {
	YCPList ret;

	for(ProcessContainer::const_iterator it = _processes.begin(); it != _processes.end(); it++)
	{
	    ret->add(YCPInteger(it->first));
	}

	return ret;
    }

    y2error("Wrong path '%s' in Dir().", path->toString().c_str());
    return YCPNull();
}

YCPValue ProcessAgent::ProcessOutput(std::string &output)
{
    // no output, return nil
    if (output.empty())
    {
	return YCPVoid();
    }

    // remove the trailing new line character, "" means empty line
    output.erase(output.end() - 1);

    return YCPString(output);
}

/**
 * Read
 */
YCPValue ProcessAgent::Read(const YCPPath &path, const YCPValue& arg, const YCPValue& opt)
{
    std::string cmd(path->component_str(0));

    if (arg.isNull() || !arg->isInteger())
    {
	y2error("ID of the process is missing");
	return YCPNull();
    }

    pid_t id = arg->asInteger()->value();

    y2debug("Requested path: %s, id: %d", cmd.c_str(), id);

    ProcessContainer::iterator proc(_processes.find(id));

    if (proc == _processes.end())
    {
	y2error("Process '%d' not found", id);
	return YCPNull();
    }

    // handler for "running" path
    if (cmd == "running")
    {
	/**
	 * @builtin Read (.process.running, integer id) -> boolean
	 * Returns true if the process is running
	 *
	 * @example Read (.process.running, 12345) -> true
	 */

	// read output to unblock the process
	proc->second->readStdoutToBuffer();
	proc->second->readStderrToBuffer();

	return YCPBoolean(proc->second->running());
    }
    // handler for "pid" path
    else if (cmd == "pid")
    {
	/**
	 * @builtin Read (.process.pid, integer id) -> integer
	 * Returns the PID of a process
	 *
	 * @example Read (.process.pid, 12345) -> 6789
	 */
	return YCPInteger(id);
    }
    else if (cmd == "read_line")
    {
	/**
	 * @builtin Read (.process.read_line, integer id) -> string
	 * Returns one line from stdout of the process, nil if there is no output
	 *
	 * @example Read (.process.read_line, 12345) -> nil
	 */
	std::string out = proc->second->readLine();
	return ProcessOutput(out);
    }
    else if (cmd == "read")
    {
	/**
	 * @builtin Read (.process.read, integer id) -> string
	 * Returns read stdout of the process, nil if there is no output.
	 * This read function is not line-oriented, the output can contain multiple lines or just part of a line.
	 *
	 * @example Read (.process.read, 12345) -> nil
	 */
	std::string out(proc->second->read());

	if (out.empty())
	{
	    return YCPVoid();
	}
	else
	{
	    return YCPString(out);
	}
    }
    else if (cmd == "read_line_stderr")
    {
	/**
	 * @builtin Read (.process.read_line_stderr, integer id) -> string
	 * Returns one line from stderr of the process, nil if there is no output
	 *
	 * @example Read (.process.read_line_stderr, 12345) -> nil
	 */
	std::string err = proc->second->readErrLine();
	return ProcessOutput(err);
    }
    else if (cmd == "read_stderr")
    {
	/**
	 * @builtin Read (.process.read_stderr, integer id) -> string
	 * Returns read stderr of the process, nil if there is no output.
	 * This read function is not line-oriented, the output can contain multiple lines or just part of a line.
	 *
	 * @example Read (.process.read_stderr, 12345) -> nil
	 */
	std::string out(proc->second->readErr());

	if (out.empty())
	{
	    return YCPVoid();
	}
	else
	{
	    return YCPString(out);
	}
    }
    else if (cmd == "status")
    {
	/**
	 * @builtin Read (.process.status, integer id) -> integer
	 * Returns exit status of the process, if the process is still running nil is returned.
	 *
	 * @example Read (.process.status, 12345) -> 0
	 */
	if (proc->second->running())
	{
	    // the process is still running
	    return YCPVoid();
	}
	else
	{
	    return YCPInteger(proc->second->close());
	}
    }
    else if (cmd == "buffer_empty")
    {
	/**
	 * @builtin Read (.process.buffer_empty, integer id) -> boolean
	 * Returns boolean whether the stdout buffer is empty, if buffer is not empty, false is returned.
	 *
	 * @example Read (.process.buffer_empty, 12345) -> false
	 */
	return YCPBoolean (proc->second->anyLineInStdout());
    }

    y2error("Wrong path '%s' in Read().", path->toString().c_str());
    return YCPNull();
}

/**
 * Write
 */
YCPBoolean ProcessAgent::Write(const YCPPath &path, const YCPValue& value,
    const YCPValue& arg)
{
    if (!path->isRoot())
    {
	y2error("Unsupported path in Write(): %s", path->toString().c_str());
	return YCPNull();
    }

    if (value.isNull() || !value->isInteger())
    {
	y2error("ID of the process is missing");
	return YCPNull();
    }

    pid_t id = value->asInteger()->value();

    y2debug("Writing to %d", id);

    ProcessContainer::iterator proc(_processes.find(id));

    if (proc == _processes.end())
    {
	y2error("Process '%d' not found", id);
	return YCPNull();
    }

    if (!arg.isNull() && arg->isString())
    {
	if (!proc->second->running())
	{
	    y2warning("Process '%d' is not running, cannot write to stdin!", id);
	    return YCPBoolean(false);
	}
	/**
	 * @builtin Write(.process, integer id, input_string) -> boolean
	 * Wrtites the input string to stdin of the process. Returns true on success.
	 *
	 * @example Write(.process, 12345, "foo") -> true
	 */

	return YCPBoolean(proc->second->send(arg->asString()->value()));
    }
    else
    {
	// wrong type of the arg
	y2error("Value '%s' is not a string", arg->toString().c_str());
	return YCPBoolean(false);
    }
}

/**
 * Execute
 */
YCPValue ProcessAgent::Execute(const YCPPath &path,
    const YCPValue& value , const YCPValue& arg)
{
    std::string pth(path->component_str(0));

    y2debug("Executing path: %s", pth.c_str());

    // handler for ".start_shell" path
    if (pth == "start_shell" || pth == "start")
    {
	/**
	 * @builtin Execute(.process.start_shell, string command, map options) -> integer
	 * Execute the command in a shell (/bin/sh). The command can contain all shell features like
	 * argument expansion, stdout/stderr redirection...
	 *
	 * The optional map can contain additional configuration: "tty" : boolean - run the command in terminal (instead of piped stdout/stderr), the default is false,
	 * "C_locale" : boolean - use C locale (default false), "env" : map<string variable, string value> - set additional environment variables.
	 *
	 * Returns ID of the started process
	 *
	 * @example Execute(.process.start_shell, "/bin/true") -> 12345
	 */
	/**
	 * @builtin Execute(.process.start, string command, map options) -> integer
	 * Execute the command. The string command is a path to the program, arguments are passed in the map - value of key "args" must be list<string> with the required arguments. For other options see .start_shell info.
	 *
	 * Returns ID of the started process
	 *
	 * @example Execute(.process.start, "/bin/echo", $[ "args" : [ "arg1", "arg2" ] ]) -> 12345
	 */

	// check type of the argument
	if (!value.isNull() && value->isString())
	{
	    std::string commandline = value->asString()->value();

	    bool use_pty = false;
	    bool default_locale = false;

	    ExternalProgram::Environment env;
	    YCPList args;

	    // set optional parameters
	    if (!arg.isNull() && arg->isMap())
	    {
		YCPMap opt_map = arg->asMap();

		// start in a terminal?
		if( ! opt_map->value( YCPString("tty")).isNull())
		{
		    if (opt_map->value(YCPString("tty"))->isBoolean())
		    {
			use_pty = opt_map->value(YCPString("tty"))->asBoolean()->value();
		    }
		    else
		    {
			y2warning("tty option '%s' is not a boolean value", opt_map->value(YCPString("tty"))->toString().c_str());
		    }
		}

		// use default locale?
		if( ! opt_map->value( YCPString("C_locale")).isNull() && opt_map->value(YCPString("C_locale"))->isBoolean())
		{
		    default_locale = opt_map->value(YCPString("C_locale"))->asBoolean()->value();
		}

		// add environment variables
		if( ! opt_map->value( YCPString("env")).isNull() && opt_map->value(YCPString("env"))->isMap())
		{
		    YCPMap envmt = opt_map->value(YCPString("env"))->asMap();

		    for(YCPMapIterator it = envmt.begin(); it != envmt.end(); it++)
		    {
			YCPValue key = it.key();
			YCPValue val = it.value();

			if (!key.isNull() && key->isString() && !val.isNull() && val->isString())
			{
			    env.insert(ExternalProgram::Environment::value_type(
				key->asString()->value(), val->asString()->value())
			    );
			}
			else
			{
			    y2error("Invalid pair in env map: $[ %s : %s ], map<string,string> is required",
				key->toString().c_str(), val->toString().c_str());
			}
		    }
		}

		if( ! opt_map->value( YCPString("args")).isNull() && opt_map->value(YCPString("args"))->isList())
		{
		    args = opt_map->value(YCPString("args"))->asList();
		}
	    }
	    else
	    {
		if (!arg.isNull())
		{
		    y2error("Argument '%s' is not a map", arg->toString().c_str());
		    return YCPNull();
		}
	    }

	    Process *p;

	    if (pth == "start_shell")
	    {
		// start using shell
		const char *argv[4];
		argv[0] = "/bin/sh";
		argv[1] = "-c";
		argv[2] = commandline.c_str();
		argv[3] = 0;

		p = new Process(argv, env, use_pty, default_locale);
	    }
	    else
	    {
		int array_size = 1 + args->size();
		const char *argv[array_size + 1];

		// store the path
		argv[0] = commandline.c_str();
		
		// store arguments
		int index = 0;
		for (; index < args->size(); index++)
		{
		    if (args->value(index)->isString())
		    {
			argv[index + 1] = args->value(index)->asString()->value().c_str();
		    }
		}

		// terminate the array
		argv[index + 1] = NULL;

		p = new Process(argv, env, use_pty, default_locale);
    	    }

	    // do not block reading
	    p->setBlocking(false);

	    pid_t pid = p->getpid();

	    if (pid > 0)
	    {
		// store the mapping PID->Process*
		_processes.insert(ProcessContainer::value_type(pid, p));
		return YCPInteger(pid);
	    }
	    else
	    {
		y2error("Program NOT started!");
		return YCPNull();
	    }
	}
	else
	{
	    y2error("Argument '%s' is not a string", value->toString().c_str());
	    return YCPNull();
	}
    }
    else
    {
	if (value.isNull() || !value->isInteger())
	{
	    y2error("ID of the process is missing");
	    return YCPNull();
	}

	pid_t id = value->asInteger()->value();

	y2debug("Requested path: %s, id: %d", pth.c_str(), id);

	ProcessContainer::iterator proc(_processes.find(id));

	if (proc == _processes.end())
	{
	    y2error("Process '%d' not found", id);
	    return YCPNull();
	}

	if (pth == "kill")
	{
	    /**
	     * @builtin Execute(.process.kill, integer id, integer signal) -> boolean
	     * Send a signal to the process, if signal is missing then SIGKILL is sent.
	     *
	     * @example Execute(.process.kill, 12345, 15) -> true // send SIGTERM
	     * @example Execute(.process.kill, 12345) -> true     // send SIGKILL
	     */
	    // check the value
	    if (!arg.isNull() && arg->isInteger())
	    {
		// send the requested signal
		int sig = arg->asInteger()->value();

		y2milestone("Sending signal %d to process %d...", sig, id);
		return YCPBoolean(proc->second->kill(sig));
	    }
	    else
	    {
		// send SIGKILL
		y2milestone("Sending SIGKILL to process %d...", id);
		return YCPBoolean(proc->second->kill());
	    }
	}
	else if (pth == "release")
	{
	    /**
	     * @builtin Execute(.process.release, integer id) -> boolean
	     * Removes the process from the internal structure and releases all allocated resources (buffers).
	     * If the process is running then it is killed by SIGKILL at first.
	     *
	     * @example Execute(.process.release, 12345) -> true
	     */
	    y2milestone("Releasing Process object %d...", id);
	    // relese the Process object
	    delete proc->second;

	    // remove the entry
	    _processes.erase(proc);
	    return YCPBoolean(true);
	}
	else if (pth == "close")
	{
	    /**
	     * @builtin Execute(.process.close, integer id) -> integer
	     * Close input/output of the process and wait until the process ends
	     *
	     * Returns Exit value of the process 
	     *
	     * @example Execute(.process.close, 12345) -> 0
	     */
	    y2milestone("Closing output of %d...", id);
	    return YCPInteger(proc->second->closeAll());
	}
    }

    y2error("Wrong path '%s' in Execute()", path->toString().c_str());
    return YCPNull();
}

/**
 * otherCommand
 */
YCPValue ProcessAgent::otherCommand(const YCPTerm& term)
{
    std::string sym = term->name();

    if (sym == "ProcessAgent") {
        /* Your initialization */
        return YCPVoid();
    }

    return YCPVoid();
}
