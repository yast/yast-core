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

/**   
   @builtin Pkg::YouStatus() -> map

   get map with status information

*/
YCPValue
PkgModuleFunctions::YouStatus (YCPList args)
{
    YCPMap result;

    result->add( YCPString( "error" ), YCPBoolean( false ) );
    
    PMYouPatchPathsPtr paths = _y2pm.youPatchManager().instYou().paths();
    
    result->add( YCPString( "product" ), YCPString( paths->product() ) );
    result->add( YCPString( "version" ), YCPString( paths->version() ) );
    result->add( YCPString( "basearch" ), YCPString( paths->baseArch() ) );
    
    return result;
}

// ------------------------
/**   
   @builtin Pkg::YouGetServers() -> list(string)

   get urls of patch servers

*/
YCPValue
PkgModuleFunctions::YouGetServers (YCPList args)
{
    std::list<Url> servers;
    PMError err = _y2pm.youPatchManager().instYou().servers( servers );
    if ( err ) {
      return YCPError( "Error getting you servers.", YCPList() );
    }

    YCPList result;
    std::list<Url>::const_iterator it;
    for( it = servers.begin(); it != servers.end(); ++it ) {
      result->add ( YCPString( (*it).asString() ) );
    }
    return result;
}

/**   
   @builtin Pkg::YouGetPatches() -> bool

   retrieve patches

*/
YCPValue
PkgModuleFunctions::YouGetPatches (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::YouGetPatches", YCPBoolean( false ) );
    }
    string urlstr = args->value(0)->asString()->value_cstr();
    Url url( urlstr );
    if ( !url.isValid() ) return YCPError( "Url not valid", YCPBoolean( false ) );    

    PMError err = _y2pm.youPatchManager().instYou().retrievePatchInfo( url );
    if ( err ) return YCPError( err.errstr(), YCPBoolean( false ) );

    return YCPBoolean( true );
}

/**
  @builtin Pkg::YouAttachSource () -> bool

  attach source of patches
*/
YCPValue
PkgModuleFunctions::YouAttachSource (YCPList args)
{
    PMError err = _y2pm.youPatchManager().instYou().attachSource();
    if ( err ) return YCPError( err.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouGetPackages () -> bool

   retrieve package data belonging to patches

*/
YCPValue
PkgModuleFunctions::YouGetPackages (YCPList args)
{
    PMError err = _y2pm.youPatchManager().instYou().retrievePatches();
    if ( err ) return YCPError( err.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouSelectPatches () -> void

   select patches based on types.

*/
YCPValue
PkgModuleFunctions::YouSelectPatches (YCPList args)
{
    _y2pm.youPatchManager().instYou().filterPatchSelection();

    int kinds = PMYouPatch::kind_security | PMYouPatch::kind_recommended;

    _y2pm.youPatchManager().instYou().selectPatches( kinds );

    return YCPVoid();
}

/**
  @builtin Pkg::YouFirstPatch () -> map

  get information about first selected patch.
*/
YCPValue
PkgModuleFunctions::YouFirstPatch (YCPList args)
{
    YCPMap result;

    PMYouPatchPtr patch = _y2pm.youPatchManager().instYou().firstPatch();
    if ( patch ) {
      result->add( YCPString( "name" ),
                   YCPString( patch->name() ) );
      result->add( YCPString( "summary" ),
                   YCPString( patch->shortDescription() ) );
    } else {
      y2debug("No more patches.");
    }

    return result;
}

/**   
   @builtin Pkg::YouNextPatch () -> map

   get information about next patch to be installed.
*/
YCPValue
PkgModuleFunctions::YouNextPatch (YCPList args)
{
    YCPMap result;

    PMYouPatchPtr patch = _y2pm.youPatchManager().instYou().nextPatch();
    if ( patch ) {
      result->add( YCPString( "name" ),
                   YCPString( patch->name() ) );
      result->add( YCPString( "summary" ),
                   YCPString( patch->shortDescription() ) );
    } else {
      y2debug("No more patches.");
    }

    return result;
}

/**
  @builtin Pkg::YouGetCurrentPatch () -> bool

  download current patch.
*/
YCPValue
PkgModuleFunctions::YouGetCurrentPatch (YCPList args)
{
    PMError err = _y2pm.youPatchManager().instYou().retrieveCurrentPatch();
    if ( err ) return YCPError( err.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**
  @builtin Pkg::YouInstallCurrentPatch () -> bool

  install current patch.
*/
YCPValue
PkgModuleFunctions::YouInstallCurrentPatch (YCPList args)
{
    PMError err = _y2pm.youPatchManager().instYou().installCurrentPatch();
    if ( err ) return YCPError( err.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouInstallPatches () -> bool

   install retrieved patches
*/
YCPValue
PkgModuleFunctions::YouInstallPatches (YCPList args)
{
    PMError err = _y2pm.youPatchManager().instYou().installPatches();
    if ( err ) return YCPError( err.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}
