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

//    y2milestone ("PkgModule::evaluate (%s, %s)", function.c_str(), args->toString().c_str());

    // general functions
    if (function == "CheckSpace")		return f.CheckSpace (args);
    // package functions
    else if (function == "GetPackages")		return f.GetPackages (args);
    else if (function == "IsProvided")		return f.IsProvided (args);
    else if (function == "IsAvailable")		return f.IsAvailable (args);
    else if (function == "IsSelected")		return f.IsSelected (args);
    else if (function == "DoProvide")		return f.DoProvide (args);
    else if (function == "DoRemove")		return f.DoRemove (args);
    else if (function == "PkgSummary")		return f.PkgSummary (args);
    else if (function == "PkgVersion")		return f.PkgVersion (args);
    else if (function == "PkgSize")		return f.PkgSize (args);
    else if (function == "PkgLocation")		return f.PkgLocation (args);
    else if (function == "PkgMediaNr")		return f.PkgMediaNr (args);
    else if (function == "IsManualSelection")	return f.IsManualSelection (args);
    else if (function == "SaveState")		return f.SaveState (args);
    else if (function == "RestoreState")	return f.RestoreState (args);
    else if (function == "PkgPrepareOrder")	return f.PkgPrepareOrder (args);
    else if (function == "PkgNextDelete")	return f.PkgNextDelete (args);
    else if (function == "PkgNextInstall")	return f.PkgNextInstall (args);
    // selection related
    else if (function == "GetSelections")	return f.GetSelections (args);
    else if (function == "SelectionData")	return f.SelectionData (args);
    else if (function == "SetSelection")	return f.SetSelection (args);
    else if (function == "ClearSelection")	return f.ClearSelection (args);
    else if (function == "ActivateSelections")	return f.ActivateSelections (args);
    // patch related functions
    else if (function == "YouStatus")		return f.YouStatus (args);
    else if (function == "YouGetServers")	return f.YouGetServers (args);
    else if (function == "YouGetPatches")	return f.YouGetPatches (args);
    else if (function == "YouAttachSource")	return f.YouAttachSource (args);
    else if (function == "YouGetPackages")	return f.YouGetPackages (args);
    else if (function == "YouSelectPatches")	return f.YouSelectPatches (args);
    else if (function == "YouFirstPatch")	return f.YouFirstPatch (args);
    else if (function == "YouNextPatch")	return f.YouNextPatch (args);
    else if (function == "YouGetCurrentPatch")	return f.YouGetCurrentPatch (args);
    else if (function == "YouInstallCurrentPatch")	return f.YouInstallCurrentPatch (args);
    else if (function == "YouInstallPatches")	return f.YouInstallPatches (args);
    else if (function == "YouRemovePackages")	return f.YouRemovePackages (args);
    // target related functions
    else if (function == "TargetInit")		return f.TargetInit (args);
    else if (function == "TargetFinish")	return f.TargetFinish (args);
    else if (function == "TargetInstall")	return f.TargetInstall (args);
    else if (function == "TargetRemove")	return f.TargetRemove (args);
    else if (function == "TargetLogfile")	return f.TargetLogfile (args);
    // source related functions
    else if (function == "SourceInit")		return f.SourceInit (args);
    else if (function == "SourceFinish")	return f.SourceFinish (args);
    else if (function == "SourceGeneralData")	return f.SourceGeneralData (args);
    else if (function == "SourceMediaData")	return f.SourceMediaData (args);
    else if (function == "SourceProductData")	return f.SourceProductData (args);
    else if (function == "SourceProvide")	return f.SourceProvide (args);
    return YCPError ("Undefined Pkg:: function");
}
