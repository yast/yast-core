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

   File:       Package.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef Package_h
#define Package_h

#include <sys/types.h>

#include <iosfwd>
#include <string>

#include <pkg/pkginfo.h>

using namespace std;

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : Package
//
//	DESCRIPTION :
//
class Package {

  protected:

    Package() {}
    virtual ~Package() {}

    DependList asDependList( const string & data_tr ) const;
    TagList    asTagList   ( const string & data_tr ) const;

  public:

    enum Type { NOTYPE, RPM, SPM };

    virtual unsigned instIdx()  const = 0;

    virtual Type   type()	const = 0;
    virtual string name()	const = 0;
    virtual string version()	const = 0;
    virtual string group()	const = 0;
    virtual string label()	const = 0;
    virtual string description()const = 0;

    virtual string requires()	const = 0;
    virtual string provides()	const = 0;
    virtual string conflicts()	const = 0;
    virtual string obsoletes()	const = 0;

    virtual unsigned onCD()	const = 0;
    virtual string instPath()	const = 0;
    virtual time_t buildTime()	const = 0;
    virtual string builtFrom()	const = 0;
    virtual off_t  rpmSize()	const = 0;
    virtual off_t  uncompSize()	const = 0;

    virtual string instNotify()	const = 0;
    virtual string delNotify()	const = 0;
    virtual string series()	const = 0;

    virtual bool   isBasepkg()	const = 0;

    virtual string author()	const = 0;
    virtual string copyright()	const = 0;

    // unused YaST1 stuff
    string category()	  const { return string(); }
    string startCommand() const { return string(); }

  public:

    // convenience stuff

    // think about introducing a Size class, that stores byte internaly
    // and provides methods to retieve the size in various units and
    // string formats. That would leave it up to the one finaly using it,
    // what kind of size he likes to use.
    off_t sizeInB() const { return uncompSize(); }
    off_t sizeInK() const { return uncompSize()/1024; }

  public: // we'd like to get rid of theese

    PackageKey asPackageKey() const { return PackageKey( name(), version() ); }

    // ***************************************************************
    // distinction between DepL and TagL is not correct as each kind
    // of dependency type might contain operator and version.
    // ***************************************************************

    DependList requiresList()  const { return asDependList( requires() ); }
    DependList conflictsList() const { return asDependList( conflicts() ); }
    TagList    providesList()  const { return asTagList   ( provides() ); }
    TagList    obsoletesList() const { return asTagList   ( obsoletes() ); }

};

///////////////////////////////////////////////////////////////////

typedef const Package *   PkdData;
typedef list<PkdData>     PkdDataList;

///////////////////////////////////////////////////////////////////
#endif // Package_h
