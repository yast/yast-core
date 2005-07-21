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

   File:       Y2CCPlugin.h

   Author:     Arvin Schnell <arvin@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

#ifndef Y2CCPlugin_h
#define Y2CCPlugin_h

#include <regex.h>

#include "Y2ComponentCreator.h"


class Y2CCPlugin : public Y2ComponentCreator
{
    /**
     * Is true, if this creator should create servers.
     */
    bool creates_servers;

    /*
     * all this mutable and const is needed since create is const.
     * and all this is needed since you have to recompile a regex
     * when you call setlocale.
     */

    /**
     * regular expression
     */
    mutable regex_t rxs1, rxr1, rxr2, rxr3;

    mutable int my_nl_msg_cat_cntr;
    void make_rxs () const;
    void free_rxs () const;

public:
    /**
     * Creates a YaST2 Component Creator that can create plugin
     * components. A plugin component is a component that is
     * realized by an dynamic loadable library.
     */
    Y2CCPlugin (bool server);

    /**
     * Return true, if this creator creates server components.
     */
    bool isServerCreator () const;

    /**
     * Creates a component. It is searched in the in YASTHOME/plugin.
     * The name must not contain any slash.
     */
    Y2Component* createInLevel (const char* name, int level, int current_level) const;
    
    /**
     * Importing a namespace from a plugin subcomponent is not possible.
     */
    Y2Component* provideNamespace(const char* name_space);

};


#endif // Y2CCPlugin_h
