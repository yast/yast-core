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

   File:       ExternalProgram.h

   Author:     Andreas Schwab <schwab@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/

#ifndef ExternalProgram_h
#define ExternalProgram_h

#include "ExternalDataSource.h"

/**
 * @short Execute a program and give access to its io
 * An object of this class encapsulates the execution of
 * an external program. It starts the program using fork
 * and some exec.. call, gives you access to the program's
 * stdio and closes the program after use.
 */
class ExternalProgram : public ExternalDataSource
{

public:
    /**
     * Define symbols for different policies on the handling
     * of stderr
     */
    enum Stderr_Disposition {
	Normal_Stderr,
	Discard_Stderr,
	Stderr_To_Stdout,
	Stderr_To_FileDesc
    };

    /**
     * Start the external program by using the shell <tt>/bin/sh<tt>
     * with the option <tt>-c</tt>. You can use io direction symbols < and >.
     * @param commandline a shell commandline that is appended to
     * <tt>/bin/sh -c</tt>.
     */
    ExternalProgram (string commandline,
		     Stderr_Disposition stderr_disp = Normal_Stderr,
		     bool use_pty = false, int stderr_fd = -1);

    /**
     * Start an external program by giving the arguments as an arry of char *pointers.
     */
    ExternalProgram (const char *const *argv,
		     Stderr_Disposition stderr_disp = Normal_Stderr,
		     bool use_pty = false, int stderr_fd = -1);

    ExternalProgram (const char *binpath, const char *const *argv_1,
		     bool use_pty = false);

    ExternalProgram (const YCPList &, const char *binpath = 0,
		     bool use_pty = false);

    // ExternalProgram(const YCPTermRep *);

    ~ExternalProgram();

    int close();

    /**
     * Kill the program
     */
    bool kill();

    /**
     * Return whether program is running
     */
    bool running();

private:

    /**
     * Set to true, if a pair of ttys is used for communication
     * instead of a pair of pipes.
     */
    bool use_pty;

    pid_t pid;

    void start_program (const char *const *argv,
			Stderr_Disposition stderr_disp = Normal_Stderr,
			int stderr_fd = -1);

};

#endif // ExternalProgram_h
