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

   File:       Parser.h

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

/-*/
/*
 * YCP interface to the bison generated parser
 */

#ifndef Parser_h
#define Parser_h

#include <stdio.h>
#include <string>

#include "ycp/Scanner.h"
#include "ycp/YCode.h"

class Scanner;
class blockstack_t;
class scannerstack_t;
class YBlock;

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
class Parser
{
    /**
     * This is where the parser gets its input from.
     */ 
    Scanner *m_scanner;

    /**
     * Is true, if the input can be buffered, i.e. more than one
     * character may be read at once in order to gain performance.
     */
    bool m_buffered;

    /**
     * Is true, if just imported modules and included files are
     * to be checked (make depends)
     */
    bool m_depends;

    /**
     * Filename of ExecutionEnvironment, restore at destructor
     */
    string m_restore_name;
    
    /**
     * If true, the scanner reached EOF.
     */
    bool m_at_eof;

public:
    /**
     * Copied from struct parserret
     * Does not need to be deleted by us
     */
    YCodePtr m_result;

    /**
     * Copied from struct parserret
     */
    int m_lineno;

    //parserret:
    //const char *filename;
    //parserret() : result (0), lineno (0) { }

    /**
     * Loop nesting level. Detects break outside a loop.
     * Was a static variable in parser.yy
     */
    int m_loop_count;

    /**
     * Errors during one parse.
     * Was a static variable in parser.yy
     */
    int m_parser_errors;

    /**
     * Stack of parsed blocks 
     */
    blockstack_t *m_block_stack;

    /**
     * Scanners used for include parsing
     */
    scannerstack_t *m_scanner_stack;
    
    /**
     * pointer to the currently parsed block
     */
    YBlockPtr m_current_block;
    
    /**
     * integer number for the depth of the current block
     */
    int m_blockstack_depth;
    
    /**
     * Initialize the internal state of the parser.
     */ 
    void init ();

public:
    /**
     * Creates a new YCP parser. Afterwards you must set an input
     * source with a call to @ref #setInput
     */
    Parser();

    /**
     * Creates a new YCP parser
     * @param filename If you have the name of the file you
     * parse available, pass it here in order to get nice error
     * location messages in case of a parse error.
     */
    Parser(FILE *file, const char *filename=0);

    /**
     * Creates a new YCP parser
     */
    Parser(const char *buf);

    /**
     * Creates a new YCP parser
     * @param filename If you have the name of the file you
     * parse available, pass it here in order to get nice error
     * location messages in case of a parse error.
     */
    Parser(int fd, const char *filename=0);

    /**
     * Cleans up.
     */
    ~Parser();

    /**
     * Reads in as many bytes from the input stream as are neccessary
     * to parse a YCP file. 
     * @return the parsed value. Returns 0, if no value could be parsed
     * (due to a parse error or the end of the input stream). The value
     * must be deleted after use.
     *
     * If gTable and lTable are set, they're used instead of local
     * ones. This is used for include files using the symbol tables
     * of the including block.
     * see: Scanner::initTables()
     */
    YCodePtr parse(SymbolTable *gTable = 0, SymbolTable *lTable = 0);

    /**
     * Accesses the scanner
     */
    Scanner *scanner ();

    /**
     * Sets the scanner.
     * For scannerstack use, this should be encapsulated better.
     */
    void setScanner (Scanner *);

    /**
     * report EOF state.
     * If parse() returns 0 the caller should check atEOF() in order
     * to distinguish between a syntax error and end-of-file.
     */
    bool atEOF ();

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
    void setInput(int fd, const char *filename = 0);

    /**
     * Makes the scanner use buffering, i.e. read more than
     * one character at once.
     */
    void setBuffered();

    /**
     * Just output dependencies.
     */
    void setDepends();

    /**
     * Only dependencies ?
     */
    bool depends() const;

    /**
     * Accesses filename from ExecutionEnvironment
     */
    const char *filename () const;

    /**
     * Sets filename in ExecutionEnvironment
     */
    void setFilename (const string f);

    /**
     * Restore filename to ExecutionEnvironment
     */
    void restoreFilename () const;

    /**
     * Resets the parser. Use this call before you parse from
     * a new source. It resets the line numbers in the scanner.
     */
//    void reset();
};

#endif // Parser_h
