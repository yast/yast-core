

/*
 *  SCRSubAgent.cc
 *
 *  Basic SCR agent handling stuff
 *
 *  Authors:	Arvin Schnell <arvin@suse.de>
 *		Klaus Kaempf <kkaempf@suse.de>
 *  Maintainer:	Arvin Schnell <arvin@suse.de>
 *
 *  $Id$
 */


#include <ycp/y2log.h>
#include <y2/Y2ComponentBroker.h>
#include "SCRSubAgent.h"


int
operator < (const SCRSubAgent *a, const YCPPath &p)
{
    return a->my_path->compare (p) == YO_LESS;
}


SCRSubAgent::SCRSubAgent (YCPPath path, YCPValue value)
    : my_path (path),
      my_value (value),
      my_comp (0)
{
}


SCRSubAgent::~SCRSubAgent ()
{
    unmount ();
}


YCPValue
SCRSubAgent::mount (SCRAgent *parent)
{
    if (!my_comp)
    {
	y2debug ("Mounting agent at '%s'", my_path->toString ().c_str ());

	// get term with description or something like that
	YCPTerm term = YCPTerm (YCPNull ());
	if (my_value->isString ())
	{
	    YCPValue confval = parent->readconf (my_value->asString ()->value_cstr ());
	    if (confval.isNull () || !confval->isTerm ())
	    {
		if (!confval.isNull () && confval->isError ())
		{
		    YCPError err = confval->asError ();
		    ycp2error ("", 0, "%s\n", err->message ().c_str ());
		    confval = err->value ();
		}
		return confval;
	    }
	    term = confval->asTerm ();
	}
	else if (my_value->isTerm ())
	{
	    term = my_value->asTerm ();
	}
	else
	{
	    return YCPError ("value has wrong type", YCPBoolean (false));
	}

	if (term.isNull ())
	{
	    return YCPError ("term is null", YCPBoolean (false));
	}

	string componentname = term->symbol ()->symbol ();
	my_comp = Y2ComponentBroker::createServer (componentname.c_str ());

	if (!my_comp)
	{
	    return YCPError ("Can't find component '" + componentname + "'",
			     YCPBoolean (false));
	}

	// set mainscragent of new agent
	SCRAgent *tmpscragent = my_comp->getSCRAgent ();
	if (tmpscragent)
	{
	    tmpscragent->mainscragent = parent;
	}

	// term's arguments are preloaded into the server component
	for (int i = 0; i < term->size (); i++)
	{
	    YCPValue arg = term->value (i);

	    // WORKAROUND/HACK: adapting the old SCR files to the new interpreter
	    // new interpreter requires every argument of an agent to
	    // be quoted - but this will not work in the old interpter
	    // =>strip quote if exists
	    if (arg->isTerm ())
	    {
		arg = YCPTerm ( YCPSymbol( arg->asTerm ()->symbol ()->symbol (), false ),
		    arg->asTerm ()->args () );
	    }
	    my_comp->evaluate (arg);
	}
    }

    return YCPBoolean (true);
}


void
SCRSubAgent::unmount ()
{
    if (my_comp)
    {
	y2debug ("Unmounting agent at '%s'", my_path->toString ().c_str ());
	my_comp->result (YCPVoid ()); // tell server to terminate
	delete my_comp;
	my_comp = 0;
    }
}
