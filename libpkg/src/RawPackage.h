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

   File:       RawPackage.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef RawPackage_h
#define RawPackage_h

#include <iosfwd>

#include <pkg/Package.h>
#include <pkg/RawPackageInfo.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : RawPackage
//
//	DESCRIPTION :
//
class RawPackage : public Package {

  friend std::ostream & operator<<( std::ostream & str, const RawPackage & obj );

  private:

    friend class RawPackageInfo;
    const RawPackageInfo & master_C;

    RawPackage( const RawPackageInfo & master_Cr );
    virtual ~RawPackage();

    typedef RawPackageInfo::FileLocation FText;

    string fTextC( const FText & at_Cr ) const { return master_C.readCommon( at_Cr ); }
    string fTextL( const FText & at_Cr ) const { return master_C.readLanguage( at_Cr ); }

  private:

    unsigned instIdx_i;

    Type   type_e;
    string name_t;
    string version_t;
    string group_t;
    FText  label_t;
    FText  description_t;

    FText  requires_t;
    FText  provides_t;
    FText  conflicts_t;
    FText  obsoletes_t;

    unsigned int onCD_i;
    string CDPath_t;
    string CDFile_t;
    time_t buildTime_i;
    FText  builtFrom_t;
    off_t  rpmSize_i;
    off_t  uncompSize_i;

    FText  instNotify_t;
    FText  delNotify_t;
    string series_t;

    bool   isBasepkg_b;

    FText  copyright_t;
    FText  author_t;

    // not shure if we want this here
    mutable PackagePartitionSizeMap partitionSizeMap_C;

  public:

    virtual unsigned instIdx()  const { return instIdx_i; }

    virtual Type   type()	const { return type_e; }
    virtual string name()	const { return name_t; }
    virtual string version()	const { return version_t; }
    virtual string group()	const { return group_t; }
    virtual string label()	const { return fTextL( label_t ); }
    virtual string description()const { return fTextL( description_t ); }

    virtual string requires()	const { return fTextC( requires_t ); }
    virtual string provides()	const { return fTextC( provides_t ); }
    virtual string conflicts()	const { return fTextC( conflicts_t ); }
    virtual string obsoletes()	const { return fTextC( obsoletes_t ); }

    virtual unsigned onCD()	const { return onCD_i; }

    virtual string instPath()	const
	{
	    // if the path already ends with .rpm (path from patch-descritption)
	    // don't add the filename
	    if (CDPath_t.size () > 4 && CDPath_t.substr (CDPath_t.size () - 4, 4) == ".rpm")
		return CDPath_t;
	    // add the filename to the path (path from common.pkd)
	    return CDPath_t + CDFile_t;
	}

    virtual time_t buildTime()	const { return buildTime_i; }
    virtual string builtFrom()	const { return fTextC( builtFrom_t ); }
    virtual off_t  rpmSize()	const { return rpmSize_i; }
    virtual off_t  uncompSize()	const { return uncompSize_i; }

    virtual string instNotify()	const { return fTextL( instNotify_t ); }
    virtual string delNotify()	const { return fTextL( delNotify_t ); }
    virtual string series()	const { return series_t; }

    virtual bool   isBasepkg()	const { return isBasepkg_b; }

    virtual string author()	const { return fTextC( author_t ); }
    virtual string copyright()	const { return fTextC( copyright_t ); }


    PackagePartitionSizeMap & partitionSizeMap() const { return partitionSizeMap_C; }
};

///////////////////////////////////////////////////////////////////

#endif // RawPackage_h
