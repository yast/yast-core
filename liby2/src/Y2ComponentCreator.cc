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

   File:	Y2ComponentCreator.cc

   Purpose:	Base class of all Y2 component creators

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Thomas Roelz <tom@suse.de>

/-*/

#include "Y2.h"

#include "y2util/y2log.h"

Y2ComponentCreator::Y2ComponentCreator(Y2ComponentBroker::order_t order)
{
    Y2ComponentBroker::registerComponentCreator(this, order);
}


Y2ComponentCreator::~Y2ComponentCreator()
{
}

bool Y2ComponentCreator::isClientCreator() const
{
    return !isServerCreator();
}

Y2Component *Y2ComponentCreator::create(const char *name) const
{
    y2warning ("default create (%s), should not happen", name);
    return 0;
}

Y2Component *Y2ComponentCreator::provideNamespace(const char *name)
{
    y2warning ("default provideNamespace (%s), should not happen", name);
    return 0;
}


Y2Component *Y2ComponentCreator::createInLevel(const char *name, int level, int current_level) const
{
    if (level == current_level) return create(name);
    else return 0;
}
