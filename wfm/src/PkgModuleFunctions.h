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

   File:	PkgModuleFunctions.h

   Author:	Klaus Kaempf <kkaempf@suse.de>
   Maintainer:  Klaus Kaempf <kkaempf@suse.de>

   Purpose:	Access to packagemanager
		Handles Pkg::function (list_of_arguments) calls
		from WFMInterpreter.
/-*/

#ifndef PkgModuleFunctions_h
#define PkgModuleFunctions_h

#include <string>

#include <Y2PM.h>

#include <ycp/YCPValue.h>
#include <ycp/YCPList.h>

/**
 * A simple class for package management access
 */
class PkgModuleFunctions
{
    private:
	Y2PM _y2pm;

    public:
	YCPValue GetGroups (YCPList args);
	YCPValue GetSelections (YCPList args);
	YCPValue IsProvided (YCPList args);
	YCPValue IsAvailable (YCPList args);
	YCPValue DoProvide (YCPList args);
	YCPValue DoRemove (YCPList args);
	YCPValue PkgSummary (YCPList args);
	YCPValue SelSummary (YCPList args);
	YCPValue SetSelection (YCPList args);
	YCPValue IsManualSelection (YCPList args);
	YCPValue SaveState (YCPList args);
	YCPValue RestoreState (YCPList args);

	YCPValue YouGetServers (YCPList args);
	YCPValue YouGetPatches (YCPList args);
	YCPValue YouGetPackages (YCPList args);
	YCPValue YouInstallPatches (YCPList args);

	YCPValue TargetInit (YCPList args);
	YCPValue TargetFinish (YCPList args);

	/**
	 * Constructor.
	 */
	PkgModuleFunctions ();

	/**
	 * Destructor.
	 */
	~PkgModuleFunctions ();


};
#endif // PkgModuleFunctions_h
