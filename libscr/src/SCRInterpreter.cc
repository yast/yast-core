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

   File:       SCRInterpreter.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/

#include <stdio.h>
#include <unistd.h>

#include <ycp/y2log.h>

#include "include/scr/SCRAgent.h"
#include "include/scr/SCRInterpreter.h"


SCRInterpreter::SCRInterpreter (SCRAgent *agent)
    : agent (agent)
{
}


SCRInterpreter::~SCRInterpreter ()
{
}


string
SCRInterpreter::interpreter_name () const
{
    return "SCR";	// must be upper case
}


YCPValue SCRInterpreter::evaluateInstantiatedTerm(const YCPTerm& term)
{
    string sym = term->symbol()->symbol();

    if (sym == "Read")
    {
	if (term->size() > 0 && term->value(0)->isPath())
	{
	    switch (term->size())
	    {
		case 1:
		    return agent->Read(term->value(0)->asPath());

		case 2:
		    return agent->Read(term->value(0)->asPath(),
				       term->value(1));
	    }
	}
	return YCPError ("Read must have 1 or 2 arguments "
			 "(and first must be a path)");
    }
    else if (sym == "Write")
    {
	if (term->size() > 0 && term->value(0)->isPath())
	{
	    switch (term->size())
	    {
		case 2:
		    return agent->Write(term->value(0)->asPath(),
					term->value(1));

		case 3:
		    return agent->Write(term->value(0)->asPath(),
					term->value(1), term->value(2));
	    }
	}
	return YCPError ("Write must have 2 or 3 arguments "
			 "(and first must be a path)", YCPBoolean (false));
    }
    else if (sym == "Execute")
    {
	if (term->size() > 0 && term->value(0)->isPath())
	{
	    switch (term->size())
	    {
		case 1:
		    return agent->Execute(term->value(0)->asPath());

		case 2:
		    return agent->Execute(term->value(0)->asPath(),
					  term->value(1));

		case 3:
		    return agent->Execute(term->value(0)->asPath(),
					  term->value(1), term->value(2));
	    }
	}
	return YCPError ("Execute must have 1, 2 or 3 arguments "
			 "(and first must be a path)");
    }
    else if (sym == "Dir")
    {
	if (term->size() > 0 && term->value(0)->isPath())
	{
	    switch (term->size())
	    {
		case 1:
		    return agent->Dir(term->value(0)->asPath());
	    }
	}
	return YCPError ("Dir must have 1 argument "
			 "(and first must be a path)");
    }
    else
	return agent->otherCommand(term);
}
