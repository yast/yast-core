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

   File:	Y2WFMComponent.h

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Mathias Kettner <kettner@suse.de>

/-*/

#ifndef Y2WFMComponent_h
#define Y2WFMComponent_h

#include <y2/Y2Component.h>


class WFMInterpreter;


class Y2WFMComponent : public Y2Component
{

public:
    /**
     * Creates a new WFM component
     */
    Y2WFMComponent();

    /**
     * Cleans up
     */
    ~Y2WFMComponent();

    /**
     * Returns "wfm";
     */
    string name() const;

    /**
     * Executes the YCP script.
     */
    YCPValue doActualWork(const YCPList& arglist, Y2Component *displayserver);

    /**
     * callback entry point
     *   usually calls back into Y2WFMInterpreter::evaluate
     *   We're not using a pointer here because the evaluate() slot
     *   already exists in the Y2Component class
     */
    YCPValue evaluate(const YCPValue& command);

private:
    /**
     * used to pass interpreter from doActualWork ()
     * to evaluate () for callback purposes
     */
    WFMInterpreter *interpreter_ptr;

};


#endif // Y2WFMComponent_h
