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

   File:       Y2Component.cc

   Author:	Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * Base class of all Y2 Components
 *
 */

#include <stdio.h>

#include <ycp/y2log.h>
#include "Y2Component.h"
#include "Y2ComponentBroker.h"


Y2Component::Y2Component()
{
}


Y2Component::~Y2Component()
{
}


YCPValue
Y2Component::evaluate(const YCPValue&)
{
    y2internal ("component %s: stub function Y2Component::evaluate() called", name ().c_str ());
    return YCPNull();
}


void
Y2Component::result(const YCPValue&)
{
    // No warning. It is legal to ignore the result
}


void
Y2Component::setServerOptions(int, char **)
{
    // No warning. It is legal to ignore the options
}


YCPValue
Y2Component::doActualWork(const YCPList&, Y2Component *)
{
    y2internal ("component %s: stub function Y2Component::doActualWork() called",
		name().c_str());
    return YCPNull();
}


Y2Namespace *
Y2Component::import (const char* name_space)
{
    y2internal ("default import (%s) called, should not happen", name_space);
    return NULL;
}


SCRAgent *
Y2Component::getSCRAgent ()
{
    return NULL;
}
