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

   File:	Point.h

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

   Point works as a linked list (file1 -> file2 -> toplevel in the above
   example) for definition points inside include files. The real structure
   is a tree since for the next include of file3 inside file2, the list
   is file3 -> file2 -> toplevel and the latter two nodes are shared.

   An identifier has a definition point. A file has a filename and
   an inclusion point (if its an included file).
/-*/
// -*- c++ -*-

#ifndef Point_h
#define Point_h

#include <string>
using std::string;

#include "ycp/SymbolEntry.h"

class Point : public MemUsage
{
  private:
    SymbolEntry *m_entry;		// filename as SymbolEntry (c_filename)
    int m_line;				// line of definition / inclusion
    const Point *m_point;		// points to toplevel point for include files
  public:
    Point (std::string filename, int line = 0, const Point *point = 0);
    Point (SymbolEntry *sentry, int line = 0, const Point *point = 0);
    Point (std::istream & str);
    ~Point (void);

    SymbolEntry *sentry (void) const;
    std::string filename (void) const;
    int line (void) const;
    const Point *point (void) const;

    std::string toString (void) const;
    std::ostream & toStream (std::ostream & str) const;
};
#endif // Point_h
