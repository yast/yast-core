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

   File:       Y2CCSerial.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component Creator that creates serial components
 *
 * Author: Thomas Roelz <tom@suse.de>
 */

#ifndef Y2CCSerial_h
#define Y2CCSerial_h

#include "Y2ComponentCreator.h"

class Y2CCSerial : public Y2ComponentCreator
{
    /**
     * Denotes a server or a client
     */
    bool creates_servers;

    /**
     * regular expression
     */
    mutable regex_t rx1;

    mutable int my_nl_msg_cat_cntr;
    void make_rxs () const;
    void free_rxs () const;

public:

    Y2CCSerial(bool creates_servers);

    bool isServerCreator() const;

    Y2Component *create(const char *name) const;

    /**
     * Importing a namespace from a serial subcomponent is not possible.
     */
    Y2Component* provideNamespace(const char* name_space);
};


#endif // Y2CCSerial_h
