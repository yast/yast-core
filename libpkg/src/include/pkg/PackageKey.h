/*************************************************************
 *
 *     YaST2      SuSE Labs                        -o)
 *     --------------------                        /\\
 *                                                _\_v
 *           www.suse.de / www.suse.com
 * ----------------------------------------------------------
 *
 * Author: 	  Stefan Schubert  <schubi@suse.de>
 *		  Michael Hager    <mike@suse.de>
 *
 * Description:   little helper class;  set/get the status of a package
  *
 * $Header$
 *
 *************************************************************/

/*
 * $Log$
 * Revision 1.1  2002/06/21 16:04:39  arvin
 * Initial revision
 *
 * Revision 1.3  2001/11/08 09:24:29  schubi
 * taking new versions from pkginfo server
 *
 * Revision 1.8  2001/07/04 16:50:47  arvin
 * - adapt for new automake/autoconf
 * - partly fix for gcc 3.0
 *
 * Revision 1.7  2001/04/11 13:54:51  mike
 * fixed deletePackage
 *
 * Revision 1.6  2001/04/04 16:39:23  schubi
 * compile errors removed
 *
 * Revision 1.5  2001/04/04 15:41:39  mike
 * first compilable
 *
 * Revision 1.4  2001/04/04 15:26:13  schubi
 * only header changes
 *
 *
 */
#ifndef PackageKey_h
#define PackageKey_h

#include <string>

using std::string;

class PackageKey
{

  public:

  PackageKey();
  PackageKey( const string name,
	      const string version );

  ~PackageKey();

  bool operator <  (const PackageKey) const;
  bool operator == (const PackageKey) const;

  string name()    	const { return _name; };
  string version()    	const { return _version; };

  private:

  string _name;
  string _version;

};

#endif

/*---------------------------- EOF ------------------------------*/
