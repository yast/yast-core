/*
 * DummyAgent.cc
 *
 * A dummy agent, only for testing purposes
 *
 * Author: Klaus Kaempf <kkaempf@suse.de>
 *         Michal Svec <msvec@suse.cz>
 *         Petr Blahos <pblahos@suse.cz>
 *         Gabriele Strattner <gs@suse.de>
 *
 * $Id$
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <string>

#include <YCP.h>
#include <ycp/y2log.h>

#include "DummyAgent.h"
#define DUMMY_LOG_STRING "LOGTHIS_SECRET_314 "

/**
 * Constructor
 */
DummyAgent::DummyAgent ()
    : defaultValue (YCPNull ()),
      readCalls(0), writeCalls(0), execCalls(0)
{
}

/**
 * Dummy 'Read' function
 */
YCPValue
DummyAgent::Read (const YCPPath& path, const YCPValue& arg , const YCPValue&)
{
    YCPValue deflt = YCPVoid ();
    YCPValue v = YCPNull ();

    if (!readList.isNull() && !readList->isEmpty())
    {
	v = checkPath (path, (readList->value(readCalls))->asMap(), defaultValue);
	readCalls = ( readCalls + 1 ) % readList->size();
    }
    else
    {
	v = checkPath(path, defaultMap, deflt);
    }

    ycpdebug("%sRead	%s", DUMMY_LOG_STRING,
	    (path->toString() + (arg.isNull()?"":(" "+arg->toString())) +
			      (v.isNull()?"":(" "+v->toString())) ).c_str());

    return v;
}


/**
 * Dummy 'Write' function
 */
YCPBoolean
DummyAgent::Write (const YCPPath& path, const YCPValue& value,
		   const YCPValue& arg)
{
    YCPBoolean deflt (true);
    YCPValue v = YCPNull();

    if ( !writeList.isNull() && !writeList->isEmpty() )
    {
	v = checkPath(path, (writeList->value(writeCalls))->asMap(), deflt);
	writeCalls = ( writeCalls + 1 ) % writeList->size();
    }
    else
    {
	v = checkPath(path, defaultMap, deflt);
    }

    ycpdebug("%sWrite	%s", DUMMY_LOG_STRING,
	    (path->toString() + " " + value->toString() +
			      (arg.isNull()?"":(" "+arg->toString())) +
			      (v.isNull()?"":(" "+v->toString())) ).c_str());

    return v.isNull () ? YCPNull () : v->asBoolean ();
}


/**
 * Dummy 'Execute' function
 */
YCPValue
DummyAgent::Execute (const YCPPath& path, const YCPValue& value,
		     const YCPValue& arg)
{
    YCPInteger deflt((long long int)0);
    YCPValue v = YCPNull();

    if ( !execList.isNull() && !execList->isEmpty() )
    {
	v = checkPath(path, (execList->value(execCalls))->asMap(), deflt);
	execCalls = ( execCalls + 1 ) % execList->size();
    }
    else
    {
	v = checkPath(path, defaultMap, deflt);
    }
    
    ycpdebug("%sExecute	%s", DUMMY_LOG_STRING,
	    (path->toString() + " " + value->toString() +
			      (arg.isNull()?"":(" "+arg->toString())) +
			      (v.isNull()?"":(" "+v->toString())) ).c_str());

    return v;
}


/**
 * Dummy 'Dir' function
 */
YCPList DummyAgent::Dir(const YCPPath& path)
{
    YCPList l;

    if (!readList.isNull() && !readList->isEmpty())
    {
	YCPValue v = checkPath(path, (readList->value(readCalls))->asMap(), defaultValue);
	if (v->isMap ())
	{
	    YCPMapIterator it = v->asMap()->begin();
	    for(;it!=v->asMap()->end();it++)
	    {
		l->add (it.key ());
	    }
	}
    }

    ycpdebug("%sDir	%s: %s", DUMMY_LOG_STRING,
	    path->toString().c_str(), l->toString ().c_str ());

    return l;
}

/**
 * Parse options and prepare internal structures
 */
YCPValue DummyAgent::otherCommand(const YCPTerm& term)
{
    // y2debug("otherCommand(%s)",term->toString().c_str());
    string sym = term->name();

    if (sym == "DataMap")
    {
	switch (term->size())
	{
	    case 4:

		if ( term->value(2)->isMap() )
		{
		    execList->add( term->value(2)->asMap() );
		}
		else if ( term->value(2)->isList() )
		{
		    execList = term->value(2)->asList();
		}
		else
		{
		    y2error("DataMap() expects map,[map,[map,]]value");
		    return YCPNull();
		}

	    case 3:

		if ( term->value(1)->isMap() )
		{
		    writeList->add( term->value(1)->asMap() );
		}
		else if ( term->value(1)->isList() )
		{
		    writeList = term->value(1)->asList();
		}
		else
		{
		    y2error("DataMap() expects map,[map,[map,]]value");
		    return YCPNull();
		}

	    case 2:

		if (term->value(0)->isMap())
		{
		    readList->add( term->value(0)->asMap() );
		}
		else if (term->value(0)->isList())
		{
		    readList = term->value(0)->asList();
		}
		else
		{
		    y2error("DataMap() expects map,[map,[map,]]value");
		    return YCPNull();
		}

		break;

	    default:
		y2error("DataMap() expects map|list,[map|list,[map|list,]]value");
		return YCPNull();
	}

	defaultValue = term->value (term->size () - 1);
	return YCPVoid();
    }

    y2error("unknown command to DummyAgent");
    return YCPNull();
}

/**
 * Check if the given path is valid and return the correct value
 */
YCPValue DummyAgent::checkPath (const YCPPath& path, const YCPMap& map, const YCPValue& defaultVal)
{
    y2debug("checkPath (%s, %s, %s)", path->toString().c_str(), map->toString().c_str(), defaultVal->toString().c_str());

    if (path->isRoot())
        return defaultVal;

    /* try to access map element as path */
    YCPValue v = map->value(path->at (0));

    /* path access failed, try as string */
    if (v.isNull()) {
        v = map->value (YCPString (path->component_str (0)));
        if (v.isNull()) {
            y2warning("Default value: no key '%s' in %s (%s)", path->component_str (0).c_str(), map->toString().c_str(), defaultVal->toString().c_str());
            return defaultVal;
        }
    }

    /* path is longer, recursive call */
    if ((path->length() > 1)
        && (v->isMap())) {
        YCPPath subpath = path;
        return checkPath (subpath->select (path->at (1))->asPath(), v->asMap(), defaultVal);
    }

    return v;
}


/* EOF */
