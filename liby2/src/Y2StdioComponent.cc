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

   File:	Y2StdioComponent.cc

   Component that communicates via stdin/out/err

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Arvin Schnell <arvin@suse.de>

/-*/

#include <unistd.h>

#include "Y2StdioComponent.h"
#include <ycp/y2log.h>

#include <ycp/YCPTerm.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPCode.h>

Y2StdioComponent::Y2StdioComponent (bool is_server, bool to_stderr,
				    bool in_batchmode)
    : is_server (is_server),
      to_stderr (to_stderr),
      batchmode (in_batchmode),
      parser (STDIN_FILENO, "<stdin>")
{
}


Y2StdioComponent::~Y2StdioComponent()
{
}


string Y2StdioComponent::name() const
{
    if (batchmode)
	return "testsuite";
    return to_stderr ? "stderr" : "stdio";
}


YCPValue Y2StdioComponent::evaluate (const YCPValue& command)
{
    if (!is_server && !batchmode) {
	// FIXME:
	return YCPVoid ();
    }

    send (command);

    // don't receive anything in batchmode
    if (batchmode)
    {
	return YCPVoid ();
    }

    while (true)
    {
	YCPValue ret = receive ();

	if (!ret.isNull ())
	    return ret;

	y2error ("Couldn't read return value from stdin. Aborting!");
	exit (10);
	return YCPVoid ();
    }
}


void Y2StdioComponent::result (const YCPValue& result)
{
    YCPTerm resultterm ("result");
    resultterm->add (result);
    send (resultterm);
}


void Y2StdioComponent::setServerOptions(int, char **)
{
}


YCPValue Y2StdioComponent::doActualWork (const YCPList& arglist,
					 Y2Component *user_interface)
{
    send (arglist);

    YCPValue value = YCPNull();
    while (!(value = receive()).isNull())
    {
	if (value->isTerm() && value->asTerm()->size() == 1 &&
	    value->asTerm()->name () == "result")
	{
	    return value->asTerm()->value(0);
	}

	send (user_interface->evaluate(value));
    }

    y2warning ("Communication ended prior to result () message");
    return YCPVoid ();
}


void
Y2StdioComponent::send (const YCPValue& v) const
{
    string s = "(" + (v.isNull () ? "(nil)" : v->toString ()) + ")\n";
    y2debug ("send begin %s", s.c_str ());
    write (to_stderr ? STDERR_FILENO : STDOUT_FILENO, s.c_str (), s.length ());
    y2debug ("send end %s", s.c_str ());
}


YCPValue
Y2StdioComponent::receive ()
{
    y2debug ("receive begin");
    YCodePtr pc = parser.parse ();
    if (pc)
    {
	y2debug ("receive end %s", pc->toString ().c_str ());
	return YCPCode (pc);
    }
    y2debug ("receive end - parse error");
    return YCPNull ();
}
