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

   File:       Version.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include "Version.h"

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Version::_init
//	METHOD TYPE : string
//
//	DESCRIPTION :
//
string Version::_init( const string & n )
{
  string name_ti;
  string::size_type idx = n.find_last_of( "-" );
  if ( idx == string::npos ) {
    version_t = n;
    release_t = "";
  } else {
    version_t = n.substr( 0, idx );
    release_t = n.substr( idx+1 );

    idx = version_t.find_last_of( "-" );
    if ( idx != string::npos ) {
      name_ti   = version_t.substr( 0, idx );
      version_t = version_t.substr( idx+1 );
    }
  }
  return name_ti;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Version::compare
//	METHOD TYPE : COMPARE
//
//	DESCRIPTION :
//
Version::COMPARE Version::compare( const Version & lhs, const Version & rhs )
{
  COMPARE ret = vcompare( lhs.version_t, rhs.version_t );

  if ( ret == EQUAL ) {
    ret = vcompare( lhs.release_t, rhs.release_t );
  }

  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : isANumber
**	FUNCTION TYPE : bool
**
**	DESCRIPTION : true if in [0-9]
*/
inline bool isANumber( const char * c )
{
  return ( '0' <= *c && *c <= '9' );
}

/******************************************************************
**
**
**	FUNCTION NAME : isAChar
**	FUNCTION TYPE : bool
**
**	DESCRIPTION : true if in [a-zA-Z]
*/
inline bool isAChar( const char * c )
{
  return (    ( 'a' <= *c && *c <= 'z' )
	   || ( 'A' <= *c && *c <= 'Z' ) );
}

/******************************************************************
**
**
**	FUNCTION NAME : nonJunk
**	FUNCTION TYPE : bool
**
**	DESCRIPTION : true if isANumber or isAChar
*/
inline bool nonJunk( const char * c )
{
  return ( isANumber( c ) || isAChar( c ) );
}

/******************************************************************
**
**
**	FUNCTION NAME : skipJunk
**	FUNCTION TYPE : void
**
**	DESCRIPTION : skip forward to the next nonJunk or \0
*/
inline void skipJunk( const char *& c )
{
  while ( *c && !nonJunk( c ) ) {
    ++c;
  }
}

/******************************************************************
**
**
**	FUNCTION NAME : skipJunk
**	FUNCTION TYPE : bool
**
**	DESCRIPTION : skipJunk for both args. return true if
**                    both args are not at \0.
*/
inline bool skipJunk( const char *& c1, const char *& c2 )
{
  skipJunk( c1 );
  skipJunk( c2 );
  return ( *c1 && *c2 );
}

/******************************************************************
**
**
**	FUNCTION NAME : skipNumber
**	FUNCTION TYPE : unsigned
**
**	DESCRIPTION : skip over Numbers and return the collected value.
**                    NOTE: must initally stay on a Number.
*/
inline unsigned skipNumber( const char *& c )
{
  unsigned n = 0;
  do {
    n = n*10 + (*c-'0');
  } while ( isANumber( ++c ) );
  return n;
}

/******************************************************************
**
**
**	FUNCTION NAME : skipChar
**	FUNCTION TYPE : string
**
**	DESCRIPTION : skip over Chars and return the collected string.
**                    NOTE: must initally stay on a Char.
*/
inline string skipChar( const char *& c )
{
  const char * s = c;
  while ( isAChar( ++c ) )
    ; // empty body
  return string( s, c-s );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : Version::vcompare
//	METHOD TYPE : COMPARE
//
//	DESCRIPTION : split args into Number and Char potions, and
//                    compare the values
//
Version::COMPARE Version::vcompare( const string & lhs, const string & rhs )
{
  const char * l = lhs.c_str();
  const char * r = rhs.c_str();

  // We return as soon as a decision is made! The loop ends iff
  // we're still EQUAL, but ran out of chars in l and/or r
  while ( skipJunk( l, r ) ) {
    // Neither l nor r ran out of chars
    bool ltype = isANumber( l );
    bool rtype = isANumber( r );
    if ( ltype == rtype ) {
      // Must compare
      if ( ltype ) {
	// Number compare
	unsigned lnum = skipNumber( l );
	unsigned rnum = skipNumber( r );
	if ( lnum != rnum ) {
	  return ( lnum < rnum ) ? OLDER : NEWER;
	}
      } else {
	// String compare
	string lstr = skipChar( l );
	string rstr = skipChar( r );
	int cmp = lstr.compare( rstr );
	if ( cmp ) {
	  return ( cmp < 0 ) ? OLDER : NEWER;
	}
      }
    } else {
      // Mixed pair
      return UNCOMPARABLE;
    }
    // Still EQUAL, process next pair...
  }

  // Still EQUAL, but ran out of chars in l and/or r
  if ( *r ) {
    // lhs is prefix of rhs
    return OLDER;
  }
  if ( *l ) {
    // rhs is prefix of lhs
    return NEWER;
  }

  return EQUAL;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, Version::COMPARE obj )
{
  switch ( obj ) {
#define PRTOUT(T) case Version::T: return str << #T
    PRTOUT( UNCOMPARABLE );
    PRTOUT( OLDER );
    PRTOUT( EQUAL );
    PRTOUT( NEWER );
#undef PRTOUT
  }
  return str << "Version::COMPARE";
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const Version & obj )
{
  return str << obj.asString();
}

