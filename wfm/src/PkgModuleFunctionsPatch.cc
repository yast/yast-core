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
#include <y2pm/PMYouProduct.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>

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
YCPMap
PkgModuleFunctions::YouStatus ()
{
    YCPMap result;

    result->add( YCPString( "error" ), YCPBoolean( false ) );
    
    InstYou &you = _y2pm.youPatchManager().instYou();
    PMYouProductPtr product = you.settings()->primaryProduct();

    if ( product )
    {
	result->add( YCPString( "product" ), YCPString( product->product() ) );
	result->add( YCPString( "version" ), YCPString( product->version() ) );
	result->add( YCPString( "basearch" ), YCPString( product->baseArch() ) );
	result->add( YCPString( "business" ), YCPBoolean( product->businessProduct() ) );

	result->add( YCPString( "mirrorurl" ), YCPString( product->youUrl() ) );
    }
    result->add( YCPString( "lastupdate" ), YCPInteger( you.lastUpdate() ) );
    result->add( YCPString( "installedpatches" ),
                 YCPInteger( you.installedPatches() ) );
    
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
PkgModuleFunctions::YouSetServer (const YCPMap& servers)
{
    PMYouServer server = convertServerObject( servers );

    InstYou &you = _y2pm.youPatchManager().instYou();

    you.settings()->setPatchServer( server );

    return YCPString( "" );
}


/**
   @builtin Pkg::YouGetUserPassword()

   Get username and password needed for access to server.

   @return map "username" success
               "password" password
*/
YCPValue
PkgModuleFunctions::YouGetUserPassword ()
{
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
PkgModuleFunctions::YouSetUserPassword (const YCPString& user, const YCPString& passwd, const YCPBoolean& p)
{
    string username = user->value_cstr();
    string password = passwd->value_cstr();
    bool persistent = p->value();

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
YCPString
PkgModuleFunctions::YouGetServers (YCPReference strings)
{
    std::list<PMYouServer> servers;
    _last_error = _y2pm.youPatchManager().instYou().servers( servers );
    if ( _last_error ) {
      if ( _last_error == YouError::E_get_youservers_failed ) return YCPString( "get" );
      if ( _last_error == YouError::E_write_youservers_failed ) return YCPString( "write" );
      if ( _last_error == YouError::E_read_youservers_failed ) return YCPString( "read" );
      return YCPString( "Error getting you servers." );
    }

    YCPList result = strings->entry ()->value ()->asList ();
    std::list<PMYouServer>::const_iterator it;
    for( it = servers.begin(); it != servers.end(); ++it ) {
      YCPMap serverMap;
      serverMap->add( YCPString( "url" ), YCPString( (*it).url().asString() ) );
      serverMap->add( YCPString( "name" ), YCPString( (*it).name() ) );
      serverMap->add( YCPString( "directory" ), YCPString( (*it).directory() ) );
      serverMap->add( YCPString( "type" ), YCPString( (*it).typeAsString() ) );
      result->add( serverMap );
    }
    
    strings->entry ()->setValue (result);

    return YCPString( "" );
}


PMYouServer
PkgModuleFunctions::convertServerObject( const YCPMap &serverMap )
{
    string url;
    string name;
    string dir;
    string typeStr;

    YCPValue urlValue = serverMap->value( YCPString( "url" ) );
    if ( !urlValue.isNull() ) url = urlValue->asString()->value();

    YCPValue nameValue = serverMap->value( YCPString( "name" ) );
    if ( !nameValue.isNull() ) name = nameValue->asString()->value();

    YCPValue dirValue = serverMap->value( YCPString( "directory" ) );
    if ( !dirValue.isNull() ) dir = dirValue->asString()->value();

    YCPValue typeValue = serverMap->value( YCPString( "type" ) );
    if ( !typeValue.isNull() ) typeStr = typeValue->asString()->value();
    
    PMYouServer::Type type = PMYouServer::typeFromString( typeStr );
    
    return PMYouServer( Url( url ), name, dir, type );
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
PkgModuleFunctions::YouGetDirectory ()
{
    InstYou &you = _y2pm.youPatchManager().instYou();

    _last_error = you.retrievePatchDirectory();
    if ( _last_error ) {
      if ( _last_error == MediaError::E_login_failed ) return YCPString( "login" );
      return YCPString( "error" );
    }

    return YCPString( "" );
}

/**   
  @builtin Pkg::YouRetrievePatchInfo( boolean download_again, boolean check_signatures ) -> error string

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
PkgModuleFunctions::YouRetrievePatchInfo (const YCPBoolean& download, const YCPBoolean& sig)
{
    bool reload = download->value();
    bool checkSig = sig->value();

    InstYou &you = _y2pm.youPatchManager().instYou();

    you.settings()->setReloadPatches( reload );
    you.settings()->setCheckSignatures( checkSig );
    
    _last_error = you.retrievePatchInfo();
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
   @builtin Pkg::YouProcessPatches () -> bool

   Download and install patches.

*/
YCPValue
PkgModuleFunctions::YouProcessPatches ()
{
    _last_error = _y2pm.youPatchManager().instYou().processPatches();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}

/**   
   @builtin Pkg::YouSelectPatches () -> void

   select patches based on types.

*/
YCPValue
PkgModuleFunctions::YouSelectPatches ()
{
    int kinds = PMYouPatch::kind_security | PMYouPatch::kind_recommended |
                PMYouPatch::kind_patchlevel;

    _y2pm.youPatchManager().instYou().selectPatches( kinds );
    
    return YCPVoid ();
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
   @builtin Pkg::YouRemovePackages () -> bool

   remove downloaded packages.
*/
YCPValue
PkgModuleFunctions::YouRemovePackages ()
{
    _last_error = _y2pm.youPatchManager().instYou().removePackages();
    if ( _last_error ) return YCPError( _last_error.errstr(), YCPBoolean( false ) );
    return YCPBoolean( true );
}
