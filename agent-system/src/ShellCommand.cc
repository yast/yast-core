/*
 * ShellCommand.cc
 *
 * Functions for running shell commands with output attached to y2log
 *
 * Author: Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#include <errno.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define y2log_component "bash"
#include "ycp/y2log.h"

#include "ShellCommand.h"
#include <y2util/ExternalProgram.h>


/**
 * Execute shell command and feed its output to y2log
 */
int
shellcommand (const string &command, const string &tempdir)
{
    y2debug ("shellcommand start");

    int ret = 0;

    int pipe1[2];
    if (pipe (pipe1))
    {
	y2error ("pipe failed, errno: %d", errno);
	return -1;
    }

    fflush (0);
    pid_t log2 = fork ();

    if (log2 == -1)
    {
	y2error ("fork failed, errno: %d", errno);
	return -1;
    }
    else if (log2)
    {
	/* parent: collect stderr */

	FILE *err = 0;
	if (!tempdir.empty ())
	    err = fopen ((tempdir + "/stderr").c_str (), "w");

	char s1[1024];
	FILE *stream1 = fdopen (pipe1[0], "r");
	if (stream1 == 0)
	{
	    y2error ("stream1 fdopen error");
	    return -1;
	}
	close (pipe1[1]);

	while (fgets (s1, 1024, stream1))
	{
	    if (err)
		fputs (s1, err);

	    /* Yes, this should be y2error but some external
	     * programs print normal messages via stderr
	     * bug reports are filed but who knows if such programs
	     * ever get fixed. Thus we set this temporarily to
	     * y2warning.
	     */
	    y2error ("%s", s1);
	}

	fclose (stream1);

	if (err)
	    fclose (err);
    }
    else
    {
	/* 1st child: fork the command and collect stdout */

	ExternalProgram::renumber_fd (pipe1[1], 2);
	close (pipe1[0]);

	int pipe2[2];
	if (pipe (pipe2))
	{
	    y2error ("pipe failed, errno: %d", errno);
	    _exit (1);
	}

	fflush (0);
	pid_t child = fork ();

	if (child == -1)
	{
	    y2error ("fork failed, errno: %d", errno);
	    _exit (1);
	}
	else if (child)
	{
	    /* parent (1st child): collect stdout */

	    FILE *out = 0;
	    if (!tempdir.empty ())
		out = fopen ((tempdir + "/stdout").c_str (), "w");

	    char s2[1024];
	    FILE *stream2 = fdopen (pipe2[0], "r");
	    if (stream2 == 0)
	    {
	        y2error ("stream2 fdopen error");
		_exit (1);
	    }
	    close (pipe2[1]);

	    while (fgets (s2, 1024, stream2))
	    {
		if (out)
		    fputs (s2, out);
		//bnc#493152#c24
		//y2debug ("%s", s2);
	    }

	    fclose (stream2);

	    if (out)
		fclose (out);

	}
	else
	{
	    /* child of child: run the command */

	    ExternalProgram::renumber_fd (pipe2[1], 1);
	    close (pipe2[0]);

	    // #223602 #441054
	    // close all file descriptors above stderr
	    for ( int i = getdtablesize() - 1; i > 2; --i ) {
		close( i );
	    }
	    
	    ret = system (command.c_str ());
	    if (WIFEXITED (ret))
		ret = WEXITSTATUS (ret);
	    else
		ret = WTERMSIG (ret) + 128;

	    //y2debug ("Exit status is %d", ret);

	    if (!tempdir.empty ())
	    {
		FILE *ex = fopen ((tempdir + "/exit").c_str (), "w");
		if (ex)
		{
		    fprintf (ex, "%d\n", ret);
		    fclose (ex);
		}
	    }

	    _exit (ret);
	}

	waitpid (child, &ret, 0);
	if (WIFEXITED (ret))
	    _exit (WEXITSTATUS (ret));
	_exit (WTERMSIG (ret) + 128);
    }

    waitpid (log2, &ret, 0);

    y2debug ("shellcommand end");

    if (WIFEXITED (ret))
	return WEXITSTATUS (ret);
    return WTERMSIG (ret) + 128;
}


/**
 * Execute shell command in background. That means start and forget
 * about it!
 */
int
shellcommand_background (const string &command)
{

    if (!getenv ("Y2DEBUGSHELL") && !getenv ("Y2DEBUGALL"))
    {
	system (string (command + " >/dev/null 2>&1 &").c_str ());
	// FIXME execl ("/bin/sh", "-c", command.c_str(), NULL);
	return 0;
    }

    /* fork the child */
    pid_t child = fork ();
    if (child == -1)
    {
	y2error("fork failed: %s", strerror(errno));
	return -1;
    }

    /* child process */
    if (!child)
    {
	shellcommand (command, "");
	_exit (0);
    }

    /* return success */
    return 0;
}


#ifdef _MAIN

/*
 * compile with:
 *   g++ -D_MAIN -I /usr/include/YaST2/ ShellCommand.cc -lycp -o shellcommand
 *   ./shelltest
 *
 * output should be:
 *
 * Oret=123456
 * Eret=123456
 * 2002-03-27 19:55:12 <0> beholder(17049) [bash] ShellCommand.cc(main):158 START
 * 2002-03-27 19:55:12 <3> beholder(17049) [bash] ShellCommand.cc(shellcommand):52 y2y
 * 2002-03-27 19:55:12 <0> beholder(17050) [bash] ShellCommand.cc(shellcommand):77 x1x
 * 2002-03-27 19:55:12 <0> beholder(17051) [bash] ShellCommand.cc(shellcommand):95 ret=3
 * 2002-03-27 19:55:12 <0> beholder(17049) [bash] ShellCommand.cc(main):162 --ret=3
 * Oret=3
 * Eret=3
 *
 */

int
main ()
{
    int r = 123456;
    fprintf (stdout, "Oret=%d\n", r);
    fprintf (stderr, "Eret=%d\n", r);

    set_log_filename ("-");
    y2debug ("START");

    //r = shellcommand_background("sleep 5; echo x1x; echo y2y >&2; exit 3;");
    //r = shellcommand("echo x1x; echo y2y >&2; exit 3;", "/tmp");
    r = shellcommand ("echo x1x; echo y2y >&2; exit 3;");
    y2debug ("--ret=%d", r);

    fprintf (stdout, "Oret=%d\n", r);
    fprintf (stderr, "Eret=%d\n", r);
    return r;
}

#endif
