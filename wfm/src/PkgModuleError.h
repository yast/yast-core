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

  File:       PkgModuleError.h

  Author:     Michael Andres <ma@suse.de>
  Maintainer: Michael Andres <ma@suse.de>

  Purpose: Definition of "PkgModule" error values.

/-*/
#ifndef PkgModuleError_h
#define PkgModuleError_h

#include <iosfwd>

#include <y2pm/ModulePkgError.h>

///////////////////////////////////////////////////////////////////
//
//	CLASS NAME : PkgModuleError
/**
 * @short Definition of "PkgModule" error values.
 * @see PMError
 **/
class PkgModuleError : private ModulePkgError {

  private:

    friend class PMError;

    static const std::string errclass;

    static std::string errtext( const unsigned e );

  public:

    enum Error {
      E_ok    = PMError::E_ok,			// no error
      E_error = PMError::C_ModulePkgError,	// some error
      // more specific errors start here:
      E_bad_args,
    };

    friend std::ostream & operator<<( std::ostream & str, const Error & obj ) {
      return str << PMError( obj );
    }

  public:

    PkgModuleError() {
      errtextfnc = errtext;
    }
};

///////////////////////////////////////////////////////////////////

#endif // PkgModuleError_h

