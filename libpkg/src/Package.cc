/*---------------------------------------------------------------------\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                        (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       Package.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <ycp/y2log.h>
#include <pkg/Package.h>
#include "TagParser.h"

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Package::asDependList
//	METHOD TYPE : DependList
//
//	DESCRIPTION :
//
DependList Package::asDependList( const string & data_tr ) const
{
  DependList ret_VCi;
  vector<string> data_Vti( TagParser::split2words( data_tr ) );

  Dependence cdep_Ci;
  DepCompare depOp_ei = NONE;

  for ( unsigned i = 0; i < data_Vti.size(); ++i ) {
    depOp_ei = string2DepCompare( data_Vti[i] );

    if ( depOp_ei == NONE ) {
       // string value
      if ( cdep_Ci.name.empty() ) {           // no previous. remember new name
	cdep_Ci.name = data_Vti[i];
      } else if ( cdep_Ci.compare != NONE ) { // remember version after op and store
	cdep_Ci.version = data_Vti[i];
	ret_VCi.push_back( cdep_Ci );
	cdep_Ci = Dependence();
      } else {                                // stroe previous and remember new name
	ret_VCi.push_back( cdep_Ci );
	cdep_Ci = Dependence( data_Vti[i] );
      }
    } else {
      // operator value
      if ( cdep_Ci.name.empty() || cdep_Ci.compare != NONE ) {
	y2error( "Missplaced operator '%s' in dependency of '%s' (%s)",
		 DepCompare2string( depOp_ei ).c_str(), name().c_str(), data_tr.c_str() );
	cdep_Ci = Dependence();
	break;
      } else {
	cdep_Ci.compare = depOp_ei;
      }
    }
  }

  if ( cdep_Ci.name.size() ) {
    if ( cdep_Ci.compare == NONE || cdep_Ci.version.size() ) {
      ret_VCi.push_back( cdep_Ci );
    } else {
      y2error( "Missplaced operator '%s' in dependency of '%s' (%s)",
	       DepCompare2string( cdep_Ci.compare ).c_str(), name().c_str(), data_tr.c_str() );
    }
  }

  return ret_VCi;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Package::asTagList
//	METHOD TYPE : TagList
//
//	DESCRIPTION :
//
TagList Package::asTagList( const string & data_tr ) const
{
  TagList ret_VCi;
  vector<string> data_Vti( TagParser::split2words( data_tr ) );

  for ( unsigned i = 0; i < data_Vti.size(); ++i ) {
    ret_VCi.insert( data_Vti[i] );
  }

  return ret_VCi;
}

