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
 * Revision 1.6  2001/04/23 13:50:35  mike
 * bugfix in Solver: deletePackage
 *
 * Revision 1.5  2001/04/11 15:08:10  gs
 * ; added
 *
 * Revision 1.4  2001/04/11 13:54:51  mike
 * fixed deletePackage
 *
 * Revision 1.3  2001/04/04 16:39:23  schubi
 * compile errors removed
 *
 *
 */

#include <pkg/PackageKey.h>

PackageKey::PackageKey( const string name,
			const string version )
   : _name   ( name    ),
     _version( version )
{
}

PackageKey::PackageKey( )
   : _name   ( "" ),
     _version( "" )
{
}

PackageKey::~PackageKey()
{
}

bool PackageKey::operator < (const PackageKey b) const
{
   if ( _name == b.name() )
   {
      return _version < b.version();
   }
   else
   {
      return _name < b.name();
   }
}
 
bool PackageKey::operator == (const PackageKey b) const
{
   if ( _name == b.name() && _version == b.version() ) 
   {
      return( true );
   }
   else
   {
      return ( false );
   }
}
 
