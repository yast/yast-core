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

   File:       Parser.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Klaus Kaempf <kkaempf@suse.de>

   interface to the bison generated parser

$Id$
/-*/

#include <stdlib.h>

#include "ycp/Parser.h"
#include "ycp/Scanner.h"
#include "ycp/y2log.h"

class SymbolTable;

int yyparse (void *parser);

Parser::Parser()
    : m_scanner(0)
    , buffered(false)
    , at_eof (false)
    , result (0)
    , lineno (0)
    , loopCount (0)
    , parserErrors (0)
    , blockStack (0)
    , scannerStack (0)
{
}

Parser::Parser(FILE *file, const char *filename)
    : m_scanner(0)
    , buffered(false)
    , at_eof (false)
    , result (0)
    , lineno (0)
    , loopCount (0)
    , parserErrors (0)
    , blockStack (0)
    , scannerStack (0)
{
    setInput(file, filename);
}


Parser::Parser(const char *buf)
    : m_scanner(0)
    , buffered(false)
    , at_eof (false)
    , result (0)
    , lineno (0)
    , loopCount (0)
    , parserErrors (0)
    , blockStack (0)
    , scannerStack (0)
{
    setInput(buf);
}


Parser::Parser(int fd, const char *filename)
    : m_scanner(0)
    , buffered(false)
    , at_eof (false)
    , result (0)
    , lineno (0)
    , loopCount (0)
    , parserErrors (0)
    , blockStack (0)
    , scannerStack (0)
{
    setInput(fd, filename);
}

Parser::~Parser()
{
    if (m_scanner) delete m_scanner;
}

void
Parser::setInput(FILE *file, const char *filename)
{
    if (filename) file_name = filename;
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner(file, filename);
    if (buffered) m_scanner->setBuffered();
    at_eof = false;
}


void
Parser::setInput(const char *buf)
{
    file_name = "";
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner(buf);
    if (buffered) m_scanner->setBuffered();
    at_eof = false;
}


void
Parser::setInput(int fd, const char *filename)
{
    if (filename) file_name = filename;
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner(fd, filename);
    if (buffered) m_scanner->setBuffered();
    at_eof = false;
}


void
Parser::setBuffered()
{
    if (m_scanner) m_scanner->setBuffered();
    buffered = true;
    at_eof = false;
}


bool
Parser::atEOF()
{
    return at_eof;
}

Scanner *
Parser::scanner ()
{
    return m_scanner;
}

void
Parser::setScanner (Scanner *s)
{
    m_scanner = s;
}

YCode *
Parser::parse (SymbolTable *gTable, SymbolTable *lTable)
{
    extern int yydebug;

    yydebug = (getenv ("YCP_YYDEBUG") == 0) ? 0 : 1;

    if (yydebug)
	y2debug ("Running with full debug");

    if (m_scanner == 0)
    {
	y2internal("Not input for the parser has been set");
	return 0;
    }

    m_scanner->initTables (gTable, lTable);

    result = 0;

    yyparse ((void *) this);
    if (lineno == -1)
    {
	at_eof = true;
    }
    return result;
}

const char *
Parser::filename () const
{
    return file_name.c_str ();
}

void
Parser::SetFilename (const string f)
{
    file_name = f;
}


