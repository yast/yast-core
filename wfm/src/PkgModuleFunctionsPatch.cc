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
#include <y2pm/YouError.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;


/**
 * helper function, get selectable by name
 */

PMSelectablePtr
PkgModuleFunctions::getPatchSelectable (const std::string& name)
{
    PMSelectablePtr selectable;
    if (!name.empty())
	selectable = _y2pm.youPatchManager().getItem(name);
    if (!selectable)
    {
	y2error ("Patch '%s' not found", name.c_str());
    }
    return selectable;
}


//-------------------------------------------------------------
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
    result->add( YCPString( "business" ), YCPBoolean( paths->businessProduct() ) );
    
    return result;
}

/**
   @builtin Pkg::YouCheckAuthorization() -> error string

   Try to login to update server.

   @param string server url
   @param string registration code
   @param string password

   @return "ok"     success
           "args" bad args
           "error_login"  authorization failed
           "url_invalid" invalid url
           "url_not_found" url not found
           "error" other error
*/
YCPValue
PkgModuleFunctions::YouCheckAuthorization (YCPList args)
{
    if ( ( args->size() != 3 ) || !( args->value( 0 )->isString() ) ||
         !( args->value( 1 )->isString() ) || !( args->value( 2 )->isString() ) )
    {
	return YCPString( "args" );
    }

    string u = args->value(0)->asString()->value_cstr();
    Url url( u );
    if ( !url.isValid() ) return YCPString( "url_invalid" );

    string regcode = args->value(1)->asString()->value_cstr();
    string password = args->value(2)->asString()->value_cstr();

    _last_error = _y2pm.youPatchManager().instYou().checkAuthorization( url, regcode, password );

    if ( _last_error ) {
        if ( _last_error == YouError::E_auth_failed ) return YCPString( "error_login" );
        else return YCPString( "error" );
    }

    return YCPString( "" );
}

// ------------------------
/**
   @builtin Pkg::YouGetServers() -> error string

   get urls of patch servers

   @param list(string)  list of strings where results are stored.

   @return ""     success
           "args" bad args
           "get"  error getting file from server
           "read" error reading file after download
*/
YCPValue
PkgModuleFunctions::YouGetServers (YCPList args)
{
    if ((args->size() != 1) || !(args->value(0)->isList()))
    {
	return YCPString( "args" );
    }

    std::list<Url> servers;
    _last_error = _y2pm.youPatchManager().instYou().servers( servers );
    if ( _last_error ) {
      if ( _last_error == YouError::E_get_suseservers_failed ) return YCPString( "get" );
      if ( _last_error == YouError::E_read_suseservers_failed ) return YCPString( "read" );
      return YCPString( "Error getting you servers." );
    }

    YCPList result = YCPList( args->value( 0 )->asList() );
    std::list<Url>::const_iterator it;
    for( it = servers.begin(); it != servers.end(); ++it ) {
      result->add ( YCPString( (*it).asString() ) );
    }

    return YCPString( "" );
}

/**   
  @builtin Pkg::YouGetPatches() -> error string

  retrieve patches
  
  @param string  url of patch server.
  @param bool    true if signatures should be checked.
  
  @return ""      success
          "args"  bad args
          "media" media error
          "sig"   signature check failed 
          "url"   url not valid
*/
YCPValue
PkgModuleFunctions::YouGetPatches (YCPList args)
{
    if ( ( args->size() != 2) || !args->value(0)->isString() ||
         !args->value(1)->isBoolean() )
    {
	return YCPString ("args");
    }
    string urlstr = args->value(0)->asString()->value_cstr();
    Url url( urlstr );
    if ( !url.isValid() ) return YCPString( "url" );

    bool checkSig = args->value(1)->asBoolean()->value();

    _last_error = _y2pm.youPatchManager().instYou().retrievePatchInfo( url,
                                                                       checkSig );
    if ( _last_error ) {
      if ( _last_error.errClass() == PMError::C_MediaError ) return YCPString( "media" );
      if ( _last_error == YouError::E_bad_sig_file ) return YCPString( "sig" );
      return YCPString( _last_error.errstr() );
    }

    return YCPString( "" );
}

/**
  @builtin Pkg::YouAttachSource () -> bool

  attach source of patches
*/
YCPValue
PkgModuleFunctions::YouAttachSource (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().attachSource();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouGetPackages () -> bool

   retrieve package data belonging to patches

*/
YCPValue
PkgModuleFunctions::YouGetPackages (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().retrievePatches();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouSelectPatches () -> void

   select patches based on types.

*/
YCPValue
PkgModuleFunctions::YouSelectPatches (YCPList args)
{
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
      result = YouPatch( patch );
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
      result = YouPatch( patch );
    } else {
      y2debug("No more patches.");
    }

    return result;
}

YCPMap
PkgModuleFunctions::YouPatch( const PMYouPatchPtr &patch )
{
    YCPMap result;

    result->add( YCPString( "kind" ), YCPString( patch->kindLabel( patch->kind() ) ) );
    result->add( YCPString( "name" ), YCPString( patch->name() ) );
    result->add( YCPString( "summary" ), YCPString( patch->shortDescription() ) );
    result->add( YCPString( "description" ), YCPString( patch->longDescription() ) );
    result->add( YCPString( "preinformation" ), YCPString( patch->preInformation() ) );
    result->add( YCPString( "postinformation" ), YCPString( patch->postInformation() ) );

    return result;
}

/**
  @builtin Pkg::YouGetCurrentPatch () -> error string

  download current patch.

  @param bool true if signatures should be checked.

  @return ""      success
          "args"  bad args
          "media" media error
          "sig"   signature check failed
*/
YCPValue
PkgModuleFunctions::YouGetCurrentPatch (YCPList args)
{
    if ((args->size() != 1) || !(args->value(0)->isBoolean()))
    {
	return YCPString( "args" );
    }

    bool checkSig = args->value(0)->asBoolean()->value();

    _last_error = _y2pm.youPatchManager().instYou().retrieveCurrentPatch( checkSig );
    if ( _last_error ) {
      if ( _last_error.errClass() == PMError::C_MediaError ) return YCPString( "media" );
      if ( _last_error == YouError::E_bad_sig_file ||
           _last_error == YouError::E_bad_sig_rpm ) return YCPString( "sig" );
      return YCPString( _last_error.errstr() );
    }
    return YCPString( "" );
}

/**
  @builtin Pkg::YouInstallCurrentPatch () -> bool

  install current patch.
*/
YCPValue
PkgModuleFunctions::YouInstallCurrentPatch (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().installCurrentPatch();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouInstallPatches () -> bool

   install retrieved patches
*/
YCPValue
PkgModuleFunctions::YouInstallPatches (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().installPatches();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**
   @builtin Pkg::YouRemovePackages () -> bool

   remove downloaded packages.
*/
YCPValue
PkgModuleFunctions::YouRemovePackages (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().removePackages();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}
