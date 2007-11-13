/* miniini.h
 *
 * INI-like files mini parser
 *
 * Author: Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#ifndef _miniini_h
#define _miniini_h

#include <stdio.h>
#include <string>
#include <map>

using std::string;
using std::map;

typedef map <const string, string> inisection;
typedef map <const string, inisection> inifile;

inifile miniini(const char *file);

#endif /* _miniini_h */

/* EOF */
