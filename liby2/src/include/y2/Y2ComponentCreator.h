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

   File:       Y2ComponentCreator.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef Y2ComponentCreator_h
#define Y2ComponentCreator_h

#include "Y2ComponentBroker.h"

/**
 * @short Can create certain YaST2 components
 * As documented in @ref Y2Component, there are many
 * ways how to realize a component. A Y2ComponentCreator is
 * and object, that creates a component or at least creates
 * a communication handle to an existing component.
 * The @ref Y2ComponentBroker keeps a list of all known Y2ComponentCreators.
 * If it is asked to create or find a certain component, it scans
 * this list and looks for a matching creator.
 *
 * If you want to implement a component creator,
 * subclass Y2ComponentCreator, override the create method such that
 * it returns a newly created component of your type and create
 * a global variable of type of your component creator. Its constructor
 * will be called before the execution goes into the main() function
 * and it will be added to the Y2ComponentBrokers list of creators.
 */
class Y2ComponentCreator
{
public:
    /**
     * Enters this component creator into the global
     * list of component creators.
     * @param order the order in which the creators are scanned. See
     * @ref Y2ComponentBroker#order_t
     */
    Y2ComponentCreator(Y2ComponentBroker::order_t order);

    /**
     * Base class must have virtual destructor
     */
    virtual ~Y2ComponentCreator();

    /**
     * Override this method to implement the actual creating
     * of the component. You must use the symbol and the
     * signature of the term to decide, which component is to be created.
     * If you are not able to create a matching component, return 0.
     */
    virtual Y2Component *create(const char *name) const;

    /**
     * Override this method to implement component creation for non-builtin
     * components such as share library plugins, scripts and external programs
     * that must be searched for in different directories. The default
     * implementation is to call create in case level == current_level.
     */
    virtual Y2Component *create(const char *name, int level, int current_level) const;

    /**
     * Override this method to implement providing a component for 
     * a given namespace. 
     * If you are not able to create a matching component, return 0.
     */
    virtual Y2Component *provideNamespace(const char *name);

    /**
     * Specifies, whether this creator creates Y2Servers.
     */
    virtual bool isServerCreator() const = 0;

    /**
     * Specifies, whether this creator creates Y2Clients.
     */
    bool isClientCreator() const;
};

#endif // Y2ComponentCreator_h
