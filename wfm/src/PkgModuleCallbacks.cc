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

static YCPTerm progressCallbackTerm("",false);

void
progressCallbackFunc (int percent, void *_wfm)
{
    y2milestone ("progressCallbackFunc(%d)", percent);
    YCPTerm callback = progressCallbackTerm;
    callback->add(YCPInteger (percent));
    ((YCPInterpreter *)_wfm)->evaluate (callback);
    return;
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
    y2milestone ("SetProgressCallback");
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetProgressCallback");
    }
    progressCallbackTerm = YCPTerm (args->value(0)->asString()->value(), false);
    _y2pm.instTarget().setPackageInstallProgressCallback (progressCallbackFunc, _wfm);
    return YCPVoid();
}

