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

   File:	PkgModule.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>

#include <PkgModule.h>
#include <ycp/YCPVoid.h>

using std::string;


/**
 * Constructor.
 */
PkgModule::PkgModule ()
{
  // todo: init packagemanager
}

/**
 * Destructor.
 */
PkgModule::~PkgModule ()
{
  // todo: release packagemanager
}

/**
 * evaluate 'function (list-of-arguments)'
 * and return YCPValue
 */
YCPValue
PkgModule::evaluate (string function, YCPList arguments)
{
    y2milestone ("PkgModule::evaluate (%s, %s)", function.c_str(), arguments->toString().c_str());

    return YCPVoid();
}

