

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

Y2Component *Y2CCSCR::create( const char* name) const
{
    string root;

    if (strncmp (name, "chroot=", 7) == 0)
    {
	const char *p = index (name, ':');

	if (p) {
	    root = string (name, 7, p - name - 7);
	    name = p + 1;
	}
    }

    if (strcmp(name, "scr") != 0)
        return NULL;

    map<string, Y2SCRComponent*>::iterator i = scr_instances.find(root);
    if (i != scr_instances.end())
        return i->second;

    scr_instances[root] = new Y2SCRComponent(root.c_str());
	  return scr_instances[root];
}


// create Component Creator for Y2SCRComp
Y2CCSCR g_y2ccscr;
