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

#include <y2util/Y2SLog.h>
#include <ycp/y2log.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

#include <y2pm/PMYouPatchManager.h>
#include <y2pm/InstYou.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPError.h>

using std::string;
using std::endl;

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

static std::string
provideDoneCallbackFunc (PMError error, const std::string& reason, const std::string& name, void *_wfm)
{
    YCPTerm callback = YCPTerm (provideDoneCallbackSymbol, provideDoneCallbackModule);
    callback->add(YCPInteger (error));
    callback->add(YCPString (reason));
    callback->add(YCPString (name));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isString())
	return "";
    return ret->asString()->value();
}

//-------------------------------------------------------------------
// at start of package installation (rpm call)
static std::string startPackageCallbackModule;
static YCPSymbol startPackageCallbackSymbol("",false);

// return "true" to continue, "false" to stop
static bool
startPackageCallbackFunc (const std::string& name, const std::string& summary, const FSize& size, bool is_delete, void *_wfm)
{
    YCPTerm callback = YCPTerm (startPackageCallbackSymbol, startPackageCallbackModule);
    callback->add(YCPString (name));		// package name
    callback->add(YCPString (summary));		// package summary
    callback->add(YCPInteger((long long)size));	// package size
    callback->add(YCPBoolean(is_delete));	// package is being deleted
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret.isNull() && ret->isBoolean())
	return ret->asBoolean()->value();
    return true;
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

static std::string
donePackageCallbackFunc (PMError err, const std::string& reason, void *_wfm)
{
    YCPTerm callback = YCPTerm (donePackageCallbackSymbol, donePackageCallbackModule);
    callback->add(YCPInteger (err));
    callback->add(YCPString (reason));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isString())
	return "";
    return ret->asString()->value();
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
 * return "" -> ok, retry
 * return "C" -> cancel installation
 * return "S" -> skip this media
 * return "I" -> ignore wrong media id
 * return "E" -> eject media
 * else url
 */
static std::string
mediaChangeCallbackFunc (const std::string& error, const std::string& url, const std::string& product, int expected, int current, void *_wfm)
{
    YCPTerm callback = YCPTerm (mediaChangeCallbackSymbol, mediaChangeCallbackModule);
    callback->add(YCPString (error));
    callback->add(YCPString (url));
    callback->add(YCPString (product));
    callback->add(YCPInteger (expected));
    callback->add(YCPInteger (current));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    if (!ret->isString())
	return "";
    return ret->asString()->value();
}

//-------------------------------------------------------------------
// for source change
static std::string sourceChangeCallbackModule;
static YCPSymbol sourceChangeCallbackSymbol("",false);
// pointer to _sources vector in PkgModuleFunctions.h, set by CallbackSourceChange
// used in callback to translate the (internal) InstSrcPtr to
// an (external) vector index
static vector<InstSrcManager::ISrcId> * sourcesptr;
static InstSrcManager::ISrcIdList * instorderptr;

/**
 * source change callback
 *
 * see packagemanager/src/include/Y2PM:_source_change_func
 *
 * calls <sourcechangecallback> (int source, int medianr)
 *
 * return void
 */
static void
sourceChangeCallbackFunc (constInstSrcPtr source, int medianr, void *_wfm)
{
    YCPTerm callback = YCPTerm (sourceChangeCallbackSymbol, sourceChangeCallbackModule);

    int sourceid = -1;					// search vector index for the source pointer

    if (instorderptr == 0
	|| instorderptr->empty())
    {
	for (unsigned int i = 0; i < sourcesptr->size(); ++i)
	{
	    if ((*sourcesptr)[i] == source)
	    {
		sourceid = i;
		break;
	    }
	}
    }
    else						// we have an install order, use this
    {
	sourceid = 0;
	for (InstSrcManager::ISrcIdList::const_iterator it = instorderptr->begin(); it != instorderptr->end(); ++it)
	{
	    if (*it == source)
	    {
		break;
	    }
	    sourceid = sourceid + 1;
	}
    }

    callback->add(YCPInteger (sourceid));
    callback->add(YCPInteger (medianr));
    YCPValue ret = ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
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
 * pass callback function to Source
 *  used for every source, defined here because mediaChangeCallbackFunc is static
 *  used also after changing the callback function in PkgModuleFunctions::CallbackMediaChange(...)
 */
void
PkgModuleFunctions::SetMediaCallback (InstSrcManager::ISrcId source_id)
{
    (InstSrcPtr::cast_away_const(source_id))->setMediaChangeCallback (mediaChangeCallbackFunc, _wfm);
}


/**
 * @builtin Pkg::CallbackMediaChange (integer source, string fun) -> nil
 *
 * set media change callback function
 */
YCPValue
PkgModuleFunctions::CallbackMediaChange (YCPList args)
{
    string name;

    // allow omission of 'src' argument. Since we can handle one callback
    // function at most, passing a src argument implies a per-source callback
    // which isn't implemented anyway.

    if ((args->size() == 1)
	&& (args->value(0)->isString()))
    {
	name = args->value(0)->asString()->value();
    }

    // optional source
    InstSrcManager::ISrcId source_id;

    // if name wasn't set above, check for (source, name) arguments
    if (name.empty())
    {
	source_id = getSourceByArgs (args, 0);
	if (!source_id)
	    return YCPVoid();

	if ((args->size() != 2)
	    || !(args->value(1)->isString()))
	{
	    return YCPError ("Bad args to Pkg::CallbackMediaChange");
	}
	name = args->value(1)->asString()->value();
    }

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

    // if source_id given, set it's callback

    if (source_id)
	SetMediaCallback (source_id);

    return YCPVoid();
}


/**
 * @builtin Pkg::CallbackSourceChange (string fun) -> nil
 *
 * set source change callback function
 */
YCPValue
PkgModuleFunctions::CallbackSourceChange (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::CallbackSourceChange");
    }

    string name = args->value(0)->asString()->value();
    string::size_type colonpos = name.find("::");
    if (colonpos != string::npos)
    {
	sourceChangeCallbackModule = name.substr (0, colonpos);
	sourceChangeCallbackSymbol = YCPSymbol (name.substr (colonpos+2), false);
    }
    else
    {
	sourceChangeCallbackModule = "";
	sourceChangeCallbackSymbol = YCPSymbol (name, false);
    }

    // setup pointers to member variables so the static callback
    // function can access them
    sourcesptr = &_sources;
    instorderptr = &_inst_order;

    _y2pm.setSourceChangeCallback (sourceChangeCallbackFunc, _wfm);
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

YCPCallbacks::YCPCallbacks( YCPInterpreter *interpreter ) :
  _interpreter( interpreter )
{
}

YCPValue YCPCallbacks::setCallback( string func, YCPList args )
{
  if ((args->size() != 1)
      || !(args->value(0)->isString()))
  {
      return YCPError ("Bad args to Pkg::Callback");
  }
  string name = args->value(0)->asString()->value();
  string::size_type colonpos = name.find("::");
  if (colonpos != string::npos)
  {
      mModules[ func ] = name.substr ( 0, colonpos );
      mSymbols[ func ] = name.substr ( colonpos + 2 );
  }
  else
  {
      mModules[ func ] = "";
      mSymbols[ func ] = name;
  }
  return YCPVoid();
}

YCPTerm YCPCallbacks::createCallback( const string &func )
{
  return YCPTerm( YCPSymbol( mSymbols[ func ], false ), mModules[ func ] );
}

YCPValue YCPCallbacks::evaluate( const YCPTerm &callback )
{
  return _interpreter->evaluate( callback );
}

class YouCallbacks : public InstYou::Callbacks, public YCPCallbacks
{
  public:
    YouCallbacks( YCPInterpreter *interpreter ) : YCPCallbacks( interpreter ) {}
  
    bool progress( int percent )
    {
      D__ << "you progress: " << percent << endl;

      YCPTerm callback = createCallback( "progress" );
      callback->add( YCPInteger ( percent ) );
      YCPValue ret = evaluate( callback );
      if ( !ret->isBoolean() ) {
        ERR << "Wrong return type." << endl;
        return false;
      } else {
        return ret->asBoolean()->value();
      }
    }

    bool patchProgress( int percent, const string &pkg )
    {
      D__ << "you patch progress: " << percent << endl;

      YCPTerm callback = createCallback( "patchProgress" );
      callback->add( YCPInteger ( percent ) );
      callback->add( YCPString ( pkg ) );
      YCPValue ret = evaluate( callback );
      if ( !ret->isBoolean() ) {
        ERR << "Wrong return type." << endl;
        return false;
      } else {
        return ret->asBoolean()->value();
      }
    }
};

void PkgModuleFunctions::initYouCallbacks()
{
  YouCallbacks *callbacks = new YouCallbacks( _wfm );
  InstYou::setCallbacks( callbacks );
  _youCallbacks = callbacks;
}

YCPValue
PkgModuleFunctions::CallbackYouProgress( YCPList args )
{
    return _youCallbacks->setCallback( "progress", args );
}

YCPValue
PkgModuleFunctions::CallbackYouPatchProgress( YCPList args )
{
    return _youCallbacks->setCallback( "patchProgress", args );
}
