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

/-*/

#include <stdlib.h>

#include "ycp/Parser.h"
#include "ycp/Scanner.h"
#include "ycp/y2log.h"
#include "ycp/StaticDeclaration.h"
#include "ycp/SymbolTable.h"

extern StaticDeclaration static_declarations;

int yyparse (void *parser);

//-------------------------------------------------------------------

Parser::Parser()
    : m_scanner(0)
    , m_buffered(false)
    , m_depends (false)
{
    init ();
    m_at_eof = false;
    m_lineno = 0;
}

Parser::Parser(FILE *file, const char *filename)
    : m_scanner(0)
    , m_buffered(false)
    , m_depends (false)
{
    setInput(file, filename);
    init ();
    m_at_eof = false;
    m_lineno = 0;
}


Parser::Parser(const char *buf)
    : m_scanner (0)
    , m_buffered (false)
    , m_depends (false)
{
    setInput(buf);
    init ();
    m_at_eof = false;
    m_lineno = 0;
}


Parser::Parser(int fd, const char *filename)
    : m_scanner (0)
    , m_buffered (false)
    , m_depends (false)
{
    setInput(fd, filename);
    init ();
    m_at_eof = false;
    m_lineno = 0;
}


Parser::~Parser()
{
    if (m_scanner) delete m_scanner;
}


void
Parser::setInput(FILE *file, const char *filename)
{
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner (file, filename);
    if (m_buffered) m_scanner->setBuffered ();
    m_at_eof = false;
}


void
Parser::setInput(const char *buf)
{
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner (buf);
    if (m_buffered) m_scanner->setBuffered ();
    m_at_eof = false;
}


void
Parser::setInput(int fd, const char *filename)
{
    if (m_scanner) delete m_scanner;
    m_scanner = new Scanner (fd, filename);
    if (m_buffered) m_scanner->setBuffered ();
    m_at_eof = false;
}


void
Parser::setBuffered()
{
    if (m_scanner) m_scanner->setBuffered ();
    m_buffered = true;
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


YCodePtr
Parser::parse (SymbolTable *gTable, SymbolTable *lTable)
{
#if 0
    extern int yydebug;

    yydebug = (getenv ("YCP_YYDEBUG") == 0) ? 0 : 1;

    if (yydebug)
	y2debug ("Running with full debug");
#endif

    if (m_scanner == 0)
    {
	y2internal("Not input for the parser has been set");
	return 0;
    }
    
    init ();

    m_scanner->initTables (gTable, lTable);

    m_result = 0;

    yyparse ((void *) this);
    
    if (m_lineno == -1)
    {
	m_at_eof = true;
    }
    return m_result;
}


void
Parser::init ()
{
    m_result = 0;
    m_loop_count = 0;
    m_parser_errors = 0;
    m_block_stack = 0;
    m_scanner_stack = 0;
    m_current_block = 0;
    m_blockstack_depth = 0;

    // initialize preloaded namespaces
    const std::list<std::pair<std::string, Y2Namespace *> > & active_predefined = static_declarations.active_predefined();
    std::list<std::pair<std::string, Y2Namespace *> >::const_iterator it;
    for (it = active_predefined.begin(); it != active_predefined.end(); it++)
    {
        Y2Namespace *name_space = it->second;
        name_space->table()->endUsage();
        name_space->table()->startUsage();
    }
}

