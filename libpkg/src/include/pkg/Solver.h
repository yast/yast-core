/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 *----------------------------------------------------------
 *
 * Author: 	  Michael Hager <mike@suse.de>
 *
 * Description:   module that solves dependencies.
 *                (especially packages)
 *
 * $Header$
 *
 *************************************************************/

/*
 * $Log$
 * Revision 1.1  2002/06/21 16:04:39  arvin
 * Initial revision
 *
 * Revision 1.5  2001/11/08 09:24:29  schubi
 * taking new versions from pkginfo server
 *
 * Revision 1.28  2001/09/05 16:36:51  schubi
 * solver recognize packages which are installed, but no more in the common.pkd
 *
 * Revision 1.27  2001/08/27 16:39:36  schubi
 * memory optimization
 *
 * Revision 1.26  2001/07/04 16:50:47  arvin
 * - adapt for new automake/autoconf
 * - partly fix for gcc 3.0
 *
 * Revision 1.25  2001/06/15 17:34:31  mike
 * new: GetBreakingPackageList()
 *      Simulates the removal of a package  and returns all
 *      selected packages, that have then unfullfilled dependencies
 *
 * Revision 1.24  2001/04/18 17:43:38  mike
 * faster findingPackagesByName
 *
 * Revision 1.23  2001/04/18 16:16:08  mike
 * faster GetTag, GetPackIndex
 *
 * Revision 1.22  2001/04/18 13:26:27  mike
 * more debug changes
 *
 * Revision 1.21  2001/04/18 11:01:38  mike
 * faster solving/ debug call
 *
 * Revision 1.20  2001/04/18 08:20:28  mike
 * new call: AddPackageList
 *
 * Revision 1.19  2001/04/17 13:44:27  mike
 * new: ignoreAdditonalPack, fix: obsolete deps
 *
 * Revision 1.18  2001/04/17 09:37:33  mike
 * bugfix conflicts with versions
 *
 * Revision 1.17  2001/04/11 17:02:59  mike
 * fixed A a difference
 *
 * Revision 1.16  2001/04/11 09:39:12  mike
 * changed interface solve_dependencies
 *
 * Revision 1.15  2001/04/10 14:11:18  mike
 * now with conflicts and obsoletes
 *
 * Revision 1.14  2001/04/09 15:36:29  schubi
 * IngnorConflicts changed
 *
 * Revision 1.13  2001/04/09 13:55:48  schubi
 * new dependecies returned by server
 *
 * Revision 1.12  2001/04/06 15:23:27  mike
 * conflict detection implemented
 *
 * Revision 1.11  2001/04/06 08:03:53  gs
 * interface changed ( respecting new package dependencies )
 *
 * Revision 1.10  2001/04/05 14:10:05  schubi
 * obsoletes parameter changed
 *
 * Revision 1.9  2001/04/04 23:45:53  mike
 * first compiling and working (hopefully)
 *
 * Revision 1.8  2001/04/04 15:41:39  mike
 * first compilable
 *
 * Revision 1.7  2001/04/04 15:26:13  schubi
 * only header changes
 *
 * Revision 1.6  2001/04/04 14:32:52  mike
 * next step
 *
 * Revision 1.4  2001/04/04 08:12:36  schubi
 * verison and release mixed
 *
 * Revision 1.3  2001/04/04 06:46:52  mike
 * new dependencie interface (prov requ ..)
 *
 * Revision 1.2  2000/05/17 14:32:04  schubi
 *
 */


#ifndef Solver_h
#define Solver_h

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;
using std::multimap;

#include <pkg/pkginfo.h>
#include <pkg/RawPackageInfo.h>


/****************************************************************/
/* little helper class				       		*/
/*--------------------------------------------------------------*/
/* set/get the status of a package				*/
/****************************************************************/

class PackStat
{

  public:

  PackStat( const PackageKey &packagekey );
  ~PackStat();

  bool operator <  (const PackStat) const;
  bool operator == (const PackStat) const;

  int  UsedBy()     	const { return _UsedBy;  };
  int  InSelection()    const { return _InSelection;  }
  int  IsInstalled()    const { return _IsInstalled;  }
  int  IsBroken()       const { return _IsBroken;  };

  string Name()    	const { return _Name; };
  string Version()    	const { return _Version; };

  void IncUsedBy();
  void DecUsedBy();

  void SetUsedBy(        int  CurrUsedBy);
  void SetInSelection(   bool CurrSelection);
  void SetIsInstalled(   bool CurrInstalled );
  void SetIsBroken(      bool CurrBroken );
  void SetHasOr(         bool CurrHasOr );
  void SetHasExcl(       bool CurrHasExcl );


  private:

   // UsedBy: after we have solved, the usedby counter is > 0
   //         for every selected and for every additional package
  int    _UsedBy;
  string _Name;
  string _Version;
  bool   _InSelection;
  bool   _IsInstalled;
  bool   _IsBroken;

};



/****************************************************************/
/* solver class							*/
/****************************************************************/


/**
 * typedefs for internal storage of package dependencies: (based on vectors)
 *
 * VecDependMap    for And-Dependencies:  [ [A,[B,K], [B,[C,L], [C,[X] ]
 *                 and Exclude-Deps.
 *                                           [0]: 1,10
 *                                           [1]: 2,11
 *                                           [2]: 3,21
 *
 * VecDependMultiMap for OR-Dependencies:   [ [A,[D,V], [A,[E,V], [B,[D,E]] ]
 *
 *                                           [0,0]: 4,18
 *                                           [0,1]: 5,18
 *                                           [1,0]: 4,5
 *
 *
 */

typedef set<int>                      VecDependList;
typedef vector<VecDependList>         VecDependMap;
typedef vector<set<VecDependList> >   VecDependMultiMap;
typedef map<int,set<int> >            IndexedUnsolvedMap;

class Solver
{

public:
  /**
   * 	Creates a solver
   */

   // Example in YCP Syntax
   //
   // require and conflicts:
   // $[ ["grep","1.3-7"]:[ ["libc","LT","5.0"],  ["libGG","GT","6.0"]  ]
   //
   // provides and obsoltes:
   // $[ ["grep","1.3-7"]:[ "grepY", "grepX" ]
   //
   // package lists:
   //  [  ["grep","1.3-7"],  ["fgrep","1.3-7"]


   Solver(
	  PackDepLMap   &rawRequiresDependencies,
	  PackDepLMap   &rawConflictsDependencies,

	  PackTagLMap   &rawProvidesDependencies,
	  PackTagLMap   &rawObsoletesDependencies,

	  PackVersList  &listOfValidPackages,
	  PackVersList  &listOfInstalledPackages);

   Solver( RawPackageInfo *rawPackageInfo,
	   PackDepLMap   &addRequiresDependencies,
	   PackDepLMap   &addConflictsDependencies,

	   PackTagLMap   &addProvidesDependencies,
	   PackTagLMap   &addObsoletesDependencies,

	   PackVersList  &addListOfValidPackages);

   // debug version: "all" debugs a lot of meta info

   Solver(
	  PackDepLMap   &rawRequiresDependencies,
	  PackDepLMap   &rawConflictsDependencies,

	  PackTagLMap   &rawProvidesDependencies,
	  PackTagLMap   &rawObsoletesDependencies,

	  PackVersList  &listOfValidPackages,
	  PackVersList  &listOfInstalledPackages,
	  const string  debugstr );



  /**
   * Cleans up
   */
  ~Solver();

  /**
   * Set a current selection to the sovler, and solve the dependencie
   * for that selection.
   */

  void SolveDependencies( const PackVersList        &packageList,
			  PackVersList              &additionalPackages,
			  TagPackVersLMap           &unsolvedRequirements,
			  PackPackVersLMap          &conflictMap,
			  ObsoleteList              &obsoleteMap );



  void SolveDependencies( PackVersList              &additionalPackages,
			  TagPackVersLMap           &unsolvedRequirements,
			  PackPackVersLMap          &conflictMap,
			  ObsoleteList          &obsoleteMap );

  /*
   * Add a package to the current selection
   * and solve the dependencies for the current selection
   *
   * > SolveDependencies( [A,E,G] )
   *
   * returns the same as
   *
   * > SolveDependencies( [A,G] )
   * > AddPackage[E]
   *
   */
  void AddPackage( const PackageKey    	 packageName,
		   PackVersList          &additionalPackages,
		   TagPackVersLMap       &unsolvedRequirements,
		   PackPackVersLMap      &conflictMap,
		   ObsoleteList          &obsoleteMap );

  void AddPackageList( const PackVersList    packageList,
		       PackVersList          &additionalPackages,
		       TagPackVersLMap       &unsolvedRequirements,
		       PackPackVersLMap      &conflictMap,
		       ObsoleteList          &obsoleteMap );


  /**
   * Delete a package from the current selection
   * and solve the dependencies for the current selection
   *
   * > SolveDependencies( [A,E,G] )
   *
   * returns the same as
   *
   * > SolveDependencies( [A,E,G,B] )
   * > DeletePackage(B)
   *
   */
  void DeletePackage( const PackageKey    selection,
		      PackVersList        &additionalPackages,
		      TagPackVersLMap     &unsolvedRequirements,
		      PackPackVersLMap    &conflictMap,
		      ObsoleteList        &obsoleteMap );


  /**
   *   IgnoreConflict* deletes all Conflict-Deps between packages
   *   packages1 and packages2
   *
   *   Therefor *ByKey delets one conflict
   *   IgnoreConflict deletes more conflicts, if one of packages*
   *   is in the list with different versions
   */

   void IgnoreConflict( const string package1,
			const string package2 );

   void IgnoreConflictByKey( const PackageKey packagekey1,
			     const PackageKey packagekey2 );

   void IgnoreUnsolvedRequirements( const string tag );

   //
   // All tags, only provided from this package can be deleted.
   // If this package provides something which is also provided
   // from other packages, only the provide tag will be deleted
   //
   void IgnoreAdditionalPackages( const string packagename );

   void SetVerboseDebug( const string debugstr );


   //////////////////////////////////////////////////////////////////////
   // Simulates the delete of the package "packagekey1" and returns all
   // selected packages, that have then unfullfilled dependencies
   //////////////////////////////////////////////////////////////////////

   void GetBreakingPackageList(  const PackageKey    packagekey,
				 PackVersList        &additionalPackages );


private:

   void Init(PackDepLMap  	&rawRequiresDependencies,
	     PackDepLMap  	&rawConflictsDependencies,

	     PackTagLMap   	&rawProvidesDependencies,
	     PackTagLMap   	&rawObsoletesDependencies,

	     PackVersList  	&listOfValidPackages,
	     PackVersList  	&listOfInstalledPackages,

	     RawPackageInfo 	*rawPackageInfo,
	     bool		readRawInfo );


   bool FindObsoletingPackIndexes( const string packagname,
				   vector<int>  &list,
				   int          &listSize,
				   int 		&basePackIndex);

   bool FindPackIndexes( const string packagname,
			 vector<int>  &list,
			 int          &listSize );

   PackageKey GetPackageKeyFromDepList( const int index );

   bool GetPackIndex(    const PackageKey  packagekey,
			 int               &index );

   bool GetTagIndex(  	const string       tagName,
			int                &index );

   void SaturatePackage( int                packageIndex,
			 IndexedUnsolvedMap &indexedUnsolvedMap);

   void BrokenPackage( const int packageIndex, string debug_stack );

   vector<PackStat>      depList;          // list of packages and saturate db
   map<PackStat,    int> depListIndex;     // Index of depList key = package
   multimap<string, int> depListStrIndex;  // Index of depList key = packagename
   vector<string>        tagList;	   // list of possible tage
   map<string,      int> tagListIndex;     // Index of depList
   VecDependMap          provideArray;     // saved indexed RawDependencies
   VecDependMap          requireArray;     // saved indexed RawDependencies
   VecDependMap          conflictArray;    // saved indexed RawDependencies
   VecDependMap          obsoleteArray;    // saved indexed RawDependencies for packages to install
   VecDependMap          instObsoleteArray; // saved indexed RawDependencies for installed packages

   int 			 tagNumber;         // list of (provided) tags
                                            // == number of different packages
                                            // == size of tagList
   int 			 packNumber;        // list of  Packages
                                            // == number of different packages
                                            // == size of depList
   int 			 packNumberI;       // listOfInstalledPackages.size();
   int 			 packNumberV;       // listOfValidPackages.size();
   PackVersList          currSelectionList; // current Selection
   bool  		 debug_all;
   bool  		 debug_sat;

};


#endif // Solver_h
