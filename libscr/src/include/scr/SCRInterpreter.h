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

   File:       SCRInterpreter.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Arvin Schnell <arvin@suse.de>

/-*/
// -*- c++ -*-

#ifndef SCRInterpreter_h
#define SCRInterpreter_h

#include <stdio.h>

#include <ycp/YCPInterpreter.h>


class SCRAgent;


class SCRInterpreter : public YCPInterpreter
{

public:

    SCRInterpreter (SCRAgent *agent);

    ~SCRInterpreter ();

    /**
     * Name of interpreter, returns "SCR".
     */
    string interpreter_name () const;

    YCPValue evaluateSCR (const YCPValue& value);

protected:

    YCPValue evaluateInstantiatedTerm (const YCPTerm& term);

private:

    /**
     * Toplevel agent that does the actual work.
     */
    SCRAgent *agent;

};


#endif // SCRInterpreter_h
