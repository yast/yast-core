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

   File:       PathInfo.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <iostream>
#include <iomanip>

#include "PathInfo.h"

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::PathInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
PathInfo::PathInfo( const Pathname & path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
{
  operator()();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::PathInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
PathInfo::PathInfo( const string & path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
{
  operator()();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::PathInfo
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
PathInfo::PathInfo( const char * path, Mode initial )
    : path_t( path )
    , mode_e( initial )
    , error_i( -1 )
{
  operator()();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::~PathInfo
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
PathInfo::~PathInfo()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PathInfo::operator()
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool PathInfo::operator()()
{
  if ( path_t.empty() ) {
    error_i = -1;
  } else {
    switch ( mode_e ) {
    case STAT:
      error_i = ::stat( path_t.asString().c_str(), &statbuf_C );
      break;
    case LSTAT:
      error_i = ::lstat( path_t.asString().c_str(), &statbuf_C );
      break;
    }
    if ( error_i == -1 )
      error_i = errno;
  }
  return !error_i;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const PathInfo & obj )
{
  ios::fmtflags state_ii = str.flags();

  str << obj.asString() << "{";
  if ( !obj.isExist() ) {
    str << "does not exist}";
  } else {
    char t = '?';
    if ( obj.isFile() )
      t = '-';
    else if ( obj.isDir() )
      t = 'd';
    else if ( obj.isLink() )
      t = 'l';
    else if ( obj.isChr() )
      t = 'c';
    else if ( obj.isBlk() )
      t = 'b';
    else if ( obj.isFifo() )
      t = 'p';
    else if ( obj.isSock() )
      t = 's';
    str << t
      << " " << setfill( '0' ) << setw( 4 ) << oct << obj.perm()
      << " " << dec << obj.owner() << "/" << obj.group();

    if ( obj.isFile() )
      str << " size " << obj.size();

    str << "}";
  }
  str.flags( state_ii );
  return str;
}
