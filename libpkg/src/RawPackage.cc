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

   File:       RawPackage.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <iostream>

#include "RawPackage.h"

using std::ostream;
using std::endl;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackage::RawPackage
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
RawPackage::RawPackage( const RawPackageInfo & master_Cr )
    : master_C( master_Cr )
    , instIdx_i   ( 0 )
    , type_e      ( NOTYPE )
    , onCD_i      ( 0 )
    , buildTime_i ( 0 )
    , rpmSize_i   ( 0 )
    , uncompSize_i( 0 )
    , isBasepkg_b ( false )
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : RawPackage::~RawPackage
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
RawPackage::~RawPackage()
{
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const RawPackage & obj )
{
  str << "Package " << obj.instIdx_i << " [" << endl;
#define STAG(s) str << "   " << #s << " {" << obj.s() << "}" << endl
  STAG( type );
  STAG( name );
  STAG( version );
  STAG( group );
  STAG( label );
  STAG( description );
  STAG( requires );
  STAG( provides );
  STAG( conflicts );
  STAG( obsoletes );
  STAG( onCD );
  STAG( instPath );
  STAG( buildTime );
  STAG( builtFrom );
  STAG( rpmSize );
  STAG( uncompSize );
  STAG( instNotify );
  STAG( delNotify );
  STAG( series );
  STAG( isBasepkg );
  STAG( author );
  STAG( copyright );
#undef STAG
  str << "]" << endl;
  return str;
}

