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

   File:       YCPParser.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCP interface to the bison generated parser
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#ifndef YCPParser_h
#define YCPParser_h

#include <stdio.h>
#include <string>

#include "YCP.h"
class YCPScanner;

/**
 * @short YCP parser
 * A YCP parser read a characters stream and outputs a sequence
 * of YCP values. Three properties of the YCP grammar are important:
 *
 * 1. The syntactical representation of a YCP value uniquely defines
 * its type.
 *
 * 2. The interpretation of the syntactical representation of a YCP
 * value is not dependend on the leading context.
 *
 * 3. The interpretation of the syntactical representation of a YCP
 * value is not dependend on the trailing context other than one
 * trailing white space.
 *
 * Property 1 allows you to call the parser without the specification,
 * which type of value you expect. This allows a YCP protocol
 * block to be of any YCP value.
 *
 * Property 2 allows the parser class to be free of variables that
 * must be kept between to parses.
 *
 * Property 3 is especially important, because it allows the parser
 * to determine the end of a value without having to look ahead more
 * that one character.
 */
class YCPParser
{
    /**
     * This is where the parser gets its input from.
     */
    YCPScanner *scanner;

    /**
     * Is true, if the input can be buffered, i.e. more than one
     * character may be read at once in order to gain performance.
     */
    bool buffered;

    /**
     * Filename of file just being parsed.
     */
    string file_name;

public:
    /**
     * Creates a new YCP parser. Afterwards you must set an input
     * source with a call to @ref #setInput
     */
    YCPParser();

    /**
     * Creates a new YCP parser
     * @param filename If you have the name of the file you
     * parse available, pass it here in order to get nice error
     * location messages in case of a parse error.
     */
    YCPParser(FILE *file, const char *filename=0);

    /**
     * Creates a new YCP parser
     */
    YCPParser(const char *buf);

    /**
     * Creates a new YCP parser
     * @param filename If you have the name of the file you
     * parse available, pass it here in order to get nice error
     * location messages in case of a parse error.
     */
    YCPParser(int fd, const char *filename=0);

    /**
     * Cleans up.
     */
    ~YCPParser();

    /**
     * Reads in as many bytes from the input stream as are neccessary
     * to parse a YCP value.
     * @return the parsed value. Returns 0, if no value could be parsed
     * (due to a parse error or the end of the input stream). The value
     * must be deleted after use.
     */
    YCPValue parse();

    /**
     * use file for further parsing.
     * @param filename If you have the name of the file you
     * parse available, pass it here in order to get nice error
     * location messages in case of a parse error.
     */
    void setInput(FILE *file, const char *filename=0);

    /**
     * use buf for further parsing
     */
    void setInput(const char *buf);

    /**
     * Parse a value from a file descriptor
     * @param filename If you have the name of the file you
     * parse available, pass it here in order to get nice error
     * location messages in case of a parse error.
     */
    void setInput(int fd, const char *filename=0);

    /**
     * Makes the scanner use buffering, i.e. read more than
     * one character at once.
     */
    void setBuffered();

    /**
     * Resets the parser. Use this call before you parse from
     * a new source. It resets the line numbers in the scanner.
     */
    void reset();
};

#endif // YCPParser_h
