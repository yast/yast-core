#include <iostream>
#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>

using namespace std;
using namespace stringutil;

void chksp( const std::string & line_r,
	    const bool          singlesep_r = false,
	    const std::string & sep_t       = ":" ) {
  std::vector<std::string> words_r;
  unsigned n = split( line_r, words_r, sep_t, singlesep_r );
  SEC << '"' << line_r << '"'
    << "\t-> words " << n << "\t";
  if ( words_r.size() ) {
    SEC << "|";
    for ( unsigned i = 0; i < words_r.size(); ++i ) {
      SEC << words_r[i] << "|";
    }
  }
  SEC << endl;
}

void chktr( const std::string & line_r ) {
  MIL << "->" << '"' << line_r << '"' << endl;
  MIL << " l" << '"' << ltrim(line_r) << '"' << endl;
  MIL << " r" << '"' << rtrim(line_r) << '"' << endl;
  MIL << " t" << '"' <<  trim(line_r) << '"' << endl;
}

void chkul( const std::string & line_r ) {
  MIL << "->" << '"' << line_r << '"' << endl;
  MIL << " l" << '"' << toLower(line_r) << '"' << endl;
  MIL << " u" << '"' << toUpper(line_r) << '"' << endl;
}

int main()
{
  set_log_filename("-");
  DBG << "START" << endl;

  chkul( "" );
  chkul( "aBcDeF" );

  chktr( "" );
  chktr( " \t \n " );
  chktr( "a  aa" );
  chktr( " a  aa   " );

  chksp( "" );
  chksp( ":" );
  chksp( "a" );
  chksp( "::a" );
  chksp( "::a::" );
  chksp( "::a::b:c::" );

  chksp( "", true );
  chksp( ":", true  );
  chksp( "a", true  );
  chksp( ":a", true  );
  chksp( "a:", true  );
  chksp( ":a:", true  );


  DBG << "==================================" << endl;
  return 0;
}
