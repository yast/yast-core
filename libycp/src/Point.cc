/*---------------------------------------------------------------------\
|								       |
|		       __   __	  ____ _____ ____		       |
|		       \ \ / /_ _/ ___|_   _|___ \		       |
|			\ V / _` \___ \ | |   __) |		       |
|			 | | (_| |___) || |  / __/		       |
|			 |_|\__,_|____/ |_| |_____|		       |
|								       |
|				core system			       |
|					Copyright 2003, SuSE Linux AG  |
\----------------------------------------------------------------------/

   File:	Point.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:	Klaus Kaempf <kkaempf@suse.de>

   Definition of "definition point" which stores
   - filename
   - line number
   - inclusion point
   to trace filenames, definition points, and include hierachies

   This helps in issuing proper error messages like
     "identifier <name>
      defined in <file1> at <line1>
      included from <file2> at <line2>
      included from <toplevel> at <line>"

   A TableEntry (identifier <name>) has a Point which stores the definition
   point (Point) of this identifier.
   If its Point is in an include file, the m_point member points to the
   inclusion point (where the 'include ".."' statement is) of the include
   file.

   Point is a linked list for definition points inside include files.

/-*/
// -*- c++ -*-

#include <string>
using std::string;

#define DO_DEBUG 0

#include "ycp/Point.h"
#include "ycp/Bytecode.h"
#include "ycp/Scanner.h"

#include "ycp/y2log.h"

//-------------------------------------------------------------------

// File point
// Filename is include at line in point
// If point == 0 then its a toplevel file (and the line should be 0)

Point::Point (std::string filename, int line, const Point *point)
    : m_entry (new SymbolEntry (Scanner::doStrdup (filename.c_str())))
    , m_line (line)
    , m_point (point)
{
#if DO_DEBUG
    y2debug ("Point (%s, %d, %s)", filename.c_str(), line, point ? point->toString().c_str() : "<nil>");
#endif
}


// Definition point
// sentry is defined in point at line

Point::Point (SymbolEntryPtr sentry, int line, const Point *point)
    : m_entry (sentry)
    , m_line (line)
    , m_point (point)
{
#if DO_DEBUG
    y2debug ("Point (<%s>, %d, %s)", sentry->toString().c_str(), line, point ? point->toString().c_str() : "<nil>");
#endif
}


Point::~Point (void)
{
}


SymbolEntryPtr 
Point::sentry (void) const
{
    return m_entry;
}


std::string
Point::filename (void) const
{
    return m_entry->name();
}


int
Point::line (void) const
{
    return m_line;
}


const Point *
Point::point (void) const
{
    return m_point;
}


std::string
Point::toString (void) const
{
    char numbuf[8]; sprintf (numbuf, "%d", m_line);
    string s = "Point <";
    s += m_entry->name();
    s += ">";
    if (m_point)
    {
	s += " included from ";
	s += m_point->toString();
	s += ":";
	s += string (numbuf);
    }
    return s;
}

//-----------------------------------------------
// stream I/O

Point::Point (bytecodeistream & str)
    : m_entry (Bytecode::readEntry (str))
    , m_line (Bytecode::readInt32 (str))
    , m_point (0)
{
#if DO_DEBUG
    y2debug ("Point::Point(<stream>): %s", toString().c_str());
#endif
    if (Bytecode::readBool (str))
    {
	m_point = new Point (str);
    }
}

std::ostream &
Point::toStream (std::ostream & str) const
{
#if DO_DEBUG
    y2debug ("Point::toStream (%s)", toString().c_str());
#endif
    Bytecode::writeEntry (str, m_entry);
    Bytecode::writeInt32 (str, m_line);
    if (m_point)
    {
	Bytecode::writeBool (str, true);
	m_point->toStream (str);
    }
    else
    {
	Bytecode::writeBool (str, false);
    }
    return str;
}


// EOF
