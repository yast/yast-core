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

   File:	YBuiltin.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include "ycp/y2log.h"
#include "ycp/Bytecode.h"

#include <ctype.h>

#include "ycp/StaticDeclaration.h"
StaticDeclaration static_declarations;


#include "ycp/YCPBuiltinVoid.h"
static YCPBuiltinVoid builtin_void;		// trigger constructor

#include "ycp/YCPBuiltinBoolean.h"
static YCPBuiltinBoolean builtin_boolean;	// trigger constructor

#include "ycp/YCPBuiltinByteblock.h"
static YCPBuiltinByteblock builtin_byteblock;	// trigger constructor

#include "ycp/YCPBuiltinInteger.h"
static YCPBuiltinInteger builtin_integer;	// trigger constructor

#include "ycp/YCPBuiltinFloat.h"
static YCPBuiltinFloat builtin_float;		// trigger constructor

#include "ycp/YCPBuiltinString.h"
static YCPBuiltinString builtin_string;		// trigger constructor

#include "ycp/YCPBuiltinPath.h"
static YCPBuiltinPath builtin_path;		// trigger constructor

#include "ycp/YCPBuiltinList.h"
static YCPBuiltinList builtin_list;		// trigger constructor

#include "ycp/YCPBuiltinMultiset.h"
static YCPBuiltinMultiset builtin_multiset;	// trigger constructor

#include "ycp/YCPBuiltinMap.h"
static YCPBuiltinMap builtin_map;		// trigger constructor

#include "ycp/YCPBuiltinTerm.h"
static YCPBuiltinTerm builtin_term;		// trigger constructor

#include "ycp/YCPBuiltinMisc.h"
static YCPBuiltinMisc builtin_misc;		// trigger constructor

// EOF
