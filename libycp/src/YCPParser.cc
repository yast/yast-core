/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|							 (C) SuSE GmbH |
\----------------------------------------------------------------------/

   File:       YCPParser.cc

   Author:     Mathias Kettner <kettner@suse.de>
   Maintainer: Thomas Roelz <tom@suse.de>

/-*/
/*
 * YCP interface to the bison generated parser
 *
 * Author: Mathias Kettner <kettner@suse.de>
 */

#include <stdlib.h>

#include "YCPParser.h"
#include "YCPScanner.h"
#include "parserret.h"
#include "y2log.h"


int yyparse(void *ycpscanner);


YCPParser::YCPParser ()
    : scanner (0),
      buffered (false)
{
}


YCPParser::YCPParser (FILE *file, const char *filename)
    : scanner (0),
      buffered (false)
{
    setInput (file, filename);
}


YCPParser::YCPParser (const char *buf)
    : scanner (0),
      buffered (false)
{
    setInput (buf);
}


YCPParser::YCPParser (int fd, const char *filename)
    : scanner (0),
      buffered (false)
{
    setInput (fd, filename);
}


YCPParser::~YCPParser()
{
    if (scanner) delete scanner;
}


void YCPParser::setInput(FILE *file, const char *filename)
{
    if (filename) file_name = filename;
    if (scanner) delete scanner;
    scanner = new YCPScanner(file, filename);
    if (buffered) scanner->setBuffered();
}


void YCPParser::setInput(const char *buf)
{
    file_name = "";
    if (scanner) delete scanner;
    scanner = new YCPScanner(buf);
    if (buffered) scanner->setBuffered();
}


void YCPParser::setInput(int fd, const char *filename)
{
    if (filename) file_name = filename;
    if (scanner) delete scanner;
    scanner = new YCPScanner(fd, filename);
    if (buffered) scanner->setBuffered();
}


void YCPParser::setBuffered()
{
    if (scanner) scanner->setBuffered();
    buffered = true;
}


YCPValue YCPParser::parse ()
{
    extern int yydebug;

    yydebug = (getenv ("YCP_YYDEBUG") == 0) ? 0 : 1;

    if (yydebug)
	y2debug ("Running with full debug");

    if (scanner == 0)
    {
	y2internal("Not input for the parser has been set");
	return YCPNull();
    }

    struct parserret pr;
    pr.scanner = scanner;
    pr.result = YCPNull();
    pr.filename = file_name.c_str();

    yyparse((void *)&pr);
    return pr.result;
}
