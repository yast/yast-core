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

  File:       ycpTools.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/
#ifndef ycpTools_h
#define ycpTools_h

#include <iosfwd>
#include <vector>
#include <list>
#include <string>

#include <YCP.h>

#include <y2util/Pathname.h>
#include <y2util/Url.h>

///////////////////////////////////////////////////////////////////
// convenience functions
///////////////////////////////////////////////////////////////////

extern std::string asString( YCPValueType obj );
extern std::ostream & operator<<( std::ostream & str, YCPValueType obj );

extern std::string asString( const YCPValue & obj );
extern std::ostream & operator<<( std::ostream & str, const YCPValue & obj );

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : YcpArgLoad
/**
 *
 **/
class YcpArgLoad {

  friend std::ostream & operator<<( std::ostream & str, const YcpArgLoad & obj );

  YcpArgLoad & operator=( const YcpArgLoad & );
  YcpArgLoad            ( const YcpArgLoad & );

  public:

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : YcpArgLoad::YcpArg
    /**
     *
     **/
    class YcpArg {
      protected:
	YCPValueType _type;
	virtual bool assign( const YCPValue & arg_r ) = 0;
      protected:
	YcpArg( YCPValueType type_r ) : _type( type_r ) {}
      public:
	virtual ~YcpArg() {}
	YCPValueType type() const { return _type; }
	enum Result { assigned = 0, wrongtype, badformat };
	Result load( const YCPValue & arg_r ) {
	  if ( arg_r->valuetype() != _type ) {
	    return wrongtype;
	  }
	  if ( ! assign( arg_r ) ) {
	    return badformat;
	  }
	  return assigned;
	}
    };
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //
    //	CLASS NAME : YcpArgLoad::Value
    /**
     *
     **/
    template<YCPValueType Ytype, typename Vtype>
    class Value : public YcpArg {
      protected:
	Vtype _value;
	virtual bool assign( const YCPValue & arg_r );
      public:
	Value()
	  : YcpArg( Ytype )
	{}
	Value( const Vtype & value_r )
	  : YcpArg( Ytype )
	  , _value( value_r )
	{}
	virtual ~Value() {}
	operator Vtype &() { return _value; }
    };
    ///////////////////////////////////////////////////////////////////

  private:

    std::string          _fnc;
    std::vector<YcpArg*> _proto;
    unsigned             _optional;

    void append( YcpArg * narg ) {
      _proto.reserve( _proto.size() + 1 );
      _proto.push_back( narg );
    }

  public:

    YcpArgLoad( const std::string & fnc_r = "" )
      : _fnc( fnc_r )
      , _optional( 0 )
    {}

    ~YcpArgLoad() {
      for ( unsigned i = 0; i < _proto.size(); ++i ) {
	delete _proto[i];
      }
    }

  public:

    template<YCPValueType Ytype, typename Vtype>
    Vtype & arg() {
      Value<Ytype,Vtype> * narg = new Value<Ytype,Vtype>();
      append( narg );
      _optional = _proto.size();
      return *narg;
    }

    template<YCPValueType Ytype, typename Vtype>
    Vtype & arg( const Vtype & d ) {
      Value<Ytype,Vtype> * narg = new Value<Ytype,Vtype>( d );
      append( narg );
      return *narg;
    }

  public:

    bool load( const YCPList & args_r );
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
// Common load templates
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// YT_BOOLEAN
///////////////////////////////////////////////////////////////////
template<>
inline bool YcpArgLoad::Value<YT_BOOLEAN, bool>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asBoolean()->value();
  return true;
}

///////////////////////////////////////////////////////////////////
// YT_INTEGER
///////////////////////////////////////////////////////////////////
template<>
inline bool YcpArgLoad::Value<YT_INTEGER, unsigned long long>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asInteger()->value();
  return true;
}
template<>
inline bool YcpArgLoad::Value<YT_INTEGER, long long>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asInteger()->value();
  return true;
}
template<>
inline bool YcpArgLoad::Value<YT_INTEGER, unsigned long>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asInteger()->value();
  return true;
}
template<>
inline bool YcpArgLoad::Value<YT_INTEGER, long>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asInteger()->value();
  return true;
}
template<>
inline bool YcpArgLoad::Value<YT_INTEGER, unsigned>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asInteger()->value();
  return true;
}
template<>
inline bool YcpArgLoad::Value<YT_INTEGER, int>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asInteger()->value();
  return true;
}

///////////////////////////////////////////////////////////////////
// YT_STRING
///////////////////////////////////////////////////////////////////
template<>
inline bool YcpArgLoad::Value<YT_STRING, std::string>::assign( const YCPValue & arg_r ) {
  _value = arg_r->asString()->value();
  return true;
}

template<>
inline bool YcpArgLoad::Value<YT_STRING, Pathname>::assign( const YCPValue & arg_r )
{
  _value = arg_r->asString()->value();
  return true;
}

template<>
inline bool YcpArgLoad::Value<YT_STRING, Url>::assign( const YCPValue & arg_r )
{
  _value = arg_r->asString()->value();
  return true;
}

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
// Common asYCP... functions
//
///////////////////////////////////////////////////////////////////

inline YCPList asYCPList( const std::list<std::string> & lst ) {
  YCPList ret;
  for ( std::list<std::string>::const_iterator it = lst.begin(); it != lst.end(); ++it ) {
    ret->add( YCPString( *it ) );
  }
  return ret;
}


///////////////////////////////////////////////////////////////////

#endif // ycpTools_h
