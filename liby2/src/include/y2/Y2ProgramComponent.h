/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Y2ProgramComponent.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component that starts an external Y2 program
 */

#ifndef Y2ProgramComponent_h
#define Y2ProgramComponent_h

#include "Y2.h"
#include <ycp/Parser.h>

class Y2ProgramComponent : public Y2Component
{
    /**
     * Chroot path for the program.
     */
    string chroot_path;

    /**
     * Filename of the executable binary.
     */
    string bin_file;

    /**
     * Specifies whether this component is a ycp program or a shell and such like.
     */
    bool is_non_y2;

    /**
     * Name of the component that is implemented by the program
     */
    string component_name;

    /**
     * Stores options for a server program
     */
    int argc;

    /**
     * Stores options for a server program
     */
    char **argv;

    /**
     * Filehandles of pipe to external programm
     */
    int to_external[2];

    /**
     * Filehandles of pipe from external programm
     */
    int from_external[2];

    /**
     * Process ID of external process. This is -1, if the process
     * is not yet launched.
     */
    pid_t pid;

    /**
     * Used to parse the values the external program sends
     */
    Parser parser;

    /**
     * The component level this program was started in. For example
     * programs started from floppy get the component level 0.
     */
    int level;

public:

    Y2ProgramComponent (string chroot_path, string binpath,
			const char *component_name, bool non_y2, int level);

    /**
     * Frees internal data.
     */
    ~Y2ProgramComponent();

    /**
     * Returns the name of this component.
     */
    string name() const;

    /**
     * Let the server evaluate a command.
     *
     * This method is only valid, if the component is a server.
     */
    YCPValue evaluate(const YCPValue& command);

    /**
     * Tells this server, that the client doesn't need it's services
     * any longer and that the exit code of the client is result.
     *
     * This method is only valid, if the component is a server.
     */
    void result(const YCPValue& result);

    /**
     * Sets the commandline options of the server.
     *
     * This method is only valid, if the component is a server.
     */
    void setServerOptions(int argc, char **argv);

    /**
     * Launches the program with the previously set parameters
     */
    YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);

    void sendToExternal(const string&);

    /**
     * Receives a YCP value from the external program.
     */
    YCPValue receiveFromExternal();


    bool remote () const;
    

private:
    /**
     * Lauches the external programm in a new process.
     * @return true, if this was successful.
     */
    void launchExternalProgram(char** argv);

    /**
     * Kills the external program (that is process) with SIGQUIT
     */
    void terminateExternalProgram();

    /**
     * Send a YCP value to the external program
     */
    void sendToExternal(const YCPValue&);

    /**
     * Determines, if the external program is running.
     */
    bool externalProgramOK() const;
};


#endif // Y2ProgramComponent_h
