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

PkgModule::PkgModule (YCPInterpreter *wfmInterpreter)
    : PkgModuleFunctions (wfmInterpreter)
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
//    y2milestone ("PkgModule::evaluate (%s, %s)", function.c_str(), args->toString().c_str());

    // general functions
    if (function == "InstSysMode")		return InstSysMode (args);
    else if (function == "CheckSpace")		return CheckSpace (args);
    else if (function == "SetLocale")		return SetLocale (args);
    else if (function == "GetLocale")		return GetLocale (args);
    else if (function == "SetAdditionalLocales")return SetAdditionalLocales (args);
    else if (function == "GetAdditionalLocales")return GetAdditionalLocales (args);
    // package functions
    else if (function == "GetPackages")		return GetPackages (args);
    else if (function == "IsProvided")		return IsProvided (args);
    else if (function == "IsAvailable")		return IsAvailable (args);
    else if (function == "IsSelected")		return IsSelected (args);
    else if (function == "DoProvide")		return DoProvide (args);
    else if (function == "DoRemove")		return DoRemove (args);
    else if (function == "PkgSummary")		return PkgSummary (args);
    else if (function == "PkgVersion")		return PkgVersion (args);
    else if (function == "PkgSize")		return PkgSize (args);
    else if (function == "PkgLocation")		return PkgLocation (args);
    else if (function == "PkgMediaNr")		return PkgMediaNr (args);
    else if (function == "IsManualSelection")	return IsManualSelection (args);
    else if (function == "SaveState")		return SaveState (args);
    else if (function == "RestoreState")	return RestoreState (args);
    else if (function == "PkgUpdateAll")	return PkgUpdateAll (args);

    else if (function == "PkgInstall")		return PkgInstall (args);
    else if (function == "PkgDelete")		return PkgDelete (args);
    else if (function == "PkgSolve")		return PkgSolve (args);
    else if (function == "PkgCommit")		return PkgCommit (args);

    else if (function == "PkgPrepareOrder")	return PkgPrepareOrder (args);
    else if (function == "PkgMediaSizes")	return PkgMediaSizes (args);
    else if (function == "PkgNextDelete")	return PkgNextDelete (args);
    else if (function == "PkgNextInstall")	return PkgNextInstall (args);
    // selection related
    else if (function == "GetSelections")	return GetSelections (args);
    else if (function == "SelectionData")	return SelectionData (args);
    else if (function == "SetSelection")	return SetSelection (args);
    else if (function == "ClearSelection")	return ClearSelection (args);
    else if (function == "ActivateSelections")	return ActivateSelections (args);
    else if (function == "SelectionsUpdateAll")	return SelectionsUpdateAll (args);
    // patch related functions
    else if (function == "YouStatus")		return YouStatus (args);
    else if (function == "YouGetServers")	return YouGetServers (args);
    else if (function == "YouGetPatches")	return YouGetPatches (args);
    else if (function == "YouAttachSource")	return YouAttachSource (args);
    else if (function == "YouGetPackages")	return YouGetPackages (args);
    else if (function == "YouSelectPatches")	return YouSelectPatches (args);
    else if (function == "YouFirstPatch")	return YouFirstPatch (args);
    else if (function == "YouNextPatch")	return YouNextPatch (args);
    else if (function == "YouGetCurrentPatch")	return YouGetCurrentPatch (args);
    else if (function == "YouInstallCurrentPatch")	return YouInstallCurrentPatch (args);
    else if (function == "YouInstallPatches")	return YouInstallPatches (args);
    else if (function == "YouRemovePackages")	return YouRemovePackages (args);
    // target related functions
    else if (function == "TargetInit")		return TargetInit (args);
    else if (function == "TargetFinish")	return TargetFinish (args);
    else if (function == "TargetInstall")	return TargetInstall (args);
    else if (function == "TargetRemove")	return TargetRemove (args);
    else if (function == "TargetLogfile")	return TargetLogfile (args);
    else if (function == "SetProgressCallback")	return SetProgressCallback (args);
    else if (function == "TargetCapacity")	return TargetCapacity (args);
    else if (function == "TargetUsed")		return TargetUsed (args);
    // source related functions
    else if (function == "SourceInit")		return SourceInit (args);
    else if (function == "SourceList")		return SourceList (args);
    else if (function == "SourceFinish")	return SourceFinish (args);
    else if (function == "SourceGeneralData")	return SourceGeneralData (args);
    else if (function == "SourceMediaData")	return SourceMediaData (args);
    else if (function == "SourceProductData")	return SourceProductData (args);
    else if (function == "SourceProvide")	return SourceProvide (args);
    else if (function == "SourceCacheCopyTo")	return SourceCacheCopyTo (args);
    return YCPError ("Undefined Pkg:: function");
}
