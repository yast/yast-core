/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 * ----------------------------------------------------------
 *
 * File:	  ConfigFile.cc
 *
 * Author: 	  Stefan Schubert <schubi@suse.de>
 *
 * Description:   Handle YaST-config-files
 *
 * $Header$
 *
 *************************************************************/

/*
 * $Log$
 * Revision 1.1  2002/06/21 16:04:39  arvin
 * Initial revision
 *
 * Revision 1.7  2002/03/28 10:25:06  schubi
 * merge with 8.0 tree
 *
 * Revision 1.6.2.1  2002/03/13 10:06:17  arvin
 * - fixed compile problem
 *
 * Revision 1.6  2001/11/08 09:24:29  schubi
 * taking new versions from pkginfo server
 *
 * Revision 1.10  2001/07/04 16:50:47  arvin
 * - adapt for new automake/autoconf
 * - partly fix for gcc 3.0
 *
 * Revision 1.9  2001/07/04 14:25:05  schubi
 * new selection groups works besides the old
 *
 * Revision 1.8  2001/07/03 13:40:39  msvec
 * Fixed all y2log calls.
 *
 * Revision 1.7  2000/12/22 10:11:16  schubi
 * error messages removed
 *
 * Revision 1.6  2000/11/30 12:03:46  schubi
 * reading and writing the correct install.inf format
 *
 * Revision 1.5  2000/09/29 09:22:38  schubi
 * new calls : readInstallInf and writeInstallInf
 *
 * Revision 1.4  2000/08/04 13:29:38  schubi
 * Changes from 7.0 to 7.1; Sorry Klaus, I do not know anymore
 *
 * Revision 1.3  2000/05/30 15:43:43  kkaempf
 * fix include paths
 *
 * Revision 1.2  2000/05/17 14:32:03  schubi
 * update Modus added after new cvs
 *
 * Revision 1.3  2000/05/11 11:47:55  schubi
 * update modus added
 *
 * Revision 1.2  2000/05/05 17:47:50  schubi
 * tested version
 *
 * Revision 1.1  2000/05/04 16:38:40  schubi
 * read and writes YaST-configure files
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <iostream.h>
#include <fstream.h>
#include <ctype.h>

#include <ycp/y2log.h>
#include <pkg/ConfigFile.h>


#define BUFFERLEN	1100
#define READLEN		1000
#define IDENTLENG	50


/****************************************************************/
/* public member-functions					*/
/****************************************************************/

/*-------------------------------------------------------------*/
/* creates a Config-file-object				       */
/*-------------------------------------------------------------*/

ConfigFile::ConfigFile( string filename )
{
   pathname  = filename;
}

/*--------------------------------------------------------------*/
/* Cleans up						       	*/
/*--------------------------------------------------------------*/
ConfigFile::~ConfigFile()
{
}


/*--------------------------------------------------------------*/
/*  Read the file						*/
/*--------------------------------------------------------------*/
bool ConfigFile::readFile ( Entries &entries, string seperators)
{
   char buffer[BUFFERLEN +1];
   FILE *file = NULL;
   bool ok = true;

   lastEntry = entries.end();

   file = fopen (pathname.c_str(), "r");
   if ( !file )
   {
      y2warning( "opening : %s",
	    pathname.c_str());
      return ( false );
   }

   if ( ok )
   {
      // parsing file
      buffer[0] = 0;
      while ( !feof(file )  )
      {
	 // reading a line

	 bool emptyLine = true;
	 unsigned i;

	 if ( fgets ( buffer, READLEN, file ) == NULL )
	 {
	    break;
	 }

	 for ( i = 0; i < strlen ( buffer ); i++ )
	 {
	    if ( buffer[i] != ' ' &&
		 buffer[i] != '\n' &&
		 buffer[i] != '\t' )
	    {
	       emptyLine = false;
	       i = strlen ( buffer );
	    }
	 }

	 if ( buffer[0] == 0 ||
	      buffer[0] == '\n' ||
	      buffer[0] == '#' ||
	      emptyLine )
	 {
	    // scipping comments and empty lines
	 }
	 else
	 {
	    ok = evalLine ( buffer, entries, seperators );
	 }
      }
   }

   if ( file )
   {
      fclose ( file );
   }

   return ( ok );
}


/*--------------------------------------------------------------*/
/*  Write the file						*/
/*--------------------------------------------------------------*/
bool ConfigFile::writeFile ( Entries &entries, string comments,
			     char seperator, string filename )
{
   bool ok = true;

   if ( filename.length() == 0 )
      filename = pathname;

   ofstream fp ( filename.c_str() );
   Entries::iterator pos;

   if ( !fp )
   {
      ok = false;
      y2warning( "opening : %s",
	 pathname.c_str());
   }

   if ( ok )
   {
      if ( comments.length() > 0 )
      {
	 fp << "#  " << comments <<endl;
	 fp << "# " << endl;
      }

      for ( pos = entries.begin(); pos != entries.end(); pos++ )
      {
	 Values::iterator posValues;

	 fp << (string)pos->first << seperator << " ";
	 if ( (pos->second).multiLine )
	    fp << endl;

	 for ( posValues = (pos->second).values.begin();
	       posValues != (pos->second).values.end();
	       posValues++ )
	 {
	    fp << *posValues;
	    if ( (pos->second).multiLine )
	    {
	       fp << endl;
	    }
	    else
	    {
	       fp << " ";
	    }
	 }
	 if ( (pos->second).multiLine )
	 {
	    string inverted = invertString ( pos->first );
	    inverted[0] = toupper( inverted[0] );
	    inverted[inverted.length()-1] =
	       tolower(inverted[inverted.length()-1]);
	    fp << inverted << seperator << endl;
	 }
	 else
	 {
	    fp << endl;
	 }
      }
   }

   return ( ok );
}


/****************************************************************/
/* private member-functions					*/
/****************************************************************/

/*--------------------------------------------------------------*/
/*  Upshifting a Flag						*/
/*--------------------------------------------------------------*/
string ConfigFile::upshift ( string str )
{
   unsigned i;
   string ret = "";

   for ( i = 0; i <  str.length(); i++ )
   {
      ret += (char) toupper( (int)str[i] );
   }

   return ( ret );
}




/*--------------------------------------------------------------*/
/*  Inverting a Flag						*/
/*--------------------------------------------------------------*/
string ConfigFile::invertString ( string str )
{
   int i;
   string ret = "";

   for ( i =  str.length()-1 ; i >= 0; i-- )
   {
      ret += str[i];
   }

   return ( ret );
}


/*--------------------------------------------------------------*/
/* Parse a line of YaST-configuration-file			*/
/* IN: string to evaluate, Entry-Map				*/
/* RETURN: bool							*/
/*--------------------------------------------------------------*/
bool ConfigFile::evalLine ( const char *line,
			    Entries &entries,
			    string seperators )
{
   string 		seperatorfirst(seperators);
   string	 	seperator(" \t");
   string 		lineBuffer( line );
   string 		value[]={"","","","","","","","","",""};
   int			counter = 0;
   string::size_type 	begin, end;
   int			ok = 1;
   bool 		newFlagFound = true;

   if ( line == NULL )
   {
      return ( 0 );
   }

   if ( lastEntry != entries.end() )
   {
      // Check, if the last entry have a multi-line values
      // Then special seperator like ":" will be ignored while
      // parsing the values
      Element element = lastEntry->second;

      if ( element.multiLine )
      {
	 string key = invertString( upshift( lastEntry->first ));
	 char insertString[IDENTLENG];

	 strncpy ( insertString, line, key.length() );
	 insertString[key.length()] = 0;

	 string compare ( insertString );
	 compare = upshift ( compare );
	 if ( key != compare )
	 {
	    seperatorfirst = seperator;
	 }
      }
   }


   // evaluate all columns
   begin = lineBuffer.find_first_not_of ( seperatorfirst );
   counter = 0;
   while ( begin != string::npos && counter <= 9 )
   {
      if ( counter == 0 )
      {
	 end = lineBuffer.find_first_of ( seperatorfirst, begin );
      }
      else
      {
	 end = lineBuffer.find_first_of ( seperator, begin );
      }

      // line-end ?
      if ( end == string::npos )
      {
	 end= lineBuffer.length();
      }
      if ( counter < 9 )
      {
	 value[counter].assign ( lineBuffer, begin, end-begin );
      }
      else
      {
	 // Rest of string into the last value
	 value[counter].assign (lineBuffer, begin, string::npos);
      }
      if ( counter == 0 )
      {
	 begin = lineBuffer.find_first_not_of ( seperatorfirst, end );
      }
      else
      {
	 begin = lineBuffer.find_first_not_of ( seperator, end );
      }
      counter++;

   }

   for ( counter = 0; counter<10 ; counter++ )
   {
      // extract \n
      if (value[counter][value[counter].length()-1] == '\n' )
      {
	 value[counter].assign ( value[counter],
				 0,
				 value[counter].length()-1 );
      }
   }

   if ( lastEntry != entries.end() )
   {
      // Check, if the last entry have a multi-line values
      Element element = lastEntry->second;

      if ( element.multiLine )
      {
	 newFlagFound = false;
      }
   }

   if ( newFlagFound )
   {
      // insert an new entry
      Values values;
      string key ( value[0] );
      Element element;

      for ( counter = 1; counter<10; counter++ )
      {
	 if ( value[counter].length() > 0 )
	    values.push_back ( value[counter] );
      }

      // search the flag in the list
      lastEntry = entries.find ( value[0] );
      if ( lastEntry != entries.end() )
      {
	 (lastEntry->second).values = values;
      }
      else
      {
	 element.values = values;
	 element.multiLine = false; // default;

	 entries.insert(pair<const string, const Element>
				 ( key, element ) );
	 lastEntry = entries.find ( key );
      }
   }
   else
   {
      // insert only new values into an existing entry ( multi-line-flag)
      if ( lastEntry != entries.end() )
      {
	 if ( upshift(value[0]) !=
	      upshift(invertString ( (string)lastEntry->first )) )
	 {
	    Values values = (lastEntry->second).values;
	    string line = "";
	    // all columns are concatinate, if it is a multi-line-entry
	    for ( counter = 0; counter<10; counter++ )
	    {
	       if ( value[counter].length() > 0 )
		  line = line + value[counter];
	       if ( counter < 9 &&
		    value[counter+1].length() > 0 )
		  line = line + " ";
	    }
	    values.push_back ( line );
	    (lastEntry->second).values = values;
	 }
	 else
	 {
	    // End of multi-line-flag reached
	    lastEntry = entries.end();
	 }
      }
   }

   return ( ok );
}


/*---------------------------- EOF ------------------------------*/
