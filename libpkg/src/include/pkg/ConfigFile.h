/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 * ----------------------------------------------------------
 *
 * Author: 	  Stefan Schubert  <schubi@suse.de>
 *
 * File:	  ConfigFile.h
 * Description:   main header file for handling YaST-Files
 *
 * $Header$
 *
 *************************************************************/

/*
 * $Log$
 * Revision 1.1  2002/06/21 16:04:39  arvin
 * Initial revision
 *
 * Revision 1.4  2001/11/08 09:24:29  schubi
 * taking new versions from pkginfo server
 *
 * Revision 1.3  2001/07/04 16:50:47  arvin
 * - adapt for new automake/autoconf
 * - partly fix for gcc 3.0
 *
 * Revision 1.2  2000/05/17 14:32:03  schubi
 * update Modus added after new cvs
 *
 * Revision 1.2  2000/05/05 17:47:50  schubi
 * tested version
 *
 * Revision 1.1  2000/05/04 16:38:40  schubi
 * read and writes YaST-configure files
 *
 *
 *
 */

// -*- C++ -*-

#ifndef ConfigFile_h
#define ConfigFile_h

#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <list>
#include <string>

using std::map;
using std::pair;
using std::list;
using std::string;

/**
 * @short Interface to handle YaST-config-files
 */

typedef list<string> Values;

typedef struct _Element {
   Values values;
   bool multiLine; // This flag has mulit-line values and is closed
                   // by the inverted flag-name.
} Element;
typedef map<string, Element> Entries;

class ConfigFile
{
public:
  /**
   * Create an new instance.
   * filename includes the whole path
   */
  ConfigFile( string filename );

  /**
   * Clean up.
   */
  ~ConfigFile();

  /**
   * Read the file. Returns all "entries" which are seperated
   * by "seperators"
   */
   bool readFile ( Entries &entries, string seperators );

  /**
   * Write the file.
   */
   bool writeFile ( Entries &entries, string comments,
		    char seperator, string filename = "" );

private:

   /**
    *  upshift a string
    */
   string upshift ( string str );

   /**
    *  Inverting a String
    */
   string invertString ( string str );

   /**
    *  Parse a line of YaST-configuration-file
    **/
   bool evalLine ( const char *line,
		   Entries &entries,
		   string seperators );

  /**
   * The name of the file with path.
   */
  string pathname;

  /**
   * Last found Entry-Flag
   */
   Entries::iterator lastEntry;

};

#endif
