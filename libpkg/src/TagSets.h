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

   File:       TagSets.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef TagSets_h
#define TagSets_h

#include <iostream>

using std::ostream;

#include "TagParser.h"

///////////////////////////////////////////////////////////////////

#define STAG(s)   case s: addTag( #s );     break
#define DTAG(s,e) case s: addTag( #s, #e ); break

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PkdPreambleTags
//
//	DESCRIPTION :
//
class PkdPreambleTags : private TagSet {

  public:

    enum Tag {
      PkdVersion,
      Encoding,
      // last entry
      Max_Tag
    };

    PkdPreambleTags() {
      for ( Tag t = (Tag)0; t < Max_Tag; t = (Tag)(t+1) ) {
	switch ( t ) {
	  STAG( PkdVersion );
	  STAG( Encoding );
	default:
	  y2internal( "Missing definition for PkdPreambleTags::Tag %d", t );
	  addTag( "" );
	  break;
	}
      }
    }

  public:

    friend ostream & operator<<( ostream & str, const PkdPreambleTags & obj ) {
      return str << "PkdPreambleTags: " << static_cast<const TagSet &>( obj );
    }

    Tag parseData( istream & in_Fr, TagParser & ctag_Cr ) const {
      unsigned idx_ii = TagSet::parseData( in_Fr, ctag_Cr );
      return (idx_ii == TagSet::unknowntag) ? Max_Tag : (Tag)idx_ii;
    }

};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME :
//
//	DESCRIPTION :
//
class CommonTags : private TagSet {

  public:

    enum Tag {
      Filename,
      RpmName,
      Version,
      Provides,
      Requires,
      Conflicts,
      Obsoletes,
      InstPath,
      Buildtime,
      BuiltFrom,
      RpmGroup,
      Series,
      Size,
      Flag,
      Copyright,
      AuthorName,
      AuthorEmail,
      AuthorAddress,
      StartCommand,
      Category,
      DepAND,
      DepOR,
      DepExcl,
      // last entry
      Max_Tag
    };

    CommonTags() {
      for ( Tag t = (Tag)0; t < Max_Tag; t = (Tag)(t+1) ) {
	switch ( t ) {
	  STAG( Filename );
	  STAG( RpmName );
	  STAG( Version );
	  STAG( Provides );
	  STAG( Requires );
	  STAG( Conflicts );
	  STAG( Obsoletes );
	  STAG( InstPath );
	  STAG( Buildtime );
	  STAG( BuiltFrom );
	  STAG( RpmGroup );
	  STAG( Series );
	  STAG( Size );
	  STAG( Flag );
	  STAG( Copyright );
	  STAG( AuthorName );
	  STAG( AuthorEmail );
	  STAG( AuthorAddress );
	  STAG( StartCommand );
	  STAG( Category );
	  STAG( DepAND );
	  STAG( DepOR );
	  STAG( DepExcl );
	default:
	  y2internal( "Missing definition for CommonTags::Tag %d", t );
	  addTag( "" );
	  break;
	}
      }
    }

  public:

    friend ostream & operator<<( ostream & str, const CommonTags & obj ) {
      return str << "CommonTags: " << static_cast<const TagSet &>( obj );
    }

    Tag parseData( istream & in_Fr, TagParser & ctag_Cr ) const {
      unsigned idx_ii = TagSet::parseData( in_Fr, ctag_Cr );
      return (idx_ii == TagSet::unknowntag) ? Max_Tag : (Tag)idx_ii;
    }

};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME :
//
//	DESCRIPTION :
//
class LanguageTags : private TagSet {

  public:

    enum Tag {
      Filename,
      Label,
      Notify,
      Delnotify,
      Description,
      // last entry
      Max_Tag
    };

    LanguageTags() {
      for ( Tag t = (Tag)0; t < Max_Tag; t = (Tag)(t+1) ) {
	switch ( t ) {
	  STAG( Filename );
	  STAG( Label );
	  DTAG( Notify, Yfiton );
	  DTAG( Delnotify, Yfitonled );
	  DTAG( Description, Noitpircsed );
	default:
	  y2internal( "Missing definition for LanguageTags::Tag %d", t );
	  addTag( "" );
	  break;
	}
      }
    }

  public:

    friend ostream & operator<<( ostream & str, const LanguageTags & obj ) {
      return str << "LanguageTags: " << static_cast<const TagSet &>( obj );
    }

    Tag parseData( istream & in_Fr, TagParser & ctag_Cr ) const {
      unsigned idx_ii = TagSet::parseData( in_Fr, ctag_Cr );
      return (idx_ii == TagSet::unknowntag) ? Max_Tag : (Tag)idx_ii;
    }

};

///////////////////////////////////////////////////////////////////

#undef STAG
#undef DTAG

#endif // TagSets_h
