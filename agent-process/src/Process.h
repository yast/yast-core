/* Process.h
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
 * Class Process
 *
 * Authors: Ladislav Slezák <lslezak@novell.com>
 *
 * $Id:$
 */

#ifndef Process_h
#define Process_h

#include <y2util/ExternalProgram.h>

/**
 * @short Execute a program and give access to its io
 * An object of this class encapsulates the execution of
 * an external program. It starts the program using fork
 * and some exec.. call, gives you access to the program's
 * stdio/stderr and closes the program after use.
 */
class Process: public ExternalProgram
{

private:

    std::string stdout_buffer;	// buffer for stdout
    std::string stderr_buffer;	// buffer for stderr
    int stderr_pipes[2];	// a pipe for stderr output

private:

    // disable copy ctor and operator=
    Process(const Process&);
    Process& operator=(const Process&);

    // create a pipe for stderr, return the end for writing
    int create_stderr_pipes();

    // a helper function
    std::string GetLineFromBuffer(std::string &buffer);

    // a helper function
    void readToBuffer(int fd, std::string &buffer_str);

    void readStdoutToBuffer();

public:

    /**
     * Start the external program by using the shell <tt>/bin/sh<tt>
     * with the option <tt>-c</tt>. You can use io direction symbols < and >.
     * @param commandline a shell commandline that is appended to
     * <tt>/bin/sh -c</tt>.
     * @param default_locale whether to set LC_ALL=C before starting
     * @param use_pty start the process in a terminal
     */
    Process(const std::string &commandline, bool use_pty = false, bool default_locale = false)
	: ExternalProgram(commandline, Stderr_To_FileDesc,
	    use_pty, create_stderr_pipes(), default_locale)
    {}

    /**
     * Start an external program by giving the arguments as an arry of char *pointers.
     * If environment is provided, variables will be added to the childs environment,
     * overwriting existing ones.
     */

    Process(const char *const *argv, const Environment &environment, bool use_pty = false, bool default_locale = false)
	: ExternalProgram(argv, environment, Stderr_To_FileDesc,
	    use_pty, create_stderr_pipes(), default_locale)
    {}


    ~Process();

    /**
     * Send a signal
     */
    bool kill(int sig);

    /**
     * Send SIGKILL
     */
    bool kill();

    /**
     * Read a line from stdout
     */
    std::string readLine();

    /**
     * Read characters from stdout (not line oriented)
     */
    std::string read();

    /**
     * Read a line from stderr
     */
    std::string readErrLine();

    /**
     * Read characters from stderr (not line oriented)
     */
    std::string readErr();

    /**
     * Read stdout and stderr of the process to buffers to unblock it
     */
    void readToBuffers();

    int closeAll();
};

#endif // Process_h
