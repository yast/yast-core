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

   File:	Y2WFMComponent.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include <ycp/y2log.h>
#include <wfm/Y2WFMComponent.h>
#include <wfm/WFMInterpreter.h>


Y2WFMComponent::Y2WFMComponent ()
{
    interpreter_ptr = 0;
}


Y2WFMComponent::~Y2WFMComponent ()
{
    if (interpreter_ptr != 0)
	delete interpreter_ptr;
}


string
Y2WFMComponent::name () const
{
    return "wfm";
}


YCPValue
Y2WFMComponent::doActualWork (const YCPList& arglist, Y2Component *displayserver)
{
    // wfm always gets three arguments:
    // 0: any script:        script to execute
    // 1: string modulename: name of the module to realize
    // 2: list arglist:      arguments for that module

    YCPValue script = YCPVoid();
    string   modulename = "unknown";
    string fullname = "unknown";
    YCPList  args_for_the_script;

    if (arglist->size() != 4
	|| !arglist->value(1)->isString()
	|| !arglist->value(2)->isString()
	|| !arglist->value(3)->isList())
    {
	y2error ("Incorrect arguments %s", arglist->toString().c_str());
    }
    else
    {
	script		    = arglist->value(0);
	modulename	    = arglist->value(1)->asString()->value();
	fullname	    = arglist->value(2)->asString()->value();
	args_for_the_script = arglist->value(3)->asList();
    }

    WFMInterpreter interpreter (this, displayserver, modulename, fullname,
				args_for_the_script);

    // if a user_interface is already initialized with a callback pointer
    // (always pointing to a workflowmanager), this must be switched since
    // we're creating a new, temporary workflowmanager here.
    // All further code runs through this workflowmanager and therefore
    // all ui callbacks must be directed to this workflowmanager.

    Y2Component *old_ui_callback = displayserver->getCallback ();
    displayserver->setCallback (this);

    y2debug ("Y2WFMComponent @ %p, WFMInterpreter @ %p, displayserver @ %p, old_ui_callback @ %p", this, &interpreter, displayserver, old_ui_callback);

    // set member pointer to interpreter, so a callback->evaluate()
    // can find the interpreter.

    interpreter_ptr = &interpreter;
    YCPValue v = interpreter.evaluate(script);
    interpreter_ptr = 0;

    // restore old callback pointer in user_interface
    displayserver->setCallback (old_ui_callback);

    return v;
}


YCPValue
Y2WFMComponent::evaluate (const YCPValue& command)
{
    // check if we have an interpreter.

    if (interpreter_ptr)
	return interpreter_ptr->evaluate (command);
    else
	return YCPError("No interpreter for WFM callback available");
}

