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

   File:       Y2CCStdio.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component Creator that creates stdio components
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef Y2CCStdio_h
#define Y2CCStdio_h

#include "Y2ComponentCreator.h"

class Y2CCStdio : public Y2ComponentCreator
{
    /**
     * If true, creates only the "cat" component,
     * if false only the "stdio".
     */
    bool creates_servers;

    /**
     * true, if stderr component should be created
     */
    bool to_stderr;

public:
    Y2CCStdio(bool creates_servers, bool to_stderr);

    bool isServerCreator() const;

    Y2Component *create(const char *name) const;

    /**
     * Importing a namespace from a stdio subcomponent is not possible.
     */
    Y2Component* provideNamespace(const char* name_space);
};


#endif // Y2CCStdio_h
