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

   File:	PkgModuleFunctionsTarget.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to InstTarget
		Handles target related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstTarget.h>
#include <y2pm/PMError.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;

// ------------------------
/** 
   @builtin Pkg::TargetInit(string root, bool new) -> bool

   initialized pa
*/
YCPValue
PkgModuleFunctions::TargetInit (YCPList args)
{
    if ((args->size() != 2)
	|| !(args->value(0)->isString())
	|| !(args->value(1)->isBoolean()))
    {
	return YCPError ("Bad args to Pkg::TargetInit");
    }
    PMError err = _y2pm.instTarget().init (Pathname (args->value(0)->asString()->value()), args->value(1)->asBoolean()->value());
    if (err)
	return YCPError (err.errstr().c_str(), YCPBoolean (false));
    return YCPBoolean (true);
}

// ------------------------
/** 
   @builtin Pkg::TargetFinish() -> bool

   finish target usage
*/
YCPValue
PkgModuleFunctions::TargetFinish (YCPList args)
{
    return YCPBoolean (true);
}

