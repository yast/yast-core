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
#include <ycp/YCPInterpreter.h>

/**
 * A simple class for package management access
 */
class PkgModuleFunctions
{
  protected:
	/**
	 * access to packagemanager
	 */
	Y2PM _y2pm;

	/**
	 * access to WFM for callbacks
	 */
	YCPInterpreter *_wfm;

    private:
	vector<InstSrcManager::ISrcId> _sources;
	unsigned int _first_free_source_slot;

    public:
	// general
	YCPValue InstSysMode (YCPList args);
	YCPValue CheckSpace (YCPList args);
	YCPValue SetLocale (YCPList args);
	YCPValue GetLocale (YCPList args);
	YCPValue SetAdditionalLocales (YCPList args);
	YCPValue GetAdditionalLocales (YCPList args);

	// source related
	YCPValue SourceInit (YCPList args);
	YCPValue SourceFinish (YCPList args);
	YCPValue SourceGeneralData (YCPList args);
	YCPValue SourceMediaData (YCPList args);
	YCPValue SourceProductData (YCPList args);
	YCPValue SourceProvide (YCPList args);
	YCPValue SourceCacheCopyTo (YCPList args);

	// target related
	YCPValue TargetInit (YCPList args);
	YCPValue TargetFinish (YCPList args);
	YCPValue TargetInstall (YCPList args);
	YCPValue TargetRemove (YCPList args);
	YCPValue TargetLogfile (YCPList args);
	YCPValue SetProgressCallback (YCPList args);
	YCPValue TargetCapacity (YCPList args);
	YCPValue TargetUsed (YCPList args);

	// selection related
	YCPValue GetSelections (YCPList args);
	YCPValue SelectionData (YCPList args);
	YCPValue SetSelection (YCPList args);
	bool SetSelectionString (std::string name);	// internal
	YCPValue ClearSelection (YCPList args);
	YCPValue ActivateSelections (YCPList args);

	// package related
	YCPValue GetPackages (YCPList args);
	YCPValue IsProvided (YCPList args);
	YCPValue IsSelected (YCPList args);
	YCPValue IsAvailable (YCPList args);
	YCPValue DoProvide (YCPList args);
	bool DoProvideString (std::string name);	// internal
	YCPValue DoRemove (YCPList args);
	bool DoRemoveString (std::string name);	// internal
	YCPValue PkgSummary (YCPList args);
	YCPValue PkgVersion (YCPList args);
	YCPValue PkgSize (YCPList args);
	YCPValue PkgLocation (YCPList args);
	YCPValue PkgMediaNr (YCPList args);
	YCPValue IsManualSelection (YCPList args);
	YCPValue SaveState (YCPList args);
	YCPValue RestoreState (YCPList args);

	YCPValue PkgPrepareOrder (YCPList args);
	YCPValue PkgMediaSizes (YCPList args);
	YCPValue PkgNextDelete (YCPList args);
	YCPValue PkgNextInstall (YCPList args);

	// you patch related
        YCPValue YouStatus (YCPList args);
	YCPValue YouGetServers (YCPList args);
	YCPValue YouGetPatches (YCPList args);
	YCPValue YouAttachSource (YCPList args);
	YCPValue YouGetPackages (YCPList args);
	YCPValue YouSelectPatches (YCPList args);
	YCPValue YouFirstPatch (YCPList args);
	YCPValue YouNextPatch (YCPList args);
	YCPValue YouGetCurrentPatch (YCPList args);
	YCPValue YouInstallCurrentPatch (YCPList args);
	YCPValue YouInstallPatches (YCPList args);
        YCPValue YouRemovePackages (YCPList args);

	/**
	 * Constructor.
	 */
	PkgModuleFunctions (YCPInterpreter *wfmInterpreter);

	/**
	 * Destructor.
	 */
	~PkgModuleFunctions ();

    protected:
        YCPMap YouPatch( const PMYouPatchPtr &patch );

};
#endif // PkgModuleFunctions_h
