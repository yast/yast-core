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

   File:       ExternalProgram.cc

   Author:     Andreas Schwab <schwab@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
#define _GNU_SOURCE 1 // for ::getline

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pty.h> // openpty

#include "ExternalProgram.h"
#include <ycp/y2log.h>


ExternalProgram::ExternalProgram (string commandline,
				  Stderr_Disposition stderr_disp, bool use_pty,
				  int stderr_fd)
    : use_pty (use_pty)
{
    const char *argv[4];
    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = commandline.c_str();
    argv[3] = 0;
    start_program (argv, stderr_disp, stderr_fd);
}


ExternalProgram::ExternalProgram (const char *const *argv,
				  Stderr_Disposition stderr_disp, bool use_pty,
				  int stderr_fd)
    : use_pty (use_pty)
{
    start_program (argv, stderr_disp, stderr_fd);
}


ExternalProgram::ExternalProgram (const char *binpath, const char *const *argv_1,
				  bool use_pty)
    : use_pty (use_pty)
{
    int i = 0;
    while (argv_1[i++])
	;
    const char *argv[i + 1];
    argv[0] = binpath;
    memcpy (&argv[1], argv_1, (i - 1) * sizeof (char *));
    start_program (argv);
}


ExternalProgram::ExternalProgram (const YCPList &args, const char *binpath,
				  bool use_pty)
    : use_pty (use_pty)
{
    const char *argv[args->size() + 1];
    if (binpath == 0)
    {
	if (!args->isTerm()) {
	    y2error ("Fatal error in %s: args is not a YCPTerm",
		     __PRETTY_FUNCTION__);
	    abort();
	}
	binpath = args->asTerm()->symbol()->symbol_cstr();
    }
    argv[0] = binpath;
    int i;
    for (i = 0; i < args->size(); i++)
	argv[i + 1] = args->value(i)->asString()->value_cstr();
    argv[i] = 0;
    start_program (argv);
}


ExternalProgram::~ExternalProgram()
{
}


void
ExternalProgram::start_program (const char *const *argv, Stderr_Disposition
				stderr_disp, int stderr_fd)
{
    pid = -1;
    int to_external[2], from_external[2];  // fds for pair of pipes
    int master_tty,	slave_tty;	   // fds for pair of ttys

    if (use_pty)
    {
	// Create pair of ttys
	y2debug ("Using ttys for communication with %s", argv[0]);
	if (openpty (&master_tty, &slave_tty, 0, 0, 0) != 0)
	{
	    y2error ("openpty failed: %m");
	    return;
	}
    }
    else
    {
	// Create pair of pipes
	if (pipe (to_external) != 0 || pipe (from_external) != 0)
	{
	    y2error ("pipe failed: %m");
	    return;
	}
    }

    // Create module process
    if ((pid = fork()) == 0)
    {
	if (use_pty)
	{
	    setsid();
	    dup2   (slave_tty, 0);	  // set new stdin
	    dup2   (slave_tty, 1);	  // set new stdout
	    ::close(slave_tty);		  // dup2 has duplicated it
	    ::close(master_tty);	  // Belongs to father process

	    // We currently have no controlling terminal (due to setsid).
	    // The first open call will also set the new ctty (due to historical
	    // unix guru knowledge ;-) )

	    char name[512];
	    ttyname_r(slave_tty, name, sizeof(name));
	    ::close(open(name, O_RDONLY));
	}
	else
	{
	    dup2   (to_external	 [0], 0); // set new stdin
	    ::close(to_external	 [0]);	  // dup2 has duplicated it
	    ::close(from_external[0]);	  // Belongs to father process

	    dup2   (from_external[1], 1); // set new stdout
	    ::close(from_external[1]);	  // dup2 has duplicated it
	    ::close(to_external	 [1]);	  // Belongs to father process
	}

	// Handle stderr
	if (stderr_disp == Discard_Stderr)
	{
	    int null_fd = open("/dev/null", O_WRONLY);
	    dup2(null_fd, 2);
	    ::close(null_fd);
	}
	else if (stderr_disp == Stderr_To_Stdout)
	{
	    dup2(1, 2);
	}
	else if (stderr_disp == Stderr_To_FileDesc)
	{
	    // Note: We don't have to close anything regarding stderr_fd.
	    // Our caller is responsible for that.
	    dup2 (stderr_fd, 2);
	}

	y2debug ("Going to execute %s", argv[0]);

	execvp(argv[0], const_cast<char *const *>(argv));
	y2error ("Cannot execute external program %s: %s",
		 argv[0], strerror(errno));
	_exit (5);			// No sense in returning! I am forked away!!
    }

    else if (pid == -1)	 // Fork failed, close everything.
    {
	if (use_pty) {
	    ::close(master_tty);
	    ::close(slave_tty);
	}
	else {
	    ::close(to_external[0]);
	    ::close(to_external[1]);
	    ::close(from_external[0]);
	    ::close(from_external[1]);
	}
	y2error ("Cannot fork: %s", strerror(errno));
    }

    else {
	if (use_pty)
	{
	    ::close(slave_tty);	       // belongs to child process
	    inputfile  = fdopen(master_tty, "r");
	    outputfile = fdopen(master_tty, "w");
	}
	else
	{
	    ::close(to_external[0]);   // belongs to child process
	    ::close(from_external[1]); // belongs to child process
	    inputfile = fdopen(from_external[0], "r");
	    outputfile = fdopen(to_external[1], "w");
	}

	if (!inputfile || !outputfile)
	{
	    y2error ("Cannot create streams to external program %s", argv[0]);
	    close();
	}
    }
}


int
ExternalProgram::close()
{
    int status = 0;
    if (pid > 0)
    {
	ExternalDataSource::close();
	// Wait for child to exit
	int ret;
	do
	{
	    ret = waitpid(pid, &status, 0);
	    y2debug ("waitpid called");
	}
	while (ret == -1 && errno == EINTR);

	if (ret != -1)
	{
	    if (WIFEXITED (status))
		status = WEXITSTATUS (status);
	    else if (WIFSIGNALED (status))
		status = WTERMSIG (status) + 128;
	    else if (WCOREDUMP (status)) {
		y2error ("Fatal error in %s: external program exited with core dump",
			 __PRETTY_FUNCTION__);
		abort ();
	    } else {
		y2error ("Fatal error in %s: external program exited with unknown error",
			 __PRETTY_FUNCTION__);
		abort ();
	    }
	}
    }
    pid = -1;
    return status;
}


bool
ExternalProgram::kill()
{
    if (pid > 0)
    {
	::kill(pid, SIGKILL);
	close();
    }
    return true;
}


bool
ExternalProgram::running()
{
    return pid != -1 && ::kill(pid, 0) == 0;
}
