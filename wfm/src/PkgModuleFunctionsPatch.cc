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

   File:	PkgModuleFunctionsPatch.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to PMPatchManager
		Handles YOU related Pkg::function (list_of_arguments) calls
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

// ------------------------
// 
// @builtin Pkg::YouGetServers() -> list(string)
//
// get urls of patch servers
//
YCPValue
PkgModuleFunctions::YouGetServers (YCPList args)
{
//    std::list<Url> servers;
//    PMError err = _y2pm.youPatchManager();
    YCPList servers;
    servers->add (YCPString("ftp://you.suse.com"));
    return servers;
}

// ------------------------
// 
// @builtin Pkg::YouGetPatches() -> bool
//
// retrieve patches
//
YCPValue
PkgModuleFunctions::YouGetPatches (YCPList args)
{
    return YCPVoid();
}

// ------------------------
// 
// @builtin Pkg::YouGetPackages () -> bool
//
// retrieve package data belonging to patches
//
YCPValue
PkgModuleFunctions::YouGetPackages (YCPList args)
{
    return YCPVoid();
}

// ------------------------
// 
// @builtin Pkg::YouInstallPatches () -> bool
//
// install retrieved patches
//
YCPValue
PkgModuleFunctions::YouInstallPatches (YCPList args)
{
    return YCPVoid();
}
