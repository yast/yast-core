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

   File:	PkgModuleFunctionsSource.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to InstSrc
		Handles source related Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/
//#include <unistd.h>
//#include <sys/statvfs.h>

#include <iostream>
#include <ycp/y2log.h>

#include <y2util/Url.h>
#include <y2util/Pathname.h>

#include <y2pm/PMError.h>
#include <y2pm/InstSrc.h>
#include <y2pm/InstSrcPtr.h>
#include <y2pm/InstSrcDescr.h>
#include <y2pm/InstSrcDescrPtr.h>
#include <y2pm/InstTarget.h>
#include <y2pm/PMSelectionManager.h>
#include <y2pm/InstSrcManager.h>

#include <ycpTools.h>
#include <PkgModule.h>
#include <PkgModuleFunctions.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////
//
// Abbreviate some common pkgError calls:
//
/////////////////////////////////////////////////////////////////////////////////////////

#define pkgError_bad_args pkgError( Error::E_bad_args, YCPError( string("Bad args to Pkg::")+__FUNCTION__ ) )

/////////////////////////////////////////////////////////////////////////////////////////
//
// Tools convetring data from/to YCP
//
/////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////
// InstSrcManager::ISrcId <-> YCPInteger
/////////////////////////////////////////////////////////////////////////////////////////

inline YCPInteger asYCPInteger( const InstSrcManager::ISrcId & id_r )
{
  return YCPInteger( id_r->srcID() );
}

template<>
inline bool YcpArgLoad::Value<YT_INTEGER, InstSrcManager::ISrcId>::assign( const YCPValue & arg_r )
{
  InstSrc::UniqueID srcID = arg_r->asInteger()->value(); // YT_INTEGER asserted

  _value = Y2PM::instSrcManager().getSourceByID( srcID );
  if ( !_value ) {
    y2warning( "Unknown InstSrc ID %u", srcID );
  }
  return _value;
}

/////////////////////////////////////////////////////////////////////////////////////////
// InstSrcManager::ISrcIdList -> YCPList of YCPInteger
/////////////////////////////////////////////////////////////////////////////////////////

inline YCPList asYCPList( const InstSrcManager::ISrcIdList & ids_r )
{
  YCPList ret;
  for ( InstSrcManager::ISrcIdList::const_iterator it = ids_r.begin(); it != ids_r.end(); ++it ) {
    ret->add( asYCPInteger( *it ) );
  }
  return ret;
}

/////////////////////////////////////////////////////////////////////////////////////////
// InstSrcManager::SrcStateVector <-> YCPList of YCPMap
/////////////////////////////////////////////////////////////////////////////////////////

inline YCPList asYCPList( const InstSrcManager::SrcStateVector & states_r )
{
  YCPList ret;
  for ( InstSrcManager::SrcStateVector::const_iterator it = states_r.begin(); it != states_r.end(); ++it ) {
    YCPMap el;
    el->add( YCPString("SrcId"),	YCPInteger( it->first ) );
    el->add( YCPString("enabled"),	YCPBoolean( it->second ) );
    ret->add( el );
  }
  return ret;
}

template<>
inline bool YcpArgLoad::Value<YT_LIST, InstSrcManager::SrcStateVector>::assign( const YCPValue & arg_r )
{
  YCPList l =  arg_r->asList(); // YT_LIST asserted
  _value.clear();
  _value.reserve( l->size() );

  bool valid = true;
  YCPString tag_SrcId( "SrcId" );
  YCPString tag_enabled( "enabled" );

  for ( unsigned i = 0; i < unsigned(l->size()); ++i ) {
    YCPValue el = l->value(i);
    if ( el->isMap() ) {
      YCPMap m = el->asMap();
      InstSrcManager::SrcState state;

      if ( (el = m->value( tag_SrcId ))->isInteger() ) {
	state.first = el->asInteger()->value();
      } else {
	y2warning( "List entry %d: MAP[SrcId]: INTEGER expected but got '%s'", i, asString( el ).c_str() );
	valid = false;
	break;
      }

      if ( (el = m->value( tag_enabled ))->isBoolean() ) {
	state.second = el->asBoolean()->value();
      } else {
	y2warning( "List entry %d: MAP[enabled]: BOOLEAN expected but got '%s'", i, asString( el ).c_str() );
	valid = false;
	break;
      }

      // store state in vector
      _value.push_back( state );

    } else {
      y2warning( "List entry %d: MAP expected but got '%s'", i, asString( el ).c_str() );
      valid = false;
      break;
    }
  }

  if ( ! valid ) {
    _value.clear();
  }

  return valid;
}

/////////////////////////////////////////////////////////////////////////////////////////
// YCPMap <-> YCPMap
/////////////////////////////////////////////////////////////////////////////////////////

template<>
inline bool YcpArgLoad::Value<YT_MAP, YCPMap>::assign( const YCPValue & arg_r )
{
  _value = arg_r->asMap(); // YT_MAP asserted
  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// YCP interface functions
//
/////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************
 * @builtin Pkg::SourceSetRamCache (boolean allow) -> true
 *
 * In InstSys: Allow/prevent InstSrces from caching package metadata on ramdisk.
 * If no cache is used the media cannot be unmounted, i.e. no CD change possible.
 **/
YCPValue
PkgModuleFunctions::SourceSetRamCache (const YCPBoolean& a)
{
  YCPList args;
  args->add (a);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  bool & allow( decl.arg<YT_BOOLEAN, bool>( true ) );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  _y2pm.setCacheToRamdisk( allow );
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceStartManager (boolean autoEnable = true) -> true
 *
 * Make sure the InstSrcManager is up and knows all available InstSrces.
 * Depending on the value of autoEnable, InstSources may be enabled on the
 * fly. It's safe to call this multiple times, and once the InstSources are
 * actually enabled, it's even cheap (enabling an InstSrc is expensive).
 *
 * @param autoEnable If true, all InstSrces are enabled according to their default.
 * If false, InstSrces will be created in disabled state, and remain unchanged if
 * the InstSrcManager is already up.
 *
 * @return true
 **/
YCPValue
PkgModuleFunctions::SourceStartManager (const YCPBoolean& enable)
{
  YCPList args;
  if( ! enable.isNull ())
    args->add (enable);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  bool & autoEnable( decl.arg<YT_BOOLEAN, bool>( true ) );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  sourceStartManager( autoEnable );
  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceStartCache (boolean enabled_only = true) -> list of SrcIds (integer)
 *
 * Make sure the InstSrcManager is up, and return the list of SrcIds.
 * In fact nothing more than:
 * <PRE>
 *   SourceStartManager( enabled_only );
 *   return SourceGetCurrent( enabled_only );
 * </PRE>
 *
 * @param enabled_only If true, make sure all InstSrces are enabled according to
 * their default, and return the Ids of enabled InstSrces only. If false, return
 * the Ids of all known InstSrces.
 *
 * @return list of SrcIds (integer)
 **/
YCPValue
PkgModuleFunctions::SourceStartCache (const YCPBoolean& enabled)
{
  YCPList args;
  if( ! enabled.isNull ())
      args->add (enabled);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  bool & enabled_only( decl.arg<YT_BOOLEAN, bool>( true ) );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  sourceStartManager( enabled_only );
  return asYCPList( _y2pm.instSrcManager().getSources( enabled_only ) );
}

/****************************************************************************************
 * @builtin Pkg::SourceGetCurrent (boolean enabled_only = true) -> list of SrcIds (integer)
 *
 * Return the list of all enabled InstSrc Ids.
 *
 * @param enabled_only If true, or omitted, return the Ids of all enabled InstSrces.
 * If false, return the Ids of all known InstSrces.
 *
 * @return list of SrcIds (integer)
 **/
YCPValue
PkgModuleFunctions::SourceGetCurrent (const YCPBoolean& enabled)
{
  YCPList args;
  if( ! enabled.isNull ())
      args->add (enabled);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  bool & enabled_only( decl.arg<YT_BOOLEAN, bool>( true ) );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  return asYCPList( _y2pm.instSrcManager().getSources( enabled_only ) );
}

/****************************************************************************************
 * @builtin Pkg::SourceFinishAll () -> true
 *
 * Disable all InstSrces.
 *
 * @return true
 **/
YCPValue
PkgModuleFunctions::SourceFinishAll ()
{
  YCPList args;

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);
  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( _y2pm.hasInstSrcManager() ) {
    _y2pm.instSrcManager().disableAllSources();
  }
  return YCPBoolean( true );
}

/////////////////////////////////////////////////////////////////////////////////////////
// Query individual sources
/////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************
 * @builtin Pkg::SourceGeneralData (integer SrcId) -> map
 *
 * @param SrcId Specifies the InstSrc to query.
 *
 * @return General data about the source as a map:
 * <TABLE>
 * <TR><TD>$[<TD>"enabled"	<TD>: YCPBoolean
 * <TR><TD>,<TD>"product_dir"	<TD>: YCPString
 * <TR><TD>,<TD>"type"		<TD>: YCPString
 * <TR><TD>,<TD>"url"		<TD>: YCPString
 * <TR><TD>];
 * </TABLE>
 **/
YCPValue
PkgModuleFunctions::SourceGeneralData (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  constInstSrcDescrPtr descr = source_id->descr();

  YCPMap data;
  data->add( YCPString("enabled"),	YCPBoolean( descr->default_activate() ) );
  data->add( YCPString("product_dir"),	YCPString( descr->product_dir().asString() ) );
  data->add( YCPString("type"),		YCPString( InstSrc::toString( descr->type() ) ) );
  data->add( YCPString("url"),		YCPString( descr->url().asString() ) );
  return data;
}

/****************************************************************************************
 * @builtin Pkg::SourceMediaData (integer SrcId) -> map
 *
 * @param SrcId Specifies the InstSrc to query.
 *
 * @return Media data about the source as a map:
 * <TABLE>
 * <TR><TD>$[<TD>"media_count"	<TD>: YCPInteger
 * <TR><TD>,<TD>"media_id"	<TD>: YCPString
 * <TR><TD>,<TD>"media_vendor"	<TD>: YCPString
 * <TR><TD>,<TD>"url"		<TD>: YCPString
 * <TR><TD>];
 * </TABLE>
 **/
YCPValue
PkgModuleFunctions::SourceMediaData (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  constInstSrcDescrPtr descr = source_id->descr();

  YCPMap data;
  data->add( YCPString("media_count"),		YCPInteger( descr->media_count() ) );
  data->add( YCPString("media_id"),		YCPString( descr->media_id() ) );
  data->add( YCPString("media_vendor"),		YCPString( descr->media_vendor() ) );
  data->add( YCPString("url"),			YCPString( descr->url().asString() ) );
  return data;
}

/****************************************************************************************
 * @builtin Pkg::SourceProductData (integer SrcId) -> map
 *
 * @param SrcId Specifies the InstSrc to query.
 *
 * @return Product data about the source as a map:
 * <TABLE>
 * <TR><TD>$[<TD>"productname"		<TD>: YCPString
 * <TR><TD>,<TD>"productversion"	<TD>: YCPString
 * <TR><TD>,<TD>"baseproductname"	<TD>: YCPString
 * <TR><TD>,<TD>"baseproductversion"	<TD>: YCPString
 * <TR><TD>,<TD>"vendor"		<TD>: YCPString
 * <TR><TD>,<TD>"defaultbase"		<TD>: YCPString
 * <TR><TD>,<TD>"architectures"		<TD>: YCPList(YCPString)
 * <TR><TD>,<TD>"requires"		<TD>: YCPString
 * <TR><TD>,<TD>"linguas"		<TD>: YCPList(YCPString)
 * <TR><TD>,<TD>"label"			<TD>: YCPString
 * <TR><TD>,<TD>"labelmap"		<TD>: YCPMap(YCPString lang,YCPString label)
 * <TR><TD>,<TD>"language"		<TD>: YCPString
 * <TR><TD>,<TD>"timezone"		<TD>: YCPString
 * <TR><TD>,<TD>"descrdir"		<TD>: YCPString
 * <TR><TD>,<TD>"datadir"		<TD>: YCPString
 * <TR><TD>];
 * </TABLE>
 **/
YCPValue
PkgModuleFunctions::SourceProductData (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  constInstSrcDescrPtr descr = source_id->descr();

  YCPMap data;
  data->add( YCPString("productname"),		YCPString( descr->content_product().asPkgNameEd().name ) );
  data->add( YCPString("productversion"),	YCPString( descr->content_product().asPkgNameEd().edition.version() ) );
  data->add( YCPString("baseproductname"),	YCPString( descr->content_baseproduct().asPkgNameEd().name ) );
  data->add( YCPString("baseproductversion"),	YCPString( descr->content_baseproduct().asPkgNameEd().edition.version() ) );
  data->add( YCPString("vendor"),		YCPString( descr->content_vendor() ) );
  data->add( YCPString("defaultbase"),		YCPString( descr->content_defaultbase() ) );

  InstSrcDescr::ArchMap::const_iterator it1 = descr->content_archmap().find ( _y2pm.baseArch() );
  if ( it1 != descr->content_archmap().end() ) {
    YCPList architectures;
    for ( std::list<PkgArch>::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2 ) {
      architectures->add ( YCPString( *it2 ) );
    }
    data->add( YCPString("architectures"),	architectures );
  }

  data->add( YCPString("requires"),		YCPString( descr->content_requires().asString() ) );

  YCPList linguas;
  for ( InstSrcDescr::LinguasList::const_iterator it = descr->content_linguas().begin();
	it != descr->content_linguas().end(); ++it ) {
    linguas->add( YCPString( *it ) );
  }
  data->add( YCPString("linguas"),		linguas );

  data->add( YCPString("label"),		YCPString( descr->content_label() ) );

  YCPMap labelmap;
  for ( InstSrcDescr::LabelMap::const_iterator it = descr->content_labelmap().begin();
	it != descr->content_labelmap().end(); ++it ) {
    labelmap->add( YCPString( it->first ), YCPString( it->second ) );
  }
  data->add( YCPString("labelmap"),		labelmap );

  data->add( YCPString("language"),		YCPString( descr->content_language() ) );
  data->add( YCPString("timezone"),		YCPString( descr->content_timezone() ) );
  data->add( YCPString("descrdir"),		YCPString( descr->content_descrdir().asString() ) );
  data->add( YCPString("datadir"),		YCPString( descr->content_datadir().asString() ) );
  return data;
}

/****************************************************************************************
 * @builtin Pkg::SourceProduct (integer SrcId) -> map
 *
 * @param SrcId Specifies the InstSrc to query.
 *
 * @return Product info as a map. See @ref Descr2Map
 **/
YCPValue
PkgModuleFunctions::SourceProduct (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  return Descr2Map( source_id->descr() );
}

/****************************************************************************************
 * @builtin Pkg::SourceProvideFile (integer SrcId, integer medianr, string file) -> string path
 *
 * Let an InstSrc provide some file (make it available at the local filesystem).
 *
 * @param SrcId	Specifies the InstSrc .
 *
 * @param medianr Number of the media the file is located on ('1' for the 1st media).
 *
 * @param file Filename relative to the media root.
 *
 * @return local path as string
 **/
YCPValue
PkgModuleFunctions::SourceProvideFile (const YCPInteger& id, const YCPInteger& mid, const YCPString& f)
{
  YCPList args;
  args->add (id);
  args->add (mid);
  args->add (f);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  int &                    medianr  ( decl.arg<YT_INTEGER, int>() );
  Pathname &               file     ( decl.arg<YT_STRING,  Pathname>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  Pathname localpath;
  PMError err = source_id->provideFile( medianr, file, localpath );

  if ( err )
    return pkgError( err );

  return YCPString( localpath.asString() );
}

/****************************************************************************************
 * @builtin Pkg::SourceProvideDir (integer SrcId, integer medianr, string dir) -> string path
 *
 * Let an InstSrc provide some directory (make it available at the local filesystem) and
 * all the files within it (non recursive).
 *
 * @param SrcId	Specifies the InstSrc .
 *
 * @param medianr Number of the media the file is located on ('1' for the 1st media).
 *
 * @param dir Directoryname relative to the media root.
 *
 * @return local path as string
 */
YCPValue
PkgModuleFunctions::SourceProvideDir (const YCPInteger& id, const YCPInteger& mid, const YCPString& d)
{
  YCPList args;
  args->add (id);
  args->add (mid);
  args->add (d);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  int &                    medianr  ( decl.arg<YT_INTEGER, int>() );
  Pathname &               dir     ( decl.arg<YT_STRING,  Pathname>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  Pathname localpath;
  PMError err = source_id->provideDir( medianr, dir, localpath );

  if ( err )
    return pkgError( err );

  return YCPString( localpath.asString() );
}

/****************************************************************************************
 * @builtin Pkg::SourceChangeUrl (integer SrcId, string url ) -> true
 *
 * Change url of an InstSrc. Used primarely when re-starting during installation
 * and a cd-device changed from hdX to srX since ide-scsi was activated.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @param url The new url to use.
 *
 * @return true
 **/
YCPValue
PkgModuleFunctions::SourceChangeUrl (const YCPInteger& id, const YCPString& u)
{
  YCPList args;
  args->add (id);
  args->add (u);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  Url &                    url      ( decl.arg<YT_STRING, Url>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().rewriteUrl( source_id, url );

  if ( err )
    return pkgError( err );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceInstallOrder (map order_map) -> true
 *
 * Explicitly set an install order.
 *
 * @param order_map A map of 'int order : int source_id'. source_ids are expected to
 * denote known and enabled sources.
 *
 * @return true
 **/
YCPValue
PkgModuleFunctions::SourceInstallOrder (const YCPMap& ord)
{
  YCPList args;
  args->add (ord);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  YCPMap & order_map( decl.arg<YT_MAP, YCPMap>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  InstSrcManager::InstOrder order;
  order.reserve( order_map->size() );
  bool error = false;

  for ( YCPMapIterator it = order_map->begin(); it != order_map->end(); ++it ) {

    if ( it.value()->isInteger() ) {
      InstSrc::UniqueID uId( it.value()->asInteger()->value() );
      InstSrcManager::ISrcId source_id( _y2pm.instSrcManager().getSourceByID( uId ) );
      if ( source_id ) {
	if ( source_id->enabled() ) {
	  order.push_back( uId );  // finaly ;)

	} else {
	  y2error ("order map entry '%s:%s': source not enabled",
		    it.key()->toString().c_str(),
		    it.value()->toString().c_str() );
	  error = true;
	}
      } else {
	y2error ("order map entry '%s:%s': bad source id",
		  it.key()->toString().c_str(),
		  it.value()->toString().c_str() );
	error = true;
      }
    } else {
      y2error ("order map entry '%s:%s': integer value expected",
		it.key()->toString().c_str(),
		it.value()->toString().c_str() );
      error = true;
    }
  }
  if ( error ) {
    return pkgError( Error::E_bad_args );
  }

  // store new instorder
  _y2pm.instSrcManager().setInstOrder( order );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceCacheCopyTo (string dir) -> true
 *
 * Copy cache data of all installation sources to the target located below 'dir'.
 * To be called at end of initial installation.
 *
 * @param dir Root directory of target.
 *
 * @return true
 **/
YCPValue
PkgModuleFunctions::SourceCacheCopyTo (const YCPString& dir)
{
  YCPList args;
  args->add (dir);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  Pathname & nroot( decl.arg<YT_STRING, Pathname>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  // Install InstSrces metadata in system
  PMError err = _y2pm.instSrcManager().cacheCopyTo( nroot );
  if ( err )
    return pkgError( err );

  // Install product data of all open sources according to installation order
#warning Review product data install here and in PM.
  InstSrcManager::ISrcIdList inst_order( _y2pm.instSrcManager().instOrderSources() );

  for ( InstSrcManager::ISrcIdList::const_iterator it = inst_order.begin(); it != inst_order.end(); ++it ) {
    _y2pm.instTarget().installProduct( (*it)->descr() );
  }

  // Actually there should be no need to do this here, as Y2PM::commitPackages
  // calls Y2PM::selectionManager().installOnTarget();
  Y2PM::selectionManager().installOnTarget();

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceScan (string media_url [, string product_dir]) -> list of SrcIds (integer)
 *
 * Load all InstSrces found at media_url, i.e. all sources mentioned in /media.1/products.
 * If no /media.1/products is available, InstSrc is expected to be located directly
 * below media_url (product_dir: /).
 *
 * If a product_dir is provided, only the InstSrc located at media_url/product_dir is loaded.
 *
 * In contrary to @ref SourceCreate, InstSrces are loaded into the InstSrcManager,
 * but not enabled (packages and selections are not provided to the PackageManager),
 * and the SrcIds of <b>all</b> InstSrces found are returned.
 *
 * @param url The media to scan.
 *
 * @param product_dir Restrict scan to a certain InstSrc located in media_url/product_dir.
 *
 * @return list of SrcIds (integer).
 **/
YCPValue
PkgModuleFunctions::SourceScan (const YCPString& media, const YCPString& pd)
{
  YCPList args;
  args->add (media);
  if( ! pd.isNull ())
      args->add (pd);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  Url &      media_url  ( decl.arg<YT_STRING, Url>() );
  Pathname & product_dir( decl.arg<YT_STRING, Pathname>( Pathname() ) ); // optional

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  PMError err;
  InstSrcManager::ISrcIdList nids;

  if ( product_dir.empty() ) {
    // scan all sources
    err = _y2pm.instSrcManager().scanMedia( nids, media_url );
  } else {
    // scan at product_dir
    InstSrcManager::ISrcId nid;
    err = _y2pm.instSrcManager().scanMedia( nid, media_url, product_dir );
    if ( nid ) {
      nids.push_back( nid );
    }
  }

  if ( nids.empty() )
    return pkgError( err, YCPList() );

  // return source_ids
  return asYCPList( nids );
}

/****************************************************************************************
 * @builtin Pkg::SourceCreate (string media_url [, string product_dir]) -> integer
 *
 * Load and enable all InstSrces found at media_url, i.e. all sources mentioned in /media.1/products.
 * If no /media.1/products is available, InstSrc is expected to be located directly below
 * media_url (product_dir: /).
 *
 * If a product_dir is provided, only the InstSrc located at media_url/product_dir is loaded
 * and enabled.
 *
 * @param url The media to scan.
 *
 * @param product_dir Restrict scan to a certain InstSrc located in media_url/product_dir.
 *
 * @return The source_id of the first InstSrc found on the media.
 **/
YCPValue
PkgModuleFunctions::SourceCreate (const YCPString& media, const YCPString& pd)
{
  YCPList args;
  args->add (media);
  if( ! pd.isNull ())
    args->add (pd);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  Url &      media_url  ( decl.arg<YT_STRING, Url>() );
  Pathname & product_dir( decl.arg<YT_STRING, Pathname>( Pathname() ) ); // optional

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  PMError err;
  InstSrcManager::ISrcIdList nids;

  if ( product_dir.empty() ) {
    // scan all sources
    err = _y2pm.instSrcManager().scanMedia( nids, media_url );
  } else {
    // scan at product_dir
    InstSrcManager::ISrcId nid;
    err = _y2pm.instSrcManager().scanMedia( nid, media_url, product_dir );
    if ( nid ) {
      nids.push_back( nid );
    }
  }

  if ( nids.empty() )
    return pkgError( err );

  // enable the sources
  for ( InstSrcManager::ISrcIdList::const_iterator it = nids.begin(); it != nids.end(); ++it ) {
    _y2pm.instSrcManager().enableSource( *it );
  }

  // return 1st source_id
  return asYCPInteger( *nids.begin() );
}

/****************************************************************************************
 * @builtin Pkg::SourceSetEnabled (integer SrcId, boolean enabled) -> bool
 *
 * Set the default activation state of an InsrSrc.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @param enabled Default activation state of source.
 *
 * @return bool
 **/
YCPValue
PkgModuleFunctions::SourceSetEnabled (const YCPInteger& id, const YCPBoolean& e)
{
  YCPList args;
  args->add (id);
  args->add (e);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );
  bool &                   enabled  ( decl.arg<YT_BOOLEAN, bool>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().setAutoenable( source_id, enabled );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceFinish (integer SrcId) -> bool
 *
 * Disable an InsrSrc.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @return bool
 **/
YCPValue
PkgModuleFunctions::SourceFinish (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().disableSource( source_id );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceDelete (integer SrcId) -> bool
 *
 * Delete an InsrSrc. The InsrSrc together with all metadata cached on disk
 * is removed. The SrcId passed becomes invalid (other SrcIds stay valid).
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @return bool
 **/
YCPValue
PkgModuleFunctions::SourceDelete (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err =_y2pm.instSrcManager().deleteSource( source_id );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceEditGet () -> list of source states (map)
 *
 * Return a list of states for all known InstSources sorted according to the
 * source priority (highest first). A source state is a map:
 * <TABLE>
 * <TR><TD>$[<TD>"SrcId"	<TD>: YCPInteger
 * <TR><TD>,<TD>"enabled"	<TD>: YCPBoolean
 * <TR><TD>];
 * </TABLE>
 *
 * @return list of source states (map)
 **/
YCPValue
PkgModuleFunctions::SourceEditGet ()
{
  YCPList args;

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);
  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  return asYCPList( _y2pm.instSrcManager().editGet() );
}

/****************************************************************************************
 * @builtin Pkg::SourceEditSet ( list source_states ) -> true
 *
 * Rearange known InstSrces rank and default state according to source_states
 * (highest priority first). Known InstSrces not mentioned in source_states
 * are deleted.
 *
 * @param source_states List of source states. Same format as returned by
 * @ref SourceEditGet.
 *
 * @return true
 **/
YCPValue
PkgModuleFunctions::SourceEditSet (const YCPList& states)
{
  YCPList args;
  args->add (states);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::SrcStateVector & source_states( decl.arg<YT_LIST, InstSrcManager::SrcStateVector>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  PMError err = _y2pm.instSrcManager().editSet( source_states );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// DEPRECATED
//
/////////////////////////////////////////////////////////////////////////////////////////

/****************************************************************************************
 * @builtin Pkg::SourceRaisePriority (integer SrcId) -> bool
 *
 * Raise priority of source.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @return bool
 **/
YCPValue
PkgModuleFunctions::SourceRaisePriority (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().rankUp( source_id );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceLowerPriority (integer SrcId) -> void
 *
 * Lower priority of source.
 *
 * @param SrcId Specifies the InstSrc.
 *
 * @return bool
 */
YCPValue
PkgModuleFunctions::SourceLowerPriority (const YCPInteger& id)
{
  YCPList args;
  args->add (id);

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);

  InstSrcManager::ISrcId & source_id( decl.arg<YT_INTEGER, InstSrcManager::ISrcId>() );

  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  if ( ! source_id )
    return pkgError( InstSrcError::E_bad_id );

  PMError err = _y2pm.instSrcManager().rankDown( source_id );
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/****************************************************************************************
 * @builtin Pkg::SourceSaveRanks () -> boolean
 *
 * Save ranks to disk. Return true on success, false on error.
 **/
YCPValue
PkgModuleFunctions::SourceSaveRanks ()
{
  YCPList args;

  //-------------------------------------------------------------------------------------//
  YcpArgLoad decl(__FUNCTION__);
  if ( ! decl.load( args ) ) {
    return pkgError_bad_args;
  }
  //-------------------------------------------------------------------------------------//

  PMError err = _y2pm.instSrcManager().setNewRanks();
  if ( err )
    return pkgError( err, YCPBoolean( false ) );

  return YCPBoolean( true );
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Helper functions
//
/////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PkgModuleFunctions::sourceStartManager
//	METHOD TYPE : bool
//
bool PkgModuleFunctions::sourceStartManager( bool autoEnable )
{
  if ( autoEnable ) {
    _y2pm.instSrcManager().enableDefaultSources();
  } else {
    _y2pm.noAutoInstSrcManager();
  }
  return true;
}

