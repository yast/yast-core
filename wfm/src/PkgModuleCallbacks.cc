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
//-------------------------------------------------------------------

//-------------------------------------------------------------------
// at start of package provide (i.e. download)
static std::string provideStartCallbackModule;
static YCPSymbol provideStartCallbackSymbol("",false);

static void
provideStartCallbackFunc (const std::string& name, const FSize& size, bool remote, void *_wfm)
{
    YCPTerm callback = YCPTerm (provideStartCallbackSymbol, provideStartCallbackModule);
    callback->add(YCPString (name));
    callback->add(YCPInteger (size));
    callback->add(YCPBoolean (remote));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}

//-------------------------------------------------------------------
// during package providal
static std::string provideProgressCallbackModule;
static YCPSymbol provideProgressCallbackSymbol("",false);

static void
provideProgressCallbackFunc (int percent, void *_wfm)
{
    YCPTerm callback = YCPTerm (provideProgressCallbackSymbol, provideProgressCallbackModule);
    callback->add(YCPInteger (percent));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}

// at end of package provide (i.e. download)
static std::string provideDoneCallbackModule;
static YCPSymbol provideDoneCallbackSymbol("",false);

static void
provideDoneCallbackFunc (PMError error, const std::string& reason, void *_wfm)
{
    YCPTerm callback = YCPTerm (provideDoneCallbackSymbol, provideDoneCallbackModule);
    callback->add(YCPInteger (error));
    callback->add(YCPString (reason));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}

//-------------------------------------------------------------------
// at start of package installation (rpm call)
static std::string startPackageCallbackModule;
static YCPSymbol startPackageCallbackSymbol("",false);

static void
startPackageCallbackFunc (const std::string& name, const std::string& summary, const FSize& size, bool is_delete, void *_wfm)
{
    YCPTerm callback = YCPTerm (startPackageCallbackSymbol, startPackageCallbackModule);
    callback->add(YCPString (name));		// package name
    callback->add(YCPString (summary));		// package summary
    callback->add(YCPInteger((long long)size));	// package size
    callback->add(YCPBoolean(is_delete));	// package is being deleted
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}


//-------------------------------------------------------------------
// during package installation (rpm progress)
static std::string progressPackageCallbackModule;
static YCPSymbol progressPackageCallbackSymbol("",false);

static void
progressPackageCallbackFunc (int percent, void *_wfm)
{
    YCPTerm callback = YCPTerm (progressPackageCallbackSymbol, progressPackageCallbackModule);
    callback->add(YCPInteger (percent));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}

//-------------------------------------------------------------------
// at end of package installation
static std::string donePackageCallbackModule;
static YCPSymbol donePackageCallbackSymbol("",false);

static void
donePackageCallbackFunc (PMError err, const std::string& reason, void *_wfm)
{
    YCPTerm callback = YCPTerm (donePackageCallbackSymbol, donePackageCallbackModule);
    callback->add(YCPInteger (err));
    callback->add(YCPString (reason));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}


//-------------------------------------------------------------------
// for media change
static std::string mediaChangeCallbackModule;
static YCPSymbol mediaChangeCallbackSymbol("",false);

/**
 * media change callback
 *
 * see packagemanager/src/inst/include/y2pm/InstSrc:_mediachangefunc
 *
 */
static std::string
mediaChangeCallbackFunc (const std::string& product, const std::string& error, int expected, int current, void *_wfm)
{
y2milestone ("mediaChangeCallbackFunc(%s,%s,%d,%d)", product.c_str(), error.c_str(), expected, current);
    YCPTerm callback = YCPTerm (mediaChangeCallbackSymbol, mediaChangeCallbackModule);
    callback->add(YCPString (product));
    callback->add(YCPString (error));
    callback->add(YCPInteger (expected));
    callback->add(YCPInteger (current));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isString())
	return "";
    return ret->asString()->value();
}

//-------------------------------------------------------------------
// for media error
static std::string mediaErrorCallbackModule;
static YCPSymbol mediaErrorCallbackSymbol("",false);

static std::string
mediaErrorCallbackFunc (PMError error, void *_wfm)
{
y2milestone ("mediaErrorCallbackFunc(%d)", (int)error);
    YCPTerm callback = YCPTerm (mediaErrorCallbackSymbol, mediaErrorCallbackModule);
    callback->add(YCPInteger (error));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isString())
	return "";
    return ret->asString()->value();
}


//-------------------------------------------------------------------
// during rpm rebuild db (rpm progress)
static std::string progressRebuildDBCallbackModule;
static YCPSymbol progressRebuildDBCallbackSymbol("",false);

static void
progressRebuildDBCallbackFunc (int percent, void *_wfm)
{
    YCPTerm callback = YCPTerm (progressRebuildDBCallbackSymbol, progressRebuildDBCallbackModule);
    callback->add(YCPInteger (percent));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
}


//*****************************************************************************
//*****************************************************************************
//
// setup functions
//
//*****************************************************************************
//*****************************************************************************

/**
 * @builtin Pkg::CallbackStartProvide (string fun) -> nil
 *
 * set provide callback function
 * will call 'fun (string name, integer size, bool remote)' before file download
 */
YCPValue
PkgModuleFunctions::CallbackStartProvide(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackStartProvide");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	provideStartCallbackModule = name.substr (0, colonpos);
	provideStartCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	provideStartCallbackModule = "";
	provideStartCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setProvideStartCallback (provideStartCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackProgressProvide (string fun) -> nil
 *
 * set provide progress callback function
 * will call 'fun (int percent)' during file download
 */
YCPValue
PkgModuleFunctions::CallbackProgressProvide(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackProgressProvide");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	provideProgressCallbackModule = name.substr (0, colonpos);
	provideProgressCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	provideProgressCallbackModule = "";
	provideProgressCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setProvideProgressCallback (provideProgressCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackDoneProvide (string fun) -> nil
 *
 * set provide callback function
 * will call 'fun (integer error, string reason)' after file download
 */
YCPValue
PkgModuleFunctions::CallbackDoneProvide(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackStartProvide");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	provideDoneCallbackModule = name.substr (0, colonpos);
	provideDoneCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	provideDoneCallbackModule = "";
	provideDoneCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setProvideDoneCallback (provideDoneCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackStartPackage (string fun) -> nil
 *
 * set start install callback function
 * will call 'fun (string name, string summary, integer size, boolean is_delete)'
 * before package install/delete
 */
YCPValue
PkgModuleFunctions::CallbackStartPackage(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackStartPackage");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	startPackageCallbackModule = name.substr (0, colonpos);
	startPackageCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	startPackageCallbackModule = "";
	startPackageCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setPackageStartCallback (startPackageCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackProgressPackage (string fun) -> nil
 *
 * set progress callback function
 * will call 'fun (integer percent)' during rpm installation
 */
YCPValue
PkgModuleFunctions::CallbackProgressPackage(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackProgressPackage");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	progressPackageCallbackModule = name.substr (0, colonpos);
	progressPackageCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	progressPackageCallbackModule = "";
	progressPackageCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setPackageProgressCallback (progressPackageCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackDonePackage (string fun) -> nil
 *
 * set start install callback function
 * will call 'fun (string name)'
 * after package install/delete
 */
YCPValue
PkgModuleFunctions::CallbackDonePackage(YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackDonePackage");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	donePackageCallbackModule = name.substr (0, colonpos);
	donePackageCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	donePackageCallbackModule = "";
	donePackageCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setPackageDoneCallback (donePackageCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackMediaChange (integer source, string fun) -> nil
 *
 * set media change callback function
 */
YCPValue
PkgModuleFunctions::CallbackMediaChange (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 2)
	|| !(args->value(1)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackMediaChange");
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
    (InstSrcPtr::cast_away_const(source_id))->setMediaChangeCallback (mediaChangeCallbackFunc, _wfm);
    return YCPVoid();
}

/**
 * @builtin Pkg::CallbackMediaError (integer source, string fun) -> nil
 *
 * set media error callback function
 * will call 'WFM::fun (string error)' from InstSrc
 */
YCPValue
PkgModuleFunctions::CallbackMediaError (YCPList args)
{
    InstSrcManager::ISrcId source_id =  getSourceByArgs (args, 0);
    if (!source_id)
	return YCPVoid();

    if ((args->size() != 2)
	|| !(args->value(1)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackMediaError");
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
    (InstSrcPtr::cast_away_const(source_id))->setMediaErrorCallback(mediaErrorCallbackFunc, _wfm);
    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackProgressRebuildDB (string fun) -> nil
 *
 * set progress callback function
 * will call 'fun (integer percent)' during rpm installation
 */
YCPValue
PkgModuleFunctions::CallbackProgressRebuildDB (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackProgressRebuildDB");
    }
    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	progressRebuildDBCallbackModule = name.substr (0, colonpos);
	progressRebuildDBCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	progressRebuildDBCallbackModule = "";
	progressRebuildDBCallbackSymbol = YCPSymbol (name, false);
    }
    _y2pm.setRebuildDBProgressCallback(progressRebuildDBCallbackFunc, _wfm);
    return YCPVoid();
}



