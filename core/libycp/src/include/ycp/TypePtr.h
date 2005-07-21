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

   File:       TypePtr.h

   Author:     Klaus Kaempf <kkaempf@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/

#ifndef TypePtr_h
#define TypePtr_h

#include <y2util/RepDef.h>

// TypePtr
// constTypePtr

DEFINE_BASE_POINTER(Type);
DEFINE_DERIVED_POINTER(FlexType, Type);
DEFINE_DERIVED_POINTER(NFlexType, Type);
DEFINE_DERIVED_POINTER(VariableType, Type);
DEFINE_DERIVED_POINTER(ListType, Type);
DEFINE_DERIVED_POINTER(MapType, Type);
DEFINE_DERIVED_POINTER(BlockType, Type);
DEFINE_DERIVED_POINTER(TupleType, Type);
DEFINE_DERIVED_POINTER(FunctionType, Type);

#endif   // TypePtr_h
