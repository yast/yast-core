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
DEFINE_DERIVED_POINTER(FlexType, Type, Type);
DEFINE_DERIVED_POINTER(VariableType, Type, Type);
DEFINE_DERIVED_POINTER(ListType, Type, Type);
DEFINE_DERIVED_POINTER(MapType, Type, Type);
DEFINE_DERIVED_POINTER(BlockType, Type, Type);
DEFINE_DERIVED_POINTER(TupleType, Type, Type);
DEFINE_DERIVED_POINTER(FunctionType, Type, Type);

#endif   // TypePtr_h
