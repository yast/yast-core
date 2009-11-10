
/* ------------------------------------------------------------------------------
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
 */

/*
   File:	$Id:$
   Author:	Ladislav Slezák <lslezak@novell.com>
   Summary:     This is a generic tty wrapper
   Description: Starts a subprocess with a new internal tty input/output
		which is redirected to stdin/stdout. It's useful for starting programs
		which need a tty when no tty is available or for redirecting
		the console output to a file.
*/


#include "Process.h"
#include <iostream>

extern "C"
{
#include <sys/select.h>
#include <signal.h>
#include <errno.h>
}


// use atomic type in signal handler
static sig_atomic_t finish = false;

// NOTE: this is a signal handler, do only really necessary tasks here!
// be aware of non-reentrant functions!
void sig_chld_handler(int signal, siginfo_t *info, void *data)
{
    if (signal == SIGCHLD)
    {
	// set the finish flag for the main loop
	finish = true;
    }
}

// register SIGCHLD handler
bool registerSignalHandler()
{
    struct sigaction new_action, old_action;

    // use sa_sigaction parameter
    new_action.sa_flags = SA_SIGINFO;
    new_action.sa_sigaction = &sig_chld_handler;
    ::sigemptyset(&new_action.sa_mask);

    if (::sigaction(SIGCHLD, &new_action, &old_action))
    {
	std::cerr << "Cannot register SIGCHLD handler!\n";
	return false;
    }

    return true;
}

void output(const std::string & s, int fd)
{
    const char *cs = s.c_str();
    ssize_t n = s.size();
    while (true) {
	ssize_t w = write(fd, cs, n);
	if (w == n)
	    break;		// success
	else if (w == -1) {
	    if (errno == EINTR) {
		// perror("gotcha"); // bnc#470645
	    }
	    else {
		perror("write"); // other cases
		break;
	    }
	}
	else {
	    errno = 0;
	    perror("short");
	    cs += w;
	    n -= w;
	}
    }
}

int main(int argc, char **argv)
{
    if (argc > 1)
    {
	// register SIGCHLD handler
	if (!registerSignalHandler())
	{
	    return 1;
	}

	sigset_t new_sigset, old_sigset;

	// set asll signals
	::sigfillset(&new_sigset);
	// get the current mask
	::sigprocmask(0, NULL, &old_sigset);

	// block all signals
	::sigprocmask(SIG_SETMASK, &new_sigset, NULL);

	// start the subprocess
	Process subprocess(argv + 1, ExternalProgram::Environment(), true /* use pty */,
	    false /* no default locale */, false /* no LF to CRLF output translation*/);

	// check stdin, subprocess::stdout and subprocess::stderr
	int stdin_fd = 0;
	int sub_stdout_fd = fileno(subprocess.outputFile());
	int sub_stderr_fd = fileno(subprocess.errorFile());
	int max_fd = (sub_stdout_fd > sub_stderr_fd) ? sub_stdout_fd : sub_stderr_fd;

	// set nonblocking IO
	subprocess.setBlocking(false);

	fd_set rfds;

	// this is a buffer for reading from stdin
	char stdin_buffer[128];

	while (true)
	{
	    // block all signals
	    ::sigprocmask(SIG_SETMASK, &new_sigset, NULL);

	    if (finish)
	    {
		// read the remaining pieces
		std::string std_out(subprocess.read());
		output(std_out, 1);
		std::string err_out(subprocess.readErr());
		output(err_out, 2);
		// SIGCHLD received, return the exit status
		return subprocess.close();
	    }

	    FD_ZERO(&rfds);

	    // watch stdin (fd 0)
	    FD_SET(stdin_fd, &rfds);

	    // watch subprocess::stdout
	    FD_SET(sub_stdout_fd, &rfds);

	    // watch subprocess::stderr
	    FD_SET(sub_stderr_fd, &rfds);

	    // wait for an IO event or for a signal
	    int retval = ::pselect(max_fd + 1, &rfds, NULL, NULL, NULL, &old_sigset);

	    // enable signals back
	    ::sigprocmask(SIG_SETMASK, &old_sigset, NULL);

	    // ignore EINTR (Interrupted system call) caused by a signal
	    if (retval == -1 && errno != EINTR)
	    {
		::perror("pselect()");
		return 1;
	    }
	    // handle the input
	    else if (retval > 0)
	    {
		// stdin available
		if (FD_ISSET(stdin_fd, &rfds))
		{
		    // read a string from stdin and pass it to the subprocess
		    ssize_t len = read(stdin_fd, stdin_buffer, sizeof(stdin_buffer) - 1);

		    if (len > 0)
		    {
			// terminate the string in the buffer
			stdin_buffer[len + 1] = '\0';
			subprocess.send(stdin_buffer);
		    }
		}

		// stdout available
		if (FD_ISSET(sub_stdout_fd, &rfds))
		{
		    // read stdout of the subprocess and print it on stdout
		    std::string std_out(subprocess.read());
		    output(std_out, 1);
		}

		// stderr available
		if (FD_ISSET(sub_stderr_fd, &rfds))
		{
		    // read stderr of the subprocess and print it on stderr
		    std::string err_out(subprocess.readErr());
		    output(err_out, 2);
		}
	    }
	}
    }
    else
    {
	std::cerr << "tty_wrapper - Generic tty wrapper" << std::endl;
	std::cerr << "Error: No argument" << std::endl;
	std::cerr << "Usage: " << argv[0] << " <command> [<option1> [<option2> [....]]" << std::endl;
	return 1;
    }
}
