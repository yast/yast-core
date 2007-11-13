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

   File:       Y2SLog.h

   Author:     Michael Andres <ma@suse.de>
   Maintainer: Michael Andres <ma@suse.de>

/-*/
#ifndef Y2SLog_h
#define Y2SLog_h

#include <iostream>

// Don't know why y2log.h insists on having a component name
// defined and throws an error if it's missing.
// However, I want Y2SLog to work out of the box.
#ifndef Y2LOG
#define Y2LOG "DEFINE_Y2LOG"
#endif

#include <y2util/y2log.h>

///////////////////////////////////////////////////////////////////
//
namespace Y2SLog {
//
///////////////////////////////////////////////////////////////////

extern bool dbg_enabled_bm;

extern std::ostream & get( const char * which, const unsigned level,
			   const char * fil, const char * fnc, const int lne );

extern std::ostream & getdbg( const char * which, const unsigned level,
			      const char * fil, const char * fnc, const int lne );

} // namespace Y2SLog
///////////////////////////////////////////////////////////////////

#define _Y2SLOG(c,l)    Y2SLog::get( c, l, __FILE__, __FUNCTION__, __LINE__ )
#define _Y2SLOD(c,l)    Y2SLog::get( c"++", 1, __FILE__, __FUNCTION__, __LINE__ )
#define _Y2SLOGDBG(c,l) Y2SLog::getdbg( c"-dbg", l, __FILE__, __FUNCTION__, __LINE__ )
#define _Y2SLODDBG(c,l) Y2SLog::getdbg( c"-dbg++", 1, __FILE__, __FUNCTION__, __LINE__ )

//
// To log to component 'foo' write:
//
//        _DBG("foo") << ....
// or
//        #define Y2LOG "foo"
//        DBG << ....
//

#define _DBG(c) _Y2SLOD( c, 0 )
#define _MIL(c) _Y2SLOG( c, 1 )
#define _WAR(c) _Y2SLOG( c, 2 )
#define _ERR(c) _Y2SLOG( c, 3 )
#define _SEC(c) _Y2SLOG( c, 4 )
#define _INT(c) _Y2SLOG( c, 5 )

#define DBG _DBG(Y2LOG)
#define MIL _MIL(Y2LOG)
#define WAR _WAR(Y2LOG)
#define ERR _ERR(Y2LOG)
#define SEC _SEC(Y2LOG)
#define INT _INT(Y2LOG)

//
// To enable debug output (using component 'foo-dbg'), set 'Y2SLog::dbg_enabled_bm = true;'.
// Unless the environmental variable Y2SLOG_DEBUG is defined (with arbitrary value),
// debug output is disabled by default.
//
//        _D__("foo") << ....
//
//        #define Y2LOG "foo"
//        D__ << ....
//
#define _D__(c) _Y2SLODDBG( c, 0 )
#define _M__(c) _Y2SLOGDBG( c, 1 )
#define _W__(c) _Y2SLOGDBG( c, 2 )
#define _E__(c) _Y2SLOGDBG( c, 3 )
#define _S__(c) _Y2SLOGDBG( c, 4 )
#define _I__(c) _Y2SLOGDBG( c, 5 )

#define D__ _D__(Y2LOG)
#define M__ _M__(Y2LOG)
#define W__ _W__(Y2LOG)
#define E__ _E__(Y2LOG)
#define S__ _S__(Y2LOG)
#define I__ _I__(Y2LOG)

///////////////////////////////////////////////////////////////////

#endif // Y2SLog_h
