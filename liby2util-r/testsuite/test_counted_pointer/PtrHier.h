#ifndef PtrHier_h
#define PtrHier_h

#include <iostream>
#include <set>

#include <y2util/RepDef.h>

using namespace std;

#define XX dumpOn ( cout ) << "  {" << __FUNCTION__ << "}" << endl

////////////////////////////////////////////////////////////////////////////////

DEFINE_BASE_POINTER( SLV );
DEFINE_DERIVED_POINTER( OBJ, SLV, SLV );
DEFINE_DERIVED_POINTER( PKG, SLV, SLV );
DEFINE_DERIVED_POINTER( YOU, SLV, SLV );

DEFINE_BASE_POINTER( XXX );

////////////////////////////////////////////////////////////////////////////////

struct SLV : public Rep {
  virtual const char * repName() const 	{ return typeid(*this).name(); }
  public:
  static unsigned _ID;
  static set<unsigned> _IDS;
  unsigned _id;
  SLV() { _id = ++_ID; _IDS.insert(_id); }
  virtual ~SLV() { _IDS.erase(_id); }
  virtual ostream & dumpOn( ostream & str ) const {
    return Rep::dumpOn( str << '[' << _id << ']' );
  }
};
unsigned SLV::_ID = 0;
set<unsigned> SLV::_IDS;

struct OBJ : public SLV {
  //REP_BODY(OBJ);
  public:
    OBJ(){}
};

struct PKG : public OBJ {
  //REP_BODY(PKG);
  public:
    PKG(){}
};

struct YOU : public OBJ {
  //REP_BODY(YOU);
  public:
    YOU(){}
};

struct XXX : public Rep {
  //REP_BODY(XXX);
  public:
  static unsigned _ID;
  static set<unsigned> _IDS;
  unsigned _id;
  XXX() { _id = ++_ID; _IDS.insert(_id); }
  virtual ~XXX() { _IDS.erase(_id); }
  virtual ostream & dumpOn( ostream & str) const {
    return Rep::dumpOn( str << '[' << _id << ']' );
  }
};
unsigned XXX::_ID = 0;
set<unsigned> XXX::_IDS;

////////////////////////////////////////////////////////////////////////////////

#define use(x) do { if ( 0 ) cout << sizeof(x); } while ( 0 )

////////////////////////////////////////////////////////////////////////////////

IMPL_BASE_POINTER( SLV );
IMPL_DERIVED_POINTER( OBJ, SLV, SLV );
IMPL_DERIVED_POINTER( PKG, SLV, SLV );
IMPL_DERIVED_POINTER( YOU, SLV, SLV );

IMPL_BASE_POINTER( XXX );

////////////////////////////////////////////////////////////////////////////////

#endif // PtrHier_h
