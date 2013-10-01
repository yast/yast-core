

/*
 *  Author: Arvin Schnell <arvin@suse.de>
 */


#include "Y2CCSCR.h"
#include "Y2SCRComponent.h"

using namespace std;

Y2CCSCR::Y2CCSCR () :
  Y2ComponentCreator (Y2ComponentBroker::BUILTIN)
{
}

Y2CCSCR::~Y2CCSCR () {
    for (map<string, Y2SCRComponent*>::iterator i = scr_instances.begin();
        i != scr_instances.end(); ++i)
      delete i->second;
}

/* local helper for CC#create */
static void split_name( const char *name, string &root, string &real_name)
{
    real_name = name;
    root = "/";

    if (strncmp (name, "chroot=", 7) == 0)
    {
      const char *p = index (name, ':');

      if (p) {
        root = string (name, 7, p - name - 7);
        real_name = string (p + 1);
      }
    }
}

Y2Component *Y2CCSCR::create( const char* name) const
{
    string root, real_name;

    split_name(name, root, real_name);

    y2debug("Create component for %s: root '%s' real_name '%s'", name,
      root.c_str(), real_name.c_str());

    if (real_name != "scr")
        return NULL;

    map<string, Y2SCRComponent*>::iterator i = scr_instances.find(root);
    if (i != scr_instances.end())
        return i->second;

    scr_instances[root] = new Y2SCRComponent(root.c_str());
	  return scr_instances[root];
}


// create Component Creator for Y2SCRComp
Y2CCSCR g_y2ccscr;
