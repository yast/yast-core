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

   File:	PkgModule.cc

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Pkg::function calls from rest of YaST.
/-*/


#include <ycp/y2log.h>
#include <PkgModule.h>

#include <y2util/Url.h>
#include <y2pm/InstData.h>

#include <ycp/YCPVoid.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>

using std::string;

PkgModule* PkgModule::current_pkg = NULL;

PkgModule* PkgModule::instance ()
{
    if (current_pkg == NULL)
    {
	current_pkg = new PkgModule ();
    }
    
    return current_pkg;
}

//-------------------------------------------------------------------
// PkgModule

PkgModule::PkgModule ()
    : PkgModuleFunctions ()
{
}

/**
 * Destructor.
 */
PkgModule::~PkgModule ()
{
}

