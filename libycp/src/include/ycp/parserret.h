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

   File:       parserret.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
// -*- c++ -*-

/*
 * Temporary struct used by bison interface
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef parserret_h
#define parserret_h

class YCPScanner;
class YCPValueRep;

struct parserret
{
    YCPScanner *scanner;
    YCPValue result;
    int lineno;
    const char *filename;
    parserret() : result(YCPNull()), lineno (0) { }
};


#endif // parserret_h
