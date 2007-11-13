#include <iostream>
#include <fstream>
#include <map>
#include <list>
#include <string>

#include <y2util/Y2SLog.h>
#include <y2util/stringutil.h>
#include <y2util/PathInfo.h>
using namespace std;

#include "PtrHier.h"

#define CASEES 3

//=====================================================================================
bool assertPath( const Pathname & path ) {
  PathInfo ddir( path );
  if ( ddir.isDir() ) {
    return true;
  }
  if ( assertPath( path.dirname() ) ) {
    return( PathInfo::mkdir( path ) == 0 );
  }
  return false;
}
//=====================================================================================

struct D {
  static string defType;
  string Ptr;
  string p;
  D( const string & Pt, const string & P ) {
    Ptr = Pt;
    p = P;
  }
};

string D::defType;

#define genSTR(C,P,p) struct C : public D { \
  C() \
    : D( stringutil::form( P, defType.c_str() ), \
	 stringutil::form( p, defType.c_str() ) ) {} \
  C( const string & type ) \
    : D( stringutil::form( P, type.c_str() ), \
	 stringutil::form( p, type.c_str() ) ) {} \
}

genSTR( DP,   "%sPtr",             "%s *" );
genSTR( cDP,  "const%sPtr",        "const %s *" );
genSTR( DPc,  "const %sPtr",       "%s *const" );
genSTR( cDPc, "const const%sPtr",  "const %s *const" );

#define O(x)    DBG << #x << "  " << x << endl
#define Osze(x) SEC << #x << "  " << sizeof( x ) << endl

static ofstream PSTR( "/dev/stdout" );

//=====================================================================================

typedef void (*bodyfnc)();

//=====================================================================================
#define TG cout << "===[" << __LINE__ << "]=============================================" << endl
#define LE " TG;\n"

void construct() {
  PSTR <<
" {\n"
"  from f = new fobj;"	LE
"  type a( f );"	LE
"  cout << \"RESULT: \" << a << endl;"	LE
" }\n"
  << endl;
}

void constructAss() {
  PSTR <<
" {\n"
"  from f = new fobj;"	LE
"  type a = f; "	LE
"  cout << \"RESULT: \" << a << endl;"	LE
" }\n"
  << endl;
}

void constructRef() {
  PSTR <<
" {\n"
"  from f = new fobj;"	LE
"  type & a( f );\n"	LE
"  cout << \"RESULT: \" << a << endl;"	LE
" }\n"
  << endl;
}

void constructRefAss() {
  PSTR <<
" {\n"
"  from f = new fobj;"	LE
"  type & a = f;\n"	LE
"  cout << \"RESULT: \" << a << endl;"	LE
" }\n"
  << endl;
}

void assign() {
  PSTR <<
" {\n"
"  from f = new fobj;"	LE
"  type a = 0;"		LE
"  a = f;"		LE
"  cout << \"RESULT: \" << a << endl;"	LE
" }\n"
  << endl;
}

//=====================================================================================

string srcfile( bodyfnc fnc ) {
  static map<bodyfnc,string> _srcfile;
  if ( _srcfile.empty() ) {
    _srcfile[construct]       = "construct";
    _srcfile[constructAss]    = "construct_via_assign";
    _srcfile[constructRef]    = "construct_reference";
    _srcfile[constructRefAss] = "construct_reference_via_assign";
    _srcfile[assign]          = "assign";
  }
  return _srcfile[fnc];
}

string srcfile( bodyfnc fnc, const string & type, const string & from ) {
  string ret( srcfile( fnc ) );
  if ( ! ret.empty() ) {
    ret = ( stringutil::form( (ret+"@%s from %s").c_str(), type.c_str(), from.c_str() ) );
    for ( unsigned i = 0; i < ret.size(); ++i ) {
      switch ( ret[i] ) {
      case ' ':
	ret[i] = '_';
	break;
      case '<':
	ret[i] = '[';
	break;
      case '>':
	ret[i] = ']';
	break;
      case '*':
	ret[i] = '+';
	break;
      }
    }
  }
  return ret;
}

struct namesplit {
  string splitfrom;
  string func;
  string type;
  bool   tp;
  string from;
  bool   fp;
  namesplit() {tp=fp=false;}
  namesplit( const string & name ) { split( name ); }
  void reconv( string & name, bool & isp ) {
    isp = false;
    for ( unsigned i = 0; i < name.size(); ++i ) {
      switch ( name[i] ) {
      case '_':
	name[i] = ' ';
	break;
      case '[':
	name[i] = '<';
	break;
      case ']':
	name[i] = '>';
	break;
      case '+':
	name[i] = '*';
	isp = true;
	break;
      }
    }
  }
  void reconv( string & name ) {
    bool isp;
    reconv( name, isp );
  }
  void split( string name ) {
    *this = namesplit();
    splitfrom = name;
    string::size_type pos = name.rfind( "/" );
    if ( pos != string::npos ) {
      name.erase( 0, pos+1 );
    }
    pos = name.rfind( "." );
    if ( pos != string::npos ) {
      name.erase( pos );
    }
    pos = name.find( "@" );
    if ( pos == string::npos ) {
      return;
    }
    func = name.substr( 0, pos );
    name.erase( 0, pos+1 );
    pos = name.find( "_from_" );
    type = name.substr( 0, pos );
    from = name.substr( pos+6 );
    reconv( func );
    reconv( type, tp );
    reconv( from, fp );
    //DBG << func << ": " << tp << fp << ": " << type << " " << from << endl;
  }
};

//=====================================================================================

void prog( bodyfnc fnc, const string & type, const string & from, const string & frombase, Pathname subd ) {
  string fname;
  Pathname ddir( Pathname("test")+subd );
  if ( assertPath( ddir ) ) {
    fname = srcfile( fnc, type, from );
  }
  if ( ! fname.empty() ) {
    fname = (ddir + (fname+".cc")).asString();
    PSTR.close();
    PSTR.open( fname.c_str() );
    DBG << "Writing " << fname << endl;
  }
  PSTR << "#include <iostream>" << endl;
  PSTR << "#include <set>" << endl;
  PSTR << "using namespace std;" << endl;
  PSTR << "#include \"PtrHier.h\"" << endl;
  PSTR << "#define TG cout << \"===[\" << __LINE__ << \"]=============================================\" << endl;" << endl;
  PSTR << "typedef " << type     << " type;" << endl;
  PSTR << "typedef " << from     << " from;" << endl;
  PSTR << "typedef " << frombase << " fobj;" << endl;
  PSTR << "ostream & operator<<( ostream & str, const set<unsigned> & obj ) {" << endl;
  PSTR << "  str << \"Set(\" << obj.size() << \")\";" << endl;
  PSTR << "  for ( set<unsigned>::const_iterator it = obj.begin(); it != obj.end(); ++it ) {" << endl;
  PSTR << "    str << endl << \"  \" << *it;" << endl;
  PSTR << "  }" << endl;
  PSTR << "  str << endl;" << endl;
  PSTR << "  return str;" << endl;
  PSTR << "}" << endl;
  PSTR << "int main() {" << endl;
  fnc();
  PSTR << "  TG;" << endl;
  PSTR << "  cout << \"SLV\" << SLV::_IDS;" << endl;
  PSTR << "  cout << \"XXX\" << XXX::_IDS;" << endl;
  PSTR << "return 0; }" << endl;

  if ( ! fname.empty() ) {
    PSTR.close();
    PSTR.open( "/dev/stdout" );
  }
}

//=====================================================================================

// fnc for all Ptr and '*' conbinations
void allProgs( bodyfnc fnc, const D & type, const D & from, const string & frombase, Pathname subd ) {
  string dname( srcfile( fnc, type.Ptr, from.Ptr ) );
  if ( dname.empty() )
    return;
  subd += dname;

#if 0
  unsigned d = 2;
#endif
  string td[] = { type.Ptr, type.p };
  string fd[] = { from.Ptr, from.p };

#if 0
  for ( unsigned t = 0; t < d; ++t ) {
    for ( unsigned f = 0; f < d; ++f ) {
      prog( fnc, td[t], fd[f], subd );
    }
  }
#endif

  prog( fnc, td[0], fd[0], frombase, subd );
  prog( fnc, td[0], fd[1], frombase, subd );
  // no (*,Ptr) combination
  prog( fnc, td[1], fd[1], frombase, subd );
}

// fnc for all const combinations of Ptr
void allComb( bodyfnc fnc, const string & type, const string & from, Pathname subd ) {
  string dname( srcfile( fnc ) );
  if ( dname.empty() )
    return;
  subd += dname;

  unsigned d = 4;
  D td[] = { DP(type), cDP(type), DPc(type), cDPc(type) };
  D fd[] = { DP(from), cDP(from), DPc(from), cDPc(from) };

  for ( unsigned t = 0; t < d; ++t ) {
    for ( unsigned f = 0; f < d; ++f ) {
      allProgs( fnc, td[t], fd[f], from, subd );
    }
  }
}

//=====================================================================================
void genTestSrc( const string & type, const string & from ) {
  Pathname subd ( type + "_from_" + from );
  allComb( construct,       type, from, subd );
  allComb( constructAss,    type, from, subd );
  allComb( constructRef,    type, from, subd );
  allComb( constructRefAss, type, from, subd );
  allComb( assign,          type, from, subd );
}

void genTestSrc() {
  genTestSrc( "SLV", "SLV" );
  genTestSrc( "OBJ", "PKG" );
  genTestSrc( "PKG", "OBJ" );
}

//=====================================================================================
void DumpOn( ostream & str, const Pathname & file ) {
  ifstream s( file.asString().c_str() );

  while ( s ) {
    string l = stringutil::getline( s );
    if ( !(s.fail() || s.bad()) ) {
       str << "    " << l << endl;
    }
  }
}

void subTestResult( const Pathname & path ) {
  list<string> retlist;
  int res = PathInfo::readdir( retlist, path, false );
  if ( res ) {
    ERR << "Error reading content of " << path << " (readdir " << res << ")" << endl;
    return;
  }
  map<string,Pathname> flst;
  for ( list<string>::const_iterator it = retlist.begin(); it != retlist.end(); ++it ) {
    PathInfo cpath( path + *it );
    if ( cpath.isFile() ) {
      if ( cpath.path().asString().find( ".testgpp" ) != string::npos ) {
	flst[*it] = ( cpath.size() ? cpath.path() : Pathname() );
      }
    } else if ( cpath.isDir() ) {
      subTestResult( cpath.path() );
    }
  }

  if ( ! flst.empty() ) {
    if ( flst.size() != CASEES ) {
      INT << "NOT " << CASEES << ": " << flst.size() << ": " << path << endl;
    }
    namesplit spl( path.asString() );
    vector<pair<Pathname,namesplit> > cases;
    cases.resize( CASEES );
    off_t casetag = -1;
    bool caseok  = true;
    for ( map<string,Pathname>::const_iterator it = flst.begin(); it != flst.end(); ++it ) {
      namesplit sp( it->first );
      if ( casetag == -1 ) {
	casetag = it->second.empty();
      }
      if ( bool(casetag) != it->second.empty() ) {
	caseok = false;
      }
      unsigned idx = CASEES;
      if ( sp.tp == sp.fp ) {
	idx = ( sp.tp ? 2 : 0 );
      } else {
	idx = ( sp.tp ? 3 : 1 ); // 3 must not occur
	if ( idx >= CASEES ) {
	  INT << "Case idx " << idx << " must not occur!" << endl;
	}
      }
      cases[idx] = pair<Pathname,namesplit>( it->second, sp );
    }

    if ( caseok ) {
      MIL << stringutil::form( "%s: %-20s from %-20s -> %s"
			       , spl.func.c_str()
			       , spl.type.c_str()
			       , spl.from.c_str()
			       , (casetag?"good":"fail") ) << endl;
      if ( casetag ) {
	for( unsigned i = 0; i < CASEES; ++i ) {
	  Pathname ff( path );
	  string nn( cases[i].second.splitfrom );
	  nn.replace( nn.end()-3, nn.end(), "run" );
	  ff += nn;
	  SEC << stringutil::form( "%s: %-20s from %-20s < %s"
				   , cases[i].second.func.c_str()
				   , cases[i].second.type.c_str()
				   , cases[i].second.from.c_str()
				   , ff.basename().c_str() ) << endl;
	 DumpOn( SEC, ff );
	}
      }
    } else {
      ERR << stringutil::form( "%s: %-20s from %-20s"
			       , spl.func.c_str()
			       , spl.type.c_str()
			       , spl.from.c_str() ) << endl;
      for( unsigned i = 0; i < CASEES; ++i ) {
	WAR << stringutil::form( "    %s: %-20s from %-20s",
				 (cases[i].first.empty()?"good":"fail"),
				 cases[i].second.type.c_str(),
				 cases[i].second.from.c_str() ) << endl;
      }
      for( unsigned i = 0; i < CASEES; ++i ) {
	if ( ! cases[i].first.empty() ) {
	  PathInfo pi( cases[i].first );
	  DBG << pi << endl;
	  DumpOn( DBG, pi.path() );
	} else {
	  Pathname ff( path );
	  string nn( cases[i].second.splitfrom );
	  nn.replace( nn.end()-3, nn.end(), "run" );
	  ff += nn;
	  SEC << stringutil::form( "%s: %-20s from %-20s < %s"
				   , cases[i].second.func.c_str()
				   , cases[i].second.type.c_str()
				   , cases[i].second.from.c_str()
				   , ff.basename().c_str() ) << endl;
	 DumpOn( SEC, ff );
	}
      }
    }
  }
}

void checkTestResult() {
  Pathname root( "test" );
  subTestResult( root /*+ "construct_via_assign"*/ );
}

//=====================================================================================

void test() {

  OBJPtr a;
  constSLVPtr b( a );

}

int main( int argc, const char * argv[] ) {
  set_log_filename("-");
  SEC << "START" << endl;

#define SZ(T) MIL << #T << " " << sizeof( T ) << endl
  SZ( SLVPtr );
  SZ( constSLVPtr );
  SZ( OBJPtr );
  SZ( constOBJPtr );
  SZ( PKGPtr );
  SZ( constPKGPtr );

  bool do_test            = false;
  bool do_genTestSrc      = false;
  bool do_checkTestResult = false;

  if ( argc <= 1 ) {
    do_test = true;
  } else {
    for( int i = 1; i < argc; ++i ) {
      if ( string(argv[i]) == "gen" ) {
	do_genTestSrc = true;
      } else if ( string(argv[i]) == "check" ) {
	do_checkTestResult = true;
      }
    }
  }

  if ( do_genTestSrc )
    genTestSrc();
  if ( do_checkTestResult )
    checkTestResult();

  if ( do_test )
    test();

  SEC << "STOP" << endl;
  return 0;
}
