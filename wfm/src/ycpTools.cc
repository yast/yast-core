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

  File:       ycpTools.cc

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose:

/-*/

#include <iostream>
#include <sstream>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>

#include "ycpTools.h"

using namespace std;

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : YcpArgLoad::load
//	METHOD TYPE : bool
//
bool YcpArgLoad::load( const YCPList & args_r )
{
  bool ret = true;
  string errstr;

  if ( unsigned(args_r->size()) > _proto.size() ) {
    errstr = stringutil::form( "takes %d arg(s) but got %d", _proto.size(), args_r->size() );
    ret = false;

  } else if ( unsigned(args_r->size()) < _optional ) {
    errstr = stringutil::form( "requires %d arg(s) but got %d", _optional, args_r->size() );
    ret = false;

  } else {
    for ( unsigned i = 0; ret && i < unsigned(args_r->size()); ++i ) {
      switch ( _proto[i]->load( args_r->value(i) ) ) {
      case YcpArg::assigned:
	// ok
	break;
      case YcpArg::wrongtype:
	errstr = stringutil::form( "arg%d: expect %s but got %s", i,
				   ::asString( _proto[i]->type() ).c_str(),
				   ::asString( args_r->value(i)->valuetype() ).c_str() );
	ret = false;
	break;
      case YcpArg::badformat:
	errstr = stringutil::form( "arg%d: malformed %s", i,
				   ::asString( _proto[i]->type() ).c_str() );
	ret = false;
	break;
      }
    }
  }

  if ( !ret ) {
    INT << *this << ": " << errstr << ": " << args_r->toString() << endl;
  } else {
    DBG << *this << ": " << args_r->toString() << endl;
  }
  return ret;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, const YcpArgLoad & obj )
{
  if ( obj._proto.empty() )
    return str << obj._fnc << "(void)";

  str << obj._fnc << "( ";
  bool closeOpt = false;

  for ( unsigned i = 0; i < obj._proto.size(); ++i ) {
    if ( i == obj._optional ) {
      str << '[';
      closeOpt = true;
    }
    if ( i )
      str << ", ";
    str << obj._proto[i]->type();
  }

  if ( closeOpt )
    str << ']';
  return str << " )";
}

/******************************************************************
**
**
**	FUNCTION NAME : asString
**	FUNCTION TYPE : string
*/
string asString( YCPValueType obj )
{
  switch ( obj ) {
#define ENUMOUT(V) case V: return #V; break
    ENUMOUT( YT_UNDEFINED );
    ENUMOUT( YT_VOID );
    ENUMOUT( YT_BOOLEAN );
    ENUMOUT( YT_INTEGER );
    ENUMOUT( YT_FLOAT );
    ENUMOUT( YT_STRING );
    ENUMOUT( YT_BYTEBLOCK );
    ENUMOUT( YT_PATH );
    ENUMOUT( YT_SYMBOL );
    ENUMOUT( YT_DECLARATION );
    ENUMOUT( YT_LOCALE );
    ENUMOUT( YT_LIST );
    ENUMOUT( YT_TERM );
    ENUMOUT( YT_MAP );
    ENUMOUT( YT_BLOCK );
    ENUMOUT( YT_BUILTIN );
    ENUMOUT( YT_IDENTIFIER );
    ENUMOUT( YT_ERROR );
#undef ENUMOUT
  }
  return stringutil::form( "YCPValueType(%d)", obj );
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
*/
ostream & operator<<( ostream & str, YCPValueType obj )
{
  return str << asString( obj );
}

/******************************************************************
**
**
**	FUNCTION NAME : asString
**	FUNCTION TYPE : std::string
*/
std::string asString( const YCPValue & obj )
{
  switch ( obj->valuetype() ) {
  case YT_UNDEFINED:
    return "YCPValue(YT_UNDEFINED)";
    break;
#define ENUMOUT(V,v) case YT_##V: return obj->as##v()->toString(); break
    ENUMOUT( VOID,	Void );
    ENUMOUT( BOOLEAN,	Boolean );
    ENUMOUT( INTEGER,	Integer );
    ENUMOUT( FLOAT,	Float );
    ENUMOUT( STRING,	String );
    ENUMOUT( BYTEBLOCK,	Byteblock );
    ENUMOUT( PATH,	Path );
    ENUMOUT( SYMBOL,	Symbol );
    ENUMOUT( DECLARATION,	Declaration );
    ENUMOUT( LOCALE,	Locale );
    ENUMOUT( LIST,	List );
    ENUMOUT( TERM,	Term );
    ENUMOUT( MAP,	Map );
    ENUMOUT( BLOCK,	Block );
    ENUMOUT( BUILTIN,	Builtin );
    ENUMOUT( IDENTIFIER,	Identifier );
    ENUMOUT( ERROR,	Error );
#undef ENUMOUT
  }
  return stringutil::form( "YCPValue(%s)", asString( obj->valuetype() ).c_str() );
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : std::ostream &
*/
std::ostream & operator<<( std::ostream & str, const YCPValue & obj )
{
  return str << asString( obj );
}

