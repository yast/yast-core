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

   File:       YCP.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Basic YCP library
 */

#ifndef YCP_h
#define YCP_h

// include all basic types

#include <ycp/YCPBoolean.h>
#include <ycp/YCPVoid.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPFloat.h>
#include <ycp/YCPString.h>
#include <ycp/YCPByteblock.h>
#include <ycp/YCPPath.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPDeclAny.h>
#include <ycp/YCPDeclType.h>
#include <ycp/YCPDeclList.h>
#include <ycp/YCPDeclStruct.h>
#include <ycp/YCPDeclTerm.h>
#include <ycp/YCPLocale.h>
#include <ycp/YCPTerm.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPBlock.h>
#include <ycp/YCPBuiltin.h>
#include <ycp/YCPIdentifier.h>
#include <ycp/YCPWhileBlock.h>
#include <ycp/YCPDoWhileBlock.h>
#include <ycp/YCPBreakStatement.h>
#include <ycp/YCPContinueStatement.h>
#include <ycp/YCPEvaluationStatement.h>
#include <ycp/YCPReturnStatement.h>
#include <ycp/YCPIfThenElseStatement.h>
#include <ycp/YCPNestedStatement.h>
#include <ycp/YCPBuiltinStatement.h>
#include <ycp/YCPError.h>

#endif // YCP_h
