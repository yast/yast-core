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

static std::string progressCallbackModule;
static YCPSymbol progressCallbackSymbol("",false);
static std::string mediaChangeCallbackModule;
static YCPSymbol mediaChangeCallbackSymbol("",false);
static std::string mediaErrorCallbackModule;
static YCPSymbol mediaErrorCallbackSymbol("",false);

void
progressCallbackFunc (int percent, void *_wfm)
{
    YCPTerm callback = YCPTerm (progressCallbackSymbol, progressCallbackModule);
    callback->add(YCPInteger (percent));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}

/**
 * media change callback
 *
 * see packagemanager/src/inst/include/y2pm/InstSrc:_mediachangefunc
 *
 */
static int
mediaChangeCallbackFunc (const std::string& product, Url& url, int expected, int current, void *_wfm)
{
y2milestone ("mediaChangeCallbackFunc(%s,%s,%d,%d)", product.c_str(), url.asString().c_str(), expected, current);
    YCPTerm callback = YCPTerm (mediaChangeCallbackSymbol, mediaChangeCallbackModule);
    callback->add(YCPString (product));
    callback->add(YCPString (url.asString()));
    callback->add(YCPInteger (expected));
    callback->add(YCPInteger (current));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isInteger())
	return 0;
    return ret->asInteger()->value();
}

static int
mediaErrorCallbackFunc (PMError error, void *_wfm)
{
y2milestone ("mediaErrorCallbackFunc(%d)", (int)error);
    YCPTerm callback = YCPTerm (mediaErrorCallbackSymbol, mediaErrorCallbackModule);
    callback->add(YCPInteger (error));
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
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	progressCallbackModule = name.substr (0, colonpos);
	progressCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	progressCallbackModule = "";
	progressCallbackSymbol = YCPSymbol (name, false);
    }
y2milestone ("progressCallback (%s::%s)", progressCallbackModule.c_str(), progressCallbackSymbol->symbol().c_str());
    _y2pm.instTarget().setPackageInstallProgressCallback (progressCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::SourceSetMediaChangeCallback (integer source, string fun) -> nil
 *
 * set media change callback function
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
    string name = args->value(1)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	mediaChangeCallbackModule = name.substr (0, colonpos);
	mediaChangeCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	mediaChangeCallbackModule = "";
	mediaChangeCallbackSymbol = YCPSymbol (name, false);
    }
y2milestone ("mediaChangeCallback (%s::%s)", mediaChangeCallbackModule.c_str(), mediaChangeCallbackSymbol->symbol().c_str());
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
    string name = args->value(1)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	mediaErrorCallbackModule = name.substr (0, colonpos);
	mediaErrorCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	mediaErrorCallbackModule = "";
	mediaErrorCallbackSymbol = YCPSymbol (name, false);
    }
y2milestone ("mediaErrorCallback (%s::%s)", mediaErrorCallbackModule.c_str(), mediaErrorCallbackSymbol->symbol().c_str());
    (InstSrcPtr::cast_away_const(source_id))->setMediaErrorCallback(mediaErrorCallbackFunc, _wfm);
    return YCPVoid();
}


