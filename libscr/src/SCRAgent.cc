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

   File:       SCRAgent.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/

#include <ycp/y2log.h>

#include "include/scr/SCRAgent.h"


SCRAgent::SCRAgent ()
    : mainscragent (0)
{
}


SCRAgent::~SCRAgent ()
{
}


YCPValue
SCRAgent::Execute (const YCPPath& path, const YCPValue& value,
		   const YCPValue& arg)
{
    return YCPNull();
}


YCPValue
SCRAgent::otherCommand (const YCPTerm&)
{
    return YCPNull();
}


YCPValue
SCRAgent::readconf (const char *filename)
{
    FILE *file = fopen (filename, "r");
    if (!file)
    {
	return YCPError (string ("Can't open '") + string (filename) +
			 "' for reading.", YCPNull());
    }

    // find first line starting with "."
    const int size = 250;
    char line[size];
    char *fgets_result;
    do
    {
	fgets_result = fgets (line, size, file);
    }
    while ((fgets_result != 0) && (line[0] != '.'));

    YCPParser parser (file, filename);
    YCPValue tmpvalue = parser.parse ();
    fclose (file);

    if (tmpvalue.isNull () || !tmpvalue->isTerm ())
    {
	return YCPError (string ("Not a term in scr file '") +
			 string (filename) + string ("'."), YCPNull());
    }

    return tmpvalue;
}

