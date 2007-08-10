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

   File:       Y2CCUI.h

   Author:     Stanislav Visnovsky <visnov@suse.de>
   Maintainer: Stanislav Visnovsky <visnov@suse.de>

/-*/
// -*- c++ -*-

/*
 * Component Creator that executes access to UI
 *
 * Author:     Stanislav Visnovsky <visnov@suse.de>
 */

#ifndef Y2CCUI_h
#define Y2CCUI_h

#include <y2/Y2ComponentCreator.h>

class Y2Component;

class Y2CCUI : public Y2ComponentCreator
{
    
public:
    Y2CCUI (Y2ComponentBroker::order_t order = Y2ComponentBroker::BUILTIN) : Y2ComponentCreator (order) {}

    virtual bool isServerCreator () const { return true; };

};

// Have this class implement the UI interface for ycpc, where we need
// only the type info and not the actual implementation.
// But let it stand after the real UIs
class Y2CCDummyUI : public Y2CCUI
{
    
public:
    Y2CCDummyUI () : Y2CCUI (Y2ComponentBroker::PLUGIN) {}

    /**
     * We provide the UI component
     */
    virtual  Y2Component *create(const char *name) const;

    /**
     * We provide the UI namespace
     */
    virtual  Y2Component *provideNamespace(const char *name);

};


#endif // Y2CCUI_h
