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

   File:       ycpless.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef ycpless_h
#define ycpless_h

#include "YCPValue.h"

using std::binary_function;

/*
 * global comparison function to be used with STL-Containers as generic
 * ordering operator. Compares two YCPValues and returns
 *    true if the first value is less than the second one,
 *    false otherwise.
 */

struct ycpless : public binary_function<YCPValue, YCPValue, bool>
{
    bool operator()(const YCPValue& x, const YCPValue& y)
	const { return x->compare(y) == YO_LESS; };
};

#endif   // ycpless_h
