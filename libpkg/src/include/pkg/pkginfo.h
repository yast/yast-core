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
 * Description:   main header file for package information retrieval
 *
 * $Header$
 *
 *************************************************************/

/*
 * $Log$
 * Revision 1.1  2002/06/21 16:04:39  arvin
 * Initial revision
 *
 * Revision 1.8  2002/03/28 10:25:06  schubi
 * merge with 8.0 tree
 *
 * Revision 1.7.2.1  2002/03/04 16:21:05  kkaempf
 * major rewrite by ma@suse.de
 * - drastic reduce in memory consumption
 * - drastic increase in speed
 *
 * (approved by schubi@suse.de and kkaempf@suse.de)
 *
 * Revision 1.7  2002/01/08 11:35:09  schubi
 * reading kernel information from common.pkd
 *
 * Revision 1.6  2001/11/08 09:24:29  schubi
 * taking new versions from pkginfo server
 *
 * Revision 1.18  2001/07/04 16:50:47  arvin
 * - adapt for new automake/autoconf
 * - partly fix for gcc 3.0
 *
 * Revision 1.17  2001/07/04 14:25:05  schubi
 * new selection groups works besides the old
 *
 * Revision 1.16  2001/04/23 13:47:41  schubi
 * no more conficts with YaST1 defines
 *
 * Revision 1.15  2001/04/10 15:42:33  schubi
 * Reading dependencies from installed packages via RPM
 *
 * Revision 1.14  2001/04/10 14:11:18  mike
 * now with conflicts and obsoletes
 *
 * Revision 1.13  2001/04/09 13:55:48  schubi
 * new dependecies returned by server
 *
 * Revision 1.12  2001/04/05 15:31:45  schubi
 * ObsoleteList added
 *
 * Revision 1.11  2001/04/04 15:41:39  mike
 * first compilable
 *
 * Revision 1.10  2001/04/04 15:26:13  schubi
 * only header changes
 *
 * Revision 1.9  2001/04/04 14:26:36  mike
 * next step
 *
 * Revision 1.8  2001/04/04 07:24:00  schubi
 * PackVersList is a vector
 *
 * Revision 1.7  2001/04/04 06:46:52  mike
 * new dependencie interface (prov requ ..)
 *
 * Revision 1.6  2001/04/03 12:46:36  schubi
 * parsing new dependencies
 *
 * Revision 1.5  2000/09/08 16:08:12  schubi
 * new server-calls:getKernelList and getInstalledKernel
 *
 * Revision 1.4  2000/06/20 10:28:25  schubi
 * Changed sel-file-format; adding utf8-description, kind and visible flag
 *
 * Revision 1.3  2000/05/19 13:20:32  schubi
 * getChangedPackageName and getInstallSplittedPackages added; description in Y2PkgInfo.h
 *
 * Revision 1.2  2000/05/17 14:32:04  schubi
 * update Modus added after new cvs
 *
 * Revision 1.4  2000/03/30 17:03:04  schubi
 * getSelGroups added; It evaluate all packagegroups from *.sel
 *
 * Revision 1.3  2000/01/25 18:41:38  schubi
 * initial
 *
 * Revision 1.2  2000/01/11 17:52:57  schubi
 * creating size
 *
 * Revision 1.1  2000/01/05 12:44:42  mike
 * initial version
 *
 */
#ifndef PkgInfo_h
#define PkgInfo_h

#include <vector>
#include <list>
#include <map>
#include <set>
#include <string>

using std::vector;
using std::map;
using std::set;
using std::string;
using std::list;

#include <pkg/PackageKey.h>

/**
 * typedefs to store package dependencies:
 *
 * PackageListType   for PackList        [ A B C D E F ]
 * DependMapType     for And-Dependencies:  [ [A,[B,K], [B,[C,L], [C,[X] ]
 *                   and Exclude-Deps.
 * DependMultiMapTyp for OR-Dependencies:   [ [A,[D,V], [A,[E,V] ]
 *
 * see Solver.h for detail
 *
 */

struct SelectionGroup {
   string description;
   string kind;
   bool visible;
   string requires;
   string conflicts;
   string suggests;
   string provides;
   string version;
   string architecture;
   string longDescription;
   string size;
   string notify;
   string delNotify;
};

///////////////////////////////////////////////////////////////////
//
// Dependency stuff
//
///////////////////////////////////////////////////////////////////

enum DepType {
  NODepType = 0,
  REQU,
  PROV,
  CONFL,
  OBSOL,
  SUGGE
};

enum DepCompare { // ('!=' is not supported by rpm)
  NONE = 0x00,
  EQ   = 0x01,
  LT   = 0x10,
  GT   = 0x20,
  LTE  = LT|EQ,
  GTE  = GT|EQ,
};

struct Dependence {

   string     name;
   DepCompare compare;
   string     version;

   Dependence( const string & name_tr = "" ) {
     name = name_tr; compare = NONE;
   }
   Dependence( const string & name_tr, DepCompare compare_er, const string & version_tr ) {
     name = name_tr; compare = compare_er; version = version_tr;
   }
};

///////////////////////////////////////////////////////////////////
//
// Check wheter a string defines a valid DepCompare. ('!=' is not supported by rpm)
//
inline DepCompare string2DepCompare( const string & str_tr ) {
  unsigned ret_ei = NONE;
  switch( str_tr.size() ) {
  case 2:
    switch ( str_tr[1] ) {
    case '=': ret_ei |= EQ; break;
    case '<': ret_ei |= LT; break;
    case '>': ret_ei |= GT; break;
    default:  return NONE;
    }
    // fall through
  case 1:
    switch ( str_tr[0] ) {
    case '=': ret_ei |= EQ; break;
    case '<': ret_ei |= LT; break;
    case '>': ret_ei |= GT; break;
    default:  return NONE;
    }
    break;
  default:
    return NONE;
    break;
  }
  return ( ret_ei == (LT|GT) ? NONE : (DepCompare)ret_ei );
}

inline string DepCompare2string( DepCompare comp_er ) {
  switch( comp_er ) {
  case NONE: return "nop";
  case EQ:   return "==";
  case LT:   return "<";
  case GT:   return ">";
  case LTE:  return "<=";
  case GTE:  return ">=";
  }
  return "?";
}

///////////////////////////////////////////////////////////////////

struct InstalledPackageElement {
      string packageName;	// without extention .rpm ..
      long   buildtime;
      long   installtime;
      string version;
      string obsoletes;
      string provides;
      string requires;
      string conflicts;
      string rpmgroup;
};

typedef map<string,InstalledPackageElement> InstalledPackageMap;

typedef vector<Dependence>                  DependList;
typedef map<PackageKey,DependList>          PackDepLMap;

typedef set<string>                         TagList;
typedef map<PackageKey,TagList>             PackTagLMap;

typedef vector<PackageKey>                  PackVersList;
typedef map<string,PackVersList>            TagPackVersLMap;
typedef map<PackageKey,PackVersList>        PackPackVersLMap;

typedef set<string>		            PartitionList;
typedef map<string,int>		       	    PackagePartitionSizeMap;
typedef map<string,PackagePartitionSizeMap> PackageSizeMap;
typedef map<string,SelectionGroup>	    SelectionGroupMap;
typedef set<string>                  	    PackList;

struct ObsoleteStruct {
   string       obsoletes;
   string       obsoletesVersion;
   PackVersList obsoletesDepPackages;
   string       isObsoleted;
   string       isObsoletedVersion;
   PackVersList isObsoletedDepPackages;
};

typedef vector<ObsoleteStruct>		    ObsoleteList;

#endif

/*---------------------------- EOF ------------------------------*/
