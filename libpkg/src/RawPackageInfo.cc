/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 * ----------------------------------------------------------
 *
 * File:	  RawPackageInfo.cc
 *
 * Author: 	  Stefan Schubert <schubi@suse.de>
 *
 * Description:   Parse the common.pkd which gives information
 *                about the packages
 *
 * $Header$
 *
 *************************************************************/

#define _GNU_SOURCE 1

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <iconv.h>

#include <fstream>
#include <sstream>

#include <ycp/y2log.h>
#include <pkg/RawPackageInfo.h>

#include "Pathname.h"
#include "PathInfo.h"
#include "TagSets.h"
#include "RawPackage.h"

#define BLOCKSIZE 2

#define PACKAGENAME	"Filename"
#define RPMNAME		"RpmName"
#define SERIES 		"Series"
#define RPMGROUP	"RpmGroup"
#define INSTPATH 	"InstPath"
#define SIZE 		"Size"
#define BUILDTIME 	"Buildtime"
#define BUILDFROM	"BuiltFrom"
#define FLAG		"Flag"
#define CATEGORY	"Category"
#define COPYRIGHT	"Copyright"
#define AUTHORNAME	"AuthorName"
#define AUTHOREMAIL	"AuthorEmail"
#define AUTHORADDRESS	"AuthorAddress"
#define VERSION		"Version"
#define OBSOLETES	"Obsoletes"
#define PROVIDES	"Provides"
#define REQUIRES	"Requires"
#define CONFLICTS	"Conflicts"
#define STARTCOMMAND	"StartCommand"

#define LABEL		"Label"
#define UTF8DESCRIPTION "Utf8Description"
#define DELNOTIFY	"Delnotify"
#define DESCRIPTION	"Description"
#define VISIBLE		"Visible"
#define CATEGORY	"Category"
#define KIND		"Kind"
#define TRUE		"true"
#define FALSE		"false"
#define NOTIFY		"Notify"
#define YFITON		"Yfiton"
#define YFITONLED	"Yfitonled"
#define NOITPIRCSED	"Noitpircsed"
#define SUGGESTS	"Suggests"
#define ARCHITECTURE	"Architecture"

#define INFO		"Info"
#define OFNI		"Ofni"
#define TOINSTALL	"Toinstall"
#define LLATSNIOT	"Llatsniot"
#define DEFAULT		"default"

#define BUFFERLEN	11000
#define READLEN		10000
#define IDENTLENG	50
#define PATHLEN		256
#define DEFAULT_DESCR   "/mnt/suse/setup/descr"
#define DEFAULT_SRC_DIR "/usr/src/packages/"
#define DEFAULT_LANGUAGE "en"
#define SPMSTRING	".src.rpm"
#define NOSRCSTRING	".nosrc.rpm"
#define SHORTSPMSTRING  ".spm"
#define RPMSTRING	".rpm"
#define PKD		".pkd"
#define PACK		"Pack:"
#define BASEPACKAGE	"A2E"
#define COMMONPKD	"common.pkd"
#define SEL		".sel"

char *program_name = "YaST2 (pkginfo)"; // is needed form resolve

/******************************************************************
**
**
**	FUNCTION NAME : searchIn
**	FUNCTION TYPE : bool
**
**	DESCRIPTION :
*/
bool searchIn( const string & stack_tr, const string & needle_tr, bool casesensitive_br )
{
  if ( casesensitive_br )
    return ( stack_tr.find( needle_tr ) != string::npos );
  // else ignorecase
  return ( strcasestr( stack_tr.c_str(), needle_tr.c_str() ) != NULL );
}

/******************************************************************
**
**
**	FUNCTION NAME : CAT_mem
**	FUNCTION TYPE : void
**
**	DESCRIPTION :
*/
#include <sys/types.h>
#include <unistd.h>
#include <fstream>
inline void CAT_mem( const string & text_tr = "" ) {
  char buf[1024];
  sprintf( buf, "/proc/%d/status", getpid() );
  ifstream in( buf );
  string out;
  do {
    in.getline( buf, 1024 );
    if ( in.gcount() > 2 && buf[0] == 'V' && buf[1] == 'm' ) {
      if (    (buf[2] == 'L' && buf[3] == 'c' && buf[4] == 'k')
	   || (buf[2] == 'E' && buf[3] == 'x' && buf[4] == 'e') ) {
	continue;
      }

      for ( char * b = buf; *b; ++b ) {
	if ( *b == '\t' ) {
	  *b = ' ';
	  break;
	}
      }
      out += buf;
      out += " | ";
    }
  } while( in.good() );
  out += text_tr;
  y2internal( "%s", out.c_str() );
}

/******************************************************************
**
**
**	FUNCTION NAME : DumpToString
**	FUNCTION TYPE : string
**
**	DESCRIPTION :
*/

template<class T> string DumpToString( const T & obj )
{
    ostringstream str;
    str << obj;
    return string (str.str ());
}

/******************************************************************
**
**
**	FUNCTION NAME : stripISOCountry
**	FUNCTION TYPE : string
**
**	DESCRIPTION : stupid version
*/
inline string stripISOCountry( const string & lang_tr )
{
  string::size_type delim_ii = lang_tr.find( '_' );

  if ( delim_ii == string::npos )
    return string();

  return lang_tr.substr( 0, delim_ii );
}

/******************************************************************
**
**
**	FUNCTION NAME : getfirsttwo
**	FUNCTION TYPE : void
**
**	DESCRIPTION :
*/
inline void getfirsttwo( const string & line_tr, string & w1_tr, string & w2_tr )
{
  string::size_type delim_ii = line_tr.find_first_of( " \t" );
  if ( delim_ii == string::npos ) {
    // no whitespace on line
    w1_tr = line_tr;
    w2_tr.erase();
  } else {
    w1_tr = line_tr.substr( 0, delim_ii );
    string::size_type start_ii = line_tr.find_first_not_of( " \t", delim_ii );
    if ( start_ii == string::npos ) {
      // no wordstart after whitespace
      w2_tr.erase();
    } else {
      delim_ii = line_tr.find_first_of( " \t", start_ii );
      if ( delim_ii == string::npos ) {
	w2_tr = line_tr.substr( start_ii );
      } else {
	w2_tr = line_tr.substr( start_ii, delim_ii-start_ii );
      }
    }
  }
}


///////////////////////////////////////////////////////////////////

static const CommonTags *      commonTags_pCm = 0;
static const LanguageTags *    languageTags_pCm = 0;
static const PkdPreambleTags * pkdPreambleTags_pCm = 0;

static const string languageFallback_tm( "en" );
static const string defaultGroup_tm( "Unsorted" );

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::RereadErr
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
inline void RawPackageInfo::RereadErr::failed( const string & fname_tr ) const {
  if ( ++cnt_i >= limit_i ) {
    y2error( "attemt %d to access already closed file '%s'", cnt_i, fname_tr.c_str() );
    if( limit_i < 1000 )
      limit_i *= 10;
    else if ( limit_i == 1000 )
      limit_i = 5000;
    else
      limit_i += 5000;
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::closeMedium
//	METHOD TYPE : bool
//
//	DESCRIPTION : Sign that we should not access the *.pkd anymore.
//
bool RawPackageInfo::closeMedium()
{
  if ( commonPKD_pF || languagePKD_pF ) {
    delete commonPKD_pF;
    commonPKD_pF = 0;
    delete languagePKD_pF;
    languagePKD_pF = 0;
    y2milestone ( "Medium of common.pkd closed" );
    rereadErr_C.reset();
  }

  return true;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::clearPackages
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
inline void RawPackageInfo::clearPackages()
{
  y2debug( "start" );

  for ( PkgStore::iterator i = packages_VpC.begin();
	i != packages_VpC.end(); ++i ) {
    delete *i;
  }
  PkgStore dummy_;
  packages_VpC.swap( dummy_ );

  name2package_VpC.clear();
  filename2package_VpC.clear();

  seriesNames_Vt.clear();
  groupNames_Vt.clear();

  maxCDNum_i = 0;

  delete commonPKD_pF;
  commonPKD_pF = 0;
  delete languagePKD_pF;
  languagePKD_pF = 0;
  rereadErr_C.reset();

  parsedLanguage_t = languageRecodeFrom_t = languageRecodeTo_t = "";
  if ( iconv_cd != (iconv_t)(-1) ) {
    iconv_close( iconv_cd );
    iconv_cd = (iconv_t)(-1);
  }

  y2debug( "done" );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::package
//	METHOD TYPE : RawPkg
//
//	DESCRIPTION :
//
inline RawPackageInfo::RawPkg RawPackageInfo::package( const string & name_rt ) const
{
  PkgLookup::const_iterator i = name2package_VpC.find( name_rt );
  if ( i == name2package_VpC.end() )
    return 0;
  return i->second;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::filename
//	METHOD TYPE : RawPkg
//
//	DESCRIPTION :
//
inline RawPackageInfo::RawPkg RawPackageInfo::filename( const string & name_rt ) const
{
  PkgLookup::const_iterator i = filename2package_VpC.find( name_rt );
  if ( i == filename2package_VpC.end() )
    return 0;
  return i->second;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::RawPackageInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
RawPackageInfo::RawPackageInfo( const string path_tr,
				const string language_tr,
				const string commonPkd_tr,
				const bool   memmoryOptimized_br )
    : descrPath_t       ( path_tr )
    , selectedLanguage_t( language_tr )
    , commonPkd_t       ( commonPkd_tr )
    , commonPKD_pF      ( 0 )
    , languagePKD_pF    ( 0 )
    , iconv_cd          ( (iconv_t)(-1) )
    , inParser_b        ( false )
    , maxCDNum_i        ( 0 )
{
  // check wheter to initialise the tagSets
  if ( !pkdPreambleTags_pCm ) {
    pkdPreambleTags_pCm = new PkdPreambleTags;
    y2debug( "Initialise: %s", DumpToString( *pkdPreambleTags_pCm ).c_str() );
  }
  if ( !commonTags_pCm ) {
    commonTags_pCm = new CommonTags;
    y2debug( "Initialise: %s", DumpToString( *commonTags_pCm ).c_str() );
  }
  if ( !languageTags_pCm ) {
    languageTags_pCm = new LanguageTags;
    y2debug( "Initialise: %s", DumpToString( *languageTags_pCm ).c_str() );
  }

  // quick check of args
  if ( descrPath_t.empty() ) {
    descrPath_t = DEFAULT_DESCR;
    y2debug( "using default descrPath \"%s\"", descrPath_t.c_str() );
  }

  if ( commonPkd_t.empty() ) {
    commonPkd_t = COMMONPKD;
  } else if ( commonPkd_t != COMMONPKD ) {
    y2debug( "using alternate common.pkd  \"%s\"", commonPkd_t.c_str() );
  }

  if ( selectedLanguage_t.empty() ) {
    selectedLanguage_t = DEFAULT_LANGUAGE;
    y2debug( "using default language \"%s\"", selectedLanguage_t.c_str() );
  }

  y2milestone( "creating RawPackageInfo: path \"%s\", \"%s\", lang \"%s\"",
	       descrPath_t.c_str(),
	       commonPkd_t.c_str(),
	       selectedLanguage_t.c_str() );

  // initialise

// FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME
//  getLinkList ( linkList );
// WARNING getLinkList cannot work here cause RawPackageInfo knows nothing
// about the targetsystem. It should belong to y2a_package or higher.
// FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME
  linkList.clear();


  parseCommonPKD();
  parseLanguagePKD();
  parseSelections();

  if ( 0 && numPackages() ) {
    y2security( "RawPackage  %d", sizeof( RawPackage ) );
    y2security( "string      %d", sizeof( string ) );
    y2security( "PPSM        %d", sizeof( PackagePartitionSizeMap ) );
    y2security( "packages    %d", numPackages() );
    y2security( "maxCDNum    %d", maxCDNum_i );
    y2security( "seriesNames %d", seriesNames_Vt.size() );
    y2security( "groupNames  %d", groupNames_Vt.size() );
    y2security( "name2package     %d", name2package_VpC.size() );
    y2security( "filename2package %d", filename2package_VpC.size() );
    y2security( "Example: %s", DumpToString( *packages_VpC[0] ).c_str() );
  }

}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::~RawPackageInfo
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
RawPackageInfo::~RawPackageInfo()
{
  y2milestone ( "deleting RawPackageInfo" );
  clearPackages();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::getPackage
//	METHOD TYPE : PkdData
//
//	DESCRIPTION :
//
PkdData RawPackageInfo::getPackage( unsigned idx_ii ) const
{
  return ( idx_ii < numPackages() ? packages_VpC[idx_ii] : 0 );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::getPackageByName
//	METHOD TYPE : PkdData
//
//	DESCRIPTION :
//
PkdData RawPackageInfo::getPackageByName( const string & name_rt ) const
{
  return package( name_rt );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::getPackageByFilename
//	METHOD TYPE : PkdData
//
//	DESCRIPTION :
//
PkdData RawPackageInfo::getPackageByFilename( const string & fname_rt ) const
{
  return filename( fname_rt );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// old interface:
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*--------------------------------------------------------------*/
/* Return the package-list of provides				*/
/* [<packagename1>,<packagename2>,<packagename3>,... ]		*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getProvides( const string packageName )
{
   PkdData cpkg_pCi = getPackageByName( packageName );

   if ( ! cpkg_pCi ) {
     y2warning( "unknown package '%s'", packageName.c_str() );
     return PackList();
   }

   return cpkg_pCi->providesList();
}


/*-------------------------------------------------------------------*/
/* Return all Requires tags of a package		             */
/*-------------------------------------------------------------------*/
DependList RawPackageInfo::getRequires( const string packageName )
{
  PkdData cpkg_pCi = getPackageByName( packageName );

  if ( ! cpkg_pCi ) {
    y2warning( "unknown package '%s'", packageName.c_str() );
    return DependList();
  }

  return cpkg_pCi->requiresList();
}

/*----------------------------------------------------------------------*/
/*  Reading all symb-links in the root-partition and saving it into	*/
/*  a list.								*/
/*----------------------------------------------------------------------*/

// FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME
//  getLinkList ( linkList );
// WARNING getLinkList cannot work here cause RawPackageInfo knows nothing
// about the targetsystem. It should belong to y2a_package or higher.
// FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME

bool RawPackageInfo::getLinkList ( LinkList &linkList )
{
// FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME
    linkList.clear();
    return true;
// FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME  FIXME

    DIR *dir = opendir("/");
    struct stat check;
    char buffer[PATHLEN];
    char sourceBuffer[PATHLEN];

    if (!dir)
    {
       y2error( "Can't open directory / for reading" );
       return false;
    }

    linkList.clear();

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
	if (!strcmp(entry->d_name, ".") ||
	    !(strcmp(entry->d_name, ".."))) continue;

	// Check if it is a link
        sprintf ( sourceBuffer, "/%s", entry->d_name );
        if ( lstat ( sourceBuffer, &check) == 0 &&
	     S_ISLNK ( check.st_mode ) )
        {
	   int len = readlink ( sourceBuffer, buffer, PATHLEN -1 );
	   if ( len > 0 )
	   {
	      buffer[len] = 0;

	      // no leading '/' except for "/"
	      string target_ti( buffer );
	      if ( target_ti.size() > 1 && target_ti[0] == '/' ) {
		target_ti.erase( 0, 1 );
	      }

	      linkList.insert(pair<const string, const string>
			      ( (string) entry->d_name,
				target_ti ) );
	      y2milestone( "add '%s' -> '%s'", entry->d_name, target_ti.c_str () );
	   }
	}
    }
    if ( dir )
    {
      closedir(dir);
    }

    return true;
}


/*--------------------------------------------------------------*/
/* Return the package-list like					*/
/* [<packagename1,version1>,<packagename2,version2>,... ]	*/
/* All packages ( with installation order )			*/
/*--------------------------------------------------------------*/
PackVersList RawPackageInfo::getRawPackageList( bool withSourcePackages_br )
{
  PackVersList ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM && !withSourcePackages_br )
      continue;

    ret_VCi.push_back( packages_VpC[i]->asPackageKey() );
  }

  return ret_VCi;
}


/*--------------------------------------------------------------*/
/* Returns a list of packages to which the searchmask-string:   */
/* [<packagename1>,<packagename2>,<packagename3>,... ]		*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getSearchResult ( const string searchmask_tr,
					   const bool   onlyName_br,
					   const bool   casesensitive_br )
{
  PackList ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM )
      continue;

    RawPkg cpkg_pCi = packages_VpC[i];

    bool match_bi = searchIn( cpkg_pCi->name(), searchmask_tr, casesensitive_br );

    if ( !match_bi && !onlyName_br ) {
      if (    searchIn( cpkg_pCi->label(),       searchmask_tr, casesensitive_br )
	   || searchIn( cpkg_pCi->description(), searchmask_tr, casesensitive_br )
	   || searchIn( cpkg_pCi->instNotify(),  searchmask_tr, casesensitive_br )
	   || searchIn( cpkg_pCi->delNotify(),   searchmask_tr, casesensitive_br ) )
	match_bi = true;
    }

    if ( match_bi ) {
      ret_VCi.insert( cpkg_pCi->name() );
    }
  }

  return ret_VCi;
}

/*-------------------------------------------------------------------*/
/* Return all Requires dependency of every package which is defined  */
/* in common.pkd. The return-value is a map which contains     	     */
/* the package-name as index and a list of package-names from	     */
/* which the package is depent.					     */
/*-------------------------------------------------------------------*/
PackDepLMap RawPackageInfo::getRawRequiresDependency()
{
  PackDepLMap ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM )
      continue;

    RawPkg cpkg_pCi = packages_VpC[i];
    ret_VCi.insert( PackDepLMap::value_type( cpkg_pCi->asPackageKey(),
					     cpkg_pCi->requiresList() ) );
  }

  return ret_VCi;
}


/*-------------------------------------------------------------------*/
/* Return all Provides dependency of every package which is defined  */
/* in common.pkd. The return-value is a map which contains     	     */
/* the package-name as index and a list of package-names from	     */
/* which the package is depent.					     */
/*-------------------------------------------------------------------*/
PackTagLMap RawPackageInfo::getRawProvidesDependency()
{
  PackTagLMap ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM )
      continue;

    RawPkg cpkg_pCi = packages_VpC[i];
    ret_VCi.insert( PackTagLMap::value_type( cpkg_pCi->asPackageKey(),
					     cpkg_pCi->providesList() ) );
  }

  return ret_VCi;
}


/*-------------------------------------------------------------------*/
/* Return all Obsolutes dependency of every package which is defined */
/* in common.pkd. The return-value is a map which contains     	     */
/* the package-name as index and a list of package-names from	     */
/* which the package is depent.					     */
/*-------------------------------------------------------------------*/
PackTagLMap RawPackageInfo::getRawObsoletesDependency()
{
  PackTagLMap ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM )
      continue;

    RawPkg cpkg_pCi = packages_VpC[i];
    ret_VCi.insert( PackTagLMap::value_type( cpkg_pCi->asPackageKey(),
					     cpkg_pCi->obsoletesList() ) );
  }

  return ret_VCi;
}

/*-------------------------------------------------------------------*/
/* Return all Conflicts dependency of every package which is defined */
/* in common.pkd. The return-value is a map which contains     	     */
/* the package-name as index and a list of package-names from	     */
/* which the package is depent 					     */
/*-------------------------------------------------------------------*/
PackDepLMap RawPackageInfo::getRawConflictsDependency()
{
  PackDepLMap ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM )
      continue;

    RawPkg cpkg_pCi = packages_VpC[i];
    ret_VCi.insert( PackDepLMap::value_type( cpkg_pCi->asPackageKey(),
					     cpkg_pCi->conflictsList() ) );
  }

  return ret_VCi;
}

/*--------------------------------------------------------------------*/
/* Return all Requires dependency of every selection class which is   */
/* defined in suse/setup/descr/ *.sel. The return-value is a map which*/
/* contains the selection class as index and a list of selections,    */
/* groups... from which the selection is depent.		      */
/*--------------------------------------------------------------------*/
PackDepLMap RawPackageInfo::getSelRequiresDependency()
{
   PackDepLMap RequiresMap = getSelDependency ( REQU );

   return ( RequiresMap );
}

/*--------------------------------------------------------------------*/
/* Return all Provides dependency of every selection class which is   */
/* defined in suse/setup/descr/ *.sel. The return-value is a map which*/
/* contains the selection class as index and a list of selections,    */
/* groups... from which the selection is depent.		      */
/*--------------------------------------------------------------------*/
PackTagLMap RawPackageInfo::getSelProvidesDependency()
{
   PackTagLMap ProvidesMap = getSelTag ( PROV );

   return ( ProvidesMap );
}

/*--------------------------------------------------------------------*/
/* Return all Suggests dependency of every selection class which is   */
/* defined in suse/setup/descr/ *.sel. The return-value is a map which*/
/* contains the selection class as index and a list of selections,    */
/* groups... from which the selection is depent.		      */
/*--------------------------------------------------------------------*/
PackTagLMap RawPackageInfo::getSelSuggestsDependency()
{
   PackTagLMap SuggestsMap = getSelTag ( SUGGE );

   return ( SuggestsMap );
}

/*--------------------------------------------------------------------*/
/* Return all Conflict dependency of every selection class which is   */
/* defined in suse/setup/descr/ *.sel. The return-value is a map which*/
/* contains the selection class as index and a list of selections,    */
/* groups... from which the selection is depent.		      */
/*--------------------------------------------------------------------*/
PackDepLMap RawPackageInfo::getSelConflictsDependency()
{
   PackDepLMap ConflictsMap = getSelDependency ( CONFL );

   return ( ConflictsMap );
}

/*-------------------------------------------------------------------------*/
/* Return label of packages in the patchdescription files. 		   */
/* return: string							   */
/*-------------------------------------------------------------------------*/


string RawPackageInfo::getLabel( const string packageName_tr )
{
  PkdData cpkg_pCi = getPackageByName( packageName_tr );

  if ( ! cpkg_pCi ) {
    y2warning( "unknown package '%s'", packageName_tr.c_str() );
    return "";
  }

  return cpkg_pCi->label();
}


/*-------------------------------------------------------------------------*/
/* Return short packagename which is used in the rpm-DB			   */
/*-------------------------------------------------------------------------*/
string RawPackageInfo::getShortName( const string packageName_tr )
{
  PkdData cpkg_pCi = getPackageByFilename( packageName_tr );

  if ( !cpkg_pCi ) {
    cpkg_pCi = getPackageByFilename( packageName_tr + RPMSTRING );
  }
  if ( !cpkg_pCi ) {
    cpkg_pCi = getPackageByFilename( packageName_tr + SPMSTRING );
  }
  if ( !cpkg_pCi ) {
    cpkg_pCi = getPackageByFilename( packageName_tr + NOSRCSTRING );
  }
  if ( !cpkg_pCi ) {
    cpkg_pCi = getPackageByFilename( packageName_tr + SHORTSPMSTRING );
  }

  if ( !cpkg_pCi ) {
    cpkg_pCi = getPackageByName( packageName_tr );
  }

  if ( ! cpkg_pCi ) {
    y2warning( "neither filename nor package '%s'", packageName_tr.c_str() );
    return "";
  }

  return cpkg_pCi->name();
}



/*-------------------------------------------------------------------------*/
/* Get package-info for the desired package.				   */
/* input: packageName							   */
/* output: shortDescription longDescription				   */
/*         delDescription ( Warning if deselected )			   */
/* return: true if package was found					   */
/*-------------------------------------------------------------------------*/
bool RawPackageInfo::getRawPackageDescritption( const string packageName_tr,
						string & shortDescription_tr,
						string & longDescription_tr,
						string & notify_tr,
						string & delDescription_tr,
						string & category_tr,
						int &    size_ir )
{
  PkdData cpkg_pCi = getPackageByName( packageName_tr );

  if ( !cpkg_pCi ) {
    y2warning( "unknown package '%s'", packageName_tr.c_str() );
    shortDescription_tr = longDescription_tr = notify_tr = delDescription_tr = category_tr = "";
    size_ir = 0;
    return false;
  }

  shortDescription_tr = cpkg_pCi->label();
  longDescription_tr  = cpkg_pCi->description();
  notify_tr           = cpkg_pCi->instNotify();
  delDescription_tr   = cpkg_pCi->delNotify();
  category_tr         = cpkg_pCi->category();
  size_ir             = cpkg_pCi->sizeInK();

  return true;
}


/*-------------------------------------------------------------------------*
 * Get package-install-info for the desired package.
 * input: packageName
 * output: basePackage , installationPosition, cdNr, instPath
 *  return: true if package was found
 *-------------------------------------------------------------------------*/


bool RawPackageInfo::getRawPackageInstallationInfo( const string packageName_tr,
						    bool &   basePackage_br,
						    int &    installationPosition_ir,
						    int &    cdNr_ir,
						    string & instPath_tr,
						    string & version_tr,
						    long &   buildTime_ir,
						    int &    rpmSize_ir	    )
{
  PkdData cpkg_pCi = getPackageByName( packageName_tr );

  if ( !cpkg_pCi ) {
    y2warning( "unknown package '%s'", packageName_tr.c_str() );
    basePackage_br = false;
    installationPosition_ir = cdNr_ir = rpmSize_ir = 0;
    buildTime_ir = 0;
    instPath_tr = version_tr = "";
    return false;
  }

  basePackage_br          = cpkg_pCi->isBasepkg();
  installationPosition_ir = cpkg_pCi->instIdx();
  cdNr_ir                 = cpkg_pCi->onCD();
  instPath_tr             = cpkg_pCi->instPath();
  version_tr              = cpkg_pCi->version();
  buildTime_ir            = cpkg_pCi->buildTime();
  rpmSize_ir	          = cpkg_pCi->rpmSize();

  return true;
}



/*-------------------------------------------------------------------------*/
/* Get the serie of a package.				   		   */
/* input: packageName							   */
/* return: name of the serie   						   */
/*-------------------------------------------------------------------------*/
string RawPackageInfo::getSerieOfPackage( const string packageName_tr )
{
  PkdData cpkg_pCi = getPackageByName( packageName_tr );

  if ( !cpkg_pCi ) {
    y2warning( "unknown package '%s'", packageName_tr.c_str() );
    return "";
  }

  return cpkg_pCi->series();
}


/*--------------------------------------------------------------*/
/* Get a list of all series					*/
/* return:[<serie1>,<serie2>,<serie3>,... ]			*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getAllSeries( )
{
  return seriesNames_Vt;
}


/*--------------------------------------------------------------*/
/* Get a list of all rpmgroups					*/
/* return:[<rpmgroup1>,<rpmgroup2>,<rpmgroup>,... ]		*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getAllRpmgroup( )
{
   return groupNames_Vt;
}


/*--------------------------------------------------------------*/
/* Get a list of all source-packages				*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getSourcePackages( )
{
  PackList ret_VCi;

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->type() == Package::SPM ) {
      ret_VCi.insert( packages_VpC[i]->name() );
    }
  }

  return ret_VCi;
}

/*--------------------------------------------------------------*/
/* Get a list of packages which belong to the serie		*/
/* input: name of the serie					*/
/* return:[<package1>,<package2>,<package3>,... ]		*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getRawPackageListOfSerie( const string serie_tr )
{
  PackList ret_VCi;

  if ( seriesNames_Vt.find( serie_tr ) == seriesNames_Vt.end() ) {
    y2warning( "unknown series '%s'", serie_tr.c_str() );
    return ret_VCi;
  }

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->series() == serie_tr ) {
      ret_VCi.insert( packages_VpC[i]->name() );
    }
  }

  if ( ret_VCi.empty() ) {
    y2internal( "empyt series '%s' should not happen", serie_tr.c_str() );
  }

  return ret_VCi;
}


/*--------------------------------------------------------------*/
/* Get a list of packages which belong to the rpmgroup		*/
/* input: name of the rpmgroup					*/
/* return:[<package1>,<package2>,<package3>,... ]		*/
/*--------------------------------------------------------------*/
PackList RawPackageInfo::getRawPackageListOfRpmgroup( const string rpmgroup_tr )
{
  PackList ret_VCi;

  if ( groupNames_Vt.find( rpmgroup_tr ) == groupNames_Vt.end() ) {
    y2warning( "unknown group '%s'", rpmgroup_tr.c_str() );
    return ret_VCi;
  }

  for ( unsigned i = 0; i < packages_VpC.size(); ++i ) {
    if ( packages_VpC[i]->group() == rpmgroup_tr ) {
      ret_VCi.insert( packages_VpC[i]->name() );
    }
  }

  if ( ret_VCi.empty() ) {
    y2internal( "empyt group '%s' should not happen", rpmgroup_tr.c_str() );
  }

  return ret_VCi;
}

/****************************************************************/
/* protected member-functions					*/
/****************************************************************/
/*-------------------------------------------------------------------*/
/* Return all  tags of every selection which is defined              */
/* in setup/descr/ *.sel. The return-value is a map which contains   */
/* the selection as index and a list of selection-names from	     */
/* which the selection depends 				     	     */
/*-------------------------------------------------------------------*/
PackTagLMap RawPackageInfo::getSelTag( DepType depType )
{
   SelectionGroupMap::iterator pos;
   string::size_type begin;
   string::size_type end;
   const string seperator(" \t");
   PackTagLMap retMap;

   // Building map
   for ( pos = selectionGroupMap.begin();
	 pos != selectionGroupMap.end();
	 ++pos )
   {
      TagList tagList;
      SelectionGroup selectionGroup = (SelectionGroup) pos->second;
      string dependency;

      switch ( depType )
      {
	 case PROV:
	    dependency = selectionGroup.provides;
	    break;
	 case SUGGE:
	    dependency = selectionGroup.suggests;
	    break;
	 default:
	    y2error ( "depType for this function is not correct" );
	    return retMap;
	    break;
      }

      begin = dependency.find_first_not_of ( seperator );
      while ( begin != string::npos )
      {
	 // Each dependency

	 end = dependency.find_first_of ( seperator, begin );

	 if ( end == string::npos )
	 {
	    // end of line
	    end = dependency.length();
	 }

	 tagList.insert( dependency.substr ( begin, end - begin) );

	 // next entry
	 begin = dependency.find_first_not_of ( seperator, end );
      }

      PackageKey packageKey ( pos->first, "" );
      // insert into dependency-map
      retMap.insert(pair<PackageKey,
		    TagList>( packageKey,
			      tagList ));
   }	 // for every selection class

   return ( retMap );
}


/*-------------------------------------------------------------------*/
/* Return all  tags of every selection which is defined              */
/* in setup/descr/ *.sel. The return-value is a map which contains   */
/* the selection as index and a list of selection-names from	     */
/* which the selection depends 				     	     */
/*-------------------------------------------------------------------*/
PackDepLMap RawPackageInfo::getSelDependency( DepType depType )
{
   SelectionGroupMap::iterator pos;
   string::size_type begin;
   string::size_type end;
   const string seperator(" \t");
   string selName;
   PackDepLMap retMap;

   // Building map
   for ( pos = selectionGroupMap.begin();
	 pos != selectionGroupMap.end();
	 ++pos )
   {
      DependList depList;
      SelectionGroup selectionGroup = (SelectionGroup) pos->second;
      Dependence dependence;
      string dependency;

      dependence.name = "";

      switch ( depType )
      {
	 case REQU:
	    dependency = selectionGroup.requires;
	    break;
	 case CONFL:
	    dependency = selectionGroup.conflicts;
	    break;
	 default:
	    y2error ( "depType for this function is not correct" );
	    return retMap;
	    break;
      }

      begin = dependency.find_first_not_of ( seperator );
      while ( begin != string::npos )
      {
	 // Each dependency

	 end = dependency.find_first_of ( seperator, begin );

	 if ( end == string::npos )
	 {
	    // end of line
	    end = dependency.length();
	 }

	 selName =
	    dependency.substr ( begin, end - begin);

	 if ( selName.length() > 0 &&
	      selName[0] != '/' )  // ignore filenames
	 {
	    if ( selName != dependence.name &&
		 !isdigit ( selName[0] ) &&
		 selName[0] != '=' &&
		 selName[0] != '<' &&
		 selName[0] != '>' &&
		 selName[0] != '!' &&
		 dependence.name.length()  > 0
		 )
	    {
	       // insert last entry
	       // entry not exists --> insert
	       depList.push_back( dependence );

	       dependence.name = selName;
	       dependence.compare = NONE;
	       dependence.version = "";
	    }
	    else if ( isdigit ( selName[0] ) )
	    {
	       // Version number
	       dependence.version = selName;
	    }

	    else if ( ( selName.length() == 1 &&
			selName[0] == '=') ||
		      ( selName.length() == 2 &&
			selName[0] == '=' &&
			selName[1] == '=' ) )
	    {
	       dependence.compare = EQ;
	    }
	    else if ( selName.length() == 1 &&
		      selName[0] == '<' )
	    {
	       dependence.compare = LT;
	    }
	    else if ( selName.length() == 1 &&
		      selName[0] == '>' )
	    {
	       dependence.compare = GT;
	    }
	    else if ( selName.length() == 2 &&
		      ( ( selName[0] == '=' &&
			  selName[1] == '<' ) ||
			( selName[0] == '<' &&
			  selName[1] == '=' )
			)
		      )
	    {
	       dependence.compare = LTE;
	    }
	    else if ( selName.length() == 2 &&
		      ( ( selName[0] == '=' &&
			  selName[1] == '>' ) ||
			( selName[0] == '>' &&
			  selName[1] == '=' )
			)
		      )
	    {
	       dependence.compare = GTE;
	    }
	    else
	    {
	       dependence.name = selName;
	    }
	 }    // ignore filenames

	 // next entry
	 begin = dependency.find_first_not_of ( seperator, end );
      }

      // insert last entry
      if ( dependence.name.length()  > 0 )
      {
	 depList.push_back( dependence );
      }

      PackageKey packageKey ( pos->first, "" );
      // insert into dependency-map
      retMap.insert(pair<PackageKey,
		    DependList>( packageKey,
			      depList ));
   }	 // for every package

   return ( retMap );
}

///////////////////////////////////////////////////////////////////
//
// parse
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::initLanguageRecode
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void RawPackageInfo::initLanguageRecode()
{
  if ( iconv_cd != (iconv_t)(-1) ) {
    iconv_close( iconv_cd );
    iconv_cd = (iconv_t)(-1);
  }
  if ( languageRecodeFrom_t == languageRecodeTo_t ) {
    y2milestone( "no need to recode: from == to == '%s'", languageRecodeFrom_t.c_str() );
  } else {
    iconv_cd = iconv_open( languageRecodeTo_t.c_str(), languageRecodeFrom_t.c_str() );
    if ( iconv_cd == (iconv_t)(-1) ) {
      y2error( "cannot init recode (errno %d): '%s' -> '%s'",
	       errno,
	       languageRecodeFrom_t.c_str(), languageRecodeTo_t.c_str() );
    } else {
      y2milestone( "will recode: '%s' -> '%s'",
		   languageRecodeFrom_t.c_str(), languageRecodeTo_t.c_str() );
    }
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::languageRecode
//	METHOD TYPE : string
//
//	DESCRIPTION :
//
string RawPackageInfo::languageRecode( const string & dataIn_tr ) const {
  if ( iconv_cd == (iconv_t)(-1) || !dataIn_tr.size() )
    return dataIn_tr; // no need or can't recode

  size_t inbuf_len  = dataIn_tr.size();
  size_t outbuf_len = inbuf_len * 6 + 1; // worst case
  char * outbuf     = new char[outbuf_len];

#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 2)
  char *       inptr = (char *) dataIn_tr.c_str();
#else
  const char * inptr = dataIn_tr.c_str();
#endif
  char * outptr = outbuf;
  char * l      = NULL;

  size_t iconv_ret = (size_t)(-1);

  do {
    iconv_ret = iconv( iconv_cd, &inptr, &inbuf_len, &outptr, &outbuf_len );

    if ( iconv_ret == (size_t)(-1) ) {

      if ( errno == EILSEQ ) {
        if( l != outptr ) {
          *outptr++ = '?';
	  outbuf_len--;
          l = outptr;
        }
        inptr++;
        continue;
      }
      else if ( errno == EINVAL ) {
        inptr++;
        continue;
        }
      else if ( errno == E2BIG ) {
        if ( !outbuf_len ) {
          y2internal ( "Recode: unexpected small output buffer" );
          break;
        }
      }
    }

  } while( inbuf_len != (size_t)(0) );

  *outptr = '\0';

  string ret_ti( outbuf );
  delete [] outbuf;
  return ret_ti;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::readCommon
//	METHOD TYPE : string
//
//	DESCRIPTION :
//
string RawPackageInfo::readCommon( const FileLocation & at_Cr ) const
{
  if ( inParser_b ) {
    return "<--still parsing-->";
  }

  if ( at_Cr.startData_i >= at_Cr.endData_i )
    return ""; // data was an empty string

  if ( !commonPKD_pF ) {
    // report error
    // rereadErr_C.failed( Pathname::cat( descrPath_t, commonPkd_t ).asString() );
    // return "";

    // Try a different strategy and automaticaly reopen the file. Klaus says keeping
    // the file open is ok. If not, set a flag and close it before return.
    string commonPKD_ti( Pathname::cat( descrPath_t, commonPkd_t ).asString() );
    commonPKD_pF = new ifstream( commonPKD_ti.c_str() );
    y2milestone ( "reopen commonPKD '%s'", commonPKD_ti.c_str() );
  }

  string Ret_ti;
  if ( !TagParser::retrieveData( *commonPKD_pF, at_Cr.startData_i, at_Cr.endData_i, Ret_ti ) ) {
    y2error( "commonPKD data reread failed: (rdstate %d) on '%s'", commonPKD_pF->rdstate(),
	     Pathname::cat( descrPath_t, commonPkd_t ).asString().c_str() );
    return "";
  }
  return Ret_ti;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::readLanguage
//	METHOD TYPE : string
//
//	DESCRIPTION :
//
string RawPackageInfo::readLanguage( const FileLocation & at_Cr ) const
{
  if ( inParser_b ) {
    return "<--still parsing-->";
  }

  if ( at_Cr.startData_i >= at_Cr.endData_i )
    return ""; // data was an empty string

  if ( !languagePKD_pF ) {
    // report error
    // rereadErr_C.failed( Pathname::cat( descrPath_t, parsedLanguage_t+PKD ).asString() );
    // return "";

    // Try a different strategy and automaticaly reopen the file. Klaus says keeping
    // the file open is ok. If not, set a flag and close it before return.
    string languagePKD_ti( Pathname::cat( descrPath_t, parsedLanguage_t+PKD ).asString() );
    languagePKD_pF = new ifstream( languagePKD_ti.c_str() );
    y2milestone ( "reopen languagePKD '%s'", languagePKD_ti.c_str() );
  }

  string Ret_ti;
  if ( !TagParser::retrieveData( *languagePKD_pF, at_Cr.startData_i, at_Cr.endData_i, Ret_ti ) ) {
    y2error( "languagePKD data reread failed: (rdstate %d) on '%s'", languagePKD_pF->rdstate(),
	     Pathname::cat( descrPath_t, parsedLanguage_t+PKD ).asString().c_str() );
    return "";
  }
  return languageRecode( Ret_ti );
}

///////////////////////////////////////////////////////////////////
//
// parseCommonPKD
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::cacheStrore
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
void RawPackageInfo::cacheStrore( NameCache & cache_Vtr, string & data_tr )
{
  if ( data_tr.empty() )
    return; // empty values are ignored

  pair<NameCache::const_iterator,bool> p_Ci = cache_Vtr.insert( data_tr );
  if ( !p_Ci.second ) {
    // entry already exists. Reassignment is just to let data_tr in RawPackage
    // and cache entry share the same stringRep.
    data_tr = *p_Ci.first;
  }
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::checkAndStore
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool RawPackageInfo::checkAndStore( int & error_ir, RawPackage *& cpkg_pCr )
{
  if ( !cpkg_pCr ) {
    error_ir = 0;
    cpkg_pCr = 0;
    return false;
  }

  bool strored_bi = false;

  ///////////////////////////////////////////////////////////////////
  // as we got all package data, do some final computation and adjustments
  ///////////////////////////////////////////////////////////////////
  if ( !error_ir ) {
    // set type according to Filename's extension
    if (    cpkg_pCr->CDFile_t.find( SPMSTRING ) != string::npos
	 || cpkg_pCr->CDFile_t.find( NOSRCSTRING) != string::npos
	 || cpkg_pCr->CDFile_t.find( SHORTSPMSTRING ) != string::npos ) {
      cpkg_pCr->type_e = Package::SPM;
    } else if ( cpkg_pCr->CDFile_t.find( RPMSTRING ) != string::npos ) {
      cpkg_pCr->type_e = Package::RPM;
    } else {
      y2error( "can't derive package type from Filename: '%s'", cpkg_pCr->CDFile_t.c_str() );
      error_ir = 1;
    }

    // oldstyle .pkd: name must be derived from 'Filename: name.(rpm|spm)'
    if ( cpkg_pCr->name_t.empty() ) {
      if ( cpkg_pCr->type_e != Package::NOTYPE ) {
	cpkg_pCr->name_t = cpkg_pCr->CDFile_t.substr( 0, cpkg_pCr->CDFile_t.size()-4 ); // '.[rs]pm'
      } else {
	y2error( "can't derive package name from Filename: '%s'", cpkg_pCr->CDFile_t.c_str() );
	error_ir = 1;
      }
    }
  }

  if ( !error_ir ) {
    // add _spm to source packages name, as we yet can't handle duplicate packagenames ;(
    if ( cpkg_pCr->type_e == Package::SPM ) {
      cpkg_pCr->name_t += "_spm";
    }

    // series may not be empty
    if ( cpkg_pCr->series_t.empty() ) {
      cpkg_pCr->series_t = defaultGroup_tm;
      y2warning( "package '%s' has no series", cpkg_pCr->CDFile_t.c_str() );
    }

    // group may be empty for source packages only.
    if ( cpkg_pCr->type_e != Package::SPM && cpkg_pCr->group_t.empty() ) {
      cpkg_pCr->group_t = defaultGroup_tm;
      y2warning( "package '%s' has no group", cpkg_pCr->CDFile_t.c_str() );
    }
  }

  if ( !error_ir ) {
    ///////////////////////////////////////////////////////////////////
    // chech whether package is duplicate
    ///////////////////////////////////////////////////////////////////
    if ( ! name2package_VpC.insert( PkgLookup::value_type( cpkg_pCr->name_t,
							   cpkg_pCr )
				    ).second ) {
      y2error( "duplicate package: '%s'", DumpToString( *cpkg_pCr ).c_str() );
      y2error( "original package:  '%s'", DumpToString( *package( cpkg_pCr->name_t ) ).c_str() );
      error_ir = 1;
    } else {
      filename2package_VpC.insert( PkgLookup::value_type( cpkg_pCr->CDFile_t, cpkg_pCr ) );
    }
  }

  if ( !error_ir ) {
    ///////////////////////////////////////////////////////////////////
    // insert package
    ///////////////////////////////////////////////////////////////////
    if ( packages_VpC.size() == packages_VpC.capacity() ) {
      packages_VpC.reserve( packages_VpC.capacity() + 512 );
    }
    packages_VpC.push_back( cpkg_pCr );
    cpkg_pCr->instIdx_i = packages_VpC.size();
    strored_bi = true;
    //      y2milestone( "stored: %u/%u \n%s",
    //		   packages_VpC.size(), packages_VpC.capacity(),
    //		   DumpToString( *cpkg_pCr ).c_str() );

    ///////////////////////////////////////////////////////////////////
    // now as we sored it, calculate some stuff on the fly
    ///////////////////////////////////////////////////////////////////
    if ( cpkg_pCr->onCD_i > maxCDNum_i )
      maxCDNum_i = cpkg_pCr->onCD_i;

    // use cpkg_pCr's internal string variable for cacheStrore!
    cacheStrore( seriesNames_Vt, cpkg_pCr->series_t );
    cacheStrore( groupNames_Vt,  cpkg_pCr->group_t );

    ///////////////////////////////////////////////////////////////////
  } else {
    ///////////////////////////////////////////////////////////////////
    // report if we have to discard the package
    ///////////////////////////////////////////////////////////////////
    y2error( "parse error %d: discard '%s'",
	     error_ir,
	     cpkg_pCr->name().c_str() );
    delete cpkg_pCr;
  }

  error_ir     = 0;
  cpkg_pCr = 0;
  return strored_bi;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::storeCommon...
//	METHOD TYPE : int
//
//	DESCRIPTION : assered: data_Vtr not empty and cpkg_pCr not NULL.
//      Return !=0 iff package has to be discarded.
//
inline int RawPackageInfo::storeCommonFilename( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->CDFile_t = data_Cr.data()[0];
  return 0;
}
inline int RawPackageInfo::storeCommonRpmName( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->name_t = data_Cr.data()[0];
  return 0;
}
inline int RawPackageInfo::storeCommonVersion( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->version_t = data_Cr.data()[0];
  return 0;
}
inline int RawPackageInfo::storeCommonProvides( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->provides_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonRequires( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->requires_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonConflicts( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->conflicts_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonObsoletes( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->obsoletes_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonInstPath( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  string w1_ti, w2_ti;
  getfirsttwo( data_Cr.data()[0], w1_ti, w2_ti );
  // InstPath is 'CDNo /PathOnCD/' for common.pkd and 'PathOnCD/Filename.rpm' for patch-description.
  if ( w2_ti.empty() ) {
      // patch-description
      cpkg_pCr->CDPath_t = w1_ti;
  } else {
      // common.pkd
      cpkg_pCr->onCD_i = atoi( w1_ti.c_str() );
      cpkg_pCr->CDPath_t = w2_ti;
      // add the "/" if necessary - but only if common.pkd is read
      if ( cpkg_pCr->CDPath_t.size() && cpkg_pCr->CDPath_t[cpkg_pCr->CDPath_t.size()-1] != '/' ) {
	  cpkg_pCr->CDPath_t += "/";
      }
  }
  // FIXME: use find_first_not_of and a single erase
  while ( cpkg_pCr->CDPath_t.size() && cpkg_pCr->CDPath_t[0] == '/' ) {
      cpkg_pCr->CDPath_t.erase( 0, 1 );
  }
  return 0;
}
inline int RawPackageInfo::storeCommonBuildtime( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->buildTime_i = atoi( data_Cr.data()[0].c_str() );
  return 0;
}
inline int RawPackageInfo::storeCommonBuiltFrom( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->builtFrom_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonRpmGroup( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->group_t = data_Cr.data()[0];
  return 0;
}
inline int RawPackageInfo::storeCommonSeries( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->series_t = data_Cr.data()[0];
  return 0;
}
inline int RawPackageInfo::storeCommonSize( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  string w1_ti, w2_ti;
  getfirsttwo( data_Cr.data()[0], w1_ti, w2_ti );
  cpkg_pCr->uncompSize_i = atoi( w1_ti.c_str() );
  cpkg_pCr->rpmSize_i    = atoi( w2_ti.c_str() );
  return 0;
}
inline int RawPackageInfo::storeCommonFlag( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->isBasepkg_b = ( data_Cr.data()[0].find( BASEPACKAGE ) != string::npos );
  return 0;
}
inline int RawPackageInfo::storeCommonCopyright( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->copyright_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonAuthorName( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->author_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeCommonAuthorEmail( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  // actualy not used. data are included in AuthorName.
  return 0;
}
inline int RawPackageInfo::storeCommonAuthorAddress( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  // actualy not used. data are included in AuthorName.
  return 0;
}
inline int RawPackageInfo::storeCommonStartCommand( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  // actualy not used.
  return 0;
}
inline int RawPackageInfo::storeCommonCategory( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  // actualy not used.
  return 0;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::parseCommonPKD
//	METHOD TYPE : int
//
//	DESCRIPTION : Return 0 on success.
//
int RawPackageInfo::parseCommonPKD()
{
  PathInfo commonPkd_Ci( Pathname::cat( descrPath_t, commonPkd_t ) );

  if ( !commonPkd_Ci.isFile() || !commonPkd_Ci.isR() || !commonPkd_Ci.size() ) {
    y2error( "Can't parseCommonPKD: %s", DumpToString( commonPkd_Ci ).c_str() );
    return 1;
  }

  y2milestone( "going to parseCommonPKD: %s", DumpToString( commonPkd_Ci ).c_str() );
  clearPackages();

  commonPKD_pF = new ifstream( commonPkd_Ci.asString().c_str() );
  int ret_ii = parseCommonPKD( *commonPKD_pF );

  y2milestone( "parseCommonPKD: found %u, discarded %u", numPackages(), ret_ii );

  return ret_ii;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::parseCommonPKD
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int RawPackageInfo::parseCommonPKD( istream & in_Fr )
{
  y2debug( "Parse start (rdstate %d)", in_Fr.rdstate() );
  inParser_b = true;

  TagParser ctag_Ci;
  unsigned  cntPackages_ii = 0;
  unsigned  cntDiscared_ii = 0;

  int          error_ii = 0;
  RawPackage * cpkg_pCi = 0;

  while ( ctag_Ci.lookupTag( in_Fr ) ) {

    CommonTags::Tag tag_ei = commonTags_pCm->parseData( in_Fr, ctag_Ci );

    if ( !ctag_Ci.dataLines() ) {
      y2error( "error on parsing data for %s", DumpToString( ctag_Ci ).c_str() );
      continue;
    }

    if ( !cpkg_pCi ) {
      // still in preamble?
      // Make shure we don't advance unless CommonTags::Filename found, which
      // creates the first cpkg_pCi
      if ( tag_ei == CommonTags::Max_Tag ) {
	// cleanup: use PkdPreambleTags
	if (        ctag_Ci.startTag() == "PkdVersion" ) {
	  if ( ctag_Ci.data()[0].size() && ctag_Ci.data()[0] != "1.1" ) {
	    y2warning( "parseCommonPKD: parser version 1.1 / file version %s",
		       ctag_Ci.data()[0].c_str() );
	  }
	} else if ( ctag_Ci.startTag() == "Encoding" ) {
	  // nop for common.pkd
	} else {
	  y2error( "unknown tag in preamble - %s", DumpToString( ctag_Ci ).c_str() );
	}
	continue;
      } else if ( tag_ei != CommonTags::Filename ) {
	y2error( "data tag in preamble - %s", DumpToString( ctag_Ci ).c_str() );
	continue;
      }
    }

#define STAG(s) case CommonTags::s: error_ii |= storeCommon ## s( ctag_Ci, cpkg_pCi ); break
    switch ( tag_ei ) {

    case CommonTags::Filename:
      if ( cpkg_pCi ) {
	++(checkAndStore( error_ii, cpkg_pCi ) ? cntPackages_ii : cntDiscared_ii);
      }
      cpkg_pCi = new RawPackage( *this );
      error_ii |= storeCommonFilename( ctag_Ci, cpkg_pCi );
      break;

    STAG( RpmName );
    STAG( Version );
    STAG( Provides );
    STAG( Requires );
    STAG( Conflicts );
    STAG( Obsoletes );
    STAG( InstPath );
    STAG( Buildtime );
    STAG( BuiltFrom );
    STAG( RpmGroup );
    STAG( Series );
    STAG( Size );
    STAG( Flag );
    STAG( Copyright );
    STAG( AuthorName );
    STAG( AuthorEmail );
    STAG( AuthorAddress );
    STAG( StartCommand );
    STAG( Category );

    case CommonTags::DepAND:
    case CommonTags::DepOR:
    case CommonTags::DepExcl:
      // YaST (1) stuff ignored
      break;

    case CommonTags::Max_Tag:
      y2error( "unknown tag - %s", DumpToString( ctag_Ci ).c_str() );
      break;

    default:
      y2internal( "unhandled tag - %s", DumpToString( ctag_Ci ).c_str() );
      break;
    }
  }
#undef STAG

  if ( cpkg_pCi ) {
    // handle last package
    (checkAndStore( error_ii, cpkg_pCi ) ? cntPackages_ii : cntDiscared_ii)++;
  }

  y2debug( "Parse done (rdstate %d) stored %u discarded %u", in_Fr.rdstate(), cntPackages_ii, cntDiscared_ii );
  inParser_b = false;

  return cntDiscared_ii;
}

///////////////////////////////////////////////////////////////////
//
// parseLanguagePKD
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::storeLanguage...
//	METHOD TYPE : int
//
//	DESCRIPTION : assered: data_Vtr not empty and cpkg_pCr not NULL
//      except for storeLanguageFilename, which looks for an existing RawPackage.
//      Return !=0 iff package has to be skipped.
//
inline int RawPackageInfo::storeLanguageFilename( const TagParser & data_Cr, RawPackage *& cpkg_pCr ) {
  // cast away const while parsing
  cpkg_pCr = const_cast<RawPackage *>( filename( data_Cr.data()[0] ) );
  if ( !cpkg_pCr ) {
    y2warning( "superfluous entry for Filename: '%s'", data_Cr.data()[0].c_str() );
    return 1;
  }
  return 0;
}
inline int RawPackageInfo::storeLanguageLabel( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->label_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeLanguageNotify( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->instNotify_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeLanguageDelnotify( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->delNotify_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}
inline int RawPackageInfo::storeLanguageDescription( const TagParser & data_Cr, RawPackage * cpkg_pCr ) {
  cpkg_pCr->description_t = FileLocation( data_Cr.posDataStart(), data_Cr.posDataEnd() );
  return 0;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::parseLanguagePKD
//	METHOD TYPE : int
//
//	DESCRIPTION : Return 0 on success.
//
int RawPackageInfo::parseLanguagePKD()
{
  parsedLanguage_t = selectedLanguage_t;
  PathInfo languagePkd_Ci( Pathname::cat( descrPath_t, parsedLanguage_t+PKD ) );

  if ( !languagePkd_Ci.isFile() ) {
    // 1st check whether there's a file without countycode, otherwise use fallback
    parsedLanguage_t = stripISOCountry( parsedLanguage_t );

    if ( parsedLanguage_t.size() ) {
      y2milestone( "try alternate '%s' because %s",
		   parsedLanguage_t.c_str(), DumpToString( languagePkd_Ci ).c_str() );
      languagePkd_Ci( Pathname::cat( descrPath_t, parsedLanguage_t+PKD ) );

      if ( !languagePkd_Ci.isFile() ) {
	parsedLanguage_t = languageFallback_tm;
	y2milestone( "try fallback '%s' because %s",
		     parsedLanguage_t.c_str(), DumpToString( languagePkd_Ci ).c_str() );
	languagePkd_Ci( Pathname::cat( descrPath_t, parsedLanguage_t+PKD ) );
      }
    } else {
      parsedLanguage_t = languageFallback_tm;
      y2milestone( "try fallback '%s' because %s",
		   parsedLanguage_t.c_str(), DumpToString( languagePkd_Ci ).c_str() );
      languagePkd_Ci( Pathname::cat( descrPath_t, parsedLanguage_t+PKD ) );
    }
  }

  if ( !languagePkd_Ci.isFile() || !languagePkd_Ci.isR() || !languagePkd_Ci.size() ) {
    y2error( "can't parseLanguagePKD: %s", DumpToString( languagePkd_Ci ).c_str() );
    return 1;
  }

  y2milestone( "going to parseLanguagePKD: %s", DumpToString( languagePkd_Ci ).c_str() );

  // preset recoding data, might be overwritten in preamble
  string lang_ti( parsedLanguage_t.substr( 0, 2 ) );
  if (    lang_ti == "pl"
       || lang_ti == "cs"
       || lang_ti == "sk"
       || lang_ti == "hu" ) {
    languageRecodeFrom_t = "ISO-8859-2";
  } else {
    languageRecodeFrom_t = "ISO-8859-1";
  }
  languageRecodeTo_t = "UTF-8"; // !!! still fix

  delete languagePKD_pF;
  languagePKD_pF = new ifstream( languagePkd_Ci.asString().c_str() );
  int ret_ii = parseLanguagePKD( *languagePKD_pF );

  y2milestone( "parseLanguagePKD: missing %d", ret_ii );

  return ret_ii;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::parseLanguagePKD
//	METHOD TYPE : int
//
//	DESCRIPTION :
//
int RawPackageInfo::parseLanguagePKD( istream & in_Fr )
{
  y2debug( "Parse start (rdstate %d)", in_Fr.rdstate() );
  inParser_b = true;

  TagParser ctag_Ci;
  unsigned  cntPackages_ii = 0;

  int          error_ii = 0;
  RawPackage * cpkg_pCi = 0;

  bool firstFilename_bi = true;

  while ( ctag_Ci.lookupTag( in_Fr ) ) {

    LanguageTags::Tag tag_ei = languageTags_pCm->parseData( in_Fr, ctag_Ci );

    if ( !ctag_Ci.dataLines() ) {
      y2error( "error on parsing data for %s", DumpToString( ctag_Ci ).c_str() );
      continue;
    }

    if ( !cpkg_pCi ) {
      if ( !error_ii ) {
	// still in preamble?
	// Make shure we don't advance unless LanguageTags::Filename found, which
	// creates the first cpkg_pCi
	if ( tag_ei == LanguageTags::Max_Tag ) {
	  // cleanup: use PkdPreambleTags
	  if (        ctag_Ci.startTag() == "PkdVersion" ) {
	    if ( ctag_Ci.data()[0].size() && ctag_Ci.data()[0] != "1.1" ) {
	      y2warning( "parseLanguagePKD: parser version 1.1 / file version %s",
			 ctag_Ci.data()[0].c_str() );
	    }
	  } else if ( ctag_Ci.startTag() == "Encoding" ) {
	    if ( ctag_Ci.data()[0].size() ) {
	      languageRecodeFrom_t = ctag_Ci.data()[0];
	      y2milestone( "parseLanguagePKD: encoding is '%s'", languageRecodeFrom_t.c_str() );
	    }
	  } else {
	    y2error( "unknown tag in preamble - %s", DumpToString( ctag_Ci ).c_str() );
	  }
	  continue;
	} else if ( tag_ei != LanguageTags::Filename ) {
	  y2error( "data tag in preamble - %s", DumpToString( ctag_Ci ).c_str() );
	  continue;
	}
      } else {
	// illegal entry: skip to next LanguageTags::Filename
	if ( tag_ei != LanguageTags::Filename )
	  continue;
      }
    }

    if ( firstFilename_bi ) {
      firstFilename_bi = false;
      initLanguageRecode();
    }

#define STAG(s) case LanguageTags::s: error_ii |= storeLanguage ## s( ctag_Ci, cpkg_pCi ); break
    switch ( tag_ei ) {

    case LanguageTags::Filename:
      error_ii = storeLanguageFilename( ctag_Ci, cpkg_pCi );
      if ( !error_ii )
	++cntPackages_ii;
      break;

    STAG( Label );
    STAG( Notify );
    STAG( Delnotify );
    STAG( Description );

    case LanguageTags::Max_Tag:
      y2error( "unknown tag - %s", DumpToString( ctag_Ci ).c_str() );
      break;
    default:
      y2internal( "unhandled tag - %s", DumpToString( ctag_Ci ).c_str() );
      break;
    }
  }
#undef STAG

  y2debug( "Parse done (rdstate %d) stored %u of total %u", in_Fr.rdstate(), cntPackages_ii, numPackages() );
  inParser_b = false;

  return numPackages() - cntPackages_ii;
}

///////////////////////////////////////////////////////////////////
//
// parse Selections
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackageInfo::parseSelections
//	METHOD TYPE : int
//
//	DESCRIPTION : Return 0 on success.
//
int RawPackageInfo::parseSelections()
{
  return getSelectionGroupMap( selectionGroupMap ) ? 0 : 1;
}

/*----------------------------------------------------------------------*/
/*  Parse the descr-directory for *.sel files and returns a map		*/
/*  of these groups.							*/
/*----------------------------------------------------------------------*/
bool RawPackageInfo::getSelectionGroupMap ( SelectionGroupMap &groupMap )
{
    DIR *dir = opendir(descrPath_t.c_str());

    if (!dir)
    {
       y2error( "Can't open directory %s for reading",
		descrPath_t.c_str());
	return false;
    }

    groupMap.clear();

    struct dirent *entry;
    while ((entry = readdir(dir)))
    {
	PackList dummyPackage;
	string groupDescription;
	string groupName;
	SelectionGroup selectionGroup;
	string kind;
	bool visible;
	string requires, conflicts, suggests, provides;
	string version, architecture, longDescription,
	   size, notify, delNotify;

	if (!strcmp(entry->d_name, ".") ||
	    !(strcmp(entry->d_name, ".."))) continue;

	// Read only *.sel files.
	if (strcmp(entry->d_name + strlen(entry->d_name) - 4, SEL)) continue;

	*(entry->d_name + strlen(entry->d_name) - 4) = 0;
	groupName = entry->d_name;
 	getSelectionGroupList ( groupName,
				dummyPackage,
				groupDescription,
				kind,
				visible,
				requires, conflicts, suggests, provides,
				version, architecture, longDescription,
				size, notify, delNotify);
	selectionGroup.description = groupDescription;
	selectionGroup.kind = kind;
	selectionGroup.visible = visible;
	selectionGroup.requires = requires;
	selectionGroup.conflicts = conflicts;
	selectionGroup.suggests = suggests;
	selectionGroup.provides = provides;
	selectionGroup.version = version;
	selectionGroup.architecture = architecture;
	selectionGroup.longDescription = longDescription;
	selectionGroup.size = size;
	selectionGroup.notify = notify;
	selectionGroup.delNotify = delNotify;
        groupMap.insert(pair<const string, const SelectionGroup>
			( groupName, selectionGroup ) );
    }
    if ( dir )
    {
      closedir(dir);
    }

    return true;
}

/*--------------------------------------------------------------*/
/* Returns list of all possible selections.			*/
/* [<selection1,version1>,<selection2,version2>,,... ]		*/
/*--------------------------------------------------------------*/
PackVersList RawPackageInfo::getSelectionList( )
{
   SelectionGroupMap::iterator pos;
   PackVersList selList;

   for ( pos = selectionGroupMap.begin();
	 pos != selectionGroupMap.end(); pos++ )
   {
      PackageKey selKey ( pos->first, "" );
      selList.push_back( selKey );
   }

   return ( selList );
}


/*--------------------------------------------------------------*/
/* Extract package names from a string				*/
/* return:[<package1>,<package2>,<package3>,... ]		*/
/*--------------------------------------------------------------*/
void RawPackageInfo::extractPackages ( string packageString,
				       PackList &destPackList )
{
   PackList packList;
   string packageName = "";
   string replacePackage = "";
   string::size_type begin;
   string::size_type end;
   const string 	seperator( ", \t)" );

   begin = packageString.find_first_of( "(");

   if ( begin != string::npos )
   {
      end = packageString.find_first_of( ")" );
      string searchString = packageString.substr( begin+1, end-begin );

      begin = searchString.find_first_not_of ( seperator );
      while ( begin != string::npos )
      {
	 // each package
	 end = searchString.find_first_of ( seperator, begin );
	 if ( end == string::npos )
	 {
	    // end of line
	    end = searchString.length();
	 }

	 string package = searchString.substr ( begin, end-begin );

	 begin = searchString.find_first_not_of ( seperator, end );
	 packList.insert ( package );
      }
   }

   // Extract original packagename
   end = packageString.find_first_of ( seperator );
   if ( end != string::npos )
   {
      packageName = packageString.substr( 0, end );
   }
   else
   {
      packageName = packageString;
   }

   string::size_type posRep = packageName.find_first_of ( ":" );
   if ( posRep != string::npos )
   {
      // replace package from another selection
      replacePackage = packageName.substr( 0, posRep );
      if ( posRep < packageName.length() -1 )
      {
	 string dummy = packageName.substr ( posRep+1 );
	 packageName = dummy;
      }
      else
      {
	 packageName = "";
      }
   }

   if ( getPackageByName( packageName ) )
   {
      // package exists
      // insert into return package-list
      if ( replacePackage.length() > 0 )
      {
	 if ( getPackageByName( replacePackage ) )
	 {
	    // replace package found
	    destPackList.insert ( replacePackage + ":" + packageName );
	 }
	 else
	 {
	    // replace package not found
	    y2milestone ( "replace package %s not found. -> only package %s found",
			  replacePackage.c_str(), packageName.c_str() );
	    destPackList.insert ( packageName );
	 }
      }
      else
      {
	 // "normal" entry
	 destPackList.insert ( packageName );
      }
   }
   else
   {
      bool found = false;
      PackList::iterator packPos;

      packPos = packList.begin();
      while ( !found &&
	      packPos != packList.end() )
      {
	 if ( getPackageByName( *packPos ) )
	 {
	    y2milestone( "package: %s not found in"
		     " general list. -> try fallbacks",
		     packageName.c_str());
	    y2milestone ( " taking %s instead", (*packPos).c_str() );
	    destPackList.insert ( *packPos );
	    found = true;
	 }
	 else
	 {
	    packPos++;
	 }
      }
   }
}


/*------------------------------------------------------------------------------*/
/* Parse the file <groupName>.sel and return a List of packages which 		*/
/* belongs to this group.							*/
/* input: groupName								*/
/* output: 	packageList							*/
/* 		groupDescription - short description				*/
/*		kind of the sel-Group ( like addon, base... )			*/
/*		show this selection groups					*/
/*		other required selection groups ( the user cannot deselect	*/
/*						  these groups )		*/
/*		confiction selection groups					*/
/*		other required selection groups ( the user can deselect		*/
/*						  these groups )		*/
/*------------------------------------------------------------------------------*/
bool RawPackageInfo::getSelectionGroupList ( const string groupName,
				PackList &packageList,
				string &groupDescription,
				string &kind,
				bool &visible,
				string &requires,
 			        string &conflicts,
				string &suggests,
				string &provides,
				string &version,
				string &architecture,
				string &longDescription,
				string &size,
				string &notify,
				string &delNotify )
{
   char buffer[BUFFERLEN +1];
   FILE *file = NULL;
   bool ok = true;
   string filename = descrPath_t + "/" + groupName + SEL;
   PackList languagePackageList;
   PackList toInstallPackageList;
   bool languagePackagesFound = false;
   bool shortlanguagePackagesFound = false;
   bool descriptionFound = false;


   kind = "";
   longDescription = "";
   visible = true;
   groupDescription = "";
   notify = "";
   delNotify = "";
   packageList.clear();
   languagePackageList.clear();
   toInstallPackageList.clear();

   file = fopen (filename.c_str(), "r");
   if ( !file )
   {
      y2error( "Can't open %s", filename.c_str());

      return ( false );
   }

   if ( ok )
   {
      // parsing file
      buffer[0] = 0;
      while ( !feof(file )  )
      {
	 // reading a line

	 if ( fgets ( buffer, READLEN, file ) == NULL )
	 {
	    break;
	 }
	 // eliminate \n
	 char *posn = strchr(buffer,'\n');
	 if ( posn != NULL ) *posn = 0;

	 if ( buffer[0] == 0 ||
	      buffer[0] == '#'  )
	 {
	    // scipping comments and empty lines
	 }
	 else
	 {
	    if ( strncmp ( buffer, LABEL,
			   strlen(LABEL)) == 0 ||
		 strncmp ( buffer, UTF8DESCRIPTION,
			   strlen(UTF8DESCRIPTION)) == 0 )
	    {
	       // Read Description
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  char ident[IDENTLENG];

		  sprintf ( ident, "%s:", LABEL );
		  if ( groupDescription.length() == 0 &&
		       strncmp ( buffer, ident, strlen(ident)) == 0 )
		  {
		     // new defaultentry
		     groupDescription = (pos+1);
		  }
		  sprintf ( ident, "%s.default:", UTF8DESCRIPTION );
		  if ( groupDescription.length() == 0 &&
		       strncmp ( buffer, ident, strlen(ident)) == 0 )
		  {
		     // old defaultentry
		     groupDescription = (pos+1);
		  }

		  sprintf ( ident, "%s.%s", LABEL, selectedLanguage_t.c_str() );
		  if ( selectedLanguage_t.length() > 0 &&
		       strncmp ( buffer, ident, strlen(ident)) == 0 )
		  {
		     groupDescription = (pos+1);
		     descriptionFound = true;
		  }

		  if ( ( groupDescription.length() == 0 ||
			 !descriptionFound  ) &&
		       selectedLanguage_t.length() >= 2 )
		  {
		     // trying cutted language like de, en ....
		     string shortlang;

		     shortlang.assign ( selectedLanguage_t, 0 , 2);
		     sprintf ( ident, "%s.%s", LABEL,
			       shortlang.c_str() );
		     if ( strncmp ( buffer, ident, strlen(ident)) == 0 )
		     {
			groupDescription = (pos+1);
		     }
		  }
	       }
	    }
	    else if ( strncmp ( buffer, VERSION,
				strlen(VERSION)) == 0 )
	    {
	       // Read VERSION of sel-group
	       version = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     version = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, SIZE,
				strlen(SIZE)) == 0 )
	    {
	       // Read SIZE of sel-group
	       size = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     size = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, ARCHITECTURE,
				strlen(ARCHITECTURE)) == 0 )
	    {
	       // Read ARCHITECTURE of sel-group
	       architecture = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     architecture = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, REQUIRES,
				strlen(REQUIRES)) == 0 )
	    {
	       // Read REQUIRES of sel-group
	       requires = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     requires = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, CONFLICTS,
				strlen(CONFLICTS)) == 0 )
	    {
	       // Read CONFLICTS of sel-group
	       conflicts = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     conflicts = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, PROVIDES,
				strlen(PROVIDES)) == 0 )
	    {
	       // Read PROVIDES of sel-group
	       provides = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     provides = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, SUGGESTS,
				strlen(SUGGESTS)) == 0 )
	    {
	       // Read SUGGESTS of sel-group
	       suggests = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     suggests = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, CATEGORY,
				strlen(CATEGORY)) == 0 ||
		      strncmp ( buffer, KIND,
				strlen(KIND)) == 0 )
	    {
	       // Read Category of sel-group
	       kind = "";
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
								   " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     kind = dummy.substr( posNonSpace );
		  }
	       }
	    }
	    else if ( strncmp ( buffer, VISIBLE,
				strlen(VISIBLE)) == 0 )
	    {
	       // Read Visible of sel-group
	       char *pos = strchr ( buffer, ':' );
	       if ( pos != NULL )
	       {
		  string dummy = pos+1;
		  string::size_type posNonSpace = dummy.find_first_not_of (
						      " \t" );
		  if ( posNonSpace != string::npos )
		  {
		     if ( dummy.substr( posNonSpace ) == FALSE )
		     {
			visible = false;
		     }
		  }
	       }
	    }
	    else if ( strncmp ( buffer, DESCRIPTION,
				strlen(DESCRIPTION)) == 0 )
	    {
	       // Reading description
	       char ident[IDENTLENG];
	       char identLanguage[IDENTLENG];

	       sprintf ( ident, "%s:", DESCRIPTION );
	       sprintf ( identLanguage, "%s.%s", DESCRIPTION,
			 selectedLanguage_t.c_str() );

	       if ( longDescription.length() == 0 &&
		    strncmp ( buffer, ident, strlen(ident)) == 0 // default entry
		    ||
		    selectedLanguage_t.length() > 0 &&
		    strncmp ( buffer, identLanguage, strlen (identLanguage)) == 0 )
	       {
		  longDescription = "";
		  while ( !feof(file) &&
			  strncmp ( buffer, NOITPIRCSED,
				    strlen(NOITPIRCSED)) != 0 )
		  {
		     if ( fgets ( buffer, READLEN, file ) != NULL &&
			  strncmp ( buffer, NOITPIRCSED,
				       strlen(NOITPIRCSED)) != 0 )
		     {
			longDescription += buffer;
		     }
		  }
	       }
	    }
	    else if ( strncmp ( buffer, NOTIFY,
				strlen(NOTIFY)) == 0 )
	    {
	       // Reading notify
	       char ident[IDENTLENG];
	       char identLanguage[IDENTLENG];

	       sprintf ( ident, "%s:", NOTIFY );
	       sprintf ( identLanguage, "%s.%s", NOTIFY,
			 selectedLanguage_t.c_str() );

	       if ( notify.length() == 0 &&
		    strncmp ( buffer, ident, strlen(ident)) == 0 // default entry
		    ||
		    selectedLanguage_t.length() > 0 &&
		    strncmp ( buffer, identLanguage, strlen (identLanguage)) == 0 )
	       {
		  notify = "";
		  while ( !feof(file) &&
			  strncmp ( buffer, YFITON,
				    strlen(YFITON)) != 0 )
		  {
		     if ( fgets ( buffer, READLEN, file ) != NULL &&
			  strncmp ( buffer, YFITON,
				       strlen(YFITON)) != 0 )
		     {
			notify += buffer;
		     }
		  }
	       }
	    }
	    else if ( strncmp ( buffer, DELNOTIFY,
				strlen(DELNOTIFY)) == 0 )
	    {
	       // Reading delnotify
	       char ident[IDENTLENG];
	       char identLanguage[IDENTLENG];

	       sprintf ( ident, "%s:", DELNOTIFY );
	       sprintf ( identLanguage, "%s.%s", DELNOTIFY,
			 selectedLanguage_t.c_str() );

	       if ( delNotify.length() == 0 &&
		    strncmp ( buffer, ident, strlen(ident)) == 0 // default entry
		    ||
		    selectedLanguage_t.length() > 0 &&
		    strncmp ( buffer, identLanguage, strlen (identLanguage)) == 0 )
	       {
		  delNotify = "";
		  while ( !feof(file) &&
			  strncmp ( buffer, YFITONLED,
				    strlen(YFITONLED)) != 0 )
		  {
		     if ( fgets ( buffer, READLEN, file ) != NULL &&
			  strncmp ( buffer, YFITONLED,
				       strlen(YFITONLED)) != 0 )
		     {
			delNotify += buffer;
		     }
		  }
	       }
	    }
	    else if ( strncmp ( buffer, TOINSTALL,
				strlen(TOINSTALL)) == 0 )
	    {
	       // Read to install packages
	       char *pos = strchr ( buffer, '.' );
	       if ( pos != NULL )
	       {
		  pos++;
		  pos[ strlen ( pos ) -1 ]= 0; // without :

		  bool readList = false;
		  // language-specific packages
		  if ( strcmp ( pos, selectedLanguage_t.c_str() ) == 0 )
		  {
		     // entry for the language found
		     languagePackagesFound = true;
		     readList = true;
		  }

		  if ( !languagePackagesFound &&
		       selectedLanguage_t.length() >= 2 )
		  {
		     // checking entry for short language
		     string shortlang;

		     shortlang.assign ( selectedLanguage_t, 0 , 2);
		     if ( strcmp ( pos, shortlang.c_str() ) == 0 )
		     {
			// entry for the short language found
			shortlanguagePackagesFound = true;
			readList = true;
		     }
		  }

		  if ( !languagePackagesFound &&
		       !shortlanguagePackagesFound &&
		       strncmp ( pos, DEFAULT, strlen(DEFAULT)) == 0 )
		  {
		      // set default
		      readList = true;
		  }

		  if ( readList )
		  {
		     languagePackageList.clear();
		     while ( !feof(file) &&
			     strncmp ( buffer, LLATSNIOT,
				 strlen(LLATSNIOT)) != 0 )
		     {
			if ( fgets ( buffer, READLEN, file ) != NULL &&
			     strncmp ( buffer, LLATSNIOT,
				       strlen(LLATSNIOT)) != 0 )
			{
			   char *pos = strchr(buffer,'\n');
			   if ( pos != NULL )
			      *pos = 0;
			   languagePackageList.insert(buffer);
			}
		     }
		  }
	       }
	       else
	       {
		  // general package list ( not depend on language )
		  while ( !feof(file) &&
			  strncmp ( buffer, LLATSNIOT,
				    strlen(LLATSNIOT)) != 0 )
		  {
		     if ( fgets ( buffer, READLEN, file ) != NULL &&
			  strncmp ( buffer, LLATSNIOT,
				    strlen(LLATSNIOT)) != 0 )
		     {
			char *pos = strchr(buffer,'\n');
			if ( pos != NULL )
			   *pos = 0;
			toInstallPackageList.insert(buffer);
		     }
		  }
	       }
	    }
	    else
	    {
	       // everything else
	    }
	 }
      }
   }

   if ( file )
   {
      fclose ( file );
   }

   PackList::iterator posPackageList;

   // List of language-packages
   for ( posPackageList = languagePackageList.begin();
         posPackageList != languagePackageList.end();
	 ++posPackageList )
   {
      string packageName = *posPackageList;
      extractPackages ( packageName, packageList);
   }

   // List of general-packages
   for ( posPackageList = toInstallPackageList.begin();
         posPackageList != toInstallPackageList.end();
	 ++posPackageList )
   {
      string packageName = *posPackageList;
      extractPackages ( packageName, packageList);
   }

   return ( ok );
}

///////////////////////////////////////////////////////////////////
//
// parse Du info
//
///////////////////////////////////////////////////////////////////

/*-----------------------------------------------------------------------*/
/* Read the installation-sizes for ALL packages				 */
/* Return a map like:							 */
/* [packageA, [[/,2kByte],[/etc,7kByte],[/var,1kByte]]]			 */
/* [pacakgeB, ........					       ]	 */
/* Input: Partitionlist like:						 */
/* [/,/etc,/var]							 */
/*-----------------------------------------------------------------------*/
void  RawPackageInfo::getAllRawPackageInstallationSizes(
				      PackageSizeMap& packageSizeMap,
				      const PartitionList& partitionList )
{
   PackagePartitionSizeMap dummyPackagePartitionSizeMap,emptyPackagePartitionSizeMap;
   PartitionList::iterator posList;

   emptyPackagePartitionSizeMap.clear();

   // generate the size-map
   for ( posList = partitionList.begin(); posList != partitionList.end();
	 ++posList )
   {
      dummyPackagePartitionSizeMap.insert(pair<const string, int >(*posList,0));
   }

   // Check, if "/" exists in the partitionList
   posList = partitionList.find("/" );
   if ( posList == partitionList.end() )
   {
      // not found --> insert
      dummyPackagePartitionSizeMap.insert(pair<const string, int >("/",0));
   }

   // evaluate partition-size for each package by parsing the
   // rawPackageList

   for ( unsigned i = 0; i < numPackages(); ++i )
   {
      // each package
      RawPkg cpkg_pCi = packages_VpC[i];
      PackagePartitionSizeMap::iterator posSizeMap1,posSizeMap2;

      // initialize size
      for ( posSizeMap1 = dummyPackagePartitionSizeMap.begin();
	    posSizeMap1 != dummyPackagePartitionSizeMap.end();
	    ++posSizeMap1 )
      {
	 posSizeMap1->second = 0;
      }

      if ( !cpkg_pCi->partitionSizeMap().empty() )
      {
	 // calculate the required size for each partition
	 for ( posSizeMap1 = cpkg_pCi->partitionSizeMap().begin();
	       posSizeMap1 != cpkg_pCi->partitionSizeMap().end();
	       ++posSizeMap1 )
	 {
	    string maskString = posSizeMap1->first;
	    bool entryFound = false;
	    for ( posSizeMap2 = dummyPackagePartitionSizeMap.begin();
		  posSizeMap2 != dummyPackagePartitionSizeMap.end();
		  posSizeMap2++ )
	    {
	       // each partition in the rawPackageList
	       string compareString = posSizeMap2->first;
#if defined __GNUC__ && __GNUC__ >= 3
	       if (maskString.compare (0, compareString.length (), compareString) == 0)
#else
	       if (maskString.compare (compareString, 0, compareString.length ()) == 0)
#endif
	       {
		  // Entry found
		  posSizeMap2->second += (int) posSizeMap1->second;
		  entryFound = true;
	       }
	    }
	    if ( !entryFound )
	    {
	       // belonging to root "/"
	       posSizeMap2 = dummyPackagePartitionSizeMap.find("/");
	       if ( posSizeMap2 != dummyPackagePartitionSizeMap.end() )
	       {
		  posSizeMap2->second += (int) posSizeMap1->second ;
	       }
	    }
	 }
	 // Insert into the packageSizeMap
	 packageSizeMap.insert(pair<const string,PackagePartitionSizeMap>
			       (cpkg_pCi->name(),dummyPackagePartitionSizeMap));
      }
      else
      {
	 // Insert empty PackagePartitionSizeMap into the packageSizeMap
	 packageSizeMap.insert(pair<const string,PackagePartitionSizeMap>
			       (cpkg_pCi->name(),emptyPackagePartitionSizeMap));
      }
   } // for every Package
}


/*-----------------------------------------------------------------------*/
/* Read the installation-size for ONE packet				 */
/* Return a map like:							 */
/* [[/,2kByte],[/etc,7kByte],[/var,1kByte]]				 */
/* Input: Partitionlist like [/,etc,var]				 */
/*        packageName							 */
/*-----------------------------------------------------------------------*/
void  RawPackageInfo::getRawPackageInstallationSize(
				      PackagePartitionSizeMap& packagePartitionSizeMap,
				      const string packageName,
				      const PartitionList& partitionList)
{
   PartitionList::iterator posList;

   packagePartitionSizeMap.clear();

   // generate the size-map
   for ( posList = partitionList.begin(); posList != partitionList.end();
	 ++posList )
   {
      packagePartitionSizeMap.insert(pair<const string, int >(*posList,0));
   }

   // Check, if "/" exists in the partitionList
   posList = partitionList.find("/" );
   if ( posList == partitionList.end() )
   {
      // not found --> insert
      packagePartitionSizeMap.insert(pair<const string, int >("/",0));
   }

   // evaluate partition-size for the package packageName in the
   // rawPackageList

   RawPkg cpkg_pCi = package( packageName );

   if ( cpkg_pCi )
   {
      // package found
      PackagePartitionSizeMap::iterator posSizeMap1,posSizeMap2;

      if ( !cpkg_pCi->partitionSizeMap().empty() )
      {
	 // calculate the required size for each partition
	 for ( posSizeMap1 = cpkg_pCi->partitionSizeMap().begin();
	       posSizeMap1 != cpkg_pCi->partitionSizeMap().end();
	       ++posSizeMap1 )
	 {
	    string maskString = posSizeMap1->first;
	    bool entryFound = false;
	    for ( posSizeMap2 = packagePartitionSizeMap.begin();
		  posSizeMap2 != packagePartitionSizeMap.end();
		  posSizeMap2++ )
	    {
	       // each partition in the rawPackageList
	       string compareString = posSizeMap2->first;
#if defined __GNUC__ && __GNUC__ >= 3
	       if (maskString.compare (0, compareString.length (), compareString) == 0)
#else
	       if (maskString.compare (compareString, 0, compareString.length ()) == 0)
#endif
	       {
		  // Entry found
		  posSizeMap2->second += (int) posSizeMap1->second;
		  entryFound = true;
	       }
	    }
	    if ( !entryFound )
	    {
	       // belonging to root "/"
	       posSizeMap2 = packagePartitionSizeMap.find("/");
	       if ( posSizeMap2 != cpkg_pCi->partitionSizeMap().end() )
	       {
		  posSizeMap2->second += (int) posSizeMap1->second ;
	       }
	    }
	 } //for
      } // empty Map
   } // if Package found
   else
   {
      y2error( "package '%s' not found", packageName.c_str());
   }
}


/*------------------------------------------------------------------------*/
/* Reads the package-installation-size from the file "pathname"		  */
/* Input: Partitionlist like:						  */
/* [/,etc,var]								  */
/* pathname = <path>/du.dir						  */
/*------------------------------------------------------------------------*/
bool  RawPackageInfo::readRawPackageInstallationSize (
					const string& pathname,
					const PartitionList& partitionList)
{
   char buffer[BUFFERLEN +1];
   FILE *file = NULL;
   bool ok = true;
   const string seperator(" \t");
   PackagePartitionSizeMap packagePartitionSizeMap;
   PartitionList::iterator posList;

   packagePartitionSizeMap.clear();
   RawPkg cpkg_pCi = 0;

   // generate the size-map
   for ( posList = partitionList.begin(); posList != partitionList.end();
	 ++posList )
   {
      packagePartitionSizeMap.insert(pair<const string, int >(*posList,0));
   }

   // Check, if "/" exists in the partitionList
   posList = partitionList.find("/" );
   if ( posList == partitionList.end() )
   {
      // not found --> insert
      packagePartitionSizeMap.insert(pair<const string, int >("/",0));
   }

   file = fopen (pathname.c_str(), "r");
   if ( !file )
   {
      y2error( "Can't open %s", pathname.c_str());
      ok = false;
      return ( ok );
   }

   if ( ok )
   {
      // parsing file
      buffer[0] = 0;
      while ( !feof(file )  )
      {
	 // reading a line

	 if ( fgets ( buffer, READLEN, file ) == NULL )
	 {
	    break;
	 }

	 if ( buffer[0] == 0 ||
	      buffer[0] == '\n' ||
	      (buffer[0] == '#' && strlen(buffer)>=2 && buffer[1] == '#') ||
	      strcmp ( buffer, "Rid:\n") == 0 ||
	      strcmp ( buffer, "Dir\n:") == 0 )
	 {
	    // scipping comments, Rid:, Dir:  and empty lines
	 }
	 else
	 {
	    string lineBuffer(buffer);
	    string::size_type 	begin, end;
	    string partition, size_str,file_str;

	    size_str = "";
	    file_str = "";
	    partition = "";

	    begin = lineBuffer.find_first_not_of ( seperator );

	    if( begin != string::npos )
	    {
	       end = lineBuffer.find_first_of ( seperator, begin );

	       // line-end ?
	       if ( end == string::npos )
	       {
		  end= lineBuffer.length();
	       }
	       partition.assign ( lineBuffer, begin, end-begin );

	       // extract \n
	       if (partition[partition.length()-1] == '\n' )
	       {
		  partition.assign ( partition,
				     0,
				     partition.length()-1 );
	       }

	       begin = lineBuffer.find_first_not_of ( seperator, end );
	    }
	    if( begin != string::npos )
	    {
	       // reading size
	       end = lineBuffer.find_first_of ( seperator, begin );

	       // line-end ?
	       if ( end == string::npos )
	       {
		  end= lineBuffer.length();
	       }
	       size_str.assign ( lineBuffer, begin, end-begin );
	       // extract \n
	       if (size_str[size_str.length()-1] == '\n' )
	       {
		  size_str.assign (  size_str,
				     0,
				     size_str.length()-1 );
	       }
	       begin = lineBuffer.find_first_not_of ( seperator, end );
	    }
	    if( begin != string::npos )
	    {
	       // reading size of sub-directories ---> not needed, skipping
	       end = lineBuffer.find_first_of ( seperator, begin );

	       // line-end ?
	       if ( end == string::npos )
	       {
		  end= lineBuffer.length();
	       }
	       begin = lineBuffer.find_first_not_of ( seperator, end );
	    }
	    if( begin != string::npos )
	    {
	       // reading file number of the directory
	       end = lineBuffer.find_first_of ( seperator, begin );

	       // line-end ?
	       if ( end == string::npos )
	       {
		  end= lineBuffer.length();
	       }
	       file_str.assign ( lineBuffer, begin, end-begin );
	       // extract \n
	       if (file_str[file_str.length()-1] == '\n' )
	       {
		  file_str.assign (  file_str,
				     0,
				     file_str.length()-1 );
	       }
	    }

	    if ( partition == PACK )
	    {
	       // new package
	       if ( cpkg_pCi )
	       {
		  // save old packaePartitionSizeMap
		  cpkg_pCi->partitionSizeMap() =  packagePartitionSizeMap;
		  // initialize packagePartitionSizeMap for next package
		  PackagePartitionSizeMap::iterator posSizeMap;
		  for ( posSizeMap = packagePartitionSizeMap.begin();
			posSizeMap != packagePartitionSizeMap.end();
			posSizeMap++ )
		  {
		     // each partition
			posSizeMap->second = 0;
		  }
	       }

	       // size_str is the filename in this case
	       cpkg_pCi = filename( size_str );
	       if ( !cpkg_pCi ) {
		 // old du.dir uses package name
		 cpkg_pCi = package( size_str );
	       }

	       if ( !cpkg_pCi )
	       {
		   y2warning( "Packagefile %s not found", size_str.c_str() );
		   ok = 0;
	       }
	    }
	    else
	    {
		int size =  atoi ( size_str.c_str() );

		if ( file_str.length() > 0 )
		{
		    int sizeFiles = 0;

		    sizeFiles = atoi ( file_str.c_str() );
		    sizeFiles = sizeFiles*BLOCKSIZE;

		    size += sizeFiles;
		}

		// partition entry

		// checking, if this partition have an link to another
		LinkList::iterator posLink;
		for ( posLink = linkList.begin();
		      posLink != linkList.end();
		      ++posLink )
		{
#if defined __GNUC__ && __GNUC__ >= 3
		  if (partition.compare (0, (posLink->first).length (), posLink->first) == 0)
#else
		  if (partition.compare (posLink->first, 0, (posLink->first).length ()) == 0)
#endif
		  {
		    // Entry found --> change it to existing path

		    // y2warning( "link %s found for %s",
		    // 	       (posLink->first).c_str(),
		    // 	       partition.c_str() );

		    partition = posLink->second + partition.substr(
								    (posLink->first).length(),
								    partition.length() -
								    (posLink->first).length() +1 );
		    // y2warning( "New partition %s", partition.c_str() );
		  }
		}

		if ( size > 0 )
		{
		    // searching an entry in the packagePartitionSizeMap
		    PackagePartitionSizeMap::iterator posSizeMap;
		    bool entryFound = false;
		    for ( posSizeMap = packagePartitionSizeMap.begin();
			  posSizeMap != packagePartitionSizeMap.end();
			  posSizeMap++ )
		    {
			// each partition
			string compareString = posSizeMap->first;
#if defined __GNUC__ && __GNUC__ >= 3
			if (partition.compare (0, compareString.length (), compareString) == 0)
#else
			    if (partition.compare (compareString, 0, compareString.length ()) == 0)
#endif
			    {
				// Entry found
				posSizeMap->second += (int) size;
				entryFound = true;
			    }
		    }
		    if ( !entryFound )
		    {
			// belonging to root "/"
			posSizeMap = packagePartitionSizeMap.find("/");
			if ( posSizeMap != packagePartitionSizeMap.end() )
			{
			    posSizeMap->second += (int) size;
			}
		    }
		}
	    }
	 }
      }
   }

   if ( ok && cpkg_pCi )
   {
     // save last package-entry
     cpkg_pCi->partitionSizeMap() = packagePartitionSizeMap;
   }


   if ( file )
   {
      fclose ( file );
   }
   return( ok );
 }

/*--------------------------------------------------------------------------*/
/* Calculate the required disk-size for the packages which were installed.  */
/* input:  partitionList - List of created partitions			    */
/*         packageList   - Packages which were installed		    */
/* output: PackagePartitionSizeMap - Map of all partitions and their required	    */
/*                            disk-space.				    */
/*--------------------------------------------------------------------------*/
void  RawPackageInfo::calculateRequiredDiskSpace(
			   const PartitionList& partitionList,
			   const PackList& packageList,
			   const PackList& deletePackageList,
			   PackagePartitionSizeMap& packagePartitionSizeMap )
{
   PackageSizeMap packageSizeMap;
   PackageSizeMap::iterator posPackageSize;
   PartitionList::iterator posPartition;
   PackList::iterator posPackage;
   PackagePartitionSizeMap::iterator posPartitionSize;

   getAllRawPackageInstallationSizes( packageSizeMap, partitionList );

   // create the packagePartitionSizeMap
   for ( posPartition = partitionList.begin();
	 posPartition != partitionList.end();
	 ++posPartition )
   {
      packagePartitionSizeMap.insert(
				pair<const string, int >(*posPartition,0));
   }
   // check if the root exists
   posPartitionSize = packagePartitionSizeMap.find ( "/" );
   if ( posPartitionSize == packagePartitionSizeMap.end() )
   {
      packagePartitionSizeMap.insert(pair<const string, int >("/",0));
   }

   // each package which have to be installed
   for ( posPackage = packageList.begin();
	 posPackage != packageList.end();
	 ++posPackage )
   {
      // search the package in the packageSizeMap
      posPackageSize = packageSizeMap.find(*posPackage);
      if ( posPackageSize != packageSizeMap.end() )
      {
	PackagePartitionSizeMap srcPackagePartitionSizeMap =
	   posPackageSize->second;
	PackagePartitionSizeMap::iterator posSrcPartitionSize;
	for ( posPartitionSize = packagePartitionSizeMap.begin();
	      posPartitionSize != packagePartitionSizeMap.end();
	      ++posPartitionSize )
	{
	   //sum each partition
	   posSrcPartitionSize =
	      srcPackagePartitionSizeMap.find( posPartitionSize->first );
	   if ( posSrcPartitionSize != srcPackagePartitionSizeMap.end() )
	   {
	      posPartitionSize->second += posSrcPartitionSize->second;
	   }
	   else
	   {
	      y2warning( "calculate size: Partition %s not found; Package %s",
			 (posPartitionSize->first).c_str(),
			 (*posPackage).c_str());
	   }
	}
      }
      else
      {
	 y2warning( "calculate size: Package %s not found",
		    (*posPackage).c_str() );
      }
   }

   // each package which have to be deleted
   for ( posPackage = deletePackageList.begin();
	 posPackage != deletePackageList.end();
	 ++posPackage )
   {
      // search the package in the packageSizeMap
      posPackageSize = packageSizeMap.find(*posPackage);
      if ( posPackageSize != packageSizeMap.end() )
      {
	PackagePartitionSizeMap srcPackagePartitionSizeMap =
	   posPackageSize->second;
	PackagePartitionSizeMap::iterator posSrcPartitionSize;
	for ( posPartitionSize = packagePartitionSizeMap.begin();
	      posPartitionSize != packagePartitionSizeMap.end();
	      ++posPartitionSize )
	{
	   //subtract for each partition
	   posSrcPartitionSize =
	      srcPackagePartitionSizeMap.find( posPartitionSize->first );
	   if ( posSrcPartitionSize != srcPackagePartitionSizeMap.end() )
	   {
	      posPartitionSize->second -= posSrcPartitionSize->second;
	   }
	   else
	   {
	      y2warning( "calculate size: Partition %s not found; Package %s",
			 (posPartitionSize->first).c_str(),
			 (*posPackage).c_str());
	   }
	}
      }
      else
      {
	 y2warning( "calculate size: Package %s not found",
		    (*posPackage).c_str() );
      }
   }
}


