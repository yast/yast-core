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

#include <y2pm/PMSelectablePtr.h>
#include <y2pm/InstSrcDescrPtr.h>
#include <y2pm/InstSrcManager.h>
#include <y2pm/PMYouServers.h>

class YCPCallbacks
{
  public:
    YCPCallbacks( YCPInterpreter *interpreter );

    YCPValue setCallback( string func, YCPList args );

    YCPTerm createCallback( const string &func );

    YCPValue evaluate( const YCPTerm &callback );
    bool evaluateBool( const YCPTerm &callback );

  private:
    map<string, string> mModules;
    map<string, string> mSymbols;
    
    YCPInterpreter *_interpreter;    
};

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

	PMError _last_error;

	int _solve_errors;

    private:
	vector<InstSrcManager::ISrcId> _sources;
	unsigned int _first_free_source_slot;

	PMSelectablePtr getPackageSelectable (const std::string& name);
	PMSelectablePtr getSelectionSelectable (const std::string& name);
	PMSelectablePtr getPatchSelectable (const std::string& name);

	InstSrcManager::ISrcId getSourceByArgs (YCPList args, int pos);
	void startCachedSources (bool enabled_only, bool force = false);
	void addSrcIds (InstSrcManager::ISrcIdList & nids, bool enable);

	bool SetSelectionString (std::string name, bool recursive = false);
	PMSelectablePtr WhoProvidesString (std::string tag);
	bool DoProvideString (std::string name);
	bool DoRemoveString (std::string name);

	YCPMap Descr2Map (constInstSrcDescrPtr descr);

	// if startCachedSources was called already
	bool _cache_started;

	// priority list, set by Pkg::SourceInstallOrder()
	// used by Pkg::PkgCommit()
	InstSrcManager::ISrcIdList _inst_order;

        YCPCallbacks *_youCallbacks;
        YCPCallbacks *_instTargetCallbacks;

        void initYouCallbacks();
        void initInstTargetCallbacks();

    public:
	// general
	YCPValue InstSysMode (YCPList args);
	YCPValue SetLocale (YCPList args);
	YCPValue GetLocale (YCPList args);
	YCPValue SetAdditionalLocales (YCPList args);
	YCPValue GetAdditionalLocales (YCPList args);
	YCPValue Error (YCPList args);
	YCPValue ErrorDetails (YCPList args);
	YCPValue ErrorId (YCPList args);

	// callbacks
	YCPValue CallbackStartProvide (YCPList args);
	YCPValue CallbackProgressProvide (YCPList args);
	YCPValue CallbackDoneProvide (YCPList args);
	YCPValue CallbackStartPackage (YCPList args);
	YCPValue CallbackProgressPackage (YCPList args);
	YCPValue CallbackDonePackage (YCPList args);
	YCPValue CallbackMediaChange (YCPList args);
	YCPValue CallbackProgressRebuildDB (YCPList args);
	YCPValue CallbackSourceChange (YCPList args);
	YCPValue CallbackYouProgress (YCPList args);
	YCPValue CallbackYouPatchProgress (YCPList args);
	YCPValue CallbackYouExecuteYcpScript (YCPList args);
        YCPValue CallbackYouScriptProgress (YCPList args);
	void SetMediaCallback (InstSrcManager::ISrcId source_id);

	// source related
        YCPValue SourceStartManager (YCPList args);
	YCPValue SourceCreate (YCPList args);
	YCPValue SourceStartCache (YCPList args);
	YCPValue SourceGetCurrent (YCPList args);
	YCPValue SourceFinish (YCPList args);
	YCPValue SourceFinishAll (YCPList args);
	YCPValue SourceGeneralData (YCPList args);
	YCPValue SourceMediaData (YCPList args);
	YCPValue SourceProductData (YCPList args);
	YCPValue SourceProvideFile (YCPList args);
	YCPValue SourceProvideDir (YCPList args);
	YCPValue SourceCacheCopyTo (YCPList args);
	YCPValue SourceSetRamCache (YCPList args);
	YCPValue SourceProduct (YCPList args);
        YCPValue SourceSetEnabled (YCPList args);
        YCPValue SourceDelete (YCPList args);
        YCPValue SourceRaisePriority (YCPList args);
        YCPValue SourceLowerPriority (YCPList args);
        YCPValue SourceSaveRanks (YCPList args);
        YCPValue SourceChangeUrl (YCPList args);
	YCPValue SourceInstallOrder (YCPList args);

	// target related
	YCPValue TargetInit (YCPList args);
	YCPValue TargetFinish (YCPList args);
	YCPValue TargetLogfile (YCPList args);
	YCPValue TargetCapacity (YCPList args);
	YCPValue TargetUsed (YCPList args);
	YCPValue TargetBlockSize (YCPList args);
	YCPValue TargetUpdateInf (YCPList args);
	YCPValue TargetInstall (YCPList args);
	YCPValue TargetRemove (YCPList args);
	YCPValue TargetProducts (YCPList args);
	YCPValue TargetRebuildDB (YCPList args);
	YCPValue TargetInitDU (YCPList args);
	YCPValue TargetGetDU (YCPList args);
	YCPValue TargetFileHasOwner (YCPList args);

	// selection related
	YCPValue GetSelections (YCPList args);
	YCPValue GetBackupPath (YCPList args);
	YCPValue SetBackupPath (YCPList args);
	YCPValue CreateBackups (YCPList args);
	YCPValue SelectionData (YCPList args);
	YCPValue SelectionContent (YCPList args);
	YCPValue SetSelection (YCPList args);
	YCPValue ClearSelection (YCPList args);
	YCPValue ActivateSelections (YCPList args);
	YCPValue SelectionsUpdateAll (YCPList args);

	// package related
	YCPValue GetPackages (YCPList args);
	YCPValue FilterPackages (YCPList args);
	YCPValue IsProvided (YCPList args);
	YCPValue IsSelected (YCPList args);
	YCPValue IsAvailable (YCPList args);
	YCPValue DoProvide (YCPList args);
	YCPValue DoRemove (YCPList args);
	YCPValue PkgSummary (YCPList args);
	YCPValue PkgVersion (YCPList args);
	YCPValue PkgSize (YCPList args);
	YCPValue PkgGroup (YCPList args);
	YCPValue IsManualSelection (YCPList args);
	YCPValue ClearSaveState (YCPList args);
	YCPValue SaveState (YCPList args);
	YCPValue RestoreState (YCPList args);
	YCPValue PkgUpdateAll (YCPList args);
	YCPValue PkgAnyToDelete (YCPList args);
	YCPValue PkgAnyToInstall (YCPList args);
	YCPValue PkgFileHasOwner (YCPList args);

	YCPValue PkgInstall (YCPList args);
	YCPValue PkgSrcInstall (YCPList args);
	YCPValue PkgDelete (YCPList args);
	YCPValue PkgNeutral (YCPList args);
	YCPValue PkgSolve (YCPList args);
	YCPValue PkgSolveErrors (YCPList args);
	YCPValue PkgCommit (YCPList args);

	YCPValue PkgPrepareOrder (YCPList args);
	YCPValue PkgMediaSizes (YCPList args);
	YCPValue PkgMediaNames (YCPList args);

	// you patch related
        YCPValue YouStatus (YCPList args);
	YCPValue YouGetServers (YCPList args);
        YCPValue YouSetServer (YCPList args);
        YCPValue YouGetUserPassword (YCPList args);
        YCPValue YouSetUserPassword (YCPList args);
	YCPValue YouGetPatches (YCPList args);
	YCPValue YouGetDirectory (YCPList args);
	YCPValue YouAttachSource (YCPList args);
	YCPValue YouGetPackages (YCPList args);
	YCPValue YouSelectPatches (YCPList args);
	YCPValue YouFirstPatch (YCPList args);
	YCPValue YouNextPatch (YCPList args);
	YCPValue YouGetCurrentPatch (YCPList args);
	YCPValue YouInstallCurrentPatch (YCPList args);
	YCPValue YouInstallPatches (YCPList args);
        YCPValue YouRemovePackages (YCPList args);
        YCPValue YouDisconnect (YCPList args);
        YCPValue YouFinish (YCPList args);

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
        PMYouServer convertServerObject( const YCPMap &serverMap );
};
#endif // PkgModuleFunctions_h
