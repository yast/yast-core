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

   File:       YCPDebugger.cc

   Author:     Arvin Schnell <arvin@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/

/*
 * Debugger for YCP
 */


#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

#include "YCPBasicInterpreter.h"
#include "YCPScope.h"
#include "YCPScopeInstance.h"
#include "YCPDebugger.h"
#include "y2log.h"


YCPDebugger::YCPDebugger (bool wait_for_frontend)
    : wait_for_frontend (wait_for_frontend)
{
    y2debug ("constructor");

    unique_id_cnt = 0;
    sock = fd = -1;

    create_socket ();
}


YCPDebugger::~YCPDebugger ()
{
    y2debug ("destructor");

    if (fd != -1) {
	write_line ("All interpreters finished, bye.");
	close (fd);
    }

    shutdown (sock, 2);
}


bool
YCPDebugger::ignore (EntryPoint entrypoint, const YCPElement &element)
{
    switch (entrypoint) {

    case BasicInterpreter: {

	const YCPValue value = element->asValue ();
	if (!value->isBlock ())
	    return false;

	const YCPBlock block = value->asBlock ();
	if (block->size () != 1)
	    return false;

	const YCPStatement statement = block->statement (0);
	if (!statement->isBuiltinStatement ())
	    return false;

	const YCPBuiltinStatement builtinstatement = statement->asBuiltinStatement ();
	if (builtinstatement->code () != YCPB_FULLNAME)
	    return false;

	return true;

    } break;

    case Block: {

	const YCPStatement statement = element->asStatement ();
	if (!statement->isBuiltinStatement ())
	    return false;

	const YCPBuiltinStatement builtinstatement = statement->asBuiltinStatement ();
	if (builtinstatement->code () != YCPB_FULLNAME)
	    return false;

	return true;

    } break;

    }

    return false;
}

// On a Ctrl-C, the telnet client sends
// IAC InterruptProcess IAC DO TimingMark
static const char *telnet_break = "\xff\xf4\xff\xfd\x06";
// So we reply IAC WON'T TimingMark, otherwise the client gets confused.
static const char *telnet_wont_timing_mark = "\xff\xfc\x06";
// Of course this is a quick hack, but it works.
// See also RFC 854, RFC 860.

void
YCPDebugger::debug (YCPBasicInterpreter *inter, EntryPoint entrypoint,
		    const YCPElement &element)
{
    Interpreter *act = find_inter (inter);

    if (settings.ignorescr && act->interpreter->interpreter_name () == "SCR")
	return;

    if (ignore (entrypoint, element))
	return;

    if (fd == -1) {
	if (wait_for_frontend) {
	    const char msg[] = "waiting for debugger frontend to connect";
	    y2debug (msg);
	    fprintf (stdout, "%s\n", msg);
	}
	check_socket (wait_for_frontend);
	if (wait_for_frontend) {
	    const char msg[] = "debugger frontend has connected";
	    y2debug (msg);
	    fprintf (stdout, "%s\n", msg);
	}
	wait_for_frontend = false;

	// Set default values.
	if (fd != -1) {
	    single_mode = true;
	    hold_level = -1;
	    leave_position.setpos ("", -1);
	    close_request = false;
	    last_command = "";
	    settings.reset ();
	}
    }

    if (fd == -1)
	return;

    bool goon = !single_mode;

    if (goon && leave_position.line != -1)
	if (act->interpreter->current_line != leave_position.line ||
	    act->interpreter->current_file != leave_position.file)
	    goon = false;

    if (goon && hold_level != -1)
	if (hold_level <= inter->scopeLevel) {
	    goon = false;
	    write_line ("hold level reached");
	}

    if (goon)
	if (check_breakpoints (act->interpreter->current_file,
			       act->interpreter->current_line)) {
	    goon = false;
	    write_line ("breakpoint reached");
	}

    // Check for "interrupt" command. Note: All other commands are
    // discarded, thus this must be the last check. Kind of ugly!
    if (goon) {
	string cmd = read_line (false);
	if (cmd == "interrupt" || cmd == "I" || cmd == "\003")
	    goon = false;
	else if (cmd == telnet_break)
	{
	    goon = false;
	    write_line (telnet_wont_timing_mark);
	}
    }

    /*
     * Avoid getting stuck in breakpoints: Normally, when we reach a
     * breakpoint and say continue, it is very likely we immediately get
     * to the same breakpoint again without leaving the source line. So
     * the user has to say continue several times which is cumbersome.
     *
     * To solve this problem, we temporarily deactivate the breakpoint
     * we reached. We reactivate it when we leave the source line.
     */

    // Reactivate breakpoints.
    for (vector <Breakpoint>::iterator cur = breakpoints.begin ();
	 cur != breakpoints.end (); cur++)
	if ((act->interpreter->current_line != (*cur).line) ||
	    (act->interpreter->current_file != (*cur).file))
	    (*cur).tmpinactive = false;

    if (goon)
	return;

    // Somehow we stopped.

    write_line ("interpreter: #%d %s %d", act->unique_id,
		act->interpreter->interpreter_name ().c_str (),
		entrypoint);

    write_line ("position: %s %d",
		act->interpreter->current_file.c_str (),
		act->interpreter->current_line);

    if (settings.printtoken) {
	write_line ("BEGIN TOKEN");
	write_line ("%s", element->toString ().c_str ());
	write_line ("END TOKEN");
    }

    // Reset some stop conditions.
    single_mode = false;
    hold_level = -1;
    leave_position.setpos ("", -1);

    // Deactivate breakpoint.
    for (vector <Breakpoint>::iterator cur = breakpoints.begin ();
	 cur != breakpoints.end (); cur++)
	if ((act->interpreter->current_line == (*cur).line) &&
	    (act->interpreter->current_file == (*cur).file))
	    (*cur).tmpinactive = true;

    // Loop over user input.
    do {
	write_prompt ();
	string command = read_line (true);
	if (command.empty () && !last_command.empty ())
	    command = last_command;
	goon = handle_command (act, command, element);
	last_command = command;
    } while (!goon);

    if (close_request) {
	close (fd);
	fd = -1;
    }
}


static int
split (const string& in, int n, string* out, const char* delim)
{
    // this ain't the fastest split implementation

    int i = 0;

    const string::size_type l = in.length ();
    string::size_type a = 0;

    while (true)
    {
        string::size_type b = in.find_first_of (delim, a);
        if (a != b && a != l)
            out[i++] = in.substr (a, b - a);
        if (b == string::npos || i == n)
            return i;
        a = b + 1;
    }
}


bool
YCPDebugger::handle_command (Interpreter *inter, const string &command,
			     const YCPElement &element)
{
    string args[5];
    int nargs = split (command, 5, args, " \t");

    if (nargs == 1 && (args[0] == "interrupt" || args[0] == "I" ||
		       args[0] == "\003")) {
	return false;
    }

    if (nargs == 1 && args[0] == telnet_break) {
	write_line (telnet_wont_timing_mark);
	return false;
    }

    if (nargs == 1 && (args[0] == "continue" ||
		       args[0] == "c" || args[0] == "fg")) {
	return true;
    }

    if (nargs == 1 && (args[0] == "single" ||
		       args[0] == "stepi" || args[0] == "si")) {
	single_mode = true;
	return true;
    }

    if (nargs == 1 && (args[0] == "step" || args[0] == "s")) {
	leave_position.setpos (inter->interpreter->current_file,
			       inter->interpreter->current_line);
	return true;
    }

    if (nargs == 1 && (args[0] == "next" || args[0] == "n")) {
	hold_level = inter->interpreter->scopeLevel;
	return true;
    }

    if (nargs == 1 && args[0] == "up") {
	hold_level = inter->interpreter->scopeLevel - 1;
	return true;
    }

    if (nargs == 1 && args[0] == "detach") {
	close_request = true;
	return true;
    }

    if (nargs == 2 && (args[0] == "set" || args[0] == "unset")) {
	bool tmp = args[0] == "set";
	if (args[1] == "ignorescr") {
	    settings.ignorescr = tmp;
	    return false;
	}
	if (args[1] == "printtoken") {
	    settings.printtoken = tmp;
	    return false;
	}
    }

    if (nargs == 1 && args[0] == "token") {
	write_line ("BEGIN TOKEN");
	write_line ("%s", element->toString ().c_str ());
	write_line ("END TOKEN");
	return false;
    }

    // scope [#<interp_no>] [<namespace>]
    if (nargs >= 1 && args[0] == "scope") {
	Interpreter * tmp = inter;
	int scopearg = 1;
	string scopename = "";

	if (nargs == 2 && args[1][0] == '#') {	
	    int unique_id = atoi (args[1].substr (1).c_str ());
	    tmp = find_inter (unique_id);
	    scopearg = 2;
	}

	if (nargs > scopearg) {
	    scopename = args[scopearg];
	}

	if (tmp)
	    print_scope (tmp, scopename);
	else
	    write_line ("unknown interpreter");
	return false;
    }

    // scopes [#<interp_no>]
    if (nargs >= 1 && args[0] == "scopes") {
	Interpreter * tmp = inter;

	if (nargs == 2 && args[1][0] == '#') {	
	    int unique_id = atoi (args[1].substr (1).c_str ());
	    tmp = find_inter (unique_id);
	}

	if (tmp) {
	    YCPScope *s = tmp->interpreter;
	    write_line ("scopeLevel\t%d", s->scopeLevel);
	    write_line ("scopeGlobal\t%p", s->scopeGlobal);
	    write_line ("scopeTop\t%p", s->scopeTop);

	    if (s->instances) {
		YCPScope::YCPScopeInstanceContainer::iterator
		    i = s->instances->begin (),
		    e = s->instances->end ();
		for (; i != e; ++i) {
		    write_line ("%s\t%p", i->first.c_str(), i->second);
		}
	    }
	    else
		write_line ("no instances");
	}
	else
	    write_line ("unknown interpreter");
	return false;
    }

    // {p|print} [#<interp_no>] [<namespace>::]<variable>
    if (nargs >= 2 && (args[0] == "print" || args[0] == "p")) {
	Interpreter * tmp = inter;
	int vararg = 1;
	string varname;
	string scopename = "";

	if (args[1][0] == '#') {
	    int unique_id = atoi (args[1].substr (1).c_str ());
	    tmp = find_inter (unique_id);
	    vararg = 2;
	}

	if (nargs > vararg) {
	    varname = args[vararg];
	}
	else {
	    write_line ("missing variable name");
	    return false;
	}

	string::size_type dcpos = varname.find ("::");
	if (dcpos != string::npos) {
	    scopename = varname.substr (0, dcpos);
	    varname = varname.substr (dcpos + 2);
	    if (scopename.empty ())
		scopename = "_"; // explicit global namespace
	}

	if (tmp)
	    print_variable (tmp, varname, scopename);
	else
	    write_line ("unknown interpreter");
	return false;
    }

    if (nargs == 2 && (args[0] == "list" || args[0] == "l")) {
	if (args[1] == "breakpoints" || args[1] == "b") {
	    list_breakpoints ();
	    return false;
	}
	if (args[1] == "interpreters" || args[1] == "i") {
	    list_interpreters ();
	    return false;
	}
	if (args[1] == "source" || args[1] == "s") {
	    list_source (inter->interpreter->current_file.c_str ());
	    return false;
	}
    }

    if (nargs == 3 && (args[0] == "break" || args[0] == "b")) {
	add_breakpoint (args[1], strtol (args[2].c_str (), 0, 10));
	list_breakpoints ();
	return false;
    }

    if (nargs == 3 && args[0] == "clear") {
	if (delete_breakpoint (args[1], strtol (args[2].c_str (), 0, 10))) {
	    list_breakpoints ();
	    return false;
	}
	write_line ("breakpoint not found");
	return false;
    }

    write_line ("wrong command \"%s\"", command.c_str ());
    return false;
}


bool
YCPDebugger::print_variable (Interpreter *inter, const string &name, const string& scopename)
{
    YCPValue value = inter->interpreter->lookupValue (name, scopename);
    if (value.isNull ()) {
	write_line ("variable not found");
	return false;
    }

    write_line ("BEGIN VARIABLE");
    write_line ("%s", value->toString ().c_str ());
    write_line ("END VARIABLE");
    return true;
}


void
YCPDebugger::print_scope (Interpreter *inter, const string& scopename)
{
    const YCPScopeInstance *instance = inter->interpreter->scopeTop;

    if (!scopename.empty())
    {
	if (scopename == "_")
	    instance = inter->interpreter->scopeGlobal;
	else
	    instance = inter->interpreter->lookupInstance (scopename);
    }

    write_line ("BEGIN SCOPE");

    while (instance && instance->container)
    {
	YCPScopeInstance::YCPScopeContainer::const_iterator var;

	for (var = instance->container->begin ();
	     var != instance->container->end (); var++)
	{
	    YCPDeclaration decl = var->second->get_type ();

	    // truncate length of variable
	    const int size = 60;
	    char buffer[size];
	    snprintf (buffer, size, "%s", var->second->get_value ()->
		      toString ().c_str ());

	    write_line ("%d %s %s = %s", instance->level,
			decl->toString ().c_str (),
			decl->isDeclTerm () ? "" : var->first.c_str (),
			buffer);

	}

	instance = instance->outer;
    }

    write_line ("END SCOPE");
}


YCPDebugger::Interpreter *
YCPDebugger::add_interpreter (YCPBasicInterpreter *inter)
{
    Interpreter tmp;
    tmp.interpreter = inter;
    tmp.unique_id = unique_id_cnt++;
    interpreters.push_back (tmp);
    return &interpreters.back ();
}


void
YCPDebugger::delete_interpreter (YCPBasicInterpreter *inter)
{
    for (vector <Interpreter>::iterator cur = interpreters.begin ();
	 cur != interpreters.end (); cur++)
	if ((*cur).interpreter == inter) {
	    interpreters.erase (cur);
	    return;
	}

    y2error ("fatal error: unknown interpreter");
    abort ();
}


int
YCPDebugger::number_of_interpreters () const
{
    return interpreters.size ();
}


void
YCPDebugger::list_interpreters ()
{
    write_line ("BEGIN INTERPRETERS");
    for (vector <Interpreter>::iterator cur = interpreters.begin ();
	 cur != interpreters.end (); cur++)
	write_line ("#%d %s", (*cur).unique_id,
		    (*cur).interpreter->interpreter_name ().c_str ());
    write_line ("END INTERPRETERS");
}


YCPDebugger::Interpreter *
YCPDebugger::find_inter (YCPBasicInterpreter *inter)
{
    for (vector <Interpreter>::iterator cur = interpreters.begin ();
	 cur != interpreters.end (); cur++)
	if ((*cur).interpreter == inter)
	    return &(*cur);

    return add_interpreter (inter);
}


YCPDebugger::Interpreter *
YCPDebugger::find_inter (int unique_id)
{
    for (vector <Interpreter>::iterator cur = interpreters.begin ();
	 cur != interpreters.end (); cur++)
	if ((*cur).unique_id == unique_id)
	    return &(*cur);

    return 0;
}


void
YCPDebugger::add_breakpoint (const string &add_file, int add_line)
{
    Breakpoint tmp;
    tmp.setpos (add_file, add_line);
    tmp.tmpinactive = false;
    breakpoints.push_back (tmp);
}


bool
YCPDebugger::delete_breakpoint (const string &delete_file, int delete_line)
{
    for (vector <Breakpoint>::iterator cur = breakpoints.begin ();
	 cur != breakpoints.end (); cur++)
	if ((delete_line == (*cur).line) && (delete_file == (*cur).file)) {
	    breakpoints.erase (cur);
	    return true;
	}

    return false;
}


bool
YCPDebugger::check_breakpoints (const string &check_file, int check_line)
{
    for (vector <Breakpoint>::iterator cur = breakpoints.begin ();
	 cur != breakpoints.end (); cur++)
	if (!(*cur).tmpinactive && (check_line == (*cur).line) &&
	    (check_file == (*cur).file))
	    return true;

    return false;
}


void
YCPDebugger::list_breakpoints ()
{
    write_line ("BEGIN BREAKPOINTS");
    for (vector <Breakpoint>::iterator cur = breakpoints.begin ();
	 cur != breakpoints.end (); cur++)
	write_line ("%s %d", (*cur).file.c_str (),
		    (*cur).line);
    write_line ("END BREAKPOINTS");
}


void
YCPDebugger::list_source (const char * filename)
{
    const int size = 250;
    char buffer[size];

    FILE * fin = fopen (filename, "r");
    if (!fin) {
	y2debug ("can't open %s", filename);
	write_line ("error reading source file");
	return;
    }

    write_line ("BEGIN SOURCE");
    while (fgets (buffer, size, fin)) {
	buffer[strlen (buffer) - 1] = '\0';
	write_line ("%s", buffer);
    }
    write_line ("END SOURCE");
}


void
YCPDebugger::create_socket ()
{
    // Create the socket.
    sock = socket (PF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
	y2error ("fatal error: socket failed: %m");
	exit (EXIT_FAILURE);
    }

    // Give the socket a name.
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons (19473);
    name.sin_addr.s_addr = htonl (INADDR_ANY);
    if (bind (sock, (struct sockaddr *) &name,
	      sizeof (struct sockaddr_in)) < 0) {
	y2error ("fatal error: bind failed: %m");
	fprintf (stderr, "fatal error: bind failed: %m\n");
	exit (EXIT_FAILURE);
    }

    // Listen on socket.
    if (listen (sock, 1) < 0) {
	y2error ("fatal error: listen failed: %m");
	exit (EXIT_FAILURE);
    }
}


void
YCPDebugger::check_socket (bool block)
{
    fd_set read_fd_set;
    FD_ZERO (&read_fd_set);
    FD_SET (sock, &read_fd_set);

    struct timeval timeout;
    timeout.tv_sec = block ? 604800 : 0;
    timeout.tv_usec = 0;

    int res;
    do {
	res = select (FD_SETSIZE, &read_fd_set, 0, 0, &timeout);
    } while (res == -1 && errno == EINTR);

    if (res < 0) {
	y2error ("fatal error: select failed: %m");
	exit (EXIT_FAILURE);
    }

    // Test for timeout.
    if (res == 0)
	return;

    // Connection request on original socket.
    struct sockaddr_in clientname;
    socklen_t size = sizeof (sockaddr_in);
    int status = accept (sock, (struct sockaddr *) &clientname, &size);
    if (status < 0) {
	y2error ("fatal error: accept failed: %m");
	exit (EXIT_FAILURE);
    }

    y2debug ("connect from host %s", inet_ntoa (clientname.sin_addr));

    if (fd != -1) {
	close (status);
	y2debug ("but we already have a dubugger attached");
	return;
    }

    fd = status;

    write_line ("Welcome to the YCP debugger.");
}


string
YCPDebugger::read_line (bool block) const
{
    const int maxmsg = 512;
    char buffer[maxmsg + 1];

    fd_set read_fd_set;
    FD_ZERO (&read_fd_set);
    FD_SET (fd, &read_fd_set);

    struct timeval timeout;
    timeout.tv_sec = block ? 604800 : 0;
    timeout.tv_usec = 0;

    int res;
    do {
	res = select (FD_SETSIZE, &read_fd_set, 0, 0, &timeout);
    } while (res == -1 && errno == EINTR);

    // Test for timeout.
    if (res == 0)
	return "";

    int nbytes = read (fd, buffer, maxmsg);

    // Read error.
    if (nbytes < 0) {
	y2error ("error: read failed: %m");
	return "";
    }

    // End of file.
    if (nbytes == 0) {
	return ""; // ???
    }

    // append '\0'
    buffer[nbytes] = '\0';

    while (buffer[nbytes - 1] == '\n' || buffer[nbytes - 1] == '\r')
	buffer[--nbytes] = '\0';

    return buffer;
}


void
YCPDebugger::write_line (const char *format, ...) const
{
    char *ptr;

    va_list ap;
    va_start (ap, format);
    const int size = vasprintf (&ptr, format, ap) + 3;
    va_end (ap);

    // replace all newlines by blanks
    for (char *tmp = ptr; *tmp != '\0'; tmp++)
	if (*tmp == '\n')
	    *tmp = ' ';

    // append line ending "\r\n"
    ptr = (char *) realloc (ptr, size);
    strcat (ptr, "\r\n");

    // don't write the trailing '\0'
    if (write (fd, ptr, size - 1) != size - 1) {
	y2error ("fatal error: write failed: %m");
	exit (EXIT_FAILURE);
    }

    free (ptr);
}


void
YCPDebugger::write_prompt () const
{
#if 0
    char *tmp = "y2d: ";
    const int size = strlen (tmp);

    // don't write the trailing '\0'
    if (write (fd, tmp, size) != size) {
	y2error ("fatal error: write failed: %m");
	exit (EXIT_FAILURE);
    }
#endif
}


void
YCPDebugger::Position::setpos (const string &set_file, int set_line)
{
    file = set_file;
    line = set_line;
}


void
YCPDebugger::Settings::reset ()
{
    ignorescr = true;
    printtoken = false;
}
