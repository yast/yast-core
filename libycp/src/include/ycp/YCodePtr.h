/*----------------------------------------------------------*- c++ -*--\
|                                                                      |
|                      __   __    ____ _____ ____                      |
|                      \ \ / /_ _/ ___|_   _|___ \                     |
|                       \ V / _` \___ \ | |   __) |                    |
|                        | | (_| |___) || |  / __/                     |
|                        |_|\__,_|____/ |_| |_____|                    |
|                                                                      |
|                               core system                            |
|                                                    (C) SuSE Linux AG |
\----------------------------------------------------------------------/

   File:       YCodePtr.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef YCodePtr_h
#define YCodePtr_h

#include <y2util/RepDef.h>

// YCodePtr
// constYCodePtr

DEFINE_BASE_POINTER(YCode);
DEFINE_DERIVED_POINTER(YError, YCode);
DEFINE_DERIVED_POINTER(YConst, YCode);
DEFINE_DERIVED_POINTER(YLocale, YCode);
DEFINE_DERIVED_POINTER(YFunction, YCode);

// needed in YCode.h already
DEFINE_DERIVED_POINTER(YBlock, YCode);

#endif   // YCodePtr_h

