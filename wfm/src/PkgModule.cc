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
#include <PkgModuleFunctions.h>

#include <y2util/Url.h>
#include <y2pm/InstData.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;

//-------------------------------------------------------------------
// PkgModule

PkgModule::PkgModule ()
{
}

/**
 * Destructor.
 */
PkgModule::~PkgModule ()
{
}

/**
 * evaluate 'function (list-of-arguments)'
 * and return YCPValue
 */
YCPValue
PkgModule::evaluate (string function, YCPList args)
{
    static PkgModuleFunctions f;

    y2debug ("PkgModule::evaluate (%s, %s)", function.c_str(), args->toString().c_str());

    // general functions
    if (function == "GetGroups")		return f.GetGroups (args);
    else if (function == "GetSelections")	return f.GetSelections (args);
    else if (function == "IsProvided")		return f.IsProvided (args);
    else if (function == "IsAvailable")		return f.IsAvailable (args);
    else if (function == "DoProvide")		return f.DoProvide (args);
    else if (function == "DoRemove")		return f.DoRemove (args);
    else if (function == "PkgSummary")		return f.PkgSummary (args);
    else if (function == "SelSummary")		return f.SelSummary (args);
    else if (function == "SetSelection")	return f.SetSelection (args);
    else if (function == "IsManualSelection")	return f.IsManualSelection (args);
    else if (function == "SaveState")		return f.SaveState (args);
    else if (function == "RestoreState")	return f.RestoreState (args);
    // patch related functions
    else if (function == "YouGetServers")	return f.YouGetServers (args);
    else if (function == "YouGetPatches")	return f.YouGetPatches (args);
    else if (function == "YouGetPackages")	return f.YouGetPackages (args);
    else if (function == "YouInstallPatches")	return f.YouInstallPatches (args);
    // target related functions
    else if (function == "TargetInit")		return f.TargetInit (args);
    else if (function == "TargetFinish")	return f.TargetFinish (args);
    // source related functions
    return YCPError ("Undefined Pkg:: function");
}
