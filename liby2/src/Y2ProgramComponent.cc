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

   File:	Y2ProgramComponent.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
/*
 * Component that starts an external Y2 program
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "Y2ProgramComponent.h"
#include <ycp/Parser.h>
#include <ycp/y2log.h>

#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPCode.h>
#include <y2util/ExternalProgram.h>

Y2ProgramComponent::Y2ProgramComponent (string chroot_path, string bin_file,
					const char *component_name, bool non_y2,
					int level)
    : chroot_path (chroot_path),
      bin_file (bin_file),
      is_non_y2 (non_y2),
      component_name (component_name),
      argc (0),
      argv (0),
      pid (-1),
      level (level)
{
}


Y2ProgramComponent::~Y2ProgramComponent ()
{
    if (pid >= 0)
	terminateExternalProgram();
}


void Y2ProgramComponent::setServerOptions (int argc, char **argv)
{
    this->argc = argc;
    this->argv = argv;
}


YCPValue Y2ProgramComponent::evaluate(const YCPValue& command)
{
    if (pid == -1)   // server component not yet started --> do it
    {
	if (is_non_y2)   // this is a nony2 program like a shell and such like
	{
	    // Prepare options: argv[0] must be server component name,
	    // argv[1] must _NOT_ be 'stdio' like for real Y2 components (see below).
	    // If no options have been set so far via setServerOptions(),
	    // fill in argv[0] correctly.

	    int l_argc = 1 + (argc > 0 ? argc - 1 : 0);
	    char **l_argv = new char *[l_argc + 1];

	    l_argv[0] = (argc >= 1) ? argv[0] : strdup(name().c_str()); // component name
	    for (int arg = 1; arg < argc; arg++) l_argv[arg] = argv[arg];
	    l_argv[l_argc] = 0; // Terminate array

	    // launch program
	    launchExternalProgram(l_argv);

	    // NonY2 servers will not send arguments to the client (me).
	    // Therefore it is not necessary to receive something in this case.

	    if (argc < 1) free (l_argv[0]);
	    if (l_argv) delete[] l_argv;  // free l_argv
	}
	else   // this is a real liby2 component
	{
	    // Prepare options: argv[0] must be server component name,
	    // argv[1] must be 'stdio' in order to make the gf of the
	    // called component communicate via stdio, argv[2...] contains
	    // all further options. If no options have been set so far
	    // via setServerOptions(), fill in argv[0] correctly.

	    int l_argc = 3 + (argc > 0 ? argc - 1 : 0);
	    char **l_argv = new char *[l_argc + 1];

	    l_argv[0] = argc >= 1 ? argv[0] : strdup(name().c_str()); // component name
	    l_argv[1] = "stdio"; // I will speak to the server via his stdio
	    l_argv[2] = strdup(name().c_str());
	    for (int arg = 1; arg < argc; arg++) l_argv[arg+2] = argv[arg];
	    l_argv[l_argc] = 0; // Terminate array

	    // launch program
	    launchExternalProgram(l_argv);

	    // I am myself a module in this context. Therefore the server
	    // will send me my arguments. Since I initiated the session
	    // myself, I am not interested in these arguments.

	    if (receiveFromExternal().isNull())
	    {
		y2error ("Couldn't launch external server %s", name().c_str ());
		return YCPNull ();
	    }

	    if (argc < 1) free (l_argv[0]);
	    free (l_argv[2]);
	    delete[] l_argv;  // free l_argv
	}
    }

    // send command
    sendToExternal (command);

    // get answer
    YCPValue retval = receiveFromExternal();
    return !retval.isNull() ? retval : YCPVoid();
}


void Y2ProgramComponent::result(const YCPValue& result)
{
    // It may be, that no evaluate() call has been issued at all
    // before the call to this function(). This is likely to happen
    // in the context of the SCR. For each MountAgent() all it
    // creates the component that handles the certain path. But
    // if it may well be that some paths are not used in one
    // run of YaST2. In that case no evaluate() for that agent
    // has been issued. Therefore at this point the external
    // program may not have been started after all. So we need
    // to check, if it's running.
    
    if (pid != -1)
    {
	YCPTerm resultterm("result");
	resultterm->add(result);
	sendToExternal(resultterm);

        y2milestone ("Sending result: %s", resultterm->toString ().c_str ());

	terminateExternalProgram();
    }
}


string Y2ProgramComponent::name() const
{
    return component_name;
}


YCPValue Y2ProgramComponent::doActualWork(const YCPList& arglist, Y2Component *user_interface)
{
    int argc;      // this shadows the corresponding member variables for servers
    char **argv;

    if (is_non_y2)   // this is a nony2 program like a shell and such like
    {
	int arg = 0;

	// Prepare arguments as command line parameters for program being called
	argc = !arglist.isNull() ? arglist->size() + 1 : 1;
	argv = new char *[argc+1];
	argv[0] = strdup((name().c_str()));
	argv[argc] = NULL;

	for (arg = 1; arg < argc; arg++)
	{
	    YCPValue a = arglist->value(arg-1);
	    if (a->isString())
	    {
		argv[arg] = strdup(a->asString()->value().c_str());
	    }
	    else
	    {
		argv[arg] = strdup(a->toString().c_str());
	    }
	}

	// launch component if not yet done
	if (pid == -1) launchExternalProgram(argv);

	if (argv)
	{
	    for (arg = 0; arg < argc; arg++)
	    {
		if (argv[arg])
		{
		    free(argv[arg]);
		}
	    }
	    delete[] argv;
	}
    }
    else   // this is a real liby2 component
    {
	// send arguments via stdio
	argc = 4;   // name, -s
	argv = new char *[argc+1];
	argv[0] = strdup(name().c_str());
	argv[1] = argv[0];
	argv[2] = "-s";    // get arguments on stdin
	argv[3] = "stdio"; // communicate via stdio
	argv[argc] = NULL;

	// launch component if not yet done
	if (pid == -1) launchExternalProgram(argv);

	sendToExternal(arglist);   // now send arguments

	if (argv)
	{
	    if (argv[0]) free(argv[0]);
	    delete[] argv;
	}
    }

    // Communication loop with module. Module sends 'result(...)',
    // when finished
    YCPValue retval = YCPNull();
    YCPValue value = YCPNull();

    while (!(value = receiveFromExternal()).isNull())
    {
	if (value->isTerm()
	    && value->asTerm()->size() == 1
	    && value->asTerm()->name() == "result")
	{
	    retval = value->asTerm()->value(0);
	    y2debug ("Got result from client component %s: %s", name().c_str(), retval->toString().c_str());
	    break;
	}
	else
	{
	    // Send this to the UI and get UI answer
	    sendToExternal(user_interface->evaluate(value));
	}
    }

    return !retval.isNull() ? retval : YCPVoid();
}


void Y2ProgramComponent::launchExternalProgram (char **argv)
{
    y2debug ("launchExternalProgram (%s, %s, ...)", argv[0], argv[1]);
    // Create socket-pair
    pipe(to_external);
    pipe(from_external);

    // Create module process

    if (0 == (pid = fork()))   // child process
    {

	// Set component level for new program
	char levelstring[32];
	snprintf(levelstring, 32, "%d", level);
	setenv("Y2LEVEL", levelstring, 1); // 1: overwrite, if variable exists

	// child input
	ExternalProgram::renumber_fd (to_external[0], 0); // set reading end to stdin
	close(to_external[1]);     // writing end belongs to father process

	// child output
        ExternalProgram::renumber_fd (from_external[1], 1); // set writing end to stdout
	close(from_external[0]);   // reading end belongs to father process

	// Depending on whether it is a server or a client component
	// I prepare the arguments on a different way.

	// Call chroot if desired.
	if (chroot_path == "" || chroot_path == "/") {
	    //bnc#493152#c24
	    //y2debug ("Going to execute %s", bin_file.c_str ());
	} else {
	    /*y2debug ("Going to execute %s with chroot %s", bin_file.c_str (),
		     chroot_path.c_str ());*/
	    if (chroot (chroot_path.c_str ()) != 0) {
		/*y2error ("Cannot chroot to %s: %s", chroot_path.c_str (),
			 strerror (errno));*/
		_exit (5);
	    }

	    chdir ("/");
	}

	execv (bin_file.c_str (), argv);	// execute program

	// this code is only reached if exec failed
	//y2error ("Cannot execute external program %s", bin_file.c_str ());
	_exit (5); // No sense in returning! I am forked away!!
    }

    // father process

    close(to_external[0]);   // reading end belongs to child process
    close(from_external[1]); // writing end belongs to child process

    // Prepare parser
    parser.setInput(from_external[0], argv[0]);  // set parser input to child output
}


void Y2ProgramComponent::terminateExternalProgram()
{
    // We do not really kill the program here. We assume
    // that it is shut down gracefully and just close the
    // pipes and collect the zombie process. The real termination
    // thing is done by result() for server components and
    // automatically in doActualWork() for client components.

    if (pid >= 0)
    {
	close(to_external[1]);
	close(from_external[0]);

	// FIXME: this does not really wait in case of signals
	waitpid(pid, 0, 0); // Wait for child to exit
    }
    pid = -1;
}


YCPValue Y2ProgramComponent::receiveFromExternal ()
{
    while (true)
    {
	if (!externalProgramOK ())
	{
	    y2error ("External program %s died unexpectedly", bin_file.c_str ());
	    return YCPNull ();
	}

	YCodePtr c = parser.parse ();
	
	if (c == NULL || c->isError())
	{
	    y2error ("External program returned invalid data.");
	    return YCPNull ();
	}
	
	// evaluate, but not as constant
	YCPValue ret = c->evaluate (true);
	if (ret.isNull ())
	{
	    y2milestone ("External program returned executable code, executing");
	    ret = c->evaluate (false);
	}

	return ret;
    }
}


void Y2ProgramComponent::sendToExternal(const YCPValue& value)
{
    if (!externalProgramOK())
    {
	y2error ("External program %s died unexpectedly", bin_file.c_str());
    }

    char *v = NULL;

    if (is_non_y2)  v = strdup((value->toString()).c_str());   // no brackets
    else            v = strdup(("(" + value->toString() + ")").c_str());

    bool error = (write(to_external[1], v, strlen(v)) < 0);
    if (error)
    {
	y2debug ("Error writing to external program %s: Couldn't send %s (%s)", bin_file.c_str(), v, strerror (errno));
	terminateExternalProgram();
    }
    free(v);

    // We send an additional linefeed. This makes it more conveniant for non
    // Y2 programs, for example that shell can do a read to get one value. For
    // Y2 programs it increases the readability if you want to dump and debug
    // the whole stream.

    // We MUST NOT trigger an error, if the sending of the linefeed is not
    // successful. This sporadically happens after we send a module the last
    // return just before the module has done its work and terminates. It then
    // justs sends the result(..) message, and closes down without reading the
    // linefeed. The pipe breaks down and the sending fails. But as long as we
    // don't collect the process by calling wait4, we can still read the
    // result (..) from the input pipe, which is very important. Otherwise the
    // result value would be dropped.

    write(to_external[1], "\n", 1);
}


void Y2ProgramComponent::sendToExternal(const string& value)
{
    if (!externalProgramOK())
    {
	y2error ("External program %s died unexpectedly", bin_file.c_str());
    }

    char *v = NULL;

    if (is_non_y2)  v = strdup(value.c_str());   // no brackets
    else            v = strdup(("(" + value + ")").c_str());

    bool error = (write(to_external[1], v, strlen(v)) < 0);
    if (error)
    {
	y2debug ("Error writing to external program %s: Couldn't send %s (%s)", bin_file.c_str(), v, strerror (errno));
	terminateExternalProgram();
    }
    free(v);

    // We send an additional linefeed. This makes it more conveniant for non
    // Y2 programs, for example that shell can do a read to get one value. For
    // Y2 programs it increases the readability if you want to dump and debug
    // the whole stream.

    // We MUST NOT trigger an error, if the sending of the linefeed is not
    // successful. This sporadically happens after we send a module the last
    // return just before the module has done its work and terminates. It then
    // justs sends the result(..) message, and closes down without reading the
    // linefeed. The pipe breaks down and the sending fails. But as long as we
    // don't collect the process by calling wait4, we can still read the
    // result (..) from the input pipe, which is very important. Otherwise the
    // result value would be dropped.

    write(to_external[1], "\n", 1);
}


bool Y2ProgramComponent::externalProgramOK() const
{
    if (pid == -1) return false;
    else return kill(pid, 0) == 0;
}


bool Y2ProgramComponent::remote() const
{
    return true;
}
