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

   File:	PkgModuleFunctions.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	PkgModuleFunctions constructor, destructor
/-*/


#include <ycp/y2log.h>
#include <PkgModuleFunctions.h>

/**
 * Constructor.
 */
PkgModuleFunctions::PkgModuleFunctions ()
{
    InstSrcManager& MGR = _y2pm.instSrcManager();
    Url url( "dir:///var/adm/mount" );

    InstSrcManager::ISrcIdList nids;
    PMError err = MGR.scanMedia( nids, url );
    if ( nids.size() ) {
	err = MGR.enableSource( *nids.begin() );
	y2milestone ("enable: %s", err.errstr().c_str());
    }
}

/**
 * Destructor.
 */
PkgModuleFunctions::~PkgModuleFunctions ()
{
    _y2pm.packageManager().REINIT();
}
