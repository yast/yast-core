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

   File:       Y2PluginComponent.cc

   Author:     Arvin Schnell <arvin@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/


#include <stdio.h>
#include <dlfcn.h>
#include <sys/time.h>

#include <ycp/y2log.h>
#include "Y2PluginComponent.h"


// #define MEASURE_SPEED


Y2PluginComponent::Y2PluginComponent (bool is_server, string filename,
				      const char* creator_name,
				      const char* component_name, int level)
    : is_server (is_server),
      filename (filename),
      creator_name (creator_name),
      component_name (component_name),
      argc (0),
      argv (0),
      level (level),
      handle (0),
      comp (0)
{
    // load the plugin right-away now, there will be no evaluate prior builtins!!!
    loadPlugin();
}


Y2PluginComponent::Y2PluginComponent (string filename, const char* creator_name,
				      const char* component_name, const char* name_space)
    : is_server (true),
      filename (filename),
      creator_name (creator_name),
      component_name (component_name),
      argc (0),
      argv (0),
      level (0),
      handle (0),
      comp (0)
{
    // load the plugin right-away now, there will be no evaluate prior builtins!!!
    loadPlugin(name_space);
}


Y2PluginComponent::~Y2PluginComponent ()
{
    if (comp)
    {
	comp->result (YCPVoid ());
	delete comp;
    }

    if (handle)
    {
	dlclose (handle);
    }
}


void
Y2PluginComponent::setServerOptions (int argc, char** argv)
{
    this->argc = argc;
    this->argv = argv;
    
    if (comp)
    {
	comp->setServerOptions (argc, argv);
    }
}


YCPValue
Y2PluginComponent::evaluate (const YCPValue& command)
{
    if (!handle)
    {
	loadPlugin ();
    }

    if (!comp)
    {
	y2error ("Error loading plugin for %s", component_name.c_str ());
	return YCPNull ();
    }

    return comp->evaluate (command);
}


SCRAgent*
Y2PluginComponent::getSCRAgent ()
{
    if (!handle)
    {
	loadPlugin ();
    }

    if (!comp)
    {
	return 0;
    }

    return comp->getSCRAgent ();
}


void
Y2PluginComponent::result (const YCPValue& result)
{
    if (comp)
    {
	comp->result (result);
	delete comp;
	comp = 0;
    }

    if (handle)
    {
	dlclose (handle);
	handle = 0;
    }

    return;
}


string
Y2PluginComponent::name () const
{
    // Note: It is also be possible to call the name function of
    // the Y2Component.
    return component_name;
}


YCPValue
Y2PluginComponent::doActualWork (const YCPList& arglist,
				 Y2Component* user_interface)
{
    if (!handle)
    {
	loadPlugin ();
    }

    if (!comp)
    {
	return YCPVoid ();
    }

    return comp->doActualWork (arglist, user_interface);
}


#ifdef MEASURE_SPEED
static double
exact_time ()
{
    timeval time;
    gettimeofday (&time, 0);
    return (double) (time.tv_sec + 1.0e-6 * (double)(time.tv_usec));
}
#endif


bool
Y2PluginComponent::loadPlugin (const char* name_space)
{
    if (handle)
    {
	return false;
    }

    y2debug ("loadPlugin (%s), namespace (%s)", filename.c_str(), name_space ?
     name_space : "nil");

    // it's too late to the change the malloc implementation
    setenv ("QT_FAST_MALLOC", "0", 1);
    setenv ("KDE_MALLOC", "0", 1);

    // First dlopen the library.

    // Note: Calls the global contructors in the library. That's the reason
    // why we have to stop registry at the ComponentBroker. Otherwise, a second
    // use of the plugin wouldn't use the PluginComponent and unloading would
    // lead to a crash.

#ifdef MEASURE_SPEED
    double t1 = exact_time ();
#endif
    // RTLD_GLOBAL is needed (at least) for alsa 0.9 lib
    handle = dlopen (filename.c_str(), RTLD_LAZY | RTLD_GLOBAL);
#ifdef MEASURE_SPEED
    double t2 = exact_time ();
    y2milestone ("loaded plugin %s in %fs", filename.c_str(), t2 - t1);
#endif
    if (!handle)
    {
	y2error ("error loading plugin %s: %s", filename.c_str(), dlerror());
	return false;
    }

    // Locate the global y2cc.

    Y2ComponentCreator* y2cc = 0;

    for (int num = -1; ; num++)
    {
	y2cc = locateSym (num);
	
	y2debug ("Component creator located; %p", y2cc);

	if (!y2cc && num == -1)		// that's ok
	{
	    continue;
	}

	if (!y2cc)			// that's bad
	{
	    break;
	}
	
	// let's try to lookup namespace if we are asked for it
	if (name_space != NULL)
	{
	    y2debug ("Trying component creator to create namespace %s", name_space);
	    comp = y2cc->provideNamespace (name_space);
	    return comp != NULL;
	}

	if (is_server == y2cc->isServerCreator())	// perhaps that's it
	{
	    comp = y2cc->create (component_name.c_str());	// try it

	    y2debug ("Y2PluginComponent @ %p created server '%s' @ %p",
		     this, component_name.c_str(), comp);

	    if (comp)		// success
	    {
		if (argc > 0)
		{
		    comp->setServerOptions (argc, argv);
		}
		return true;
	    }
	}

    }

    y2error ("error loading plugin %s: can't locate componentcreator or "
	     "componentcreator can't create component", filename.c_str());

    return false;
}


Y2ComponentCreator*
Y2PluginComponent::locateSym (int num)
{
    const int size = 100;
    char buffer[size];

    if (num == -1)
    {
	snprintf (buffer, size, "g_y2cc%s", creator_name.c_str());
    }
    else
    {
	snprintf (buffer, size, "g_y2cc%s%d", creator_name.c_str(), num);
    }
    

    Y2ComponentCreator* y2cc = (Y2ComponentCreator*) dlsym (handle, buffer);
    if (dlerror() != 0)
    {
	y2debug ("Not found symbol: %s", buffer);
	return 0;
    }

    y2debug ("Found symbol: %s", buffer);

    return y2cc;
}
