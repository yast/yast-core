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

   File:       Y2CCProgram.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component Creator for external program components
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef Y2CCProgram_h
#define Y2CCProgram_h

#include "Y2ComponentCreator.h"

class Y2CCProgram : public Y2ComponentCreator
{
    /**
     * Is true, if this creator only should create servers.
     */
    bool creates_servers;

    /**
     * Is true, if this creator only should create non_y2 programs.
     */
    bool creates_non_y2;

public:
    /**
     * Creates a YaST2 Component Creator that can create program
     * components. A program component is a component that is
     * realized by an Unix executable that is executed and communicates
     * via a pipe.
     * @param server true, if server components are created, false if client
     * components are created.
     * @param non_y2 true, if components other than YCP programs like shell scripts
     * are created.
     */
    Y2CCProgram(bool server, bool non_y2);

    /**
     * Return true, if this creator creates server components.
     */
    bool isServerCreator() const;

    /**
     * Creates a component. If the name contains a slash, it is considered
     * to be a relative or absolute path name to a Unix executable. It is
     * not possible to specifiy a server this way.
     * If it does not contain a slash, it is searched for in YASTHOME/modules or
     * YASTHOME/servers, resp.
     */
    Y2Component *createInLevel(const char *name, int level, int current_level) const;
    
    /**
     * Importing a namespace from a program-based subcomponent is not possible.
     */
    virtual Y2Component* provideNamespace(const char* name_space);
};


#endif // Y2CCProgram_h
