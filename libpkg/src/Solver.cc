/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 * ----------------------------------------------------------
 *
 * Author: 	  Michael Hager <mike@suse.de>
 *
 * Description:   module that solves package dependencies.
 *
 * $Header$
 *
 *************************************************************/

/*
 * $Log$
 * Revision 1.1  2002/06/21 16:04:39  arvin
 * Initial revision
 *
 * Revision 1.11  2002/03/28 10:25:06  schubi
 * merge with 8.0 tree
 *
 * Revision 1.10.2.1  2002/03/09 17:24:37  ma
 * - version 2.5.15
 * - Let solver at least report dropped dependencies (those required,
 *   but not provided) to y2log.
 *
 * Revision 1.10  2001/11/08 09:24:29  schubi
 * taking new versions from pkginfo server
 *
 * Revision 1.39  2001/09/12 12:37:00  schubi
 * Bugfix core dump while parsing the common.pkd
 *
 * Revision 1.38  2001/09/10 12:02:39  schubi
 * memory optimization
 *
 * Revision 1.37  2001/09/06 16:04:42  schubi
 * memory optimization
 *
 * Revision 1.36  2001/09/05 16:36:51  schubi
 * solver recognize packages which are installed, but no more in the common.pkd
 *
 * Revision 1.35  2001/08/27 16:39:36  schubi
 * memory optimization
 *
 * Revision 1.34  2001/08/10 13:15:51  kkaempf
 * replace y2error with y2debug to avoid cluttering y2log during normal
 * installation
 *
 * Revision 1.33  2001/07/04 16:50:47  arvin
 * - adapt for new automake/autoconf
 * - partly fix for gcc 3.0
 *
 * Revision 1.32  2001/07/03 09:04:10  mike
 * y2log deleted
 *
 * Revision 1.31  2001/06/15 17:34:31  mike
 * new: GetBreakingPackageList()
 *      Simulates the removal of a package  and returns all
 *      selected packages, that have then unfullfilled dependencies
 *
 * Revision 1.30  2001/05/08 13:35:01  schubi
 * remove rpmlib requires #7504
 *
 * Revision 1.29  2001/04/24 13:37:41  schubi
 * logging reduced; function for version evaluation
 *
 * Revision 1.28  2001/04/23 13:50:35  mike
 * bugfix in Solver: deletePackage
 *
 * Revision 1.27  2001/04/23 13:47:41  schubi
 * no more conficts with YaST1 defines
 *
 * Revision 1.26  2001/04/18 17:43:38  mike
 * faster findingPackagesByName
 *
 * Revision 1.25  2001/04/18 16:16:08  mike
 * faster GetTag, GetPackIndex
 *
 * Revision 1.24  2001/04/18 13:26:27  mike
 * more debug changes
 *
 * Revision 1.23  2001/04/18 11:01:38  mike
 * faster solving/ debug call
 *
 * Revision 1.22  2001/04/18 08:20:28  mike
 * new call: AddPackageList
 *
 * Revision 1.21  2001/04/17 15:43:48  mike
 * fix in solve: clear output variabales
 *
 * Revision 1.18  2001/04/11 17:02:59  mike
 * fixed A a difference
 *
 * Revision 1.17  2001/04/11 13:54:51  mike
 * fixed deletePackage
 *
 * Revision 1.16  2001/04/11 10:46:25  schubi
 * solver selections with versionnumber; rpm stack added ( decrease rpm calls)
 *
 * Revision 1.15  2001/04/11 09:39:12  mike
 * changed interface solve_dependencies
 *
 * Revision 1.14  2001/04/11 07:43:13  schubi
 * IgnoreAdditionalPackages added
 *
 * Revision 1.13  2001/04/10 14:11:18  mike
 * now with conflicts and obsoletes
 *
 * Revision 1.12  2001/04/09 15:36:29  schubi
 * IngnorConflicts changed
 *
 * Revision 1.11  2001/04/09 13:55:48  schubi
 * new dependecies returned by server
 *
 * Revision 1.10  2001/04/06 15:23:27  mike
 * conflict detection implemented
 *
 * Revision 1.9  2001/04/05 14:10:05  schubi
 * obsoletes parameter changed
 *
 * Revision 1.8  2001/04/04 23:45:53  mike
 * first compiling and working (hopefully)
 *
 * Revision 1.7  2001/04/04 15:41:39  mike
 * first compilable
 *
 * Revision 1.3  2000/05/30 15:43:43  kkaempf
 * fix include paths
 *
 * Revision 1.2  2000/05/17 14:32:04  schubi
 * update Modus added after new cvs
 *
 * Revision 1.10  2000/03/02 15:22:19  mike
 * a excludes a now ignored
 *
 * Revision 1.9  2000/02/10 16:48:46  mike
 * added new SolveDependencies
 *
 * Revision 1.8  2000/02/10 15:48:21  schubi
 * changing delete..
 *
 * Revision 1.7  2000/02/10 13:47:53  mike
 * Delete*Deps implemented
 *
 * Revision 1.6  2000/02/10 12:28:27  mike
 * added Delete*Dependencie
 *
 * Revision 1.5  2000/02/02 07:50:09  schubi
 * little Bugfix
 *
 * Revision 1.4  2000/02/01 14:35:50  mike
 * SolveDeps now returns only dependend packages
 *
 * Revision 1.3  2000/01/26 08:31:08  mike
 * changed interface
 *
 */


//////////////////////////////////////////
//
//          To DO
// - obsoletes: List of dependend packages
// - obsoleting a already installed -> saturate a possibly
//   "unique provide" in such a package
// - compares versions
//
// - deselect deleted as new call
//
//
//////////////////////////////////////////

#include <stdlib.h>
#include <algorithm>
#include <stdio.h>
#include <iostream.h>
#include <string>
#include <list>
#include <algorithm>
#include <unistd.h>

#include <ycp/y2log.h>
#include <pkg/Solver.h>


/*-------------------------------------------------------------*/
/* creates a solver					       */
/*-------------------------------------------------------------*/

Solver::Solver(PackDepLMap  	&rawRequiresDependencies,
	       PackDepLMap  	&rawConflictsDependencies,
	       PackTagLMap   	&rawProvidesDependencies,
	       PackTagLMap   	&rawObsoletesDependencies,
	       PackVersList  	&listOfValidPackages,
	       PackVersList  	&listOfInstalledPackages,
	       const string   	debugstr )
{
   SetVerboseDebug( debugstr );
   Init(rawRequiresDependencies,
	rawConflictsDependencies,

	rawProvidesDependencies,
	rawObsoletesDependencies,

	listOfValidPackages,
	listOfInstalledPackages,
	NULL,
	false );
}




Solver::Solver(PackDepLMap  	&rawRequiresDependencies,
	       PackDepLMap  	&rawConflictsDependencies,
	       PackTagLMap   	&rawProvidesDependencies,
	       PackTagLMap   	&rawObsoletesDependencies,
	       PackVersList  	&listOfValidPackages,
	       PackVersList  	&listOfInstalledPackages)
{
   SetVerboseDebug( "" );
   Init(rawRequiresDependencies,
	rawConflictsDependencies,

	rawProvidesDependencies,
	rawObsoletesDependencies,

	listOfValidPackages,
	listOfInstalledPackages,
	NULL,
	false );
}


Solver::Solver( RawPackageInfo *rawPackageInfo,
		PackDepLMap   &addRequiresDependencies,
		PackDepLMap   &addConflictsDependencies,

		PackTagLMap   &addProvidesDependencies,
		PackTagLMap   &addObsoletesDependencies,

		PackVersList  &addListOfValidPackages)
{
    PackVersList listOfInstalledPackages; // not supported

    SetVerboseDebug( "" );
    Init(addRequiresDependencies,
	 addConflictsDependencies,

	 addProvidesDependencies,
	 addObsoletesDependencies,

	 addListOfValidPackages,
	 listOfInstalledPackages,
	 rawPackageInfo,
	 true );
}




void Solver::Init(PackDepLMap  	&rawRequiresDependencies,
	     PackDepLMap  	&rawConflictsDependencies,

	     PackTagLMap   	&rawProvidesDependencies,
	     PackTagLMap   	&rawObsoletesDependencies,

	     PackVersList  	&listOfValidPackages,
	     PackVersList  	&listOfInstalledPackages,
	     RawPackageInfo 	*rawPackageInfo,
	     bool		readRawInfo		  )
{


  int    i;
  bool 	 ok         = true;

  PackVersList::iterator pos;

  y2milestone( "### Creating Solver start" );

  /////////////////////////////////////////////////////////
  // Build Package index

  if ( rawPackageInfo != NULL &&
       readRawInfo )
  {
      // reading direct from pkginfo-server
      PackVersList dummyList = rawPackageInfo->getRawPackageList( false );
      PackVersList::iterator pos;

      for ( pos = dummyList.begin(); pos != dummyList.end(); pos++)
      {
	  listOfValidPackages.push_back ( *pos );
	  PackageKey pack = *pos;
      }
  }

  packNumberV = listOfValidPackages.size();          // number of packages
  packNumberI = listOfInstalledPackages.size();      // number of packages

  packNumber = packNumberI + packNumberV;

  depList.reserve(packNumber);

  /*  Build depList:
   *  Insert all possible Items
   *  sort these items, so that later on we can use GetIndex
   */

  for ( pos =  listOfInstalledPackages.begin();
	pos != listOfInstalledPackages.end();     pos ++)
  {
    PackStat packStat( *pos );
    packStat.SetIsInstalled( true );

    if ( debug_all ) y2debug( "INSTALLED: packageList:  %s ", packStat.Name().c_str());
    depList.push_back( packStat );
  }

  for ( pos =  listOfValidPackages.begin();
	pos != listOfValidPackages.end();     pos ++)
  {
    PackStat packStat( *pos );

    if ( debug_all ) y2debug( "VALID:     packageList:  %s ", packStat.Name().c_str());
    depList.push_back( packStat );
  }

  packNumber = depList.size();

  listOfValidPackages.clear();


  ////////////////////////////
  // generate depList indexes:

  for ( i=0; i<packNumber; i++)
  {
     depListIndex.insert(   map<PackageKey,int>::value_type( GetPackageKeyFromDepList(i), i));
     depListStrIndex.insert(map<string    ,int>::value_type( depList[i].Name(),           i));
  }


  /////////////////////////////
  // DEBUG

  if ( debug_all )
  {
     for ( i=0; i<packNumber; i++)
     {
	y2debug( "packageList: %d:  %s %s", i, depList[i].Name().c_str(), depList[i].Version().c_str());
     }
  }


  // sort( depList.begin(), depList.end(), StrCompare );

  y2debug( "### Build Tag index");


  /////////////////////////////////////////////////////////
  // Build Tag index
  //
  // they are builded from the provided tags!
  //
  // we need also the implizit provides:
  // every package provides his name

  set<string> prevTagList;

  TagList    ::iterator lpos;
  PackTagLMap::iterator tpos;
  set<string>::iterator ptpos;

  tagList.reserve( 3*packNumber );

  if ( rawPackageInfo != NULL &&
       readRawInfo )
  {
      // reading from pkginfo-server
      PackTagLMap dummyMap= rawPackageInfo->getRawProvidesDependency();
      PackTagLMap::iterator pos;

      for ( pos = dummyMap.begin(); pos != dummyMap.end(); pos++ )
      {
	  PackageKey packageKey = pos->first;
	  TagList tagList = pos->second;

	  rawProvidesDependencies.insert(pair<PackageKey,
					 TagList>( packageKey,
						   tagList ));
      }
  }

  for ( tpos =  rawProvidesDependencies.begin();
	tpos != rawProvidesDependencies.end();  tpos++)
  {
     for ( lpos =  tpos->second.begin();
	   lpos != tpos->second.end();  lpos++)
     {
	prevTagList.insert( *lpos );
     }
  }

  for ( i=0; i<packNumber; i++)
  {
     prevTagList.insert( depList[i].Name() );
  }


  // now we have all tags (no doubles)
  for ( ptpos =  prevTagList.begin();
	ptpos != prevTagList.end();   ptpos++)
  {
     tagList.push_back( *ptpos );
  }

  prevTagList.clear();

  tagNumber = tagList.size();      // number of tags
  y2debug( "estimated %d  real %d",   3*packNumber, tagNumber );

  for ( i=0; i<tagNumber; i++)
  {
     tagListIndex.insert(map<string,int>::value_type(tagList[i], i));
  }


  if ( debug_all )
  {
     for ( i=0; i<tagNumber; i++)
     {
	y2debug( "tagList: %02d: %s", i, tagList[i].c_str());
     }
  }

  y2debug( "### Init Arrays");

  //////////////////////////////////////////////////////////////////////
  // initialise the depend index arrays for all dependencies

  VecDependList		         emptyList;
  set<VecDependList>             emptySet;
  PackTagLMap      ::iterator    ptPos;
  TagList          ::iterator    tPos;
  PackDepLMap      ::iterator    pdPos;
  DependList       ::iterator    dPos;

  provideArray.reserve(       tagNumber );
  requireArray.reserve(       packNumber );
  obsoleteArray.reserve(      packNumber );
  conflictArray.reserve(      packNumber );
  instObsoleteArray.reserve(  packNumber );

  for ( i=0; i<packNumber; i++)
  {
     requireArray.push_back( emptyList );
  }

  for ( i=0; i<tagNumber; i++)
  {
     provideArray.push_back( emptyList );
  }

  for ( i=0; i<packNumber; i++)
  {
     conflictArray.push_back( emptyList );
  }

  for ( i=0; i<packNumber; i++)
  {
     obsoleteArray.push_back( emptyList );
  }

  for ( i=0; i<packNumber; i++)
  {
     instObsoleteArray.push_back( emptyList );
  }

  y2debug( "### Index provides");
  /*------------------------------------------------------------------------------------
   * VecDependMap    for provides-Dependencies:  [ [A,[B,K], [B,[C,L], [C,[X] ]
   *
   *         this are an index of the tagList --       --- this are an index of the depList
   *                                            \     /
   *                                             v   v
   *                                           [0]: 1,10
   *                                           [1]: 2,11
   *                                           [2]: 3,21
   *
   */

  //-------------------------------
  // for all provides Dependencies do:
  //-------------------------------
  for ( ptPos =  rawProvidesDependencies.begin();
	ptPos != rawProvidesDependencies.end();   ++ptPos)
  {
     int  		providesIndex;
     int  		providesItemIndex;
     int  		itselfItemIndex     = -1;

     ok = GetPackIndex( ptPos->first, providesIndex);

     if (ok)
     {
	// every package provides itself:
	 ok = GetTagIndex( depList[providesIndex].Name(), providesItemIndex);

	 if (ok)
	 {
	    provideArray[ providesItemIndex ].insert(providesIndex);
	    itselfItemIndex = providesItemIndex;
	 }
	 else   y2error( "invalid-PROVIDES-dependenc., itself" );


	// bulding a index of all PROVIDES-Dependencies [B,K] -> [1,10]
	for ( tPos =  ptPos->second.begin();
	      tPos != ptPos->second.end();   ++tPos)
	{
	   ok = GetTagIndex( *tPos, providesItemIndex);
	   if ( ok )
	   {
	       if ( providesItemIndex != itselfItemIndex )
	       {
		   provideArray[ providesItemIndex ].insert(providesIndex);
	       }
	   }
	   else
	   {
	       y2error( "invalid-PROVIDES-dependenc. %s", ptPos->first.name().c_str() );
	   }
	}
     }
  }

  rawProvidesDependencies.clear();

  y2debug( "### Index requires");

  /*------------------------------------------------------------------------------------
   * VecDependMap    for requires-Dependencies:  [ [A,[B,K], [B,[C,L], [C,[X] ]
   *
   *       this are an index of the _depList_ --       --- this are an index of the _tagList_
   *                                            \     /
   *                                             v   v
   *                                           [0]: 1,10
   *                                           [1]: 2,11
   *                                           [2]: 3,21
   *
   *
   */

  //-------------------------------
  // for all requires Dependencies do:
  //-------------------------------

  for ( pdPos =  rawRequiresDependencies.begin();
	pdPos != rawRequiresDependencies.end();   ++pdPos)
  {
      int  		requiresIndex;
      int  		requiresItemIndex;

      ok = GetPackIndex( pdPos->first, requiresIndex);

      if (ok)
      {

	  // bulding a index of all Require-Dependencies [B,K] -> [1,10]
	  for ( dPos =  pdPos->second.begin();
		dPos != pdPos->second.end();   ++dPos)
	  {
	      string tagname = dPos->name;
	      string compareString = tagname.substr( 0, 7 );
	      if ( compareString != "rpmlib(" )
	      {
		  // no rpmlib requires
		  ok = GetTagIndex( dPos->name, requiresItemIndex);
		  if (ok)
		  {
		      requireArray[ requiresIndex ].insert(requiresItemIndex);
		  }
		  else
		  {
		    y2warning( "drop requires '%s'", pdPos->first.name().c_str() );
		  }
	      }
	  }
      }
  }

  if ( rawPackageInfo != NULL &&
       readRawInfo )
  {
     // reading direct from pkginfo-server
      int  		requiresItemIndex;
      int  		requiresIndex;

      for ( requiresIndex = 0;
	    requiresIndex < packNumber;
	    requiresIndex++ )
      {
	  PackageKey packageKey = GetPackageKeyFromDepList( requiresIndex );
	  DependList dummyMap = rawPackageInfo->getRequires( packageKey.name() );

	  for ( dPos =  dummyMap.begin();
		dPos != dummyMap.end();   ++dPos)
	  {
	      string tagname = dPos->name;
	      string compareString = tagname.substr( 0, 7 );
	      if ( compareString != "rpmlib(" )
	      {
		  // no rpmlib requires
		  ok = GetTagIndex( dPos->name, requiresItemIndex);
		  if (ok)
		  {
		      requireArray[ requiresIndex ].insert(requiresItemIndex);
		  }
		  else
		  {
		    y2warning( "drop requires '%s' from package '%s'",
			       tagname.c_str(),
			       packageKey.name().c_str() );
		  }
	      }
	  }
      }
  }

  rawRequiresDependencies.clear();

  y2debug( "### Index conflicts");

  /*------------------------------------------------------------------------------------
   * VecDependMap    for conflicts-Dependencies:  [ [A,[B,K], [B,[C,L], [C,[X] ]
   *
   *       this are an index of the _depList_ --       --- this are an index of the _depList_
   *                                            \     /
   *                                             v   v
   *                                           [0]: 1,10
   *                                           [1]: 2,11
   *                                           [2]: 3,21
   *
   *
   */

  //-------------------------------
  // for all conflicts Dependencies do:
  //-------------------------------

  if ( rawPackageInfo != NULL &&
       readRawInfo )
  {
     // reading from pkginfo-server
      PackDepLMap dummyMap = rawPackageInfo->getRawConflictsDependency();
      PackDepLMap::iterator pos;

      for ( pos = dummyMap.begin(); pos != dummyMap.end(); pos++ )
      {
	  PackageKey packageKey = pos->first;
	  DependList depList = pos->second;
	  rawConflictsDependencies.insert(pair<PackageKey,
					  DependList>( packageKey,
						       depList ));
      }
  }

  for ( pdPos =  rawConflictsDependencies.begin();
	pdPos != rawConflictsDependencies.end();   ++pdPos)
  {
     int  		conflictsIndex;

     ok = GetPackIndex( pdPos->first, conflictsIndex);

     if (ok)
     {
	// bulding a index of all Conflict-Dependencies [B,K] -> [1,10]
	for ( dPos =  pdPos->second.begin();
	      dPos != pdPos->second.end();   ++dPos)
	{
	   int         listSize1, index;
	   vector<int> list1;

	   ok = FindPackIndexes( dPos->name, list1, listSize1 );

	   if ( ok )
	   {
	      for( index=0; index<listSize1; index++)
	      {
		 conflictArray[ conflictsIndex ].insert(list1[index]);
	      }
	   }
	}
     }
  }

  rawConflictsDependencies.clear();

  y2debug( "### Index obsoletes");
  /*------------------------------------------------------------------------------------
   * VecDependMap    for obsolete-Dependencies:  [ [A,[B,K], [B,[C,L], [C,[X] ]
   *
   *         this are an index of the depList --       --- this are an index of the depList
   *                                            \     /
   *                                             v   v
   *                                           [0]: 1,10
   *                                           [1]: 2,11
   *                                           [2]: 3,21
   *
   * !! Note !! we get an pakage to tag, dependencie, but regarding to RPM
   *            the "obsolete" action is done, if the tag the NAME of a INSTALLED package
   *            this tag has nothing to do with the provide tag!
   */

  //-------------------------------
  // for all obsolete Dependencies do:
  //-------------------------------

  // - first step: for packages, wich will be installed

  if ( rawPackageInfo != NULL &&
       readRawInfo )
  {
     // reading from pkginfo-server
      PackTagLMap dummyMap = rawPackageInfo->getRawObsoletesDependency();
      PackTagLMap::iterator pos;

      for ( pos = dummyMap.begin(); pos != dummyMap.end(); pos++ )
      {
	  PackageKey packageKey = pos->first;
	  TagList tagList = pos->second;
	  rawObsoletesDependencies.insert(pair<PackageKey,
					  TagList>( packageKey,
						    tagList ));
      }
  }

  for ( ptPos =  rawObsoletesDependencies.begin();
	ptPos != rawObsoletesDependencies.end();   ++ptPos)
  {
     int  		obsoleteIndex;
     vector<int>  	obsoleteItemIndexList;
     int 		listSize;

     ok = GetPackIndex( ptPos->first, obsoleteIndex);

     if (ok)
     {

	// bulding a index of all OBSOLETE-Dependencies [B,K] -> [1,10]
	for ( tPos =  ptPos->second.begin();
	      tPos != ptPos->second.end();   ++tPos)
	{
	   ok = FindObsoletingPackIndexes( *tPos, obsoleteItemIndexList, listSize, obsoleteIndex);

	   if ( ok )
	   {
	      for ( i=0; i < listSize; ++i)
	      {
		 obsoleteArray[ obsoleteIndex ].insert(obsoleteItemIndexList[i]);
	      }
	   }
	}
     }
  }

  // - first step: for packages, which are installed

  for ( ptPos =  rawObsoletesDependencies.begin();
	ptPos != rawObsoletesDependencies.end();   ++ptPos)
  {
     int  		obsoleteIndex;
     vector<int>  	obsoleteItemIndexList;
     int 		listSize;

     ok = GetPackIndex( ptPos->first, obsoleteIndex);

     if (ok)
     {

	// bulding a index of all OBSOLETE-Dependencies [B,K] -> [1,10]
	for ( tPos =  ptPos->second.begin();
	      tPos != ptPos->second.end();   ++tPos)
	{
	   ok = FindPackIndexes( *tPos, obsoleteItemIndexList, listSize);

	   if ( ok )
	   {
	      for ( i=0; i < listSize; ++i)
	      {
		 instObsoleteArray[ obsoleteIndex ].insert(obsoleteItemIndexList[i]);
	      }
	   }
	}
     }
  }

  rawObsoletesDependencies.clear();

  y2milestone( "### Creating Solver end" );

}



/*--------------------------------------------------------------*/
/* Cleans up						       	*/
/*--------------------------------------------------------------*/
Solver::~Solver()
{
}



/*--------------------------------------------------------------*/
/* Set a current selection to the sovler, and solve the		*/
/* dependencies for that selection				*/
/*--------------------------------------------------------------*/





void Solver::SolveDependencies( PackVersList              &additionalPackages,
				TagPackVersLMap           &unsolvedRequirements,
				PackPackVersLMap          &conflictMap,
				ObsoleteList          &obsoleteMap )
{
   SolveDependencies( currSelectionList,
		      additionalPackages,
		      unsolvedRequirements,
		      conflictMap,
		      obsoleteMap );
}

void Solver::SolveDependencies( const PackVersList        &packageList,
				PackVersList              &additionalPackages,
				TagPackVersLMap           &unsolvedRequirements,
				PackPackVersLMap          &conflictMap,
				ObsoleteList              &obsoleteMap )
{

   y2debug( "### Start start" );

   additionalPackages.clear();
   unsolvedRequirements.clear();
   conflictMap.clear();
   obsoleteMap.clear();


   int i     = 0;

   IndexedUnsolvedMap  indexedUnsolvedMap; // current Unsolveds

   currSelectionList = packageList;

   /*--------------------------------------------------------
    * Solve provide/requires Dependencies:
    *------------------------
    * For all packages in the packageList:
    * find this package in depList and try to saturate it
    *--------------------------------------------------------*/

   // clear old deps
   for ( i=0; i<packNumber; i++)
   {
      depList[i].SetUsedBy(0);
      depList[i].SetInSelection(false);
   }


   y2debug( "### Init start" );

   for (PackVersList::const_iterator pos = packageList.begin();
	pos != packageList.end(); ++pos)
   {

      int  index = 0;
      bool announced;

      announced = GetPackIndex( *pos, index );

      if ( announced )
      {
	 // mark package (needed for generating the output list)
	 depList[index].SetInSelection(true);

	 // Do the dependencies
	 SaturatePackage( index, indexedUnsolvedMap );
      }
      else
      {
	 // if the package isn't announced: log it
	 // shouldn't happen!

	 y2error( "unregistered package %s %s", pos->name().c_str(), pos->version().c_str() );
      }
   }

   y2debug( "### Del prev inserted start" );

   /////////////////////////////////////////////////
   //  I have to look if previous inserted choises are
   //  solved by later solved packages
   //
   //  	  prov  req
   //  	1::x
   //  	2::-    <- x
   //  	3::x,y  <- z
   //  	4::-    <- x,y
   //  						  sum:
   //  	select 2 ->    choose[ x:[ 1,3]]    -> choose[ x:[ 1,3]]
   //  		       add []                  add []
   //
   //  	select 4 ->    choose[ ]           ->  choose[ x:[ 1,3]]
   //  		       add [ 3 ]               add [ 3 ]
   //
   //  	     now sum is wrong, cause the selection of 4 made
   //  	     the choise x:[1,3] useless, we have NOW 3 so, we must
   //  	     throw away this choice


   IndexedUnsolvedMap::iterator unsolved = indexedUnsolvedMap.begin();

   while ( unsolved != indexedUnsolvedMap.end() )
   {
      bool now_solved = false;

      // check if the tag unsolved->first is NOW provided
      // y2debug( "UNSOLVED Tag %d", unsolved->first );

      set<int>::iterator pack_index;

      for ( pack_index =  unsolved->second.begin();
	    pack_index != unsolved->second.end();    ++pack_index )
      {
         // y2debug( "              - %d", *pack_index );
	 if ( depList[*pack_index].UsedBy() > 0) now_solved = true;
      }


      if ( now_solved )
      {
	 // y2debug( "____________DELETE  %d", unsolved->first );

	 IndexedUnsolvedMap::iterator old_unsolved = unsolved;
	 ++unsolved;
	indexedUnsolvedMap.erase(old_unsolved);
      }
      else
      {
	 ++unsolved;
      }
   }

   y2debug( "### Conflicts start" );

   ////////////////////////////////////////////////////////////////
   //  Conflicts
   for ( i=0; i<packNumber; i++)
   {
      // for all packages which will be installed or are already installed  ...
      if ( depList[i].UsedBy() > 0
	   || depList[i].IsInstalled() )
      {

	 set<int>  curr               = conflictArray[i];
	 PackVersList  currConflictList;

	 set<int>::iterator cpos;

	 for ( cpos =  curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    if ( depList[*cpos].UsedBy() > 0
		|| depList[*cpos].IsInstalled()  )
	    {
	       y2debug( "add conflict %s", depList[*cpos].Name().c_str());

	       currConflictList.push_back( GetPackageKeyFromDepList( *cpos ) );
	    }
	 }

	 if ( !currConflictList.empty() )
	 {
	    // insert
	    conflictMap.insert(PackPackVersLMap::
			       value_type(PackageKey(depList[i].Name(),
						     depList[i].Version()),
					  currConflictList));
	 }
      }
   }


   ////////////////////////////////////////////////////////////////
   //  Obsoletes
   // we have to see installed packages, where obsoletes depend on the
   // sequence of installation
   // if A obs B
   //  i)   if nothing is installed, then I install A , then B nothing happens!
   //  ii)  if nothing is installed, then I install B , then A -> A obsoletes B!
   //  iii) if B is already installed, then A is installed     -> A obsoletes B!
   //
   // difference between i) and ii) is handled in obsoleteArray,
   // iii) is handeled in instObsoleteArray

   y2debug( "### Obsoletes start" );

   for ( i=0; i<packNumber; i++)
   {
      // for all packages which will be installed ...
      if ( depList[i].UsedBy() > 0)
      {

	 ////////////////////////////////
	 // now we handle i) and ii)
	 ////////////////////////////////

	 set<int>  curr = obsoleteArray[i];

	 set<int>::iterator cpos;

	 // Now we have a look at all packages depList[i] obsoletes ...
	 for ( cpos =  curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    //   !!!
	    //   v
	    if ( ! depList[*cpos].IsInstalled()
		 && depList[*cpos].UsedBy() > 0 )
	    {
	       ObsoleteStruct curr_obs;

	       PackageKey key1 =  GetPackageKeyFromDepList( i     );
	       PackageKey key2 =  GetPackageKeyFromDepList( *cpos );

	       curr_obs.obsoletes          = key1.name();
	       curr_obs.obsoletesVersion   = key1.version();
	       curr_obs.isObsoleted        = key2.name();
	       curr_obs.isObsoletedVersion = key2.version();

	       obsoleteMap.push_back( curr_obs );

	       // todo
	       // which packages depend from the "is_obsoleted" and "obsoletes"
	       // we have pick those packages, which are no longer saturated, if
	       // one deselects the "obsoletes" or the "is_obsoleted"
	       // that means, look for the tags which are unique to one of the both,
	       // and determin all packages which requires direct or indirect those
	       // tags
	    }
	 }

	 ////////////////////////////////
	 // now we handle iii)
	 ////////////////////////////////

	 curr = instObsoleteArray[i];

	 // Now we have a look at all packages depList[i] obsoletes ...
	 for ( cpos =  curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    if ( depList[*cpos].IsInstalled() )
	    {
	       ObsoleteStruct curr_obs;

	       PackageKey key1 =  GetPackageKeyFromDepList( i     );
	       PackageKey key2 =  GetPackageKeyFromDepList( *cpos );

	       curr_obs.obsoletes          = key1.name();
	       curr_obs.obsoletesVersion   = key1.version();
	       curr_obs.isObsoleted        = key2.name();
	       curr_obs.isObsoletedVersion = key2.version();

	       obsoleteMap.push_back( curr_obs );

	       // todo
	       // which packages depend from the "is_obsoleted" and "obsoletes"
	       // we have pick those packages, which are no longer saturated, if
	       // one deselects the "obsoletes" or the "is_obsoleted"
	       // that means, look for the tags which are unique to one of the both,
	       // and determin all packages which requires direct or indirect those
	       // tags
	    }
	 }

      }
   }



   ////////////////////////////////////////////////////////////////
   // Build output lists:
   // - list of additionalPackages:
   //   look for all used_by > 0 AND NOT IsSelected
   // - unsolvedRequirements:
   //   transform indexedUnsolvedMap
   ////////////////////////////////////////////////////////////////

   // additionalPackages
   y2debug( "### Build Output start" );

   additionalPackages.clear();

   for ( i=0; i<packNumber; i++)
   {
      PackStat curr_pack_stat = depList[i];

      if (    !curr_pack_stat.InSelection()
	   && !curr_pack_stat.IsInstalled()
	   && curr_pack_stat.UsedBy() > 0 )
      {
	 PackageKey packagekey(  curr_pack_stat.Name(), curr_pack_stat.Version());

	 additionalPackages.push_back( packagekey );
      }
   }


   // unsolvedRequirements

   unsolvedRequirements.clear();

   for ( unsolved =  indexedUnsolvedMap.begin();
	 unsolved != indexedUnsolvedMap.end();    ++unsolved )
   {
      set<int>::iterator pack_index;

      PackVersList packlist;

      for ( pack_index =  unsolved->second.begin();
	    pack_index != unsolved->second.end();    ++pack_index )
      {
	 PackageKey packagekey( depList[*pack_index].Name(),  depList[*pack_index].Version() );

	 packlist.push_back(  packagekey );
      }

      unsolvedRequirements.insert( map<string,PackVersList>::
				   value_type(tagList[unsolved->first],packlist ));
   }

   y2debug( "#### Build Output end" );




   /////////////////////////////////////////////////////////////////////////////
   // Now only debug code follows: ...

   if ( debug_all )
   {
      y2debug( "size req %d vs %d  ,  prov %d vs %d ", packNumber, requireArray.size(),
	       tagNumber, provideArray.size());

      char buffer[100];

      y2debug( "-----" );
      y2debug( "        Tags |   Packages" );

      for ( i=0; i<tagNumber; i++)
      {

	 sprintf( buffer,  "provArr: %02d  : ", i);
	 string outp = buffer;

	 set<int> curr = provideArray[i];

	 set<int>::iterator cpos;

	 for ( cpos = curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    sprintf( buffer,  " %d", *cpos );
	    outp = outp + string(buffer);
	 }
	 y2debug( outp.c_str());
      }
      y2debug( "----" );
      y2debug( "        Pack |   Tags " );

      for ( i=0; i<packNumber; i++)
      {
	 sprintf( buffer,  "requArr: %02d  : ", i);
	 string outp = buffer;

	 set<int> curr = requireArray[i];

	 set<int>::iterator cpos;

	 for ( cpos = curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    sprintf( buffer,  " %d", *cpos );
	    outp = outp + string(buffer);
	 }

	 y2debug( outp.c_str());
      }

      y2debug( "        Pack |   Tags " );

      for ( i=0; i<packNumber; i++)
      {
	 sprintf( buffer,  "confArr: %02d  : ", i);
	 string outp = buffer;

	 set<int> curr = conflictArray[i];

	 set<int>::iterator cpos;

	 for ( cpos = curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    sprintf( buffer,  " %d", *cpos );
	    outp = outp + string(buffer);
	 }

	 y2debug( outp.c_str());
      }

      y2debug( "        Pack |   Pack " );

      for ( i=0; i<packNumber; i++)
      {
	 sprintf( buffer,  "obsoArr: %02d  : ", i);
	 string outp = buffer;

	 set<int> curr = obsoleteArray[i];

	 set<int>::iterator cpos;

	 for ( cpos = curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    sprintf( buffer,  " %d", *cpos );
	    outp = outp + string(buffer);
	 }

	 y2debug( outp.c_str());
      }

      y2debug( "        Pack |   Pack " );

      for ( i=0; i<packNumber; i++)
      {
	 sprintf( buffer,  "in_obAr: %02d  : ", i);
	 string outp = buffer;

	 set<int> curr = instObsoleteArray[i];

	 set<int>::iterator cpos;

	 for ( cpos = curr.begin();
	       cpos != curr.end();  ++cpos )
	 {
	    sprintf( buffer,  " %d", *cpos );
	    outp = outp + string(buffer);
	 }

	 y2debug( outp.c_str());
      }

      y2debug( "----" );

      for ( i=0; i<packNumber; i++)
      {
	 PackStat pa = depList[i];
	 y2debug( "depList: %d:  used %d  sel %s inst %s  name %s", i, pa.UsedBy(), pa.InSelection() ? "X":"-", pa.IsInstalled() ? "X":"-",
		  pa.Name().c_str());
      }
   }

}



/*--------------------------------------------------------------*/
/* try to saturate package in depList				*/
/*--------------------------------------------------------------*/
// always do this:
//
//   mark it as used_by, cause here we know we need this package
//
//   look for the required tags:
//   for every tag do:
//   	 look if one of the packages, which provide this
//   	 tag will be already installed (used_by != 0 || isInstalled)
//
//   	 If there is one:
//   	    inc its used_by counter and all is done
//   	 If there is none
//   	     look if the list has exactly one entry:
//   	     one entry:
//   		set this used_by to 1 (it was 0) (done by saturate)
//   		try to saturate this package
//   	     more entrys:
//   		add this entry to unsolvedRequirements
//
//
/*--------------------------------------------------------------*/


void Solver::SaturatePackage( int packageIndex, IndexedUnsolvedMap &indexedUnsolvedMap )
{
   set<int>::iterator curr_tag_idx;

   string debug_stack = "";

   if ( debug_sat ) y2debug( "Start Saturate: %s %d (is %s) ", debug_stack.c_str(), packageIndex, depList[packageIndex].Name().c_str() );

   depList[packageIndex].IncUsedBy();

   for (curr_tag_idx =  requireArray[ packageIndex ].begin();
	curr_tag_idx != requireArray[ packageIndex ].end();   ++curr_tag_idx )
   {
      // curr_tag_idx one of the required tags
      if ( debug_sat ) y2debug( "Start Saturate: %s %d  requires %d ", debug_stack.c_str(), packageIndex, *curr_tag_idx );

      //////////////////////////////////////////
      // Now we try to solve this requirment ...

      bool is_solved = false;
      set<int>::iterator curr_pack_idx;

      for (curr_pack_idx =  provideArray[ *curr_tag_idx ].begin();
	   curr_pack_idx != provideArray[ *curr_tag_idx ].end();   ++curr_pack_idx )
      {
	 // curr_pack_idx is one of the packages wich provides curr_tag_idx

	 if ( !is_solved
	      && ((depList[*curr_pack_idx].UsedBy() > 0)
		  || depList[*curr_pack_idx].IsInstalled()) )
	 {
	    // there is already a added/selected package that provides this tag
	    is_solved = true;
	    depList[*curr_pack_idx].IncUsedBy();
	 }

      }
      if ( debug_sat ) y2debug( "Start Saturate: %s %d  requires %d is already provided: %s  ",
				debug_stack.c_str(), packageIndex, *curr_tag_idx, is_solved ? "YES":"NO" );


      if ( ! is_solved )
      {
	 // there is NO added/selected package that already provides this tag
	 if ( provideArray[ *curr_tag_idx ].size() == 1 )
	 {
	    // only one package provides the tag -> we add it
	   if ( debug_sat )  y2debug( "Start Saturate: %s %d  requires %d WE ADD IT start saturate",  debug_stack.c_str(), packageIndex, *curr_tag_idx );
	    debug_stack = debug_stack + "  ";
	    SaturatePackage(  *(provideArray[ *curr_tag_idx ].begin()), indexedUnsolvedMap );
	    debug_stack = debug_stack.substr( 0, debug_stack.size()-2);
	   if ( debug_sat )  y2debug( "Start Saturate: %s %d  requires %d WE ADD IT end   saturate",  debug_stack.c_str(), packageIndex, *curr_tag_idx);
	 }
	 else
	 {
	    if ( debug_sat ) y2debug( "Start Saturate: %s %d  requires %d WE ADD  -- UNSOLVED --", debug_stack.c_str(), packageIndex, *curr_tag_idx);
	    indexedUnsolvedMap.insert(map<int,set<int> >::
				      value_type( *curr_tag_idx,provideArray[ *curr_tag_idx ]));
	 }
      }
   }

   return;
}


/*--------------------------------------------------------------*/
/* Add a package to the current selection			*/
/* and solve the dependencies for the current selection		*/
/*--------------------------------------------------------------*/
void Solver::AddPackage( const PackageKey    packageName,
			 PackVersList              &additionalPackages,
			 TagPackVersLMap           &unsolvedRequirements,
			 PackPackVersLMap          &conflictMap,
			 ObsoleteList              &obsoleteMap )
{
   currSelectionList.push_back(packageName);

   SolveDependencies( currSelectionList,
		      additionalPackages,
		      unsolvedRequirements,
		      conflictMap,
		      obsoleteMap );
}


void Solver::AddPackageList( const PackVersList    packageList,
			     PackVersList          &additionalPackages,
			     TagPackVersLMap       &unsolvedRequirements,
			     PackPackVersLMap      &conflictMap,
			     ObsoleteList          &obsoleteMap )
{
   for (PackVersList::const_iterator pos = packageList.begin();
	pos != packageList.end(); pos++ )
   {
      currSelectionList.push_back(*pos);
   }

   SolveDependencies( currSelectionList,
		      additionalPackages,
		      unsolvedRequirements,
		      conflictMap,
		      obsoleteMap );

}

// todo  deselectDeleted as new Call


/*--------------------------------------------------------------*/
/* Delete a package from the current selection			*/
/* OR a installed package will be deleted		        */
/* and solve the dependencies for the current selection		*/
/*--------------------------------------------------------------*/

void Solver::DeletePackage( const PackageKey   selection,
			    PackVersList          	&additionalPackages,
			    TagPackVersLMap       	&unsolvedRequirements,
			    PackPackVersLMap      	&conflictMap,
			    ObsoleteList      	&obsoleteMap )
{


   PackVersList::iterator pos;

   pos = find( currSelectionList.begin(), currSelectionList.end(), selection);

   if ( pos != currSelectionList.end() )
   {
      y2debug( "delete %s from curr_list %d", selection.name().c_str(), currSelectionList.size() );
      currSelectionList.erase( pos );
      y2debug( "size of selected: %d",  currSelectionList.size());
   }
   else
   {
      // "delte" a already selected
      int index;
      bool ok = GetPackIndex( selection, index );

      if ( ok )
      {
	 y2debug( "delete %s from depList", selection.name().c_str() );
	 depList[index].SetIsInstalled( false );
      }
      else
      {
	 y2error( "cannot delete %s: unknown", selection.name().c_str() );
      }
   }

   SolveDependencies( currSelectionList,
		      additionalPackages,
		      unsolvedRequirements,
		      conflictMap,
		      obsoleteMap );

}



/*--------------------------------------------------------------*/
/* IgnoreConflictByKey()					*/
/*   Delete all conflict dependencies regarding the mentioned	*/
/*   packages 							*/
/*--------------------------------------------------------------*/


void Solver::IgnoreConflictByKey( const PackageKey packagekey1, const PackageKey packagekey2 )
{
   int   packageNameNumber1;
   int   packageNameNumber2;
   bool  ok1;
   bool  ok2;

   ok2 = GetPackIndex( packagekey1, packageNameNumber1 );
   ok1 = GetPackIndex( packagekey2, packageNameNumber2 );

   if (ok1 && ok2)
   {
      // remove the Dep, if it exist in this rule
      conflictArray[packageNameNumber1].erase( packageNameNumber2 );
      conflictArray[packageNameNumber2].erase( packageNameNumber1 );
   }

}



/*--------------------------------------------------------------*/
/* IgnoreConflict()						*/
/*   Delete all conflict dependencies regarding the mentioned	*/
/*   packages ( deletes conflicts for all versions) 		*/
/*--------------------------------------------------------------*/

void Solver::IgnoreConflict( const string package1, const string package2 )
{
   int         listSize1;
   vector<int> list1;
   int         listSize2;
   vector<int> list2;
   bool        ok1;
   bool        ok2;
   int i,j;

   ok2 =  FindPackIndexes( package1, list1, listSize1 );
   ok1 =  FindPackIndexes( package2, list2, listSize2 );

   y2debug( "ignore: ok1 %s ok2 %s :", ok1?"ok":"err", ok2?"ok":"err");

   if (ok1 && ok2)
   {
      for( i=0; i<listSize1; i++)
      {
	 for ( j=0; j<listSize2; j++ )
	 {
	    y2debug( "ignore: %d %d",list1[i],  list2[j] );

	    // remove the Dep, if it exist in this rule
	    conflictArray[ list1[i] ].erase( list2[j] );
	    conflictArray[ list2[j] ].erase( list1[i] );
	 }
      }
   }
}



/*--------------------------------------------------------------*/
/* IgnoreUnsolvedRequirements()					*/
/*   Delete all requires dependencies to the regarding tag	*/
/*   for all current selected packages				*/
/*--------------------------------------------------------------*/

void Solver::IgnoreUnsolvedRequirements( const string tag )
{
   int           tagIndex;
   bool          ok;
   unsigned int  i;

   ok = GetTagIndex( tag, tagIndex );

   if (ok)
   {
      // over all packages
      for ( i=0; i<depList.size(); i++)
      {
	 // if they are selected
	 if ( depList[i].UsedBy() > 0
	      || depList[i].IsInstalled() )
	 {
	    // remove the AND-Dep, if it exist in this rule
	    requireArray[i].erase( tagIndex );
	 }
      }
   }
}


void Solver::IgnoreAdditionalPackages( const string packagename )
{
   bool          ok;
   int           i;
   unsigned int  j,k;
   int         listSize;
   vector<int> list;

   ok = FindPackIndexes( packagename, list, listSize );


   // for all packages
   for ( i=0;
	 i<listSize;
	 i++)
   {
      // look for all provided tags:
      // if one tag is provided ONLY by current package then
      // I want to ignore it:
      //   I delete requires from all selected and needed packages

      for ( j=0; j<provideArray.size(); j++)
      {
	 if ( (*provideArray[j].begin()) == list[i]
	      && provideArray[j].size() == 1 )
	 {
	    // over all packages
	    for ( k=0; k<depList.size(); k++)
	    {
	       // if they are selected
	       if ( depList[k].UsedBy() > 0
		    || depList[k].IsInstalled() )
	       {
		  // remove the Dep, if it exist in this rule
		  requireArray[k].erase( j );
	       }
	    }
	 }
      }
   }
}


/*--------------------------------------------------------------*/
/*  Simulates the delete of the package "packagekey" and	*/
/*  returns all  selected packages, that have then unfullfilled	*/
/*  dependencies						*/
/*--------------------------------------------------------------*/

void Solver::GetBreakingPackageList(  const PackageKey    packagekey,
			      PackVersList        &additionalPackages )
{
   int  packageNameNumber;
   bool ok;
   int  i;
   int  packNumber = depList.size();

   additionalPackages.clear();

   ////////////////////////////
   // set Isbroken in all packages to false

   for ( i=0; i<packNumber; i++)
   {
      depList[i].SetIsBroken(false);
   }


   ok = GetPackIndex( packagekey, packageNameNumber );

   if (ok)
   {
      depList[packageNameNumber].SetIsBroken(true);

      ///////////////////////////////////////////////////
      BrokenPackage( packageNameNumber, "" );
      ///////////////////////////////////////////////////


      ////////////////////////////////////////////////////////////////
      // Build output lists:
      // - list of additionalPackages:
      //   look for all used_by > 0 AND NOT IsSelected
      // - unsolvedRequirements:
      //   transform indexedUnsolvedMap
      //
      // todo installed: is it ok that they break?
      ////////////////////////////////////////////////////////////////

      // additionalPackages
      y2debug( "### Build Output Breaking Package List" );


      for ( i=0; i<packNumber; i++)
      {
	 PackStat curr_pack_stat = depList[i];

	 if (    ( curr_pack_stat.InSelection() || curr_pack_stat.IsInstalled() )
		 && curr_pack_stat.IsBroken() )
	 {
	    PackageKey packagekey(  curr_pack_stat.Name(), curr_pack_stat.Version());

	    additionalPackages.push_back( packagekey );
	 }
      }

   }

   y2debug( "----" );

   for ( i=0; i<packNumber; i++)
   {
      PackStat pa = depList[i];
      y2debug( "depList: %d:  used %d  sel %s inst %s %s name %s", i, pa.UsedBy(), pa.InSelection() ? "X":"-", pa.IsInstalled() ? "X":"-",
	       pa.IsBroken()?"broken ":"not-brk", pa.Name().c_str()  );
   }
   return;
}



/*--------------------------------------------------------------*/
/* INTERNAL:							*/
/*--------------------------------------------------------------*/


/*--------------------------------------------------------------*/
/* Solver:BrokenPackage  					*/
/*--------------------------------------------------------------*/
/*  Simulates the removal of the package "packageIndex" and	*/
/*  marks them as broken in depList 				*/
/*--------------------------------------------------------------*/

void Solver::BrokenPackage( const int packageIndex, string debug_stack )
{

   if ( debug_sat ) y2debug( "Start detect broken: %s broken: %d (is %s) ",
			     debug_stack.c_str(), packageIndex, depList[packageIndex].Name().c_str() );

   depList[packageIndex].SetIsBroken(true);

   set<int>	      packageProvides; // all tags which are provided by packageIndex
   set<int>::iterator curr_pack_idx;   // current package in provide line
   unsigned int       curr_tag_idx;    // current provide "line"
   set<int>::iterator curr_prtag_idx;  // current provide in packageProvides
   set<int>	      lostPackageProvides; // all tags which are provided by packageIndex
                                           // and lost

   // build packageProvides
   //   Look for all tags that are provided by Package packageIndex (Px)
   //   -> Tx1 .. Txn     { Tx1 .. Txn } == packageProvides

   for ( curr_tag_idx=0; curr_tag_idx<provideArray.size(); curr_tag_idx++)
   {
      for (curr_pack_idx =  provideArray[ curr_tag_idx ].begin();
	   curr_pack_idx != provideArray[ curr_tag_idx ].end();   ++curr_pack_idx )
      {
         if ( *curr_pack_idx == packageIndex )
	 {
	    packageProvides.insert( curr_tag_idx );
	 }
      }
   }

   // If we deselect Px we probably will loose the Tags Tx1 .. Txn, if
   // no other package provides them so ..
   //
   //
   // Look if we loose such a tag:
   // For each of Tx1..Txn do:          Tx(i) == curr_prtag_idx

   for (curr_prtag_idx =   packageProvides.begin();
	curr_prtag_idx !=  packageProvides.end();   ++curr_prtag_idx )
   {
      if ( debug_sat ) y2debug( "Probably loose tag : %s %d (is %s) ",
				debug_stack.c_str(), *curr_prtag_idx, tagList[*curr_prtag_idx].c_str() );

      if ( provideArray[ *curr_prtag_idx ].size() == 1 )
      {
	 // we loose curr_prtag_idx
	    lostPackageProvides.insert( *curr_prtag_idx );

	    // debug_sat ) y2debug( "Probably loose XY  : %s %d (is %s) ",
	    //		  debug_stack.c_str(), *curr_prtag_idx, tagList[*curr_prtag_idx].c_str() );
      }
      else
      {
	 // we have a closer look to the other packages that provide curr_prtag_idx:
	 //
	 //  let {Py1 ..Pyn} be the set of all packages Pyj which provides ALSO Txi
   	 //    if there is an Pyi, which is selected or automaticly
   	 // 	  selected, and is not broken so far(**)
   	 // 			--------------------
   	 //    then
   	 // 	  we do not loose Txi
   	 //    else
   	 // 	  we loose Txi

	 bool we_loose_Txi = true;

	 for (curr_pack_idx =  provideArray[ *curr_prtag_idx ].begin();
	      curr_pack_idx != provideArray[ *curr_prtag_idx ].end();   ++curr_pack_idx )
	 {
	    if (     depList[*curr_pack_idx].UsedBy()  > 0
		 && !depList[*curr_pack_idx].IsBroken()    )
	    {
	       // we do not loose Txi
	       we_loose_Txi = false;
	    }

	    // debug_sat ) y2debug( "Probably loose XX  : %s %d (is %s) - used %d %s %s  ",
	    //			debug_stack.c_str(), *curr_pack_idx, depList[*curr_pack_idx].Name().c_str(),
	    //			depList[*curr_pack_idx].UsedBy(), depList[*curr_pack_idx].IsBroken()?"broken":"not-broken",
	    //			we_loose_Txi?"loose":"no-loose");

	 }

	 if ( we_loose_Txi ) lostPackageProvides.insert( *curr_prtag_idx );
      }
   }

   //---------------------------------------------------------------------
   // We now have a set of tags we loose: { Ty1 .. Tyn } if we loose Txi
   //
   //  { Ty1 .. Tyn } == lostPackageProvides
   //
   // For all Tyi from  { Ty1 .. Tyn } do:
   //    Look for all package Pz that require Tyi
   //    -> every Pz breaks!
   //      for all Pz1 to Pzn do
   //	  if Pzi is selected or automaticly selected and not broken
   //	  	run BrokenPackage( Pzi )
   //		    --------------------

   set<int>::iterator  curr_losttag_idx;
   set<int>::iterator  curr_rqtag_idx;

   for (curr_losttag_idx =   lostPackageProvides.begin();
	curr_losttag_idx !=  lostPackageProvides.end();   ++curr_losttag_idx )
   {
      if ( debug_sat ) y2debug( "REALLY loose tag   : %s %d (is %s) ",
				debug_stack.c_str(), *curr_losttag_idx, tagList[*curr_losttag_idx].c_str() );

      unsigned int curr_loosepack_idx;

      for ( curr_loosepack_idx=0; curr_loosepack_idx<requireArray.size(); curr_loosepack_idx++)
      {
	 for (curr_rqtag_idx =  requireArray[ curr_loosepack_idx ].begin();
	      curr_rqtag_idx != requireArray[ curr_loosepack_idx ].end();   ++curr_rqtag_idx )
	 {

	    // 	debug_sat ) y2debug( "REALLY loose tag --: %s %d (is %s) %d %d",
	    // 			 debug_stack.c_str(), *curr_losttag_idx, tagList[*curr_losttag_idx].c_str(),
	    // 			 *curr_rqtag_idx, *curr_losttag_idx );

	    if ( *curr_rqtag_idx == *curr_losttag_idx )
	    {
	       // package is "lost" / dependencies are broken

	       if (         depList[curr_loosepack_idx].UsedBy()  > 0
			    && !depList[curr_loosepack_idx].IsBroken()    )
	       {
		  BrokenPackage( curr_loosepack_idx, debug_stack + "  ");
	       }
	    }
	 }
      }
   }
}



/*--------------------------------------------------------------*/
/* Solver::GetPackageKeyFromDepList				*/
/*--------------------------------------------------------------*/
/*--------------------------------------------------------------*/

PackageKey Solver::GetPackageKeyFromDepList( const int index )
{
   return( PackageKey( depList[index].Name(), depList[index].Version() ) );
}


/*--------------------------------------------------------------*/
/* Solver::GetPackIndex						*/
/*--------------------------------------------------------------*/
/* Returns the Index of the package with the name packageName   */
/* in the depList						*/
/*                     						*/
/* in:  packageName						*/
/* out: index   						*/
/* return: true:   Index is valid   				*/
/* return: false:  Index is not valid, package not found        */
/*--------------------------------------------------------------*/

bool Solver::GetPackIndex( const PackageKey packagekey, int &index )
{
   PackStat                   packStat( packagekey );

   map<PackStat,int>::iterator pos;

   pos = depListIndex.find(packStat);

   if ( pos != depListIndex.end() )        // has found something
   {
      index = pos->second;
      return true;
   }
   else
   {
      y2error("Unknown package name: %s",
	    packagekey.name().c_str() );
      index  = 0;
      return false;
   }


}



/*--------------------------------------------------------------*/
/* Solver::FindObsoletingPackIndexes				*/
/*--------------------------------------------------------------*/
/* Returns al list of Indexes of the package with the name      */
/* packagename  in the depList, if their index is lower than	*/
/* packIndex							*/
/* 								*/
/* This behavior reimplements the behavior of RPM, to obsolete	*/
/* previous installed packages, if they are marked as obsolete	*/
/* in a rpm							*/
/*                     						*/
/* return: true:   Index is valid   				*/
/* return: false:  Index is not valid, no packages found        */
/*--------------------------------------------------------------*/

bool Solver::FindObsoletingPackIndexes( const string packagname,
					vector<int>  &list,
					int          &listSize,
					int          &basePackIndex )
{
   // todo installed packages are always obsoleted! not second<base!

   list.clear();

   map<string,int>::iterator pos;

   list.clear();

   for ( pos =  depListStrIndex.lower_bound(packagname);
         pos != depListStrIndex.upper_bound(packagname); ++pos )
   {
      if ( pos->second < basePackIndex )  //
      {
	 list.push_back( pos->second );
      }
   }

   if ( list.size() == 0 )
   {
      listSize = 0;
      return false;
   }
   else
   {
      listSize = list.size();
      return true;
   }
}



/*--------------------------------------------------------------*/
/* Solver::FindPackIndexes					*/
/*--------------------------------------------------------------*/
/* Returns al list of Indexes of the package with the name      */
/* packagename  in the depList, 				*/
/* 								*/
/* return: true:   Index is valid   				*/
/* return: false:  Index is not valid, no packages found        */
/*--------------------------------------------------------------*/

bool Solver::FindPackIndexes( const string packagname,
			      vector<int>  &list,
			      int          &listSize )
{
   map<string,int>::iterator pos;

   list.clear();

   for ( pos =  depListStrIndex.lower_bound(packagname);
         pos != depListStrIndex.upper_bound(packagname); ++pos )
   {
      list.push_back( pos->second );
   }

   if ( list.size() == 0 )
   {
      listSize = 0;
      return false;
   }
   else
   {
      listSize = list.size();
      return true;
   }
}



/*--------------------------------------------------------------*/
/* Solver::GetTagIndex						*/
/*--------------------------------------------------------------*/
/* Returns the Index of the tag with the name packageName   	*/
/* in the depList						*/
/*                     						*/
/* in:  tagname							*/
/* out: index   						*/
/* return: true:   Index is valid   				*/
/* return: false:  Index is not valid, package not found        */
/*--------------------------------------------------------------*/

bool Solver::GetTagIndex( const string tagName, int &index )
{
   map<string,int>::iterator pos;

   pos = tagListIndex.find(tagName);

   if ( pos != tagListIndex.end() )        // has found something
   {
      index = pos->second;
      return true;
   }
   else
   {
      if ( debug_all ) {
	y2debug("Unknown tag name: %s",
		 tagName.c_str() );
      }
      index  = 0;
      return false;
   }
}



void Solver::SetVerboseDebug( const string debugstr )
{
   debug_all = false;
   debug_sat = false;

   y2debug( "SetDebug %s", debugstr.c_str());

   if ( debugstr == "all-sat")
   {
      y2debug( "all-sat");
      debug_all = true;
      debug_sat = true;
   }

   if ( debugstr == "all")
   {
      y2debug( "all");
      debug_all = true;
   }

}




/****************************************************************/
/* little helper class PackStat			      		*/
/*								*/
/* set/get the status of a package				*/
/* - a package can have a "UsedBy" from 0 to X			*/
/*   this means, a package is used by X other packages          */
/*   you can incement, decrement and set this counter		*/
/****************************************************************/

PackStat::PackStat( const PackageKey &packagekey )
  : _UsedBy (0),
    _Name   ( packagekey.name()),
    _Version( packagekey.version() ),
    _InSelection(false),
    _IsInstalled(false),
    _IsBroken(false)
{
}



PackStat::~PackStat()
{
}

bool PackStat::operator < (const PackStat b) const
{
   if (  _Name ==  b.Name() )
   {
       return _Version < b.Version();
   }
   {
      return  _Name    < b.Name();
   }
}

bool PackStat::operator == (const PackStat b) const
{
   if ( (_Name ==  b.Name())   &&   (_Version == b.Version()) )
   {
      return( true );
   }
   else
   {
      return( false);
   }
}



/*--------------------------------------------------------------*/
/*   set"data":   						*/
/*--------------------------------------------------------------*/


void PackStat::IncUsedBy()
{
  _UsedBy += 1;
  return;
}

/*--------------------------------------------------------------*/
void PackStat::DecUsedBy()
{
  if (_UsedBy > 0)
  {
    _UsedBy -= 1;
    return;
  }
  else
  {
    _UsedBy = 0;
    y2error("decrement Used_By: lower than zero");
    // ASSERT(_UsedBy != 0);
  }
}

/*--------------------------------------------------------------*/
void PackStat::SetUsedBy(   int  CurrUsedBy)
{
  _UsedBy  = CurrUsedBy;
  return;
}

/*--------------------------------------------------------------*/
void PackStat::SetInSelection(    bool CurrSelection )
{
  _InSelection   = CurrSelection;
  return;
}

void PackStat::SetIsInstalled(    bool CurrInstalled )
{
  _IsInstalled   = CurrInstalled;
  return;
}

void PackStat::SetIsBroken(    bool CurrBroken )
{
  _IsBroken   = CurrBroken;
  return;
}



/*---------------------------- EOF ------------------------------*/
