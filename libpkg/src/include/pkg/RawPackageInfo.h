/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 * ----------------------------------------------------------
 *
 * Author: 	  Stefan Schubert  <schubi@suse.de>
 *		  Michael Hager    <mike@suse.de>
 *
 * File:	  RawPackageInfo.h
 * Description:   header file for class RawPackageInfo
 *
 *
 *************************************************************/

#ifndef RawPackageInfo_h
#define RawPackageInfo_h

#include <iconv.h>
#include <cstdio>

#include <iosfwd>
#include <map>
#include <set>
#include <string>

using std::map;
using std::pair;
using std::set;
using std::string;

#include <pkg/Package.h>

class RawPackage;
class TagParser;

/**
 * @short Parse the common.pkd which gives information about the packages
 *
 */


class RawPackageInfo {

  public:

    RawPackageInfo( const string path      = "",
		    const string language  = "",
		    const string commonPkd = "",
		    const bool   memmoryOptimized = false );
    // Konstruktor; path is the path the suse/setup/descr directory

    ~RawPackageInfo();

    bool closeMedium();
    // Let us known, that he cannot access the common.pkd

    PackVersList getRawPackageList( bool withSourcePackages );
    // Return the package-list
    // [<packagename1,version1>,<packagename2,version2>,... ]

    bool getRawPackageDescritption( const string packageName,
				    string& shortDescription,
				    string& longDescription,
				    string& notify,
				    string& delDescription,
				    string& category,
				    int& size);
    // Get package-info for the desired package.
    // input: packageName
    // output: shortDescription longDescription notify
    //         delDescription ( Warning if deselected )
    // return: true if package was found

    PackList getProvides(const string packageName );
    // Return the package-list of Provides
    // [<packagename1>,<packagename2>,<packagename3>,... ]

    DependList getRequires(const string packageName );
    // Return all Requires tags of a package


    bool getRawPackageInstallationInfo( const string packageName,
					bool& basePackage,
					int& installationPosition,
					int& cdNr,
					string& instPath,
					string& version,
					long& buildTime,
					int& rpmSize);
    // Get package-install-info for the desired package.
    // input: packageName
    // output: basePackage , installationPosition, cdNr, instPath
    // return: true if package was found

    string getSerieOfPackage(const string packageName );
    // Get the serie of the packet

    PackList getSourcePackages ( void );
    // Get a list of source-packages

    PackList getAllSeries( void );
    // Get a list of all series

    PackList getRawPackageListOfSerie ( const string serie );
    // Get a list of packages which belong to the serie

    PackList getAllRpmgroup( void );
    // Get a list of all rpmgroups

    PackList getRawPackageListOfRpmgroup ( const string rpmgroup );
    // Get a list of packages which belong to the rpmgroup

    // ***************************************************************
    // distinction between DepL and TagL is not correct as each kind
    // of dependency type might contain operator and version.
    // ***************************************************************

    PackDepLMap getRawRequiresDependency();
    // Return all Requires dependency of every package which is defined
    // in common.pkd. The return-value is a map which contains
    // the package-name as index and a list of package-names,groups...
    // from which the package is depent.

    PackTagLMap getRawProvidesDependency();
    // Return all Provides dependency of every package which is defined
    // in common.pkd. The return-value is a map which contains
    // the package-name as index and a list of package-names,groups...
    // from which the package is depent.

    PackDepLMap getRawConflictsDependency();
    // Return all Conflicts dependency of every package which is defined
    // in common.pkd. The return-value is a map which contains
    // the package-name as index and a list of package-names,groups,...
    // from which the package is depent.

    PackTagLMap getRawObsoletesDependency();
    // Return all Obsoletes dependency of every package which is defined
    // in common.pkd. The return-value is a map which contains
    // the package-name as index and a list of package-names,groups...
    // from which the package is depent.

    PackDepLMap getSelRequiresDependency();
    // Return all Requires dependency of every selection class which is defined
    // in suse/setup/descr/*.sel. The return-value is a map which contains
    // the selection class as index and a list of selections,groups...
    // from which the selection is depent.

    PackTagLMap getSelProvidesDependency();
    // Return all Provides dependency of every selection class which is defined
    // in suse/setup/descr/*.sel. The return-value is a map which contains
    // the selection class as index and a list of selections,groups...
    // from which the selection is depent.

    PackTagLMap getSelSuggestsDependency();
    // Return all Suggests dependency of every selection class which is defined
    // in suse/setup/descr/*.sel. The return-value is a map which contains
    // the selection class as index and a list of selections,groups...
    // from which the selection is depent.

    PackDepLMap getSelConflictsDependency();
    // Return all Conflict dependency of every selection class which is defined
    // in suse/setup/descr/*.sel. The return-value is a map which contains
    // the selection class as index and a list of selections,groups...
    // from which the selection is depent.

    bool readRawPackageInstallationSize ( const string& pathname,
					  const PartitionList& partitionList);
    // Reads the package-installation-size from the file "pathname"
    // Input: Partitionlist like:
    // [/,etc,var]
    // pathname = <path>/du.dir

    void getAllRawPackageInstallationSizes( PackageSizeMap& packageSizeMap,
					    const PartitionList& partitionList );
    // Read the installation-sizes for ALL packages
    // Return a map like:
    // [packageA, [[/,2kByte],[etc,7kByte],[var,1kByte]]]
    // [packageB, ........					       ]
    // Input: Partitionlist like:
    // [/,etc,var]

    void getRawPackageInstallationSize( PackagePartitionSizeMap& packagePartitionSizeMap,
					const string packageName,
					const PartitionList& partitionList);
    // Read the installation-size for ONE packet
    // Return a map like:
    // [[/,2kByte],[etc,7kByte],[var,1kByte]]
    // Input: Partitionlist like [/,etc,var]
    //        packageName



    void calculateRequiredDiskSpace( const PartitionList& partitionList,
				     const PackList& packageList,
				     const PackList& deletePackageList,
				     PackagePartitionSizeMap& packagePartitionSizeMap );
    // Calculate the required disk-size for the packages which were installed.
    // input:  partitionList - List of created partitions
    //         packageList   - Packages which will be installed
    //	      deletePackageList - Packages which will be deleted
    // output: PackagePartitionSizeMap - Map of all partitions and their
    //		required disk-space.

    bool getSelectionGroupList ( const string groupName,
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
				 string &delNotify );
    // Parse the file <groupName>.sel and return a List of packages which
    // belongs to this group.
    // input: groupName
    // output: 	packageList
    // 		groupDescription - short description in UTF8 Code
    //		kind of the sel-Group ( like addon, base... )
    //		show this selection groups
    //		other required selection groups ( the user cannot deselect
    //						  these groups )
    //		confiction selection groups
    //		other required selection groups ( the user can deselect
    //						  these groups )

    bool getSelectionGroupMap ( SelectionGroupMap &groupMap );
    // Parse the descr-directory for *.sel files and returns a map
    // of these groups.

    PackVersList getSelectionList();
    // Returns list of all possible selections.
    // [<selection1,version1>,<selection2,version2>,... ]

    PackList getSearchResult ( const string searchmask,
			       const bool onlyName,
			       const bool casesensitive );
    // Returns a list of packages to which the searchmask-string
    // fits.

    string getLabel ( const string packageName );

    // Return label of packages in the patchdescription files.

    string getShortName ( const string packageName );

    // Return short packagename which is used in the rpm-DB


  public:

    class FileLocation {
      friend class RawPackageInfo;
      private:
	streampos startData_i;
	streampos endData_i;
	FileLocation( streampos startData_ir, streampos endData_ir ) {
	startData_i = startData_ir;
	endData_i   = endData_ir;
      }
      public:
	FileLocation()  { startData_i = endData_i = 0; }
	~FileLocation() {}
    };

    string readCommon  ( const FileLocation & at_Cr ) const;
    string readLanguage( const FileLocation & at_Cr ) const;

  private:

    struct RereadErr {
      mutable unsigned cnt_i;
      mutable unsigned limit_i;
      void reset() const { cnt_i = 0; limit_i = 1; }
      void failed( const string & fname_tr ) const;
      RereadErr()  { reset(); }
    };

    RereadErr rereadErr_C;

  private:

    typedef map<string,string> LinkList;
    LinkList linkList;                 // List of symb-links in root

    bool getLinkList ( LinkList &linkList );
    // Reading all symb-links in the root-partition and saving it into
    //  a list.

  private:

    string  descrPath_t;               // Path of the *.pkd *.sel
    string  selectedLanguage_t;        // Current selected language

    string  commonPkd_t;               // Name of common.pkd file
    string  parsedLanguage_t;          // mot necssarily the selected language

    mutable ifstream * commonPKD_pF;   // mutable, because they may be automaticaly
    mutable ifstream * languagePKD_pF; // reopened by readCommon/Language()

  private:

    string  languageRecodeFrom_t;
    string  languageRecodeTo_t;
    iconv_t iconv_cd;                  // needed for string converting

    void    initLanguageRecode();
    string  languageRecode( const string & dataIn_tr ) const;

  private:

    typedef vector<RawPackage*>  PkgStore;
    typedef const RawPackage *   RawPkg;

    typedef map<string,RawPkg>   PkgLookup;
    typedef set<string>          NameCache;

    bool      inParser_b;
    unsigned  maxCDNum_i;

    PkgStore  packages_VpC;           // in install order

    PkgLookup name2package_VpC;
    PkgLookup filename2package_VpC;

    NameCache seriesNames_Vt;
    NameCache groupNames_Vt;

    void clearPackages();
    void cacheStrore( NameCache & cache_Vtr, string & data_tr );

    RawPkg package ( const string & name_rt ) const;
    RawPkg filename( const string & name_rt ) const;

    bool   checkAndStore( int & error_ir, RawPackage *& cpkg_pCr );

    // Parse the common.pkd
    int parseCommonPKD();
    int parseCommonPKD( istream & in_Fr );
    int storeCommonFilename     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonRpmName      ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonVersion      ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonProvides     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonRequires     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonConflicts    ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonObsoletes    ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonInstPath     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonBuildtime    ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonBuiltFrom    ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonRpmGroup     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonSeries       ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonSize         ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonFlag         ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonCopyright    ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonAuthorName   ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonAuthorEmail  ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonAuthorAddress( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonStartCommand ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeCommonCategory     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );

    // Parse the <language>.pkd
    int parseLanguagePKD();
    int parseLanguagePKD( istream & in_Fr );
    int storeLanguageFilename   ( const TagParser & data_Cr, RawPackage *& cpkg_pCr ); // '*&': needs to adjust cpkg_pC
    int storeLanguageLabel      ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeLanguageNotify     ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeLanguageDelnotify  ( const TagParser & data_Cr, RawPackage * cpkg_pCr );
    int storeLanguageDescription( const TagParser & data_Cr, RawPackage * cpkg_pCr );

  private:

    SelectionGroupMap   selectionGroupMap;

    int parseSelections();

  private:

    // Evaluate all dependencies requires, provides, conflicts, obsolets
    PackDepLMap getSelDependency( DepType depType );
    PackTagLMap getSelTag       ( DepType depType );


    void extractPackages ( string packageString,
			   PackList &destPackList );
    // Extract package names from a string

  public:

    unsigned numPackages() const { return packages_VpC.size(); }

    PkdData getPackage          ( unsigned idx_ii )         const;
    PkdData getPackageByName    ( const string & name_rt )  const;
    PkdData getPackageByFilename( const string & fname_rt ) const;

    // Return the number of available CDs.
    unsigned getCDNumbers() const { return maxCDNum_i; }
};


#endif // RawPackageInfo_h
