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

   File:       Y2.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef Y2_h
#define Y2_h


#include <y2/Y2Component.h>
#include <y2/Y2ComponentCreator.h>
#include <y2/Y2ComponentBroker.h>

/**
 * \page liby2 liby2 Library
 *
 * <h2>The YaST2 component architecture</h2>
 * 
 * <p>The YaST2 system consists of different components. These are
 * user interfaces, modules, the system configuration repository (scr),
 * the generic workflow manager (wfm), the scr agents and possibly
 * other types. 
 *
 * <p>Each of the components can be realized in different ways, for example
 * as independent Unix executables, as processes communicating over a fixed
 * socket, as shared library plugins, as statically linked object files or
 * as YCP scripts.
 * 
 * <p>The Y2 library provides a framework for unifying all these kinds of
 * component types into a single interface called @ref Y2Component. It also
 * provides a generic frontend containing a main function, and the class 
 * @ref Y2ComponentBroker, whose task is to create the interfaces to components
 * when needed.
 *
 * <p>It is possible to link <i>all</i> YaST2 components together to one big monolith and
 * even statically link that! The components can communicate by exchanging pointers to
 * data structures rather that having to parse ASCII strings. And just one process is needed.
 * On the other hand it is possible to split it up into many self contained binaries and it
 * nevertheless has the same functionality. You very well can do it in both ways the same
 * time and provide single isolated binaries as well as clusters of components, that need
 * not be disjunct.
 *
 * <h2>The generic frontend (y2base)</h2>
 * 
 * <p>The liby2 provides a \ref main function. A component linked to liby2
 * therefore need not and cannot have its own main function. This
 * generic main function does:
 *
 * <ul>
 *   <li>parse the commandline in a way consistent to all components</li>
 *   <li>find the server and the client component regardless, whether they are realized
 *       as external programs, 
 *       shared library plugins, YCP scripts or objects that are linked to the main binary</li>
 *   <li>parameter checking 
 *   <li>launches the server and the client component with the correct paramters</li>
 *   <li>start the communication between client and server</li>
 * </ul>
 * 
 * <p>A YaST2 binary does always consist of the generic frontend and zero or more
 *  components that are linked in. The binary <tt>y2base</tt> just contains the frontend
 *  and the builtin components <tt>cat</tt> and <tt>stdio</tt>. <tt>cat</tt> is
 *  a server component that can be used instead of a user interface. It simply prints
 *  all commands it gets to stdout and waits for the answer at stdin. <tt>stdio</tt>
 *  works similary, but is a client.
 *
 * <p>The general synopsis of a call of a YaST2 binary is:
 * <pre>y2base client server [client-options] [server-options]</pre>
 * 
 * <p><font size="-1">( Please don't ask yet, why the server-options are stated <i>after</i>
 * the client options. We will see later. )</font> </p>
 * 
 * <p>An example would be:
 * <pre>y2base menu qt</pre>
 *
 * <p>This call would use the component <tt>qt</tt>, which is the Qt-lib base graphical
 * user interface, as display server and start the module <tt>menu</tt>.
 *
 * <h3>Server options</h3>
 * <p>Every command line argument
 */

#endif // Y2_h
