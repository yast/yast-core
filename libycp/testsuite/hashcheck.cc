/* hashcheck.cc
 *
 * hash checker
 *
 * Authors: Klaus Kaempf <kkaempf@suse.de>
 *
 * $Id$
 */

#include <stdio.h>
#include <string.h>

#include "ycp/SymbolTable.h"


int
main ()
{
#define BSIZE 511

    SymbolTable ht(511);
    char buf[BSIZE+1];

    while (fgets (buf, BSIZE, stdin) != 0)
    {
//	printf ("'%s' @ %p\n", buf, ht.enter (buf, 0));
	if (!ht.find (buf))
	    ht.enter ((const char *)strdup(buf), 0, 0, 0);
    }

    return 0;
}
