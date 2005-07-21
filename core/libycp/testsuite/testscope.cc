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

   File:       testscope.cc

   Author:     Klaus Kaempf (kkaempf@suse.de)
   Maintainer: Klaus Kaempf (kkaempf@suse.de)

/-*/

#include <stdio.h>
#include <ycp/YCPScope.h>
#include <ycp/y2log.h>

//extern int yydebug;

int
main (int argc, const char *argv[])
{
    YCPScope scope;

    string name = "toplevel";
    string nextname = "sublevel";
    scope.openScope ();

    const string symname = "i";
    const YCPDeclaration symdecl = YCPDeclType (YT_INTEGER);
    YCPValue symvalue = YCPInteger (42);
    YCPValue newvalue = YCPInteger (13);
    YCPList dumparg = YCPList();
    dumparg->add (symvalue);

    scope.declareSymbol (symname, symdecl, symvalue, false);

    if (!scope.symbolDeclared (symname))
    {
	printf ("Not declared\n");
    }
printf ("first dump\n"); fflush (stdout);
    scope.dumpScope (dumparg);

    if (symvalue->compare (scope.lookupValue (symname, "")) != YO_EQUAL)
    {
	printf ("Bad value\n");
    }

    if (symdecl->compare (scope.lookupDeclaration (symname)) != YO_EQUAL)
    {
	printf ("Bad declaration\n");
    }

    // subscope

    scope.openScope ();
    scope.declareSymbol (symname, symdecl, newvalue, false);

    if (!scope.symbolDeclared (symname))
    {
	printf ("Not declared\n");
    }
printf ("second dump\n"); fflush (stdout);
    scope.dumpScope (dumparg);

    if (newvalue->compare (scope.lookupValue (symname, "")) != YO_EQUAL)
    {
	printf ("Bad sub value\n");
    }

    if (symdecl->compare (scope.lookupDeclaration (symname)) != YO_EQUAL)
    {
	printf ("Bad sub declaration\n");
    }

    scope.assignSymbol (symname, newvalue);
    if (newvalue->compare (scope.lookupValue (symname, "")) != YO_EQUAL)
    {
	printf ("Bad newvalue\n");
    }
printf ("Closing subscope\n");
    scope.closeScope();

    if (symvalue->compare (scope.lookupValue (symname, "")) != YO_EQUAL)
    {
	printf ("Bad value\n");
    }

printf ("remove toplevel i\n");
    scope.removeSymbol (symname);

    if (scope.symbolDeclared (symname))
    {
	printf ("Still declared\n");
    }

    scope.closeScope();
    return 0;
}
