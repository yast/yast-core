/* miniini.cc
 *
 * INI-like files mini parser
 *
 * Author: Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#include "y2util/miniini.h"
#include <string.h>
#define MAXLINE 1024

char *cutblanks(char *c) {
    if(!*c) return c;
    char *end = c + strlen(c) - 1;
    while((*end == ' ' || *end == '\t' || *end == '\n') && end != c) *end-- = 0;
    while((*c == ' ' || *c == '\t' || *c == '\n') && *c != 0) c++;
    return c;
}

inifile miniini(const char *file) {

    inifile ret;

    FILE *in = fopen(file, "r");
    if(in == NULL) return ret;

    char line[MAXLINE], *c;
    char section[MAXLINE], *key;

    while(fgets(line, MAXLINE, in) != NULL) {
	c = cutblanks(line);
	switch(*c) {
	    /* comments */
	    case 0:
	    case '#':
	    case ';':
	    case '/':
		continue;
	    /* sections */
	    case '[':
		*c++;
		c[strlen(c) - 1] = 0;
		strncpy(section, cutblanks(c), MAXLINE);
		break;
	    /* keys and values */
	    default:
		key = strchr(c, '=');
		if(!key) break;
		*key++ = 0;
		ret[section][cutblanks(c)] = cutblanks(key);
		break;
	}
    }

    fclose(in);
    return ret;
}

#if 0
int main() {
    inifile i = miniini("/etc/samba/smb.conf");
    // inifile i = miniini("f.conf");

    inifile::const_iterator it = i.begin();
    for(; it != i.end(); ++it) {
	printf("[%s]\n", it->first.c_str());
	inisection is = it->second;
	inisection::const_iterator itt = is.begin();
	for(; itt != is.end(); ++itt)
	    printf("%s = %s\n", itt->first.c_str(), itt->second.c_str());
    }

    return 0;
}
#endif

/* EOF */
