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

   File:       Y2CCWFM.h

   Author:     Mathias Kettner <kettner@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

/*
 * Component Creator that executes YCP script via wfm
 *
 * Author: Mathias Kettner <kettner@suse.de>
 *         Stanislav Visnovsky <visnov@suse.cz>
 */

#ifndef Y2CCWFM_h
#define Y2CCWFM_h

#include <y2/Y2ComponentCreator.h>

class Y2WFMComponent;

/**
 * @short Creates client components realized by YCP scripts
 * This component broker creates components of the type
 * Y2WFMComponent. You can give it a module name or
 * a path name to some file. First it tries to find some
 * script in <tt>YASTHOME/clients/<modulename>.ycp</tt>. If it doesn't
 * find one here, it considers the name to be a filename and
 * examines that file. If it:
 * <ul>
 * <li>is <i>not executable</i> or</li>
 * <li>its name has the suffix <tt>.ycp</tt></li>
 * <li>the file begins with <tt>#!/bin/y2gf</li>
 * </ul>
 */
class Y2CCWFM : public Y2ComponentCreator
{
public:
    /**
     * Creates a script component creator.
     */
    Y2CCWFM();

    /**
     * Tries to create a script module from the given name. See the
     * class description for details.
     */
    virtual Y2Component *createInLevel(const char *name, int level, int current_level) const;

    /**
     * Returns always true, since currently only client components
     * can be realized by a YCP script.
     */
    virtual bool isServerCreator() const;
    
    /**
     * We do provide namespaces from YCP
     */
    virtual  Y2Component *provideNamespace(const char *name);
    
    /**
     * Creates a new wfm component.
     */
    Y2Component *create (const char *name) const;
};


#endif // Y2CCWFM_h
