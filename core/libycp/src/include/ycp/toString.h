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

   File:       toString.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * String conversion functions
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef toString_h
#define toString_h

#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using std::string;


inline string toString(int d)
{
    char s[32];
    snprintf(s, 32, "%d", d);
    return string(s);
}


inline string toString(long ld)
{
    char s[32];
    snprintf(s, 32, "%ld", ld);
    return string(s);
}

inline string toString(long long Ld)
{
    char s[32];
    snprintf(s, 32, "%Ld", Ld);
    return string(s);
}

#endif // toString_h

