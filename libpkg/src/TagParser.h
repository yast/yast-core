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

 File:       TagParser.h

 Author:     Michael Andres <ma@suse.de>
 Maintainer: Michael Andres <ma@suse.de>

 /-*/
#ifndef TagParser_h
#define TagParser_h

#include <iosfwd>
#include <string>
#include <vector>
#include <map>

using std::vector;
using std::map;
using std::streampos;
using std::istream;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagParser
//
//
//	DESCRIPTION :
//
class TagParser {

  private:

    static const unsigned bufferLen_i;
    static char           buffer_ac[];

  private:

    streampos lookupStart_i;
    streampos startTag_i;
    streampos startData_i;
    streampos endData_i;
    streampos endTag_i;

    string    startTag_t;
    string    endTag_t;

    vector<string> data_Vt;

  private:

    void _reset();
    void _datareset();

    static streampos  readLine( istream & stream_fr, string & cline_tr );
    static bool       tagOnLine( const string & cline_tr, string & tag_tr, string::size_type & delim_ir );

  public:

    TagParser();
    virtual ~TagParser();

    static const streampos nopos;

    streampos      lookupStart() const { return lookupStart_i; }

    streampos      posStartTag() const { return startTag_i; }
    const string & startTag()    const { return startTag_t; }

    streampos      posEndTag()   const { return endTag_i; }
    const string & endTag()      const { return endTag_t; }

  public:

    streampos      posDataStart()const { return startData_i; }
    streampos      posDataEnd()  const { return endData_i; }
    unsigned       dataLength()  const { return endData_i - startData_i; }
    unsigned       dataLines()   const { return data_Vt.size(); }

    const vector<string> & data() const { return data_Vt; }

  public:

    bool lookupTag( istream & stream_fr, const string & stag_tr = "" );
    bool parseData( istream & stream_fr, const string & etag_tr );

    static bool retrieveData( istream & stream_fr,
			      streampos startData_ir, streampos endData_ir,
			      string & data_tr );
    static bool retrieveData( istream & stream_fr,
			      streampos startData_ir, streampos endData_ir,
			      vector<string> & data_Vtr );

    static string data2string( const vector<string> & data_Vtr );

    static vector<string> split2words( const string & line_tr, const string & sepchars_tr = " \t\n" );
};

///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : TagSet
//
//	DESCRIPTION :
//
class TagSet {

  friend std::ostream & operator<<( std::ostream & str, const TagSet & obj );

  private:

    map<string,unsigned> lookupStart_V;
    vector<string>       lookupEnd_V;

  public:

    static const unsigned unknowntag = (unsigned)-1;

    TagSet() {}
    virtual ~TagSet() {}

    void addTag( const string & start_tr, const string & end_tr = "" );

    unsigned parseData( istream & in_Fr, TagParser & ctag_Cr ) const;
};

///////////////////////////////////////////////////////////////////

extern std::ostream & operator<<( std::ostream & str, const TagParser & obj );
extern std::ostream & operator<<( std::ostream & str, const TagSet & obj );

///////////////////////////////////////////////////////////////////

#endif // TagParser_h
