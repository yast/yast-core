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
#include <y2util/Y2SLog.h>
#include <y2pm/InstData.h>
#include <y2pm/YouError.h>
#include <y2pm/PMYouPatchManager.h>
#include <y2pm/InstYou.h>
#include <y2pm/PMPackage.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPError.h>

using std::string;
using std::endl;

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
    
    InstYou &you = _y2pm.youPatchManager().instYou();

    PMYouPatchPathsPtr paths = you.paths();
    
    result->add( YCPString( "product" ), YCPString( paths->product() ) );
    result->add( YCPString( "version" ), YCPString( paths->version() ) );
    result->add( YCPString( "basearch" ), YCPString( paths->baseArch() ) );
    result->add( YCPString( "business" ), YCPBoolean( paths->businessProduct() ) );

    result->add( YCPString( "mirrorurl" ), YCPString( paths->youUrl() ) );
    
    result->add( YCPString( "lastupdate" ), YCPInteger( you.lastUpdate() ) );
    
    return result;
}

/**
   @builtin Pkg::YouSetServer()

   Set server to be used for getting patches.

   @param map  you server map as returned from YouGetServers.
   
   @return ""      success
           "args"  wrong arguments
           "error" other error
*/
YCPValue
PkgModuleFunctions::YouSetServer (YCPList args)
{
    if ( ( args->size() != 1) || !args->value(0)->isMap() )
    {
	return YCPString ("args");
    }

    PMYouServer server = convertServerObject( args->value( 0 )->asMap() );

    InstYou &you = _y2pm.youPatchManager().instYou();

    you.paths()->setPatchServer( server );

    return YCPString( "" );
}

/**
   @builtin Pkg::YouGetUserPassword()

   Get username and password needed for access to server.

   @return map "username" success
               "password" password
*/
YCPValue
PkgModuleFunctions::YouGetUserPassword (YCPList args)
{
    if ( ( args->size() != 0) )
    {
	return YCPString ("args");
    }

    InstYou &you = _y2pm.youPatchManager().instYou();

    you.readUserPassword();

    string username = you.username();
    string password = you.password();

    YCPMap result;
    result->add( YCPString( "username" ), YCPString( username ) );
    result->add( YCPString( "password" ), YCPString( password ) );

    return result;
}

/**
   @builtin Pkg::YouSetUserPassword()

   Set username and password needed for access to server.

   @param string  username
   @param string  password
   @param boolean true, if username/password should be saved to disk

   @return ""      success
           "args"  wrong arguments
           "error" other error
*/
YCPValue
PkgModuleFunctions::YouSetUserPassword (YCPList args)
{
    if ( ( args->size() != 3 ) || !( args->value( 0 )->isString() ) ||
         !( args->value( 1 )->isString() ) ||
         !( args->value( 2 )->isBoolean() ) )
    {
	return YCPString( "args" );
    }

    string username = args->value(0)->asString()->value_cstr();
    string password = args->value(1)->asString()->value_cstr();
    bool persistent = args->value(2)->asBoolean()->value();

    _last_error =
        _y2pm.youPatchManager().instYou().setUserPassword( username,
                                                           password,
                                                           persistent );

    if ( _last_error ) {
        return YCPString( "error" );
    }

    return YCPString("");
}

// ------------------------
/**
   @builtin Pkg::YouGetServers() -> error string

   get urls of patch servers

   @param list(map)  list of maps where results are stored. The maps have the following fields
                     set:

                       "url"        URL of server.
                       "name"       Descriptive name of server.
                       "directory"  Directory file used to get list of patches. 

   @return ""      success
           "args"  bad args
           "get"   error getting file from server
           "write" error writing file to disk
           "read"  error reading file after download
*/
YCPValue
PkgModuleFunctions::YouGetServers (YCPList args)
{
    if ((args->size() != 1) || !(args->value(0)->isList()))
    {
	return YCPString( "args" );
    }

    std::list<PMYouServer> servers;
    _last_error = _y2pm.youPatchManager().instYou().servers( servers );
    if ( _last_error ) {
      if ( _last_error == YouError::E_get_youservers_failed ) return YCPString( "get" );
      if ( _last_error == YouError::E_write_youservers_failed ) return YCPString( "write" );
      if ( _last_error == YouError::E_read_youservers_failed ) return YCPString( "read" );
      return YCPString( "Error getting you servers." );
    }

    YCPList result = YCPList( args->value( 0 )->asList() );
    std::list<PMYouServer>::const_iterator it;
    for( it = servers.begin(); it != servers.end(); ++it ) {
      YCPMap serverMap;
      serverMap->add( YCPString( "url" ), YCPString( (*it).url().asString() ) );
      serverMap->add( YCPString( "name" ), YCPString( (*it).name() ) );
      serverMap->add( YCPString( "directory" ), YCPString( (*it).directory() ) );
      result->add( serverMap );
    }

    return YCPString( "" );
}


PMYouServer
PkgModuleFunctions::convertServerObject( const YCPMap &serverMap )
{
    string url;
    string name;
    string dir;

    YCPValue urlValue = serverMap->value( YCPString( "url" ) );
    if ( !urlValue.isNull() ) url = urlValue->asString()->value();
    
    YCPValue nameValue = serverMap->value( YCPString( "name" ) );
    if ( !nameValue.isNull() ) name = nameValue->asString()->value();

    YCPValue dirValue = serverMap->value( YCPString( "directory" ) );
    if ( !dirValue.isNull() ) dir = dirValue->asString()->value();

    return PMYouServer( Url( url ), name, dir );
}


/**
  @builtin Pkg::YouGetDirectory() -> error string

  retrieve directory file listing all available patches
  
  @return ""       success
          "url"    url not valid
          "login"  login failed
          "error"  other error
*/
YCPValue
PkgModuleFunctions::YouGetDirectory (YCPList args)
{
    if ( ( args->size() != 0) )
    {
	return YCPString ("args");
    }

    InstYou &you = _y2pm.youPatchManager().instYou();

    _last_error = you.retrievePatchDirectory();
    if ( _last_error ) {
      if ( _last_error == MediaError::E_login_failed ) return YCPString( "login" );
      return YCPString( "error" );
    }

    return YCPString( "" );
}

/**   
  @builtin Pkg::YouGetPatches() -> error string

  retrieve patches
  
  @param bool    true if patches should be downloaded again
  @param bool    true if signatures should be checked.
  
  @return ""      success
          "args"  bad args
          "media" media error
          "sig"   signature check failed 
          "abort" user aborted operation
          "url"   url not valid
          "login" login failed
*/
YCPValue
PkgModuleFunctions::YouGetPatches (YCPList args)
{
    if ( ( args->size() != 2) || 
         !args->value( 0 )->isBoolean() || !args->value( 1 )->isBoolean() )
    {
	return YCPString ("args");
    }

    bool reload = args->value( 0 )->asBoolean()->value();
    bool checkSig = args->value( 1 )->asBoolean()->value();

    InstYou &you = _y2pm.youPatchManager().instYou();

    _last_error = you.retrievePatchInfo( reload, checkSig );
    if ( _last_error ) {
      if ( _last_error == MediaError::E_login_failed ) return YCPString( "login" );
      if ( _last_error.errClass() == PMError::C_MediaError ) return YCPString( "media" );
      if ( _last_error == YouError::E_bad_sig_file ) return YCPString( "sig" );
      if ( _last_error == YouError::E_user_abort ) return YCPString( "abort" );
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

  @param bool If true progress is reset, if false or missing it isn't touched.

  get information about first selected patch.
*/
YCPValue
PkgModuleFunctions::YouFirstPatch (YCPList args)
{
    bool resetProgress = true;
    if ( ( args->size() == 1 ) && ( args->value(0)->isBoolean() ) )
    {
	resetProgress = args->value( 0 )->asBoolean()->value();
    }

    YCPMap result;

    PMYouPatchPtr patch = _y2pm.youPatchManager().instYou().firstPatch( resetProgress );
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

    bool ok;

    PMYouPatchPtr patch = _y2pm.youPatchManager().instYou().nextPatch( &ok );
    if ( patch ) {
      result = YouPatch( patch );
    } else {
      y2debug("No more patches.");
    }

    if ( !ok ) {
      result->add( YCPString( "error" ), YCPString( "abort" ) );
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
    
    YCPList packageList;
    
    std::list<PMPackagePtr> packages = patch->packages();
    std::list<PMPackagePtr>::const_iterator itPkg;
    for ( itPkg = packages.begin(); itPkg != packages.end(); ++itPkg ) {
      packageList->add( YCPString( (*itPkg)->nameEd() ) );
    }
    
    result->add( YCPString( "packages" ), packageList );

    return result;
}

/**
  @builtin Pkg::YouGetCurrentPatch () -> error string

  download current patch.

  @param bool true if patch should be reloaded from the server.
  @param bool true if signatures should be checked.

  @return ""      success
          "args"  bad args
          "media" media error
          "sig"   signature check failed
          "abort" user aborted operation
*/
YCPValue
PkgModuleFunctions::YouGetCurrentPatch (YCPList args)
{
    if ( (args->size() != 2) || !args->value(0)->isBoolean() ||
         !args->value( 1 )->isBoolean() )
    {
	return YCPString( "args" );
    }

    bool reload = args->value( 0 )->asBoolean()->value();
    bool checkSig = args->value( 1 )->asBoolean()->value();

    _last_error =
        _y2pm.youPatchManager().instYou().retrieveCurrentPatch( reload, checkSig );
    if ( _last_error ) {
      if ( _last_error.errClass() == PMError::C_MediaError ) return YCPString( "media" );
      if ( _last_error == YouError::E_bad_sig_file ||
           _last_error == YouError::E_bad_sig_rpm ) return YCPString( "sig" );
      if ( _last_error == YouError::E_user_abort ) return YCPString( "abort" );
      return YCPString( _last_error.errstr() );
    }
    return YCPString( "" );
}

/**
  @builtin Pkg::YouInstallCurrentPatch () -> bool

  install current patch.

  @return ""        success
          "skipped" patch was been skipped during download
          "abort"   user aborted installation of patch
          "error"   install error
*/
YCPValue
PkgModuleFunctions::YouInstallCurrentPatch (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().installCurrentPatch();
    if ( !_last_error ) return YCPString( "" );
    if ( _last_error == YouError::E_empty_location ) return YCPString( "skipped" );
    else if ( _last_error == YouError::E_user_abort ) return YCPString( "abort" );
    
    return YCPString( "error" );
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

/**
   @builtin Pkg::YouFinish () -> bool

   finish update. Writes date of last update.
*/
YCPValue
PkgModuleFunctions::YouFinish (YCPList args)
{
    _last_error = _y2pm.youPatchManager().instYou().writeLastUpdate();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}
