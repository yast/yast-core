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
#include <ycpTools.h>
#include <PkgModuleError.h>

#include <Y2PM.h>

#include <ycp/YCPBoolean.h>
#include <ycp/YCPValue.h>
#include <ycp/YCPList.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPSymbol.h>
#include <ycp/YCPString.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPVoid.h>
#include <ycp/YBlock.h>

#include <y2/Y2Namespace.h>
#include <y2/Y2NamespaceCPP.h>

#include <y2pm/PMSelectablePtr.h>
#include <y2pm/InstSrcDescrPtr.h>
#include <y2pm/InstSrcManager.h>
#include <y2pm/PMYouServers.h>

#define Y2REFFUNCTIONCALL1(namespace, name, signature, param1type, impl_class, impl_func)  \
class namespace##name##Function1 : public Y2CPPFunctionCall <impl_class> {      \
public:                                                 \
    namespace##name##Function1(impl_class* instance) :  \
        Y2CPPFunctionCall <impl_class> (signature, instance)    \
    {}                                                  \
    virtual void registerParameters (YBlockPtr decl)    \
    {                                                   \
        TypePtr t = Type::Const##param1type->clone ();  \
        t->asReference();                               \
        newParameter (decl, 1, t );                     \
    }                                                   \
    virtual YCPValue evaluate (bool cse=false)          \
    {                                                   \
        if (cse) return YCPNull ();                     \
        return m_instance->impl_func (m_param1->value ()->asReference ());   \
    }                                                   \
}


/**
 * A simple class for package management access
 */
class PkgModuleFunctions : public Y2Namespace
{
  public:

    /**
     * default error class
     **/
    typedef PkgModuleError Error;

    /**
     * Handler for YCPCallbacks received or triggered.
     * Needs access to WFM.
     **/
    class CallbackHandler;

  protected:
       /**
        * Access to packagemanager
        **/
       Y2PM _y2pm;

        /**
         * Remembered last error.
         **/
        PMError _last_error;

        /**
         * Return the provided YCPValue and on the fly remember _last_error.
         **/
        YCPValue pkgError( PMError err_r, const YCPValue & ret_r = YCPVoid() );

        int _solve_errors;

    protected:

        PMSelectablePtr getPackageSelectable (const std::string& name);
        PMSelectablePtr getSelectionSelectable (const std::string& name);
        PMSelectablePtr getPatchSelectable (const std::string& name);

        bool SetSelectionString (std::string name, bool recursive = false);
        PMSelectablePtr WhoProvidesString (std::string tag);
        bool DoProvideString (std::string name);
        bool DoRemoveString (std::string name);

        YCPMap Descr2Map (constInstSrcDescrPtr descr);

    private: // source related

      bool sourceStartManager( bool autoEnable );
      void registerFunctions ();

    private:

      /**
       * Handler for YCPCallbacks received or triggered.
       * Needs access to WFM. See @ref CallbackHandler.
       **/
      CallbackHandler & _callbackHandler;


    public:
	// general
	YCPValue InstSysMode ();
	YCPValue SetLocale (const YCPString& locale);
	YCPValue GetLocale ();
	YCPValue SetAdditionalLocales (YCPList args);
	YCPValue GetAdditionalLocales ();
	YCPValue LastError ();
	YCPValue LastErrorDetails ();
	YCPValue LastErrorId ();

	// callbacks
	YCPValue CallbackStartProvide (const YCPString& func);
	YCPValue CallbackProgressProvide (const YCPString& func);
	YCPValue CallbackDoneProvide (const YCPString& func);
	YCPValue CallbackStartPackage (const YCPString& func);
	YCPValue CallbackProgressPackage (const YCPString& func);
	YCPValue CallbackDonePackage (const YCPString& func);
	YCPValue CallbackMediaChange (const YCPString& func);
	YCPValue CallbackSourceChange (const YCPString& func);
	YCPValue CallbackYouProgress (const YCPString& func);
	YCPValue CallbackYouPatchProgress (const YCPString& func);
	YCPValue CallbackYouError (const YCPString& func);
	YCPValue CallbackYouMessage (const YCPString& func);
	YCPValue CallbackYouLog (const YCPString& func);
        YCPValue CallbackYouExecuteYcpScript (const YCPString& func);
        YCPValue CallbackYouScriptProgress (const YCPString& func);
        YCPValue CallbackStartRebuildDb (const YCPString& func);
        YCPValue CallbackProgressRebuildDb (const YCPString& func);
        YCPValue CallbackNotifyRebuildDb (const YCPString& func);
        YCPValue CallbackStopRebuildDb (const YCPString& func);
        YCPValue CallbackStartConvertDb (const YCPString& func);
        YCPValue CallbackProgressConvertDb (const YCPString& func);
        YCPValue CallbackNotifyConvertDb (const YCPString& func);
        YCPValue CallbackStopConvertDb (const YCPString& func);
	void SetMediaCallback (InstSrcManager::ISrcId source_id);


	// source related
        YCPValue SourceStartManager (const YCPBoolean&);
	YCPValue SourceCreate (const YCPString&, const YCPString&);
	YCPValue SourceStartCache (const YCPBoolean&);
	YCPValue SourceGetCurrent (const YCPBoolean& enabled);
	YCPValue SourceFinish (const YCPInteger&);
	YCPValue SourceFinishAll ();
	YCPValue SourceGeneralData (const YCPInteger&);
	YCPValue SourceMediaData (const YCPInteger&);
	YCPValue SourceProductData (const YCPInteger&);
	YCPValue SourceProvideFile (const YCPInteger&, const YCPInteger&, const YCPString&);
	YCPValue SourceProvideDir (const YCPInteger&, const YCPInteger&, const YCPString&);
	YCPValue SourceCacheCopyTo (const YCPString&);
	YCPValue SourceSetRamCache (const YCPBoolean&);
	YCPValue SourceProduct (const YCPInteger&);
        YCPValue SourceSetEnabled (const YCPInteger&, const YCPBoolean&);
        YCPValue SourceDelete (const YCPInteger&);
        YCPValue SourceRaisePriority (const YCPInteger&);
        YCPValue SourceLowerPriority (const YCPInteger&);
        YCPValue SourceSaveRanks ();
        YCPValue SourceChangeUrl (const YCPInteger&, const YCPString&);
	YCPValue SourceInstallOrder (const YCPMap&);
        YCPValue SourceEditGet ();
        YCPValue SourceEditSet (const YCPList& args);
        YCPValue SourceScan (const YCPString& media, const YCPString& product_dir);

	// target related
	YCPValue TargetInit (const YCPString& root, const YCPBoolean& n);
	YCPBoolean TargetFinish ();
	YCPBoolean TargetLogfile (const YCPString&);
	YCPInteger TargetCapacity (const YCPString&);
	YCPInteger TargetUsed (const YCPString&);
	YCPInteger TargetBlockSize (const YCPString&);
	YCPValue TargetUpdateInf (const YCPString&);
	YCPBoolean TargetInstall (const YCPString&);
	YCPBoolean TargetRemove (const YCPString&);
	YCPList TargetProducts ();
	YCPBoolean TargetRebuildDB ();
	YCPValue TargetInitDU (const YCPList&);
	YCPValue TargetGetDU ();
	YCPBoolean TargetFileHasOwner (const YCPString&);

	// selection related
	YCPValue GetSelections (const YCPSymbol& stat, const YCPString& cat);
	YCPValue GetBackupPath ();
	YCPValue SetBackupPath (const YCPString& path);
	YCPValue CreateBackups (const YCPBoolean& flag);
	YCPValue SelectionData (const YCPString& cat);
	YCPValue SelectionContent (const YCPString&, const YCPBoolean&, const YCPString&);
	YCPBoolean SetSelection (const YCPString&);
	YCPValue ClearSelection (const YCPString&);
	YCPBoolean ActivateSelections ();

	// package related
	YCPValue GetPackages (const YCPSymbol& which, const YCPBoolean& names_only);
	YCPValue FilterPackages (const YCPBoolean& byAuto, const YCPBoolean& byApp, const YCPBoolean& byUser, const YCPBoolean& names_only);
	YCPValue IsProvided (const YCPString& tag);
	YCPValue IsSelected (const YCPString& tag);
	YCPValue IsAvailable (const YCPString& tag);
	YCPValue DoProvide (const YCPList& args);
	YCPValue DoRemove (const YCPList& args);
	YCPValue PkgSummary (const YCPString& package);
	YCPValue PkgVersion (const YCPString& package);
	YCPValue PkgSize (const YCPString& package);
	YCPValue PkgGroup (const YCPString& package);
	YCPValue PkgLocation (const YCPString& package);
	YCPValue PkgProperties (const YCPString& package);
	YCPValue IsManualSelection ();
	YCPValue ClearSaveState ();
	YCPValue SaveState ();
	YCPValue RestoreState (const YCPBoolean&);
	YCPValue PkgUpdateAll (const YCPBoolean& del);
	YCPValue PkgAnyToDelete ();
	YCPValue PkgAnyToInstall ();
	YCPValue PkgFileHasOwner (YCPList args);

	YCPValue PkgInstall (const YCPString& p);
	YCPValue PkgSrcInstall (const YCPString& p);
	YCPValue PkgDelete (const YCPString& p);
	YCPValue PkgNeutral (const YCPString& p);
	YCPValue PkgReset ();
	YCPValue PkgSolve (const YCPBoolean& filter);
	YCPValue PkgSolveErrors ();
	YCPValue PkgCommit (const YCPInteger& medianr);

	YCPValue PkgPrepareOrder (YCPList args);
	YCPValue PkgMediaSizes ();
	YCPValue PkgMediaNames ();

	// you patch related
        YCPMap YouStatus ();
	YCPString YouGetServers (YCPReference strings);
	YCPValue YouSetServer (const YCPMap& strings);
	YCPValue YouGetUserPassword ();
	YCPValue YouSetUserPassword (const YCPString& user, const YCPString& passwd, const YCPBoolean& persistent);
	YCPValue YouRetrievePatchInfo (const YCPBoolean& download, const YCPBoolean& sign);
	YCPValue YouProcessPatches ();
	YCPValue YouGetDirectory ();
	YCPValue YouSelectPatches ();
        YCPValue YouRemovePackages ();

	// function call definitions
	Y2FUNCTIONCALL  ( Pkg, InstSysMode, 		"void ()",		PkgModuleFunctions, InstSysMode);
	Y2FUNCTIONCALL1 ( Pkg, SetLocale, 		"void (string)",String, 	PkgModuleFunctions, SetLocale);
	Y2FUNCTIONCALL  ( Pkg, GetLocale, 		"string ()",		PkgModuleFunctions, GetLocale);
	Y2FUNCTIONCALL1 ( Pkg, SetAdditionalLocales, 	"void (list<string>)",List, 		PkgModuleFunctions, SetAdditionalLocales);
	Y2FUNCTIONCALL  ( Pkg, GetAdditionalLocales, 	"list<string> ()",		PkgModuleFunctions, GetAdditionalLocales);
	Y2FUNCTIONCALL  ( Pkg, LastError, 		"string ()",		PkgModuleFunctions, LastError);
	Y2FUNCTIONCALL  ( Pkg, LastErrorDetails, 	"string ()",		PkgModuleFunctions, LastErrorDetails);
	Y2FUNCTIONCALL  ( Pkg, LastErrorId, 		"string ()",		PkgModuleFunctions, LastErrorId);

	// callbacks
	Y2FUNCTIONCALL1 ( Pkg, CallbackStartProvide, 	"void (string)",String,		PkgModuleFunctions, CallbackStartProvide);
	Y2FUNCTIONCALL1 ( Pkg, CallbackProgressProvide, "void (string)",String,		PkgModuleFunctions, CallbackProgressProvide);
	Y2FUNCTIONCALL1 ( Pkg, CallbackDoneProvide, 	"void (string)",String,		PkgModuleFunctions, CallbackDoneProvide);
	Y2FUNCTIONCALL1 ( Pkg, CallbackStartPackage, 	"void (string)",String,		PkgModuleFunctions, CallbackStartPackage);
	Y2FUNCTIONCALL1 ( Pkg, CallbackProgressPackage, "void (string)",String,		PkgModuleFunctions, CallbackProgressPackage);
	Y2FUNCTIONCALL1 ( Pkg, CallbackDonePackage, 	"void (string)",String,		PkgModuleFunctions, CallbackDonePackage);
	Y2FUNCTIONCALL1 ( Pkg, CallbackMediaChange, 	"void (string)",String,		PkgModuleFunctions, CallbackMediaChange);
	Y2FUNCTIONCALL1 ( Pkg, CallbackSourceChange, 	"void (string)",String, 	PkgModuleFunctions, CallbackSourceChange);
	Y2FUNCTIONCALL1 ( Pkg, CallbackYouProgress, 	"void (string)",String,		PkgModuleFunctions, CallbackYouProgress);
	Y2FUNCTIONCALL1 ( Pkg, CallbackYouPatchProgress, "void (string)",String,	PkgModuleFunctions, CallbackYouPatchProgress);
	Y2FUNCTIONCALL1 ( Pkg, CallbackYouError, 	"void (string)",String,		PkgModuleFunctions, CallbackYouError);
	Y2FUNCTIONCALL1 ( Pkg, CallbackYouMessage, 	"void (string)",String,		PkgModuleFunctions, CallbackYouMessage);
	Y2FUNCTIONCALL1 ( Pkg, CallbackYouLog,          "void (string)",String,	        PkgModuleFunctions, CallbackYouLog);
        Y2FUNCTIONCALL1 ( Pkg, CallbackYouExecuteYcpScript, "void (string)",String,	PkgModuleFunctions, CallbackYouExecuteYcpScript);
        Y2FUNCTIONCALL1 ( Pkg, CallbackYouScriptProgress, "void (string)",String,	PkgModuleFunctions, CallbackYouScriptProgress);
        Y2FUNCTIONCALL1 ( Pkg, CallbackStartRebuildDb, 	"void (string)",String,		PkgModuleFunctions, CallbackStartRebuildDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackProgressRebuildDb, "void (string)",String,	PkgModuleFunctions, CallbackProgressRebuildDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackNotifyRebuildDb, "void (string)",String,		PkgModuleFunctions, CallbackNotifyRebuildDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackStopRebuildDb, 	"void (string)",String,		PkgModuleFunctions, CallbackStopRebuildDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackStartConvertDb, 	"void (string)",String,		PkgModuleFunctions, CallbackStartConvertDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackProgressConvertDb, "void (string)",String,	PkgModuleFunctions, CallbackProgressConvertDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackNotifyConvertDb, "void (string)",String,		PkgModuleFunctions, CallbackNotifyConvertDb);
        Y2FUNCTIONCALL1 ( Pkg, CallbackStopConvertDb, 	"void (string)",String,		PkgModuleFunctions, CallbackStopConvertDb);

	// source related
        Y2FUNCTIONCALL1 ( Pkg, SourceStartManager, 	"boolean (boolean)",Boolean,	PkgModuleFunctions, SourceStartManager);
	Y2FUNCTIONCALL2 ( Pkg, SourceCreate, 		"integer (string, string)",String, String,	PkgModuleFunctions, SourceCreate);
	Y2FUNCTIONCALL1 ( Pkg, SourceStartCache, 	"list<integer> (boolean)",Boolean,	PkgModuleFunctions, SourceStartCache);
	Y2FUNCTIONCALL1 ( Pkg, SourceGetCurrent, 	"list<integer> (boolean)",Boolean,	PkgModuleFunctions, SourceGetCurrent);
	Y2FUNCTIONCALL1 ( Pkg, SourceFinish, 		"boolean (integer)",Integer,	PkgModuleFunctions, SourceFinish);
	Y2FUNCTIONCALL  ( Pkg, SourceFinishAll, 	"boolean ()",		PkgModuleFunctions, SourceFinishAll);
	Y2FUNCTIONCALL1 ( Pkg, SourceGeneralData, 	"map<string,any> (integer)",Integer,	PkgModuleFunctions, SourceGeneralData);
	Y2FUNCTIONCALL1 ( Pkg, SourceMediaData, 	"map<string,any> (integer)",Integer,	PkgModuleFunctions, SourceMediaData);
	Y2FUNCTIONCALL1 ( Pkg, SourceProductData, 	"map<string,any> (integer)",Integer,	PkgModuleFunctions, SourceProductData);
	Y2FUNCTIONCALL3 ( Pkg, SourceProvideFile, 	"string (integer, integer, string)",Integer, Integer, String, PkgModuleFunctions, SourceProvideFile);
	Y2FUNCTIONCALL3 ( Pkg, SourceProvideDir, 	"string (integer, integer, string)",Integer, Integer, String, PkgModuleFunctions, SourceProvideDir);
	Y2FUNCTIONCALL1 ( Pkg, SourceCacheCopyTo, 	"boolean (string)",String,		PkgModuleFunctions, SourceCacheCopyTo);
	Y2FUNCTIONCALL1 ( Pkg, SourceSetRamCache, 	"boolean (boolean)",Boolean,	PkgModuleFunctions, SourceSetRamCache);
	Y2FUNCTIONCALL1 ( Pkg, SourceProduct, 		"map<string,string> (integer)",Integer,	PkgModuleFunctions, SourceProduct);
        Y2FUNCTIONCALL2 ( Pkg, SourceSetEnabled, 	"boolean (integer, boolean)",Integer, Boolean, PkgModuleFunctions, SourceSetEnabled);
        Y2FUNCTIONCALL1 ( Pkg, SourceDelete, 		"boolean (integer)",Integer,	PkgModuleFunctions, SourceDelete);
        Y2FUNCTIONCALL1 ( Pkg, SourceRaisePriority, 	"void (integer)",Integer,	PkgModuleFunctions, SourceRaisePriority);
        Y2FUNCTIONCALL1 ( Pkg, SourceLowerPriority, 	"void (integer)",Integer,	PkgModuleFunctions, SourceLowerPriority);
        Y2FUNCTIONCALL  ( Pkg, SourceSaveRanks, 	"boolean ()",		PkgModuleFunctions, SourceSaveRanks);
        Y2FUNCTIONCALL2 ( Pkg, SourceChangeUrl, 	"boolean (integer, string)",Integer, String,PkgModuleFunctions, SourceChangeUrl);
	Y2FUNCTIONCALL1 ( Pkg, SourceInstallOrder, 	"boolean (map<integer,integer>)",Map,		PkgModuleFunctions, SourceInstallOrder);
        Y2FUNCTIONCALL  ( Pkg, SourceEditGet, 		"list<map<string,any>> ()",		PkgModuleFunctions, SourceEditGet);
        Y2FUNCTIONCALL1 ( Pkg, SourceEditSet, 		"boolean (list<map<string,any>>)",List, 		PkgModuleFunctions, SourceEditSet);
        Y2FUNCTIONCALL2 ( Pkg, SourceScan, 		"list<integer> (string, string)",String, String, PkgModuleFunctions, SourceScan);

	// target related
	Y2FUNCTIONCALL2 ( Pkg, TargetInit, 		"boolean (string, boolean)",String, Boolean,PkgModuleFunctions, TargetInit);
	Y2FUNCTIONCALL  ( Pkg, TargetFinish, 		"boolean ()",		PkgModuleFunctions, TargetFinish);
	Y2FUNCTIONCALL1 ( Pkg, TargetLogfile, 		"boolean (string)",String,		PkgModuleFunctions, TargetLogfile);
	Y2FUNCTIONCALL1 ( Pkg, TargetCapacity, 		"integer (string)",String,		PkgModuleFunctions, TargetCapacity);
	Y2FUNCTIONCALL1 ( Pkg, TargetUsed, 		"integer (string)",String,		PkgModuleFunctions, TargetUsed);
	Y2FUNCTIONCALL1 ( Pkg, TargetBlockSize, 	"integer (string)",String,		PkgModuleFunctions, TargetBlockSize);
	Y2FUNCTIONCALL1 ( Pkg, TargetUpdateInf, 	"map<string,string> (string)",String,		PkgModuleFunctions, TargetUpdateInf);
	Y2FUNCTIONCALL1 ( Pkg, TargetInstall, 		"boolean (string)",String,		PkgModuleFunctions, TargetInstall);
	Y2FUNCTIONCALL1 ( Pkg, TargetRemove, 		"boolean (string)",String,		PkgModuleFunctions, TargetRemove);
	Y2FUNCTIONCALL  ( Pkg, TargetProducts, 		"list<any> ()",		PkgModuleFunctions, TargetProducts);
	Y2FUNCTIONCALL  ( Pkg, TargetRebuildDB, 	"boolean ()",		PkgModuleFunctions, TargetRebuildDB);
	Y2FUNCTIONCALL1 ( Pkg, TargetInitDU, 		"void (list<map<any,any> >)",List,		PkgModuleFunctions, TargetInitDU);
	Y2FUNCTIONCALL  ( Pkg, TargetGetDU, 		"map<string,list<integer>> ()",		PkgModuleFunctions, TargetGetDU);
	Y2FUNCTIONCALL1 ( Pkg, TargetFileHasOwner, 	"boolean (string)",String, 	PkgModuleFunctions, TargetFileHasOwner);

	// selection related
	Y2FUNCTIONCALL2 ( Pkg, GetSelections, 		"list<string> (symbol, string)",Symbol, String, PkgModuleFunctions, GetSelections);
	Y2FUNCTIONCALL  ( Pkg, GetBackupPath, 		"string ()",		PkgModuleFunctions, GetBackupPath);
	Y2FUNCTIONCALL1 ( Pkg, SetBackupPath, 		"void (string)",String,		PkgModuleFunctions, SetBackupPath);
	Y2FUNCTIONCALL1 ( Pkg, CreateBackups, 		"void (boolean)",Boolean,	PkgModuleFunctions, CreateBackups);
	Y2FUNCTIONCALL1 ( Pkg, SelectionData, 		"map<string,any> (string)",String,		PkgModuleFunctions, SelectionData);
	Y2FUNCTIONCALL3 ( Pkg, SelectionContent, 	"list<string> (string, boolean, string)",String, Boolean, String, PkgModuleFunctions, SelectionContent);
	Y2FUNCTIONCALL1 ( Pkg, SetSelection, 		"boolean (string)",String,		PkgModuleFunctions, SetSelection);
	Y2FUNCTIONCALL1 ( Pkg, ClearSelection, 		"boolean (string)",String, 	PkgModuleFunctions, ClearSelection);
	Y2FUNCTIONCALL  ( Pkg, ActivateSelections, 	"boolean ()",		PkgModuleFunctions, ActivateSelections);

	// package related
	Y2FUNCTIONCALL2 ( Pkg, GetPackages, 		"list<string> (symbol, boolean)",Symbol, Boolean, PkgModuleFunctions, GetPackages);
	Y2FUNCTIONCALL4 ( Pkg, FilterPackages, 		"list<string> (boolean, boolean, boolean, boolean)",Boolean, Boolean, Boolean, Boolean, PkgModuleFunctions, FilterPackages);
	Y2FUNCTIONCALL1 ( Pkg, IsProvided, 		"boolean (string)",String,		PkgModuleFunctions, IsProvided);
	Y2FUNCTIONCALL1 ( Pkg, IsSelected, 		"boolean (string)",String,		PkgModuleFunctions, IsSelected);
	Y2FUNCTIONCALL1 ( Pkg, IsAvailable, 		"boolean (string)",String,		PkgModuleFunctions, IsAvailable);
	Y2FUNCTIONCALL1 ( Pkg, DoProvide, 		"map<string,any> (list<string>)",List,		PkgModuleFunctions, DoProvide);
	Y2FUNCTIONCALL1 ( Pkg, DoRemove, 		"map<string,any> (list<string>)",List, 		PkgModuleFunctions, DoRemove);
	Y2FUNCTIONCALL1 ( Pkg, PkgSummary, 		"string (string)",String, 	PkgModuleFunctions, PkgSummary);
	Y2FUNCTIONCALL1 ( Pkg, PkgVersion, 		"string (string)",String,		PkgModuleFunctions, PkgVersion);
	Y2FUNCTIONCALL1 ( Pkg, PkgSize, 		"integer (string)",String,		PkgModuleFunctions, PkgSize);
	Y2FUNCTIONCALL1 ( Pkg, PkgGroup, 		"string (string)",String,		PkgModuleFunctions, PkgGroup);
	Y2FUNCTIONCALL1 ( Pkg, PkgLocation, 		"string (string)",String,		PkgModuleFunctions, PkgLocation);
	Y2FUNCTIONCALL1 ( Pkg, PkgProperties,		"map<string,any> (string)",String,		PkgModuleFunctions, PkgProperties);
	Y2FUNCTIONCALL  ( Pkg, IsManualSelection, 	"boolean ()",		PkgModuleFunctions, IsManualSelection);
	Y2FUNCTIONCALL  ( Pkg, ClearSaveState, 		"boolean ()",		PkgModuleFunctions, ClearSaveState);
	Y2FUNCTIONCALL  ( Pkg, SaveState, 		"boolean ()",		PkgModuleFunctions, SaveState);
	Y2FUNCTIONCALL1 ( Pkg, RestoreState, 		"boolean (boolean)",Boolean, 	PkgModuleFunctions, RestoreState);
	Y2FUNCTIONCALL1 ( Pkg, PkgUpdateAll, 		"list<integer> (boolean)",Boolean,	PkgModuleFunctions, PkgUpdateAll);
	Y2FUNCTIONCALL  ( Pkg, PkgAnyToDelete, 		"boolean ()",		PkgModuleFunctions, PkgAnyToDelete);
	Y2FUNCTIONCALL  ( Pkg, AnyToInstall, 		"boolean ()",		PkgModuleFunctions, PkgAnyToInstall);

	Y2FUNCTIONCALL1 ( Pkg, PkgInstall, 		"boolean (string)",String,		PkgModuleFunctions, PkgInstall);
	Y2FUNCTIONCALL1 ( Pkg, PkgSrcInstall, 		"boolean (string)",String,		PkgModuleFunctions, PkgSrcInstall);
	Y2FUNCTIONCALL1 ( Pkg, PkgDelete, 		"boolean (string)",String,		PkgModuleFunctions, PkgDelete);
	Y2FUNCTIONCALL1 ( Pkg, PkgNeutral, 		"boolean (string)",String,		PkgModuleFunctions, PkgNeutral);
	Y2FUNCTIONCALL  ( Pkg, PkgReset, 		"boolean ()",		PkgModuleFunctions, PkgReset);
	Y2FUNCTIONCALL1 ( Pkg, PkgSolve, 		"boolean (boolean)",Boolean,	PkgModuleFunctions, PkgSolve);
	Y2FUNCTIONCALL  ( Pkg, PkgSolveErrors, 		"integer ()",		PkgModuleFunctions, PkgSolveErrors);
	Y2FUNCTIONCALL1 ( Pkg, PkgCommit, 		"list<any> (integer)",Integer,	PkgModuleFunctions, PkgCommit);

	Y2FUNCTIONCALL  ( Pkg, PkgMediaSizes, 		"list<list<integer>> ()",		PkgModuleFunctions, PkgMediaSizes);
	Y2FUNCTIONCALL  ( Pkg, PkgMediaNames, 		"list<string> ()",		PkgModuleFunctions, PkgMediaNames);

	// you patch related
        Y2FUNCTIONCALL  ( Pkg, YouStatus, 		"map<any,any> ()",		PkgModuleFunctions, YouStatus);
	Y2REFFUNCTIONCALL1 ( Pkg, YouGetServers, 	"string (list<any>&)",	List,	PkgModuleFunctions, YouGetServers);
	Y2FUNCTIONCALL1	( Pkg, YouSetServer,		"string (map<any,any>)",Map, PkgModuleFunctions, YouSetServer);
	Y2FUNCTIONCALL 	( Pkg, YouGetUserPassword,	"map<any,any> ()",	PkgModuleFunctions, YouGetUserPassword);
	Y2FUNCTIONCALL3	( Pkg, YouSetUserPassword,	"string (string, string, boolean)", String, String, Boolean, PkgModuleFunctions, YouSetUserPassword);
	Y2FUNCTIONCALL2 ( Pkg, YouRetrievePatchInfo, 	"string (boolean, boolean)",Boolean, Boolean, PkgModuleFunctions, YouRetrievePatchInfo);
	Y2FUNCTIONCALL  ( Pkg, YouProcessPatches, 	"boolean ()",		PkgModuleFunctions, YouProcessPatches);
	Y2FUNCTIONCALL  ( Pkg, YouGetDirectory, 	"string ()",		PkgModuleFunctions, YouGetDirectory);
	Y2FUNCTIONCALL  ( Pkg, YouSelectPatches, 	"void ()",		PkgModuleFunctions, YouSelectPatches);
        Y2FUNCTIONCALL  ( Pkg, YouRemovePackages, 	"boolean ()",		PkgModuleFunctions, YouRemovePackages);


	/**
	 * Constructor.
	 */
	PkgModuleFunctions ();

	/**
	 * Destructor.
	 */
	virtual ~PkgModuleFunctions ();

	virtual const string name () const
	{
    	    return "Pkg";
	}

	virtual const string filename () const
	{
    	    return "Pkg";
	}

	virtual string toString () const
	{
    	    return "// not possible toString";
	}

	virtual YCPValue evaluate (bool cse = false )
	{
    	    if (cse) return YCPNull ();
    	    else return YCPVoid ();
	}

	virtual Y2Function* createFunctionCall (const string name);

        static YCPMap YouPatch( const PMYouPatchPtr &patch );

    protected:
	PMYouServer convertServerObject( const YCPMap &serverMap );

};
#endif // PkgModuleFunctions_h
