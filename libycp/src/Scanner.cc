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

   File:       Scanner.cc

   YCP interface to the flex generated scanner

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

$Id$
/-*/

#include <string>
#include <unistd.h>
#include <stdarg.h>

#include "ycp/Scanner.h"
#include "ycp/y2log.h"

#include "ycp/SymbolTable.h"

Scanner::Scanner (FILE *inputfile, const char *fname)
    : m_filename (fname ? fname : "")
    , m_inputBuffer (0)
    , m_inputFile (inputfile)
    , m_inputFd (-1)
    , m_scannedType ("")
    , m_lineNumber (1)
    , m_scandataBuffer (0)
    , m_scandataBufferSize (0)
    , m_buffered (false)
    , m_globalTable (0)
    , m_localTable (0)
    , m_owningGlobal (false)
    , m_owningLocal (false)
{
    m_scannedValue.val = 0;
}


Scanner::Scanner (const char *inputbuffer)
    : m_filename ("")
    , m_inputBuffer (inputbuffer)
    , m_inputFile (0)
    , m_inputFd (-1)
    , m_scannedType ("")
    , m_lineNumber (1)
    , m_scandataBuffer (0)
    , m_scandataBufferSize (0)
    , m_buffered (false)
    , m_globalTable (0)
    , m_localTable (0)
    , m_owningGlobal (false)
    , m_owningLocal (false)
{
    m_scannedValue.val = 0;
}


Scanner::Scanner (int input_fd, const char *fname)
    : m_filename (fname ? fname : "")
    , m_inputBuffer (0)
    , m_inputFile (0)
    , m_inputFd (input_fd)
    , m_scannedType ("")
    , m_lineNumber (1)
    , m_scandataBuffer (0)
    , m_scandataBufferSize (0)
    , m_buffered (false)
    , m_globalTable (0)
    , m_localTable (0)
    , m_owningGlobal (false)
    , m_owningLocal (false)
{
    m_scannedValue.sval = 0;
}


Scanner::~Scanner ()
{
    if (m_scandataBuffer != 0)
	free (m_scandataBuffer);

    if (m_owningGlobal)
    {
	delete (m_globalTable);
    }
    if (m_owningLocal)
    {
	delete (m_localTable);
    }
}


void
Scanner::setBuffered ()
{
    m_buffered = true;
}


void
Scanner::initTables (SymbolTable *globalTable, SymbolTable *localTable)
{
    if (globalTable != 0)
    {
	m_owningGlobal = false;
	m_globalTable = globalTable;
    }
    else if (m_globalTable == 0)
    {
	m_owningGlobal = true;
	m_globalTable = new SymbolTable (211);
	y2debug ("m_globalTable %p", m_globalTable);
    }

    if (localTable != 0)
    {
	m_owningLocal = false;
	m_localTable = localTable;
    }
    else if (m_localTable == 0)
    {
	m_owningLocal = true;
	m_localTable = new SymbolTable (211);
	y2debug ("m_localTable %p", m_localTable);
    }

    return;
}


SymbolTable *
Scanner::globalTable () const
{
    return m_globalTable;
}


SymbolTable *
Scanner::localTable () const
{
    return m_localTable;
}


// Scanner::int yylex()
// This method is created by flex++ and defined in scanner.cc

int
Scanner::LexerInput (char* buf, int maxnum)
{
    // reading from a buffer

    if (m_inputBuffer)
    {
	if (m_buffered)
	{
	    size_t len = strlen (m_inputBuffer);
	    size_t size = (len <= (size_t)maxnum) ? len : maxnum;
	    memcpy (buf, m_inputBuffer, size);
	    m_inputBuffer += size;
	    return size;
	}
	else if (*m_inputBuffer)
	{
	    *buf = *m_inputBuffer++;
	    return 1;
	}
	else
	{
	    return 0;
	}
    }

    // reading from an open file

    else if (m_inputFile)
    {
	return fread (buf, 1, m_buffered ? maxnum : 1, m_inputFile);
    }

    // reading from a file descriptor

    else if (m_inputFd >= 0)
    {
	ssize_t read_bytes = read (m_inputFd, buf, m_buffered ? maxnum : 1);
	if (read_bytes >= 0)
	    return read_bytes;
	else
	    return 0;
    }

    // no input established

    else
    {
	y2internal ("LexerInput without input defined");
	return 0;
    }
}


void
Scanner::LexerError (const char* msg)
{
    logError(msg, 0);
}


void
Scanner::logError (const char *loginfo, int lineno, ...)
{
    char logtext[4096];

    // Prepare info text

    va_list ap;
    va_start (ap, lineno);
    vsnprintf (logtext, 4095, loginfo, ap);
    va_end (ap);

    y2scanner (m_filename.empty() ? "YCP stream" : m_filename.c_str(), lineno, "Error: %s", logtext);
    return;
}


void
Scanner::logWarning (const char *loginfo, int lineno, ...)
{
    char logtext[4096];

    // Prepare info text

    va_list ap;
    va_start (ap, lineno);
    vsnprintf (logtext, 4095, loginfo, ap);
    va_end (ap);

    y2scanner ((m_filename.empty() ? "YCP stream" : m_filename.c_str()),
	       lineno, "Warning: %s", logtext);
}


void
Scanner::setScannedToken (tokenValue value, const TypeCode &type)
{
    m_scannedValue = value;
    m_scannedType = type;
}


tokenValue
Scanner::scannedValue () const
{
    return m_scannedValue;
}


TypeCode
Scanner::scannedType () const
{
    return m_scannedType;
}


int
Scanner::lineNumber () const
{
    return m_lineNumber;
}


char *
Scanner::extend_scanbuffer (int addsize)
{
    if (m_scandataBufferSize == 0)
    {
	m_scandataBuffer = 0;
    }

    addsize = (addsize > m_scandataBufferSize) ? addsize : m_scandataBufferSize;

    if (addsize < STRING_HUNK)
    {
	addsize = STRING_HUNK;
    }

    m_scandataBuffer = (char *)realloc (m_scandataBuffer, m_scandataBufferSize + addsize);
    if (m_scandataBuffer == 0)
    {
	y2error ("Scanner: no memory, bailing out");
	return 0;
    }
    m_scandataBufferSize += addsize;
    return m_scandataBuffer;
}
