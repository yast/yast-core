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

   File:       Pathname.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef Pathname_h
#define Pathname_h

#include <iosfwd>
#include <string>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Pathname
//
//	DESCRIPTION :
//
class Pathname {

  private:

    string::size_type prfx_i;
    string            name_t;

  protected:

    void _assign( const string & name_tv );

  public:

    virtual ~Pathname() {}

    Pathname() {
      prfx_i = 0;
      name_t = "";
    }
    Pathname( const Pathname & path_tv ) {
      prfx_i = path_tv.prfx_i;
      name_t = path_tv.name_t;
    }
    Pathname( const string & name_tv ) {
      _assign( name_tv );
    }
    Pathname( const char * name_tv ) {
      _assign( name_tv ? name_tv : "" );
    }

    Pathname & operator= ( const Pathname & path_tv );
    Pathname & operator+=( const Pathname & path_tv );

    const string & asString() const { return name_t; }

    bool empty()    const { return !name_t.size(); }
    bool absolute() const { return !empty() && name_t[prfx_i] == '/'; }
    bool relative() const { return !empty() && name_t[prfx_i] != '/'; }

    Pathname dirname()       const { return dirname( *this ); }
    string   basename()      const { return basename( *this ); }
    Pathname absolutename()  const { return absolutename( *this ); }
    Pathname relativename()  const { return relativename( *this ); }

    static Pathname dirname     ( const Pathname & name_tv );
    static string   basename    ( const Pathname & name_tv );
    static Pathname absolutename( const Pathname & name_tv ) { return name_tv.relative() ? cat( "/", name_tv ) : name_tv; }
    static Pathname relativename( const Pathname & name_tv ) { return name_tv.absolute() ? cat( ".", name_tv ) : name_tv; }

    Pathname        cat( const Pathname & r ) const { return cat( *this, r ); }
    static Pathname cat( const Pathname & l, const Pathname & r );

    Pathname        extend( const string & r ) const { return extend( *this, r ); }
    static Pathname extend( const Pathname & l, const string & r );

    bool            equal( const Pathname & r ) const { return equal( *this, r ); }
    static bool     equal( const Pathname & l, const Pathname & r );
};

///////////////////////////////////////////////////////////////////

inline bool operator==( const Pathname & l, const Pathname & r ) {
  return Pathname::equal( l, r );
}

inline bool operator!=( const Pathname & l, const Pathname & r ) {
  return !Pathname::equal( l, r );
}

inline Pathname operator+( const Pathname & l, const Pathname & r ) {
  return Pathname::cat( l, r );
}

inline Pathname & Pathname::operator=( const Pathname & path_tv ) {
  if ( &path_tv != this ) {
    prfx_i = path_tv.prfx_i;
    name_t = path_tv.name_t;
  }
  return *this;
}

inline Pathname & Pathname::operator+=( const Pathname & path_tv ) {
  return( *this = *this + path_tv );
}

///////////////////////////////////////////////////////////////////

extern std::ostream & operator<<( std::ostream & str, const Pathname & obj );

///////////////////////////////////////////////////////////////////

#endif // Pathname_h
