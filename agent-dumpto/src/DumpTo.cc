/**

  DumpTo.cc

  Purpose:	hardware autoprobe repository access
		handling of .probe paths
  Creator:	kkaempf@suse.de
  Maintainer:	kkaempf@suse.de

  Written by Klaus K"ampf (kkaempf@suse.de) 1999

  see doc/dumpto.html for a description
*/


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "DumpTo.h"
#include <ycp/y2log.h>


DumpTo::DumpTo()
{
    if (getenv("YCP_DUMPTODEBUG") != 0)
	do_debug = true;
    else
	do_debug = false;
}


DumpTo::~DumpTo()
{
}


// ------------------------------------------------------------------

/**
 * Write
 *
 * write value to relative path
 *
 */

YCPValue
DumpTo::Write(const YCPPath& path, const YCPValue& value, const YCPValue& arg)
{
    y2debug ( "DumpTo: Write (%s:%s)\n", path->toString().c_str(), value->toString().c_str());
    FILE *f = openFile (path, true);
    if (f == 0)
	return YCPBoolean (false);
    dumpValue (f, 0, value);
    fputc ('\n', f);
    fclose (f);
    return YCPBoolean (true);
}


/**
 * Dir
 *
 * show subtree possibilities
 */

YCPValue
DumpTo::Dir(const YCPPath& path)
{
    return YCPVoid ();
}

// -----------------------------------------------------------------
// file i/o

/**
 * indent output by level
 */

void
DumpTo::indentOutput (FILE *f, int level)
{
    while (level-- > 0)
	fputs ("  ", f);
    return;
}

/**
 * recursively dump value to file
 */

int
DumpTo::dumpValue (FILE *f, int level, const YCPValue& value)
{
    if (value.isNull())
	return -1;

    y2debug ( "dumpValue (%s)\n", value->toString().c_str());

    switch (value->valuetype()) {
	case YT_LIST: {
	    YCPList list = value->asList();
	    fputs ("[", f);
	    for (int i = 0; i < list->size(); i++) {
		fputc ('\n', f);
		indentOutput (f, level+1);
		if (dumpValue (f, level+1, list->value (i)) != 0)
		    return 1;
		if ( i != list->size()-1) fputc(',', f);
	    }
	    fputc ('\n', f);
	    indentOutput (f, level);
	    fputs ("]", f);
	}
	break;
	case YT_MAP: {
	    YCPMap map = value->asMap();
	    fputs ("$[", f);
	    for (YCPMapIterator i = map->begin(); i != map->end(); i++) {
		if ( i != map->begin() ) fputc(',', f);
		fputc ('\n', f);
		indentOutput (f, level+1);
		if (dumpValue (f, level+1, i.key()) != 0)
		    return 1;
		fprintf (f, " : ");
		if (dumpValue (f, level+1, i.value ()) != 0)
		    return 1;
	    }
	    fputc ('\n', f);
	    indentOutput (f, level);
	    fputs ("]", f);
	}
	break;
	default: {
	    fprintf (f, "%s", value->toString().c_str());
	}
	break;
    }

    return 0;
}

/**
 * open file according to path
 */

FILE *
DumpTo::openFile (const YCPPath& path, bool writing)
{
    y2debug ( "openFile (%s, %s)\n", path->toString().c_str(), (writing?"w+":"r"));
    string fname;
    FILE *f = 0;

    for (int i = 0; i < path->length(); i++) {
	fname += "/" + path->component_str (i);
    }

    if (!fname.empty()) {
	f = fopen (fname.c_str(), (writing?"w+":"r"));
	y2debug ( "openFile (%s) = %p\n", fname.c_str(), f);
    }
    else {
	y2debug ( "openFile, oops empty filename ?!\n");
    }

    return f;
}
