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

   File:       YCPScanner.cc

   YCP interface to the flex generated scanner

   Author:	Klaus Kaempf <kkaempf@suse.de>
		Mathias Kettner <kettner@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

/-*/

#include <string>
#include <unistd.h>
#include <stdarg.h>

#include "YCPScanner.h"
#include "YCP.h"
#include "y2log.h"

using std::min;
using std::max;

extern int yylineno;

YCPScanner::YCPScanner(FILE *inputfile, const char *fname)
    : filename(fname ? fname : "")
    , inputbuffer(0)
    , inputfile(inputfile)
    , input_fd(-1)
    , scanned_value(YCPNull())
    , scandata_buffer(0)
    , scandata_buffer_size(0)
    , buffered(false)
{
    line_number = 0;
    yylineno = 1;
}


YCPScanner::YCPScanner(const char *inputbuffer)
    : filename ("")
    , inputbuffer(inputbuffer)
    , inputfile(0)
    , input_fd(-1)
    , scanned_value(YCPNull())
    , scandata_buffer(0)
    , scandata_buffer_size(0)
    , buffered(false)
{
    line_number = 0;
    yylineno = 1;
}


YCPScanner::YCPScanner(int input_fd, const char *fname)
    : filename(fname ? fname : "")
    , inputbuffer(0)
    , inputfile(0)
    , input_fd(input_fd)
    , scanned_value(YCPNull())
    , scandata_buffer(0)
    , scandata_buffer_size(0)
    , buffered(false)
{
    line_number = 0;
    yylineno = 1;
}


YCPScanner::~YCPScanner()
{
    if (scandata_buffer != 0)
	free(scandata_buffer);
}


void YCPScanner::setBuffered()
{
    buffered = true;
}



// YCPScanner::int yylex()
// This method is created by flex++ and defined in scanner.cc

int YCPScanner::LexerInput( char* buf, int maxnum )
{
    // reading from a buffer

    if (inputbuffer)
    {
	if (buffered)
	{
	    size_t size = min(strlen(inputbuffer), (size_t)maxnum);
	    memcpy(buf, inputbuffer, size);
	    inputbuffer += size;
	    return size;
	}
	else if (*inputbuffer)
	{
	    *buf = *inputbuffer++;
	    return 1;
	}
	else
	{
	    return 0;
	}
    }

    // reading from an open file

    else if (inputfile)
    {
	return fread(buf, 1, buffered ? maxnum : 1,inputfile);
    }

    // reading from a file descriptor

    else if (input_fd >= 0)
    {
	ssize_t read_bytes = read(input_fd, buf, buffered ? maxnum : 1);
	if (read_bytes >= 0) return read_bytes;
	else return 0;
    }

    // no input established

    else
    {
	y2internal ("LexerInput without input defined");
	return 0;
    }
}


void YCPScanner::LexerError( const char* msg )
{
    logError(msg, 0);
}


void YCPScanner::logError(const char *loginfo, int lineno, ...)
{
    char logtext[4096];

    // Prepare info text

    va_list ap;
    va_start(ap, lineno);
    vsnprintf(logtext, 4095, loginfo, ap);
    va_end(ap);

    y2scanner ((filename.empty() ? "YCP stream" : filename.c_str()),
	       lineno, "Syntax error: %s", logtext);
}


void YCPScanner::setScannedValue(const YCPValue& v, int lineno)
{
    scanned_value = v;
    line_number = lineno;
}


YCPValue YCPScanner::getScannedValue()
{
    return scanned_value;
}


int YCPScanner::getLineNumber()
{
    return line_number;
}


char *YCPScanner::extend_scanbuffer (int addsize)
{
    if (scandata_buffer_size == 0)
	scandata_buffer = 0;

    addsize = max(addsize, scandata_buffer_size);

    if (addsize < STRING_HUNK)
	addsize = STRING_HUNK;

    scandata_buffer = (char *)realloc (scandata_buffer, scandata_buffer_size + addsize);
    if (scandata_buffer == 0)
    {
	y2error ("Scanner: no memory, bailing out");
	return 0;
    }
    scandata_buffer_size += addsize;
    return scandata_buffer;
}
