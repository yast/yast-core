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

   File:	PkgModuleCallbacks.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Implement callbacks from package manager to UI/WFM


All callbacks have a common interface:
 (string  <name>	name-version-release.arch of package
  string  <label>	summary of package
  integer <percent>	percent of package
  integer <number>	number of package
  integer <disk_used>	amount of bytes used
  integer <disk_size>	amount of bytes on disk
  string  <error>	error message from rpm (empty string if no error)
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPError.h>

using std::string;

//-------------------------------------------------------------------
// PkgModuleCallbacks

static YCPSymbol progressCallback("",false);
static YCPSymbol mediaChangeCallback("",false);
static YCPSymbol mediaErrorCallback("",false);

void
progressCallbackFunc (int percent, void *_wfm)
{
    YCPTerm callback = YCPTerm (progressCallback);
    callback->add(YCPInteger (percent));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}

int
mediaChangeCallbackFunc (int code, int expected, int current, void *_wfm)
{
    YCPTerm callback = YCPTerm (mediaChangeCallback);
    callback->add(YCPInteger (code));
    callback->add(YCPInteger (expected));
    callback->add(YCPInteger (current));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isInteger())
	return 0;
    return ret->asInteger()->value();
}

int
mediaErrorCallbackFunc (const std::string& error, void *_wfm)
{
    YCPTerm callback = YCPTerm (mediaErrorCallback);
    callback->add(YCPString (error));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isInteger())
	return 0;
    return ret->asInteger()->value();
}

/**
 * @builtin Pkg::SetProgressCallback (string fun) -> nil
 *
 * set progress callback function
 * will call 'WFM::fun (integer percent)' during rpm
 */
YCPValue
PkgModuleFunctions::SetProgressCallback(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetProgressCallback");
    }
    progressCallback = YCPSymbol (args->value(0)->asString()->value(), false);
    _y2pm.instTarget().setPackageInstallProgressCallback (progressCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::SourceSetMediaChangeCallback (integer source, string fun) -> nil
 *
 * set media change callback function
 * will call 'WFM::fun (int code, int currentnr, int expectednr)' from InstSrc
 */
YCPValue
PkgModuleFunctions::SourceSetMediaChangeCallback (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 2)
	|| !(args->value(1)->isString()))
    {
	return YCPError ("Bad args to Pkg::SourceSetMediaChangeCallback");
    }
    mediaChangeCallback = YCPSymbol (args->value(1)->asString()->value(), false);
    (InstSrcPtr::cast_away_const(source_id))->setMediaChangeCallback (mediaChangeCallbackFunc, _wfm);
    return YCPVoid();
}

/**
 * @builtin Pkg::SourceSetMediaErrorCallback (integer source, string fun) -> nil
 *
 * set media error callback function
 * will call 'WFM::fun (string error)' from InstSrc
 */
YCPValue
PkgModuleFunctions::SourceSetMediaErrorCallback (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 2)
	|| !(args->value(1)->isString()))
    {
	return YCPError ("Bad args to Pkg::SourceSetMediaErrorCallback");
    }
    mediaErrorCallback = YCPSymbol (args->value(1)->asString()->value(), false);
    (InstSrcPtr::cast_away_const(source_id))->setMediaErrorCallback(mediaErrorCallbackFunc, _wfm);
    return YCPVoid();
}


