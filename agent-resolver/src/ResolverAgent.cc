/* ResolverAgent.cc
 *
 * Classes for reading the resolv.conf configuration file.
 *
 * Author: Klaus Kaempf <kkaempf@suse.de>
 *         Daniel Vesely <dan@suse.cz>
 *         Michal Svec <msvec@suse.cz>
 *
 * $Id$
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>

#include <string>

#include <config.h>
#include <YCP.h>
#include <ycp/y2log.h>

#include <ycp/YCPMap.h>

#include "ResolverAgent.h"

static YCPMap localCache;
static bool cacheDirty = false;
static bool cacheValid = false;
static bool writeHeader  = false;

// list allowed keys
// TODO: define semantics and value syntax

static const char *resolver_keys[] = {
  "search",
  "nameserver",
  "domain",
  "sortlist",
  "options",
  0
};


typedef struct info_entry {
    const char *key;
    const char *tag;
};

// pairs of key tag, in this order will be written
static const info_entry headers[] = {
    {"modified",    "# Modified_by:"},
    {"backup",      "# Backup:"},
    {"process",     "# Process: "},
    {"process_id",  "# Process_id: "},
    {"script",      "# Script:"},
    {"info",        "# Info:"},
    {0, 0}
};

// looks-up in the list of known tags, returns appropriate key, NULL ortherwise
static const char *findKey (const char *tag)
{
    int i = 0;
    while (headers[i].key != 0) {
        if (strncmp (tag, headers[i].tag, strlen (headers[i].tag)) == 0)
            return headers[i].key;
        i = i + 1;
    }

    return NULL;
}

//determinates wheather the key if info key
bool allowedInfoKey (const char *key)
{
    int i = 0;
    while (headers[i].key != 0) {
        if (key && (strcmp (key, headers[i].key) == 0))
            return true;
        i = i + 1;
    }

    return false;
}

// find next whitespace or eol
static char *nextWhitespace (const char *lptr)
{
    if (lptr == 0)
	return 0;

    while (*lptr && !isspace (*lptr))
	lptr++;

    return (char *)lptr;
}

// eat all whitespaces
static char *eatWhitespaces (const char *lptr)
{
    if (lptr == 0)
	return 0;

    while (*lptr && isspace (*lptr))
	lptr++;

    return (char *)lptr;
}


// fill localCache with data from /etc/resolv.conf

static int fillCache (const char *filename)
{
    if (cacheValid)
	return 0;

    y2debug ("fillCache");

    FILE *f;

    f = fopen (filename, "r");
    if (f == 0) {
	y2error ("Can't access %s: %s", filename, strerror(errno));
	return -1;
    }

    const int lbufsize = 1024;
    char lbuf[lbufsize+2];	// add \n\0
    int retval = 0;
    char *lptr;
    bool processing_info = false;
    string info_buf = "";
    const char *last_key = 0;
    YCPMap info_map;

    localCache = YCPMap();	// initialize with empty map

    while (fgets (lbuf, lbufsize, f) != 0) {
	y2debug ("fread(%s)", lbuf);

	lptr = strchr (lbuf, '#');	// process comment
	if (lptr != 0)
        {
            if (strncmp (lbuf, "### BEGIN INFO", 14) == 0)
            {
                processing_info = true;
                continue;
            }
            if (strncmp (lbuf, "### END INFO", 12) == 0)
            {
                if (last_key)   // one more key to process
                    localCache->add (YCPString (last_key), YCPString ((info_buf.substr (0, info_buf.length () - 1)).c_str ()));

                if (!processing_info) y2warning ("End of info without beggining!");
                processing_info = false;
                continue;
            }
            if (const char *key = findKey (lbuf))
            {
                if (last_key)
                    localCache->add (YCPString (last_key), YCPString ((info_buf.substr (0, info_buf.length () - 1)).c_str ()));

                if (char *info = strchr (lbuf, ':'))
                    info_buf = eatWhitespaces (strlen (info) > 0 ? info + 1 : info);

                last_key = key;
                continue;
            }
            else if (processing_info)
                info_buf = info_buf + lbuf; // keep the other lines with '#' at the begging and '\n' in the end

	    *lptr = 0;          // don't care about other comments
        }

	if (lbuf[0] == '\0'		// skip other comment lines
	    || lbuf[0] == '\n')
	    continue;

	lptr = nextWhitespace (lbuf);	// break line at first whitespace
	y2debug ("next(%s)", lptr);

	if (lptr == 0
	    || *lptr == '\n'
	    || *lptr == 0)			// skip lines without <key><ws><value> syntax
	    continue;

	*lptr++ = 0;

	YCPValue value = YCPNull();
	char *vptr = lptr;

	y2debug ("key(%s)", lbuf);

	const YCPString key (lbuf);

	// list of strings as value

	if ((strcmp  (lbuf, "search") == 0)
	   || (strcmp  (lbuf, "sortlist") == 0)
	   || (strcmp  (lbuf, "options") == 0)) {

	    y2debug ("list of strings '%s'", lbuf);

	    YCPList ret;

	    for (;;) {
		vptr = lptr;
		lptr = nextWhitespace (vptr);
		if ((lptr == 0)
		    || (lptr == vptr))
		    break;
		*lptr++ = 0;
		ret->add (YCPString (vptr));
	    }
	    
	    value = ret;
	}

	// single value, but multiple lines allowed

	else if (strcmp (lbuf, "nameserver") == 0) {

	    y2debug ("multiple lines '%s':'%s'", lbuf, vptr);

	    value = localCache->value (key);
	    if (value.isNull())
		value = YCPList();
	    lptr = nextWhitespace (vptr);
	    *lptr = 0;
	    value = value->asList()->functionalAdd (YCPString (vptr));
	}

	// single value

	else if (strcmp (lbuf, "domain") == 0) {

	    y2debug ("single value '%s'", lbuf);

	    lptr = nextWhitespace (vptr);
	    *lptr = 0;
	    value = YCPString (vptr);
	}
	else {
	    y2warning ("key '%s' not recognized", lbuf);
	}

	localCache->add (key, value);
    }

    if (!feof (f)) {
	y2error ("Can't completely read %s", filename);
	retval = -1;
    }

    fclose (f);

    cacheValid = true;

    return retval;
}


// flush localCache to /etc/resolv.conf

static int flushCache (const char *filename)
{
    if (!cacheValid)
	return 0;
    if (!cacheDirty)
	return 0;

    FILE* f = fopen (filename, "w+");
    if (f == 0) {
	y2error ("Can't open %s for writing", filename);
	return -1;
    }

    fchmod (fileno (f), 0644);

    // first fill the info header, if needed
    if (writeHeader)
    {
        int i = 0;
        YCPValue info = YCPNull ();
        fprintf (f, "### BEGIN INFO\n#\n");
        while (headers[i].key != 0)
	{
	    info = localCache->value (YCPString (headers[i].key));
	    if (info.isNull () || info->isVoid ())
		y2warning ("Info key %s not found!", headers[i].key);
	    else
	    {
		if (info->isString ())
		    fprintf (f, "%s %s\n", headers[i].tag, info->asString ()->value_cstr ());
		else
		    y2error ("Wrong type for info key %s!", headers[i].tag);
	    }
	    i = i + 1;
	}
        fprintf (f, "#\n### END INFO\n#\n");
    }

    int retval = 0;
    YCPMapIterator mptr = localCache->begin();

    while (mptr != localCache->end()) {
	YCPValue key = mptr.key();
	if (key.isNull() || !key->isString()) {
	    y2error ("Bad key in localCache");
	    retval = -1;
	    break;
	}
	string skey = key->asString()->value();
	YCPValue value = mptr.value();

	if ((skey == "search")
	    || (skey == "sortlist")
	    || (skey == "options")) {

	    if (value.isNull() || !value->isList()) {
		y2error ("Bad value for key '%s'", skey.c_str());
		break;
	    }
	    YCPList list = value->asList();
	    if (list->size() < 0) {
		y2error ("Bad list size for key '%s'", skey.c_str());
		break;
	    }
	    if (list->size() == 0) {
		break;
	    }
	    fprintf (f, skey.c_str());
	    int i = 0;
	    for (i = 0; i < list->size(); i++) {
		if (list->value(i).isNull()
		    || !list->value(i)->isString()) {
		    y2error ("Skipping bad list element for key '%s'", skey.c_str());
		}
		else {
		    fprintf (f, " %s", list->value(i)->asString()->value().c_str());
		}
	    }
	    fprintf (f, "\n");
	}
	else if (skey == "nameserver") {
	    if (value.isNull() || !value->isList()) {
		y2error ("Bad value for key '%s'", skey.c_str());
		break;
	    }
	    YCPList list = value->asList();
	    if (list->size () > 0)
	    {
		int i;
		for (i = 0; i < list->size(); i++)
		{
		    if (list->value(i).isNull()
			|| !list->value(i)->isString())
		    {
			y2error ("Skipping bad list element for key '%s'", skey.c_str());
		    }
		    else
		    {
			fprintf (f, "%s %s\n", skey.c_str(), list->value(i)->asString()->value().c_str());
		    }
		}
	    }
	}
	else if (skey == "domain") {
	    if (value.isNull() || !value->isString()) {
		y2error ("Bad value for key '%s'", skey.c_str());
		break;
	    }
            if (value->asString ()->value ().size () > 0)
                fprintf (f, "%s %s\n", skey.c_str(), value->asString()->value().c_str());
	}
        else if (allowedInfoKey (skey.c_str ())) {
            y2debug ("Skipping info key '%s'", skey.c_str());
        }
	else {
	    y2error ("Skipping invalid key '%s'", skey.c_str());
	}
	mptr++;
    }
    fclose (f);
    cacheDirty = false;
    return retval;
}

// check if key is allowed

static int allowedKey (const char *key)
{
    int i = 0;
    while (resolver_keys[i] != 0) {
	if (strcmp (key, resolver_keys[i]) == 0)
	    return 0;
	i = i + 1;
    }
    return -1;
}


//========================================================================

ResolverAgent::ResolverAgent () : file_name ("/etc/resolv.conf")
{
}


ResolverAgent::~ResolverAgent ()
{
    if (cacheValid && cacheDirty)
	flushCache(file_name.c_str ());
}


YCPValue
ResolverAgent::Read (const YCPPath& path, const YCPValue& arg, const YCPValue& optarg)
{
    y2debug ("Read(.resolver%s)", path->toString().c_str());

    fillCache (file_name.c_str ());

    if (path->isRoot()) {
	return localCache;
    }

    YCPValue ret = localCache->value (YCPString (path->component_str(0)));
    if (ret.isNull ())
	return YCPVoid ();
    return ret;
}


YCPBoolean
ResolverAgent::Write (const YCPPath& path, const YCPValue& value,
		      const YCPValue& arg)
{
    y2debug ("Write (.resolver%s)", path->toString().c_str());

    fillCache (file_name.c_str ());

    if (path->isRoot()) {
	if (value.isNull() || value->isVoid())
	    return YCPBoolean (flushCache (file_name.c_str ()) == 0);
	if (!value->isMap())
	{
	    ycp2error ("Bad value to Write (.resolver)");
	    return YCPBoolean (false);
	}
	localCache = value->asMap();
    }
    else {
	const char *key = path->component_str(0).c_str();
	if ((allowedKey (key) == 0) || allowedInfoKey (key)) {
	    localCache->add (YCPString (key), value);
	}
        else if (strcmp (key, "write_header") == 0) {
            if (value->isBoolean ()) {
                writeHeader = value->asBoolean ()->value ();
            }
            else
	    {
                ycp2error ("Bad value to Write (.resolver.write_header)");
		return YCPBoolean (false);
	    }
        }
	else
	    ycp2error ("Bad key %s for Write(.resolver...)", path->component_str(0).c_str ());
	    return YCPBoolean (false);
    }

    cacheDirty = true;
    return YCPBoolean (true);
}


/**
 * Get a list of all subtrees.
 */

YCPList ResolverAgent::Dir (const YCPPath& path)
{
    YCPList retval;

    if (path->isRoot()) {
	int i = 0;
	while (resolver_keys[i] != 0) {
	    retval->add (YCPString (resolver_keys[i]));
	    i = i + 1;
	}
    }
    return retval;
}

YCPValue ResolverAgent::otherCommand (const YCPTerm& term)
{
    string fname = "/etc/resolv.conf";

    string symbol = term->name ();
    if (symbol == "ResolverAgent" && term->size () == 1) {
	if (term->value(0)->isString()) {
    	    fname = term->value (0)->asString ()->value ();
            y2debug ("resolving file now: %s", fname.c_str ());
	    cacheValid = false;
	    file_name = fname;
        }
    }
    return term;
}

