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
#include "ycp/ExecutionEnvironment.h"

class SymbolTable;

// for logging
extern ExecutionEnvironment ee;

int yyparse (void *parser);

//-------------------------------------------------------------------

Parser::Parser()
    : m_scanner(0)
    , buffered(false)
    , m_depends (false)
{
    init ();
    m_at_eof = false;
    lineno = 0;
}

Parser::Parser(FILE *file, const char *filename)
    : m_scanner(0)
    , buffered(false)
    , m_depends (false)
{
    setInput(file, filename);
    init ();
    m_at_eof = false;
    lineno = 0;
}


Parser::Parser(const char *buf)
    : m_scanner (0)
    , buffered (false)
    , m_depends (false)
{
    setInput(buf);
    init ();
    m_at_eof = false;
    lineno = 0;
}


Parser::Parser(int fd, const char *filename)
    : m_scanner (0)
    , buffered (false)
    , m_depends (false)
{
    setInput(fd, filename);
    init ();
    m_at_eof = false;
    lineno = 0;
}


Parser::~Parser()
{
    if (m_scanner) delete m_scanner;
    restoreFilename ();
}


void
Parser::setInput(FILE *file, const char *filename)
{
    if (filename)
    {
	setFilename (filename);
    }
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner (file, filename);
    if (buffered) m_scanner->setBuffered ();
    m_at_eof = false;
}


void
Parser::setInput(const char *buf)
{
    setFilename ("");
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner (buf);
    if (buffered) m_scanner->setBuffered ();
    m_at_eof = false;
}


void
Parser::setInput(int fd, const char *filename)
{
    if (filename)
    {
	setFilename (filename);
    }
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner (fd, filename);
    if (buffered) m_scanner->setBuffered ();
    m_at_eof = false;
}


void
Parser::setBuffered()
{
    if (m_scanner) m_scanner->setBuffered ();
    buffered = true;
    m_at_eof = false;
}


void
Parser::setDepends()
{
    m_depends = true;
}


bool
Parser::atEOF()
{
    return m_at_eof;
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
    
    init ();

    m_scanner->initTables (gTable, lTable);

    result = 0;

    yyparse ((void *) this);
    if (lineno == -1)
    {
	m_at_eof = true;
    }
    return result;
}


const char *
Parser::filename () const
{
    return ee.filename().c_str ();
}


void
Parser::setFilename (const string f)
{
    m_restore_name = ee.filename();
    ee.setFilename (f);
}


void
Parser::restoreFilename () const
{
    ee.setFilename (m_restore_name);
}


void
Parser::init ()
{
    result = 0;
    loopCount = 0;
    parserErrors = 0;
    blockStack = 0;
    scannerStack = 0;
    current_block = 0;
    blockstack_depth = 0;
}

