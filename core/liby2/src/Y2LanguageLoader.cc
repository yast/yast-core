/**
 * File:	Y2LanguageLoader.cc
 * Author:	Martin Vidner <mvidner@suse.cz>
 *
 * In the transparent language binding architecture, created to bring
 * Perl to YaST, component creators are asked whether they can provide
 * a namespace. In order for this to function, a component creator for
 * each language needs to be loaded. At the same time, we want the
 * languages to be independent.
 *
 * Therefore, the component creators of the languages (not necessarily
 * the whole interpreters) are loaded dynamically in a constructor of
 * a singleton static object, Y2LanguageLoader.
 * The libraries loaded are ${plugindir}/libpy2lang_*.so
 */

// the implementation is inspired by Y2PluginComponent

#include <ycp/pathsearch.h>
#include <ycp/y2log.h>

#include <dlfcn.h>
#include <glob.h>
#include <stack>

class Y2LanguageLoader
{
private:
    std::stack <void *> plugins;
public:
    Y2LanguageLoader ();
    ~Y2LanguageLoader ();
};

Y2LanguageLoader y2LanguageLoader;


Y2LanguageLoader::~Y2LanguageLoader ()
{
/*
THis does not work because the environment may be already destroyed
    if (getenv ("Y2LANGUAGELOADER"))
    {
	// don't unload the libraries when we need their symbols for
	// valgrind tracking
	return;
    }
*/
    while (!plugins.empty ())
    {
	// only unloaded when not needed, says man
// The process is ending anyway
//	dlclose (plugins.top ());
	plugins.pop ();
    }
}

Y2LanguageLoader::Y2LanguageLoader ()
{
    glob_t glob_buf;
    int glob_flags = 0;
    if (YCPPathSearch::numberOfComponentLevels() == 0)
    {
	y2milestone ("No language components");
	memset (&glob_buf, 0, sizeof (glob_t));
    }
    for (int i = 0; i < YCPPathSearch::numberOfComponentLevels (); ++i)
    {
	string pattern = YCPPathSearch::searchPath (YCPPathSearch::PLUGIN, i)
	    + "/libpy2lang_*.so";

	int res = glob (pattern.c_str (), glob_flags, NULL, &glob_buf);
	if (res != 0 && res != GLOB_NOMATCH)
	{
// Cannot use logging, may not be initialized yet (esp. with logconf)
	    fprintf (stderr, "Glob error %d for pattern %s\n",
		     res, pattern.c_str ());
	}
	glob_flags |= GLOB_APPEND;
    }

    // load them
    for (size_t i = 0; i < glob_buf.gl_pathc; ++i)
    {
	char *p = glob_buf.gl_pathv[i];
	y2debug ("Loading language plugin %s", p);
	void *handle = dlopen (p, RTLD_LAZY | RTLD_GLOBAL);
	if (handle)
	{
	    plugins.push (handle);
	}
	else
	{
//	    y2error ("Error loading language plugin %s: %s", p, dlerror ());
	    fprintf (stderr, "Error loading language plugin %s: %s\n", p, dlerror ());
	}
    }

    if (glob_flags != 0)
	globfree (&glob_buf);
}
