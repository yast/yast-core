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

   File:       YCPInterpreter.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-
/*
 * YCP interpreter that defines the builtins
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef YCPInterpreter_h
#define YCPInterpreter_h

#include "YCPBasicInterpreter.h"

/**
 * @short YCP interpreter that implements many builtin functions
 * The class @ref YCPBasicInterpreter only implements the YCP core
 * language. But there are many functions and operators that are
 * common to all application specific YCP interpreters, like
 * number arithmetic, boolean algebra and so on. These functions
 * are implemented in this class.
 */
class YCPInterpreter : public YCPBasicInterpreter
{
public:

    /**
     * This method is called by YCPBasicInterpreter when evaluating
     * a builtin.
     */
    YCPValue evaluateBuiltinBuiltin (builtin_t code, const YCPList& args);

    /**
     * This method is called by YCPBasicInterpreter when evaluating
     * a term.
     */
    YCPValue evaluateBuiltinTerm (const YCPTerm& term);

};

// Builtin Ops
//

/**
 * Implements the builtin select
 */
YCPValue evaluateSelect(YCPInterpreter *interpreter, const YCPList& list);

/**
 * Implements the builtin remove
 */
YCPValue evaluateRemove(YCPInterpreter *interpreter, const YCPList& list);

/**
 * Implements the builtin lookup
 */
YCPValue evaluateLookup(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin size
 */
YCPValue evaluateSize(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin foreach
 */
YCPValue evaluateForeach(YCPInterpreter *interpreter, const YCPList& args);


// Builtin Terms
//

/**
 * Implements the builtin add
 */
YCPValue evaluateAdd(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin change
 */
YCPValue evaluateChange(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin contains
 */
YCPValue evaluateContains(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin haskey
 */
YCPValue evaluateHasKey(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin union
 */
YCPValue evaluateUnion(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin symbolof
 */
YCPValue evaluateSymbolOf(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin time
 */
YCPValue evaluateTime (YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin sleep
 */
YCPValue evaluateSleep(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin random
 */
YCPValue evaluateRandom(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin srandom
 */
YCPValue evaluateSrandom(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin crypt
 */
YCPValue evaluateCrypt(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin cryptmd5
 */
YCPValue evaluateCryptMD5(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin cryptbigcrypt
 */
YCPValue evaluateCryptBigcrypt(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin cryptblowfish
 */
YCPValue evaluateCryptBlowfish(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin tointeger
 */
YCPValue evaluateToInteger(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin tofloat
 */
YCPValue evaluateToFloat(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin tostring
 */
YCPValue evaluateToString(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin tostring
 */
YCPValue evaluateToHexString(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin topath
 */
YCPValue evaluateToPath(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin topath
 */
YCPValue evaluateToTerm (YCPInterpreter* interpreter, const YCPList& args);

/**
 * Implements the builtin substring
 */
YCPValue evaluateSubString(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin issubstring
 */
YCPValue evaluateIsSubString(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin findfirstnotof
 */
YCPValue evaluateFindFirstNotOf(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin findfirstof
 */
YCPValue evaluateFindFirstOf(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin findlastof
 */
YCPValue evaluateFindLastOf(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin tolower
 */
YCPValue evaluateToLower(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin toupper
 */
YCPValue evaluateToUpper(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin toascii
 */
YCPValue evaluateToASCII(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin filterchars
 */
YCPValue evaluateFilterChars(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin deletechars
 */
YCPValue evaluateRemoveChars(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin filter
 */
YCPValue evaluateFilter(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin maplist
 */
YCPValue evaluateMaplist(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin mapmap
 */
YCPValue evaluateMapmap(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin listmap
 */
YCPValue evaluateListmap(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin flatten
 */
YCPValue evaluateFlatten(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implementes the builtin toset
 */
YCPValue evaluateToSet(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin sformat
 */
YCPValue evaluateSformat(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin sort
 */
YCPValue evaluateSort(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin find
 */
YCPValue evaluateFind(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin splitstring
 */
YCPValue evaluateSplitString(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin mergestring
 */
YCPValue evaluateMergeString(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin fileexist
 */
YCPValue evaluateFileExist(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin prepend
 */
YCPValue evaluatePrepend(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin regexptokenize
 */
YCPValue evaluateRegexpTokenize(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin regexpsub
 */
YCPValue evaluateRegexpSub(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin regexpmatch
 */
YCPValue evaluateRegexpMatch(YCPInterpreter *interpreter, const YCPList& args);

/**
 * Implements the builtin y2log
 */
YCPValue evaluateY2Debug(YCPInterpreter *interpreter, const YCPList& args);
YCPValue evaluateY2Milestone(YCPInterpreter *interpreter, const YCPList& args);
YCPValue evaluateY2Warning(YCPInterpreter *interpreter, const YCPList& args);
YCPValue evaluateY2Error(YCPInterpreter *interpreter, const YCPList& args);
YCPValue evaluateY2Security(YCPInterpreter *interpreter, const YCPList& args);
YCPValue evaluateY2Internal(YCPInterpreter *interpreter, const YCPList& args);

#endif // YCPInterpreter_h
