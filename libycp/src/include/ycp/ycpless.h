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

   File:	ycpless.h

   Authors:	Mathias Kettner <kettner@suse.de>
		Arvin Schnell <aschnell@suse.de>
   Maintainer:	Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

#ifndef ycpless_h
#define ycpless_h

#include "YCPValue.h"


/*
 * Global comparison functor usable as generic ordering operator for
 * STL-containers and as predicate for STL-algorithms.
 *
 * Compares two YCPValues and returns true if the first value is less than the
 * second one, false otherwise. Optionally the comparison is locale aware.
 */
class ycpless : public std::binary_function<YCPValue, YCPValue, bool>
{

public:

    ycpless(bool respect_locale = false) : respect_locale(respect_locale) {}

    bool operator()(const YCPValue& x, const YCPValue& y) const
    {
	return x->compare(y, respect_locale) == YO_LESS;
    }

private:

    const bool respect_locale;

};


/*
 * Global comparison functor usable as predicate for STL-algorithms.
 *
 * Compares two YCPValues and returns true if they are equal, false otherwise.
 * Optionally the comparison is locale aware.
 */
class ycpequal_to : public std::binary_function<YCPValue, YCPValue, bool>
{

public:

    ycpequal_to(bool respect_locale = false) : respect_locale(respect_locale) {}

    bool operator()(const YCPValue& x, const YCPValue& y) const
    {
	return x->compare(y, respect_locale) == YO_EQUAL;
    }

private:

    const bool respect_locale;

};


#endif   // ycpless_h
