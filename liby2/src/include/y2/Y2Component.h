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

   File:       Y2Component.h

   Author:     Mathias Kettner <kettner@suse.de>
	       Thomas Roelz <tom@suse.de>
	       Stanislav Visnovsky <visnov@suse.cz>
   Maintainer: Stanislav Visnovsky <visnov@suse.cz>

/-*/
// -*- c++ -*-

#ifndef Y2Component_h
#define Y2Component_h

#include <string>

using std::string;

class SCRAgent;
class Y2Namespace;
class YCPValue;
class YCPList;

/**
 * \page components YaST2 Component Architecture
 *
 * \author Mathias Kettner
 *
 * \todo This is partially obsolete!
 * 
 * <h2>Design principles</h2>
 * 
 * <p>The YaST2 component model is the foundation of the
 * YaST2 architecture. It is important to understand at least
 * the basic ideas in order to be able to write new Y2 components.
 * It's based upon the following design principles:
 * 
 * <p><table cellspacing=0 BGCOLOR="#f96500" width="100%"><tr><td>
 * <table width="100%" bgcolor="#ffc080" cellpadding=10><TR><TD>
 * <ul>
 * 
 * <p><li><i>YaST2 should be easily extensable. One should be able
 * to exchange or add functionality independent
 * of all other parts.</i>This leads to a modular architecture.
 * The interchangable parts are called 'components'.</li>
 * 
 * <p><li><i>It should be possible, that a component can be added
 * or exchanged merely through the fact, that a file has been
 * added or exchanged.</i> This allows you to put configuration
 * modules for a software into the same package as that software.
 * For example you could write a configuration module for
 * sendmail and put it into the sendmail package. As long as
 * the sendmail package is installed, its YaST2 configuration
 * module is available.</li>
 * 
 * <p><li><i>Despite the need for Y2 to be modular in concept,
 * during execution time this should not lead to high communication
 * overhead or large need in memory usage or the number of
 * concurrently running processes.</i></li>
 * 
 * <p><li><i>The inter-component communication should be human
 * readable, at least in debugging situations.</i></li>
 * 
 * <p><li><i>The inter-component communication should be network
 * transparent.</i></li>
 * 
 * <p><li><i>It should be easy to write a component. Component implementing
 * should not be restricted to a certain programming language.</i>
 * 
 * </ul>
 * </td></tr></table>
 * </td></tr></table>
 * 
 * <h2>Communication</h2>
 * 
 * <p>All components speak a common language and act according to
 * defined protocol. Both the language and the protocol are
 * called <i>YCP (YaST2 communication protocol)</i>. One protocol
 * step consists of one of the partners sending exactly one
 * <a href="../libycp/YCP-datatypes.html">YCP value</a>
 *  to the other and the other receiving that value. In the
 * next step, the component that just was receiving, is now sending and
 * vice versa. The only exception is, that one of that partners
 * may terminate the session and send a last <i>result</i> message
 * after which - of course - the partner won't send another value.
 * 
 * <h2>Clients and Servers</h2>
 * 
 * <p>There are two different kinds of components: server components and
 * client components, which differ in how the control flows.
 * 
 * <p>A <i>server</i> component is one that - once it's initialized -
 * just waits for jobs to do. Another component can send a <i>request</i>
 * and thus pass the control to the server. That does what is neccessary
 * in order to do handle the request and returns an
 * <i>answer</i>. Prominent examples for server components are the user
 * interfaces. In YaST2 they play a <i>passive</i> role. They just wait
 * for a command like &quot;Show me this dialog, wait for the next user
 * input and return me that input&quot;. A server component can also use
 * the service of another server component. For example the <i>SCR
 * (System configuration repository)</i> makes use of servers called
 * <i>agents</i>, which realize subtrees of the SCR tree.
 * 
 * <p>A <i>client</i> component is one that controls the flow. It may
 * contain something like an &quot;event loop&quot;. In order to do its
 * work, it can use the services of other server components. Client
 * components are sometimes called <i>modules</i>. Examples for
 * modules are the single steps of the YaST2 &quot;Installation
 * Wizard&quot; or the program that calls <tt>rpm</tt> to install
 * packages. Other examples could be a  module that configures the network
 * setup of a workstation or one, that just sets the IP-number and the
 * hostname of that workstation.
 * 
 * <p>Modules can be hiearchically structured
 * and hand over control to other modules that acomplish sub tasks and
 * are called <i>submodules</i> in this context. An example is the
 * structure of the YaST2 installer. The installation itself is
 * a module. For each of the wizard window steps, it calls a submodule.
 * 
 * <h2>How components are realized</h2>
 * <p>There are quite a number of ways how you can implement a component.
 * A component can be:
 * <p><table cellspacing=0 BGCOLOR="#f96500" width="100%"><tr><td>
 * <table width="100%" bgcolor="#ffc080" cellpadding=10><TR><TD>
 * <ul>
 * 
 * <li>An executable program (ELF, /bin/sh, or whatsoever)</li>
 * <li>A C++ class using <tt><a href="../libycp/autodocs/intro.html">libycp</a></tt>
 * and <tt><a href="../liby2/autodocs/intro.html">liby2</a></tt></li>
 * <li>A YCP script executed by the <i>Workflowmanager</i></li>
 * <li>Youself typing some YCP code at a terminal</li>
 * 
 * </ul>
 * </td></tr></table>
 * </td></tr></table>
 * 
 * <p>Don't laugh over the last possibility! For debugging this is sometimes
 * very helpful. You can simulate <i>any</i> component by typing to a terminal.
 * 
 * <h3>Executable programs</h3>
 * 
 * <p>A component that exists as <i>executable program</i> is realized by
 * a process that is created when the component is needed. If a component
 * needs the services of an external program component, it launches a
 * seperate process and starts a communication via two unix pipes, one in
 * each direction.
 * 
 * <p>The data flowing through these pipes is YCP Ascii
 * representation. Each communication side needs a parser to analyse the
 * data and a YCP syntax generater to write data to the pipe. Both can be
 * found in the <tt><a href="../libycp/autodocs/intro.html">libycp</a></tt>,
 * which can only used from C++ programs.
 * 
 * <p>But the production of YCP code
 * can in many cases very easily be done be printing to stdout, for
 * example with <tt>echo</tt> (shell) or <tt>printf</tt> (C).
 * 
 * <p>The parsing of YCP code is as bit more tricky. But in many cases you
 * don't need a full featured parser, because you know beforehand what structure
 * the value have that you get. This especially holds for client components,
 * because they can decide, how the output from the server should look like.
 * 
 * <h3>C++ class using libycp and liby2</h3>
 * <p>If you anyway decide to write your component in C++,
 * it's by far the most conveniant way to use the functionality
 * of libycp and liby2, whose only purpose is excactly to support
 * component implementation.
 * 
 * <p>What you have to do is to subclass at least two classes:
 * <tt><a href="../liby2/autodocs/Y2ComponentCreator.html">Y2ComponentCreator</a></tt> and
 * <tt><a href="../liby2/autodocs/Y2Component.html">Y2Component</a></tt> and
 * 
 * <p>Depending on whether you want to implement a server or a client component
 * you have to override different methods. Many examples can be found within the liby2
 * itself.
 * 
 * <p>One big advantage of writing a component in C++ is, that
 * you can very easily create three external appearances of the component:
 * <ul>
 * <li>A selfcontained executable program</li>
 * <li>A shared library plugin to load during runtime</li>
 * <li>A static library that can be linked together with other components</li>
 * </ul>
 * 
 * <p>The YaST2 installer makes usage of the third variant only. All required components
 * are linked together to <tt>y2base</tt>, which is statically linked against liby2 and
 * libycp. The memory usage is reduced as well as the required disk space. Furthermore
 * no creating and parsing of Ascii streams between the components is required. Protocol
 * steps are simple function calls. Even for very large data structures, only one pointer
 * has to be passed.
 * 
 * <h3>A YCP script executed by the <i>Workflowmanager</i></h3>
 * <p>If you have installed YaST2, you will find some files ending in <tt>.ycp</tt>
 * lying around  in <tt>/lib/YaST2/clients</tt>. These are YCP scripts implementing
 * client components (modules). YCP is not only a protocol, it is also a full features
 * programming language, which is in this case used to implement components. This is
 * very conveniant as the language can directly operate on the protocol values and
 * has some other nice features.
 * 
 * <p>The client scripts are executed by the <i>Workflowmanager</i>, which is an
 * extension to the core YCP language. It implements a couple of builtin functions
 * that allow communication the the system and with other
 * components. Here is a
 * <a href="../y2wfm/YCP-builtins-wfm.html">of builtins.</a>.
 * 
 * <h3>Youself typing YCP code at a terminal</h3>
 * <p>You can be a component yourself :-). Just tell another component to communicate
 * via stdio and speak with it. For example you can launch the component <tt>ycp</tt> by
 * typing <tt>y2ycp stdio</tt>. Now you can enter YCP expressions and get the evaluation
 * as answer.
 */

/**
 * @short Communication handle to a YaST2 component.
 * @see componentbroker
 * @see Y2ComponentBroker
 *
 * YaST2 is a network oriented client/server architecture.
 * Currently there exist five differnt types of components:
 * userinterfaces, modules, the workflowmanagers, the
 * repository (scr) and scr agents. All components are
 * communicating using the same protocol.
 *
 * The communication between two components is asymmetric. On
 * component is said to be the <i>server</i>, the other to be
 * the <i>client</i>. The protocol begins with the client sending
 * an YCPValueRep, the <i>command</i> to the server. The server
 * <i>evaluates</i> it and returns an YCPValueRep, the <i>answer</i>.
 * Now the client sends the next command and so on. When the client
 * has finished the work with the server, it sends a <i>result</i>
 * value to the server and both sides close the connection. The result
 * is a special YCPValueRep that is a term with the symbol "result" and
 * exactly one argument.
 *
 * Each component may be realized by a binary program (under Linux: ELF),
 * by a process listening to an internet-domain socket, by a shared
 * library plugin in ELF format (ending in .so), by a YCP script that
 * is executed by the Workflowmanager (WFM), or by a C++ class that
 * is linked to the Generic YaST2 Frontend (GF).
 *
 * When you want to implement a component you need to subclass Y2Component
 * and define the virtual functions (some of them only for a client component, some
 * of them only for a server component). Furthermore you need to subclass
 * @ref Y2ComponentCreator and create a global variable from this class.
 *
 * When you implement a component that needs the services of another
 * component then you ask the @ref Y2ComponentBroker to get a handle
 * to it. That handle is of type Y2Component. Once you
 * have got the handle, you can use it communicate with the component,
 * regardless whether it is realized by a program, a plugin or what so ever.
 *
 * See @ref Y2ComponentBroker for examples.
 */

class Y2Component
{
public:

    /* ================ common ================ */

    Y2Component();

    /**
     * Base class must have virtual destructor.
     */
    virtual ~Y2Component();

    /**
     * Returns the name of the module.
     */
    virtual string name() const = 0;

    /* ================ server ================ */

    /**
     * Starts the server, if it is not already started and
     * does what a server is good for: Gets a command, evaluates
     * (or executes) it and returns the result.
     * @param command The command to be executed. Any YCPValueRep can
     * be executed. The execution is performed by some @ref YCPInterpreter.
     * @return the result. Destroy it after use with @ref YCPElementRep#destroy.
     *
     * This method is only defined, if the component is a server.
     */
    virtual YCPValue evaluate(const YCPValue& command);

    /**
     * Tells this server, that the client doesn't need it's services
     * any longer and that the exit code of the client is result.
     *
     * This method is only defined, if the component is a server.
     */
    virtual void result(const YCPValue& result);

    /**
     * Sets the commandline options of the server.
     * @param argc number of arguments including argv[0], the
     * name of the component
     * @param argv a pointer to a field of argc+1 char *,
     * where the last one must be 0. The caller of the function
     * must assure that the data field is persistent. This method
     * will _not_ make a copy of it.
     *
     * This method is only defined, if the component is a server.
     */
    virtual void setServerOptions(int argc, char **argv);

    /**
     * Try to import a given namespace. This method is used
     * for transparent handling of namespaces (YCP modules)
     * through whole YaST.
     * NOTICE: there is no reverse operation to import.
     * Semantics of YCP modules is there is a single instance
     * and it is available from the first import
     * until the end of YaST run.
     * @param name_space the name of the required namespace
     * @return on errors, NULL should be returned. The
     * error reporting must be done by the component itself
     * (typically using y2log). On success, the method
     * should return a proper instance of the imported namespace
     * ready to be used. The returned instance is still owned
     * by the component, any other part of YaST will try to
     * free it. Thus, it's possible to share the instance.
     */
    virtual Y2Namespace* import(const char* name_space);

    /* ================ client ================ */

    /**
     * This function must be overridden by an actual client.
     * Here the client does its actual work.
     * @param arglist YCPList of client arguments.
     * @param user_interface Option display server (user interface)
     * Most clients need interaction with the user. The different
     * YaST2 user interfaces are servers. If the user interace
     * is already active and running, give a handle to it with this
     * parameter. Give 0 here if no user interface is running yet and
     * either the module launches the user interace itself or it it
     * does not need one.
     * @return The result value (exit code) of the called client. The
     * result code has <i>not</i> yet been sent to the display server.
     * Destroy it after use.
     *
     * This method is only defined, if the component is a client.
     */
    virtual YCPValue doActualWork(const YCPList& arglist, Y2Component *user_interface);
    
    /* ================ misc ================ */

    /**
     * Returns the SCRAgent of the Y2Component or NULL, if it doesn't have
     * one. Note: This might trigger the creation of the Interpreter and
     * Agent associated with the Y2Component. For plugins, this might trigger
     * the loading of the plugin as well.
     */
    virtual SCRAgent * getSCRAgent ();
    
    virtual bool remote () const;
};

#endif // Y2Component_h
