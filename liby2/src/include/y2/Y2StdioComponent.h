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

   File:       Y2StdioComponent.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component that communicates via stdin/out/err
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef Y2StdioComponent_h
#define Y2StdioComponent_h

#include "Y2Component.h"
#include <ycp/Parser.h>

/**
 * @short Interface to a component via stdio
 */
class Y2StdioComponent : public Y2Component
{
    /**
     * It this component a server or a client?
     */
    const bool is_server;

    /**
     * If true, this is the 'stderr' component, which
     * reads from stdin, but writes to stderr.
     */
    bool to_stderr;

    /**
     * If true, we're running in batchmode (i.e testsuite)
     * As a client (called via doActualWork()) nothing changes
     * As a server (called via evaluate()) no input is read
     */
    bool batchmode;

    /**
     * Parser used to parse stdin
     */
    Parser parser;

public:

    /**
     * Creates a new cat/stdio component.
     * @param to_stderr: give true, if stderr should be used instead of stdout.
     */
    Y2StdioComponent (bool is_server, bool to_stderr, bool in_batchmode = false);

    /**
     * Cleans up
     */
    ~Y2StdioComponent();

    /**
     * Returns "cat", if this is a server, or "stdio"
     * if it's a module.
     */
    string name() const;

    /**
     * Defined only for the "cat" component: Prints
     * a command to stdout and waits for the answer
     * on stdin.
     */
    YCPValue evaluate(const YCPValue& command);

    /**
     * Defined only for the "cat" component: Prints
     * result(..result..) on stdout.
     */
    void result(const YCPValue& result);

    /**
     * Sets the commandline options of the server. Server options
     * for the cat server are simply ignored.
     *
     * This method is only defined, if the component is a server.
     */
    void setServerOptions(int argc, char **argv);

    /**
     * Here the client does its actual work.
     * @param arglist YCPList of client arguments.
     * @param user_interface Option display server (user interface)
     * @return The result value (exit code) of the called client. The
     * result code has </i>not<i> yet been sent to the display server.
     * Destroy it after use.
     *
     * This method is only defined, if the component is a client.
     */
    YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);

private:
    /**
     * Sends a YCP value to stdout.
     */
    void send (const YCPValue& v) const;

    /**
     * Reads one YCP value from stdin. Return 0 if no one could
     * be read.
     */
    YCPValue receive ();

};


#endif // Y2StdioComponent_h
