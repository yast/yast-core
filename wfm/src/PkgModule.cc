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
    else if (function == "SetLocale")		return SetLocale (args);
    else if (function == "GetLocale")		return GetLocale (args);
    else if (function == "SetAdditionalLocales")return SetAdditionalLocales (args);
    else if (function == "GetAdditionalLocales")return GetAdditionalLocales (args);
    else if (function == "Error")		return Error (args);
    else if (function == "ErrorDetails")	return ErrorDetails (args);
    else if (function == "ErrorId")		return ErrorId (args);

    // callback functions
    else if (function == "CallbackStartProvide")	return CallbackStartProvide (args);
    else if (function == "CallbackProgressProvide")	return CallbackProgressProvide (args);
    else if (function == "CallbackDoneProvide")		return CallbackDoneProvide (args);
    else if (function == "CallbackStartPackage")	return CallbackStartPackage (args);
    else if (function == "CallbackProgressPackage")	return CallbackProgressPackage (args);
    else if (function == "CallbackDonePackage")		return CallbackDonePackage (args);
    else if (function == "CallbackMediaChange")		return CallbackMediaChange (args);
    else if (function == "CallbackProgressRebuildDB")	return CallbackProgressRebuildDB (args);
    else if (function == "CallbackSourceChange")	return CallbackSourceChange (args);
    else if (function == "CallbackYouProgress")		return CallbackYouProgress (args);
    else if (function == "CallbackYouPatchProgress")	return CallbackYouPatchProgress (args);

    // package functions
    else if (function == "GetPackages")		return GetPackages (args);
    else if (function == "GetBackupPath")	return GetBackupPath (args);
    else if (function == "SetBackupPath")	return SetBackupPath (args);
    else if (function == "CreateBackups")	return CreateBackups (args);
    else if (function == "IsProvided")		return IsProvided (args);
    else if (function == "IsAvailable")		return IsAvailable (args);
    else if (function == "IsSelected")		return IsSelected (args);
    else if (function == "DoProvide")		return DoProvide (args);
    else if (function == "DoRemove")		return DoRemove (args);
    else if (function == "PkgSummary")		return PkgSummary (args);
    else if (function == "PkgVersion")		return PkgVersion (args);
    else if (function == "PkgSize")		return PkgSize (args);
    else if (function == "PkgGroup")		return PkgGroup (args);
    else if (function == "IsManualSelection")	return IsManualSelection (args);
    else if (function == "ClearSaveState")	return ClearSaveState (args);
    else if (function == "SaveState")		return SaveState (args);
    else if (function == "RestoreState")	return RestoreState (args);
    else if (function == "PkgUpdateAll")	return PkgUpdateAll (args);
    else if (function == "PkgAnyToDelete")	return PkgAnyToDelete (args);
    else if (function == "PkgAnyToInstall")	return PkgAnyToInstall (args);

    else if (function == "PkgInstall")		return PkgInstall (args);
    else if (function == "PkgDelete")		return PkgDelete (args);
    else if (function == "PkgNeutral")		return PkgNeutral (args);
    else if (function == "PkgSolve")		return PkgSolve (args);
    else if (function == "PkgSolveErrors")	return PkgSolveErrors (args);
    else if (function == "PkgCommit")		return PkgCommit (args);

    else if (function == "PkgMediaSizes")	return PkgMediaSizes (args);
    else if (function == "PkgMediaNames")	return PkgMediaNames (args);

    // selection related
    else if (function == "GetSelections")	return GetSelections (args);
    else if (function == "SelectionData")	return SelectionData (args);
    else if (function == "SetSelection")	return SetSelection (args);
    else if (function == "ClearSelection")	return ClearSelection (args);
    else if (function == "ActivateSelections")	return ActivateSelections (args);
    else if (function == "SelectionsUpdateAll")	return SelectionsUpdateAll (args);
    // patch related functions
    else if (function == "YouStatus")		return YouStatus (args);
    else if (function == "YouCheckAuthorization")	return YouCheckAuthorization (args);
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
    else if (function == "YouFinish")		return YouFinish (args);
    // target related functions
    else if (function == "TargetInit")		return TargetInit (args);
    else if (function == "TargetFinish")	return TargetFinish (args);
    else if (function == "TargetInstall")	return TargetInstall (args);
    else if (function == "TargetRemove")	return TargetRemove (args);
    else if (function == "TargetLogfile")	return TargetLogfile (args);
    else if (function == "TargetCapacity")	return TargetCapacity (args);
    else if (function == "TargetUsed")		return TargetUsed (args);
    else if (function == "TargetBlockSize")	return TargetBlockSize (args);
    else if (function == "TargetUpdateInf")	return TargetUpdateInf (args);
    else if (function == "TargetProducts")	return TargetProducts (args);
    else if (function == "TargetRebuildDB")	return TargetRebuildDB (args);
    else if (function == "TargetInitDU")	return TargetInitDU (args);
    else if (function == "TargetGetDU")		return TargetGetDU (args);
    // source related functions
    else if (function == "SourceStartManager")	return SourceStartManager(args);
    else if (function == "SourceCreate")	return SourceCreate (args);
    else if (function == "SourceStartCache")	return SourceStartCache (args);
    else if (function == "SourceGetCurrent")	return SourceGetCurrent (args);
    else if (function == "SourceFinish")	return SourceFinish (args);
    else if (function == "SourceFinishAll")	return SourceFinishAll (args);
    else if (function == "SourceGeneralData")	return SourceGeneralData (args);
    else if (function == "SourceMediaData")	return SourceMediaData (args);
    else if (function == "SourceProductData")	return SourceProductData (args);
    else if (function == "SourceProvideFile")	return SourceProvideFile (args);
    else if (function == "SourceProvideDir")	return SourceProvideDir (args);
    else if (function == "SourceCacheCopyTo")	return SourceCacheCopyTo (args);
    else if (function == "SourceSetRamCache")	return SourceSetRamCache (args);
    else if (function == "SourceProduct")	return SourceProduct (args);
    else if (function == "SourceSetEnabled")	return SourceSetEnabled (args);
    else if (function == "SourceDelete")	return SourceDelete (args);
    else if (function == "SourceRaisePriority")	return SourceRaisePriority (args);
    else if (function == "SourceLowerPriority")	return SourceLowerPriority (args);
    else if (function == "SourceSaveRanks")	return SourceSaveRanks (args);
    else if (function == "SourceChangeUrl")	return SourceChangeUrl(args);
    else if (function == "SourceInstallOrder")	return SourceInstallOrder(args);
    return YCPError (string ("Undefined Pkg::")+function);
}
