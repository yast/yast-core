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

   File:       Version.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef Version_h
#define Version_h

#include <iosfwd>
#include <string>

using std::string;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Version
//
//	DESCRIPTION :
//
class Version {

  public:

    enum COMPARE {
      UNCOMPARABLE = 0,
      OLDER,
      EQUAL,
      NEWER
    };

  private:

    string version_t;
    string release_t;

    string _init( const string & n );

    static COMPARE vcompare( const string & lhs, const string & rhs );

  public:

    Version() {}
    Version( const string & n ) { _init( n ); }
    Version( const char * n )   { _init( n?n:"" ); }
    Version( const string & v, const string & r ) : version_t( v ), release_t( r ) {}
    Version( const string & v, const char * r )   : version_t( v ), release_t( r?r:"" ) {}
    Version( const char * v, const string &r )    : version_t( v?v:"" ), release_t( r ) {}
    Version( const char * v, const char * r )     : version_t( v?v:"" ), release_t( r?r:"" ) {}

    ~Version() {}

  public:

    const string & version()  const { return version_t; }
    const string & release()  const { return release_t; }
    string         asString() const { return version_t + '-' + release_t; }

    string assign( const string & n ) { return _init( n ); }

    static COMPARE compare( const Version & lhs, const Version & rhs );

    COMPARE compare( const Version & rhs ) const { return compare( *this, rhs ); }

    static string prefix( const string & n ) { return Version()._init( n ); }

};

///////////////////////////////////////////////////////////////////

extern std::ostream & operator<<( std::ostream & str, const Version & obj );
extern std::ostream & operator<<( std::ostream & str, Version::COMPARE obj );

///////////////////////////////////////////////////////////////////

#include <functional>

namespace std {
struct less<Version> : public binary_function<Version,Version,bool>
{
  bool operator()(const Version& __x, const Version& __y) const {
    return __x.version() <  __y.version()
      || ( __x.version() == __y.version() && __x.release() < __y.release() );
  }
};
}

///////////////////////////////////////////////////////////////////

#endif // Version_h
