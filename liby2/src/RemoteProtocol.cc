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

   File:	RemoteProtocol.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/
#include "RemoteProtocol.h"
#include "Y2Component.h"

#include <ycp/y2log.h>


RemoteProtocol::RemoteProtocol (string commandline, bool use_pty)
    : ExternalProgram ("LANG=POSIX " + commandline, Stderr_To_Stdout, use_pty)
{
}


int
RemoteProtocol::expectOneOf (const string* strings, int number_strings,
			     int maxread)
{
    char readchars[maxread + 1];
    char *writepointer = readchars;
    *writepointer = 0;

    while (writepointer < readchars + maxread)
    {
	y2debug ("Buffer now: [%s]", readchars);
	for (int i = 0; i < number_strings; i++)
	{
	    if ((int)(strings[i].length()) <= writepointer - readchars &&
		strings[i] == writepointer - strings[i].length())
	    {
		y2debug ("Found [%s]", strings[i].c_str ());
		return i;
	    }
	}

	int c = fgetc (inputFile ());

	if (c == EOF)
	    return -1;

	*writepointer++ = c;
	*writepointer = 0;
    }

    return -1;
}

