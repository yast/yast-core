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

   File:       TagParser.cc

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/

#include <iostream>
#include <ycp/y2log.h>

#include "TagParser.h"

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagParser
//
///////////////////////////////////////////////////////////////////

const unsigned TagParser::bufferLen_i = 1024;
char           TagParser::buffer_ac[bufferLen_i];
const streampos TagParser::nopos = streampos(-1);

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::_datareset
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
inline void TagParser::_datareset()
{
  startData_i = endData_i = endTag_i = nopos;
  endTag_t.erase();
  data_Vt.clear();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::_reset
//	METHOD TYPE : void
//
//	DESCRIPTION :
//
inline void TagParser::_reset()
{
  lookupStart_i = startTag_i = nopos;
  startTag_t.erase();
  _datareset();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::TagParser
//	METHOD TYPE : Constructor
//
//	DESCRIPTION :
//
TagParser::TagParser()
{
  _reset();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::~TagParser
//	METHOD TYPE : Destructor
//
//	DESCRIPTION :
//
TagParser::~TagParser()
{
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::readLine
//	METHOD TYPE : streampos
//
//	DESCRIPTION :
//
inline streampos TagParser::readLine( istream & stream_fr, string & cline_tr )
{
  streampos lineBegin_ii = stream_fr.tellg();
  cline_tr.erase();

  do {
    stream_fr.clear();
    stream_fr.getline( buffer_ac, bufferLen_i ); // always writes '\0' terminated
    cline_tr += buffer_ac;
  } while( stream_fr.rdstate() == ios::failbit );

  return lineBegin_ii;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::tagOnLine
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
inline bool TagParser::tagOnLine( const string & cline_tr, string & tag_tr, string::size_type & delim_ir )
{
  if ( !cline_tr.size() || cline_tr[0] == '#' ) {
    delim_ir = string::npos;
    tag_tr.erase();
    return false; // empty or comment line
  }

  delim_ir = cline_tr.find_first_of( ": \t" );

  if ( delim_ir == string::npos || delim_ir == 0 || cline_tr[delim_ir] != ':' ) {
    delim_ir = string::npos;
    tag_tr.erase();
    return false; // no tag or empty tag or whitespace in tag
  }

  tag_tr = cline_tr.substr( 0, delim_ir );

  return true;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::lookupTag
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool TagParser::lookupTag( istream & stream_fr, const string & stag_tr )
{
  _reset();
  if ( stream_fr.good() ) {
    lookupStart_i = stream_fr.tellg();

    streampos         lineBegin_ii = nopos;
    string            cline_ti;
    string::size_type delim_ii = string::npos;
    string            maybe_ti;

    do {
      lineBegin_ii = readLine( stream_fr, cline_ti );

      if ( !tagOnLine( cline_ti, maybe_ti, delim_ii ) )
	continue; // no tag on cline

      if ( stag_tr.size() && maybe_ti != stag_tr )
	continue; // tag does not match given stag_tr

      // here we've got a valid tag
      startTag_i = lineBegin_ii;
      startTag_t = maybe_ti;

      // look for data on this line
      delim_ii = cline_ti.find_first_not_of( " \t", delim_ii+1 );
      if ( delim_ii == string::npos ) {
	// no data on this line
	startData_i = endData_i = startTag_i + streamoff(cline_ti.size());
	data_Vt.push_back( "" );
      } else {
	startData_i = startTag_i + streamoff(delim_ii);
	maybe_ti =  cline_ti.substr( delim_ii, cline_ti.find_last_not_of( " \t" ) + 1 - delim_ii );
	endData_i   = startData_i + streamoff(maybe_ti.size());
	data_Vt.push_back( maybe_ti );
      }

      return true;

    } while( stream_fr.good() );
  }

  return false;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::parseData
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool TagParser::parseData( istream & stream_fr, const string & etag_tr )
{
  _datareset();
  if ( stream_fr.good() ) {
    startData_i = stream_fr.tellg();

    streampos         lineBegin_ii = nopos;
    string            cline_ti;
    string::size_type delim_ii = string::npos;
    string            maybe_ti;

    do {
      lineBegin_ii = readLine( stream_fr, cline_ti );

      if ( tagOnLine( cline_ti, maybe_ti, delim_ii ) && maybe_ti == etag_tr ) {
	endData_i = endTag_i = lineBegin_ii;
	endTag_t = maybe_ti;

	if ( data_Vt.empty() )
	  data_Vt.push_back( "" );

	return true;
      }

      // here we've got collect the line;
      data_Vt.push_back( cline_ti );

    } while( stream_fr.good() );

    // here saw no endTag
    _datareset();
  }

  return dataLines();
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::retrieveData
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool TagParser::retrieveData( istream & stream_fr,
			      streampos startData_ir, streampos endData_ir,
			      string & data_tr )
{
  data_tr.erase();
  if ( startData_ir == nopos || endData_ir == nopos || endData_ir < startData_ir ) {
    y2internal( "positions make no sense %d(%lld/%lld)", stream_fr.rdstate(), (long long)streamoff(startData_ir), (long long)streamoff(endData_ir) );
    return false; // positions make no sense
  }

  stream_fr.clear();
  stream_fr.seekg( startData_ir );

  if ( !stream_fr.good() ) {
    y2internal( "illegal startData position %d(%lld/%lld)", stream_fr.rdstate(), (long long)streamoff(startData_ir), (long long)streamoff(endData_ir) );
    return false; // illegal startData position
  }

  unsigned expect_ii = endData_ir-startData_ir;

  if ( !expect_ii ) {
    return true; // startData position is valid, but we don't expect any data
  }

  for ( unsigned toread_ii = min( expect_ii, bufferLen_i );
	toread_ii;
	expect_ii -= toread_ii, toread_ii = min( expect_ii, bufferLen_i ) ) {
    stream_fr.read( buffer_ac, toread_ii );
    if ( stream_fr.gcount() != toread_ii ) {
      data_tr.erase();
      y2internal( "not as many data available as expected %d(%lld/%lld)", stream_fr.rdstate(), (long long)streamoff(startData_ir), (long long)streamoff(endData_ir) );
      return false; // not as many data available as expected
    }
    data_tr += string( buffer_ac, toread_ii );
  }

  // strip a trailing NL
  if ( data_tr[data_tr.size()-1] == '\n' )
    data_tr.erase( data_tr.size()-1 );

  return true;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::retrieveData
//	METHOD TYPE : bool
//
//	DESCRIPTION :
//
bool TagParser::retrieveData( istream & stream_fr,
			      streampos startData_ir, streampos endData_ir,
			      vector<string> & data_Vtr )
{
  data_Vtr.clear();
  string data_ti;
  if ( !retrieveData( stream_fr, startData_ir, endData_ir, data_ti ) ) {
    return false;
  }

  string::size_type subStart_ii = 0;
  for ( string::size_type delim_ii = data_ti.find( '\n', subStart_ii );
	delim_ii != string::npos;
	subStart_ii = delim_ii+1, delim_ii = data_ti.find( '\n', subStart_ii ) ) {
    data_Vtr.push_back( data_ti.substr( subStart_ii, delim_ii - subStart_ii ) );
  }
  data_Vtr.push_back( data_ti.substr( subStart_ii ) );

  return true;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::data2string
//	METHOD TYPE : string
//
//	DESCRIPTION :
//
string TagParser::data2string( const vector<string> & data_Vtr )
{
  if ( !data_Vtr.size() )
    return "";

  string ret_ti( data_Vtr[0] );
  for ( unsigned i = 1; i < data_Vtr.size(); ++i ) {
    ret_ti += '\n';
    ret_ti += data_Vtr[i];
  }
  return ret_ti;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagParser::split2words
//	METHOD TYPE : vector<string>
//
//	DESCRIPTION :
//
vector<string> TagParser::split2words( const string & line_tr, const string & sepchars_tr )
{
  vector<string> Ret_Vti;

  string::size_type wstart_ii = 0;
  string::size_type wend_ii   = string::npos;
  do {
    wstart_ii = line_tr.find_first_not_of( sepchars_tr, wstart_ii );
    if ( wstart_ii != string::npos ) {
      wend_ii = line_tr.find_first_of( sepchars_tr, wstart_ii );
      if ( wend_ii != string::npos ) {
	Ret_Vti.push_back( line_tr.substr( wstart_ii, wend_ii-wstart_ii ) );
      } else {
	Ret_Vti.push_back( line_tr.substr( wstart_ii ) );
      }
      wstart_ii = wend_ii;
    }
  } while ( wstart_ii != string::npos );

  return Ret_Vti;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const TagParser & obj )
{
  str << "TagParser{"
      << obj.lookupStart()
      << " [" << obj.posStartTag() << ":" << obj.startTag() << "]"
      << " [" << obj.posEndTag() << ":" << obj.endTag() << "]"
      << " [" << obj.posDataStart() << "-" << obj.posDataEnd() << "=" << obj.dataLength() << "]"
      << " (" << obj.dataLines() << ")";
  if ( obj.dataLines() )
    str << " \"" << obj.data()[0] << '"';

  return str << '}';
}

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagSet
//
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagSet::addTag
//	METHOD TYPE : void
//
//	DESCRIPTION : For sake of derived classes we always create a new
//      index entry in lookupEnd_V, even if the tag is alredy defined,
//      i.e. does not occurr in lookupStart_V.
//
void TagSet::addTag( const string & start_tr, const string & end_tr )
{
  map<string,unsigned>::value_type val_Ci( start_tr, lookupEnd_V.size() );

  if ( !lookupStart_V.insert( val_Ci ).second ) {
    y2internal( "Duplicate definition for Tag(%s,%s)", start_tr.c_str(), end_tr.c_str() );
  }

  lookupEnd_V.push_back( end_tr );
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : TagSet::parseData
//	METHOD TYPE : unsigned
//
//	DESCRIPTION :
//
unsigned TagSet::parseData( istream & in_Fr, TagParser & ctag_Cr ) const
{
  map<string,unsigned>::const_iterator i = lookupStart_V.find( ctag_Cr.startTag() );

  if ( i == lookupStart_V.end() )
    return unknowntag;

  unsigned ret_ii = i->second;

  if ( lookupEnd_V[ret_ii].size() )
    ctag_Cr.parseData( in_Fr, lookupEnd_V[ret_ii] );

  return ret_ii;
}

/******************************************************************
**
**
**	FUNCTION NAME : operator<<
**	FUNCTION TYPE : ostream &
**
**	DESCRIPTION :
*/
ostream & operator<<( ostream & str, const TagSet & obj )
{
  str << "TagSet(tags " << obj.lookupStart_V.size() << ", indices " << obj.lookupEnd_V.size();
  if ( obj.lookupStart_V.size() ) {
    str << endl;
    for ( map<string,unsigned>::const_iterator i = obj.lookupStart_V.begin();
	  i != obj.lookupStart_V.end(); ++i ) {
      str << "    [" << i->first << "] [" << obj.lookupEnd_V[i->second] << "] (" << i->second << ") " << endl;
    }
  }
  return str << ")" << endl;
}

