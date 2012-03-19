/* ProcessAgent.cc
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
 * Implementation of class Process.
 *
 * Authors: Ladislav Slezák <lslezak@novell.com>
 *
 * $Id: ProcessAgent.cc 27914 2006-02-13 14:32:08Z locilka $
 */

#include "y2util/Process.h"

#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <cstring>
#include <y2util/Y2SLog.h>
#include <unistd.h>


/**
 * Destructor
 */
Process::~Process()
{
    if (running())
    {
	MIL << "Terminating running process '" << getpid() << "'..." << std::endl;
	kill();
    }
}

// send a signal
bool Process::kill(int sig)
{
    if (getpid() > 0)
    {
	return ::kill(getpid(), sig) == 0;
    }

    return false;
}

// send SIGKILL
bool Process::kill()
{
    return ExternalProgram::kill();
}

void Process::BufferNewStdoutLines ()
{
    const std::string new_output = receiveUpto('\n');
    stdout_buffer += new_output;
}

// read a line from stdout
std::string Process::readLine()
{
    BufferNewStdoutLines();

    return GetLineFromBuffer(stdout_buffer);
}

// return whether stdout buffer contains any (finished) line
bool Process::anyLineInStdout()
{
    BufferNewStdoutLines();

    return IsAnyLineInBuffer(stdout_buffer);
}

void Process::readStdoutToBuffer()
{
    const size_t b_size = 4096;
    char buffer[b_size];
    size_t read;

    do
    {
	read = receive(buffer, b_size);
	stdout_buffer.append(buffer, read);
    }
    while(read == b_size);
}

// read stdout and return the data
std::string Process::read()
{
    readStdoutToBuffer();

    std::string ret(stdout_buffer);

    stdout_buffer.clear();

    return ret;
}

// read data from a fd to a buffer
void Process::readStderrToBuffer()
{
    if (!stderr_output)
    {
	ERR << "stderr output is not open!" << std::endl;
	return;
    }

    const int b_size = 4096;
    char buffer[b_size];
    int len;

    do
    {
	len = ::fread(buffer, 1, b_size, stderr_output);

	if (len > 0)
	{
	    stderr_buffer.append(buffer, len);
	}
    }
    while(len == b_size);
}

// cut off the first line from a buffer and return it
std::string Process::GetLineFromBuffer(std::string &buffer)
{
    // find end of the first line
    std::string::size_type line_pos = buffer.find("\n");

    // a line found?
    if (line_pos != std::string::npos)
    {
	// remove the line from bufer and return it
	std::string ret(buffer, 0, line_pos + 1);
	buffer.erase(0, line_pos + 1);

	return ret;
    }

    // no new line, return empty string
    return std::string();
}

// returns whether the buffer is empty
bool Process::IsAnyLineInBuffer(const std::string &buffer)
{
    return buffer.empty();
}

// read a line from stderr
std::string Process::readErrLine()
{
    readStderrToBuffer();

    return GetLineFromBuffer(stderr_buffer);
}

// read data from stderr
std::string Process::readErr()
{
    // read from stderr to the buffer
    readStderrToBuffer();

    // return the buffer and clear it 
    std::string ret(stderr_buffer);

    stderr_buffer.clear();

    return ret;
}

// set the filedscriptor to unblocked mode
void UnblockFD(int fd)
{
    // get the current flags
    int flags = ::fcntl(fd,F_GETFL);

    if (flags == -1)
    {
	ERR << strerror(errno) << std::endl; return;
    }

    // set the non-blocking flag
    flags = flags | O_NONBLOCK;

    // set the updated flags
    flags = ::fcntl(fd,F_SETFL,flags);

    if (flags == -1)
    {
	ERR << strerror(errno) << std::endl; return;
    }
}

// create a pipe for stderr output
int Process::create_stderr_pipes()
{
    int stderr_pipes[2];

    // create a pair of pipes
    if (pipe(stderr_pipes) != 0)
    {
	// return current stderr
	return 2;
    }

    // set the stderr pipe to non-blocking mode
    UnblockFD(stderr_pipes[0]);

    stderr_output = ::fdopen(stderr_pipes[0], "r");

    // return fd for writing
    return stderr_pipes[1];
}

int Process::closeAll()
{
    if (!stderr_output)
    {
	// close stderr pipe
	::fclose(stderr_output);

	stderr_output = NULL;
    }

    return ExternalProgram::close();
}

FILE* Process::errorFile()
{
    return stderr_output;
}

