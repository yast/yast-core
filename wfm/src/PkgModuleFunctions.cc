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
#include <ycp/YExpression.h>
#include <ycp/YBlock.h>
#include "PkgModuleFunctions.h"
#include "PkgModuleCallbacks.h"

#include <y2pm/InstSrcDescr.h>
#include <y2pm/InstSrcManager.h>
#include <y2pm/InstSrcDescr.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPString.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPMap.h>
#include <ycp/YCPVoid.h>

/**
 * Constructor.
 */
PkgModuleFunctions::PkgModuleFunctions ()
    : _callbackHandler( *new CallbackHandler( ) )
{
  // it's cheap to launch them
  _y2pm.packageManager();
  _y2pm.selectionManager();
  
  registerFunctions ();
}

/**
 * Destructor.
 */
PkgModuleFunctions::~PkgModuleFunctions ()
{
    SourceFinishAll ();
    TargetFinish ();
    delete &_callbackHandler;
}

Y2Function* PkgModuleFunctions::createFunctionCall (const string name)
{
    TableEntry *func_te = table ()->find (name.c_str (), SymbolEntry::c_function);
    if (func_te)
    {
        return new YEFunction (func_te->sentry ());
    }
    y2error ("No such function %s", name.c_str ());
    return NULL;
}

void PkgModuleFunctions::registerFunctions()
{
    REGISTERFUNCTIONCALL  ( 0,	Pkg, ActivateSelections);
    REGISTERFUNCTIONCALL  ( 1,	Pkg, AnyToInstall);
    REGISTERFUNCTIONCALL  ( 2,	Pkg, ClearSaveState);
    REGISTERFUNCTIONCALL1 ( 3,	Pkg, ClearSelection);
    REGISTERFUNCTIONCALL1 ( 4,	Pkg, CreateBackups);
    REGISTERFUNCTIONCALL1 ( 5,	Pkg, DoProvide);
    REGISTERFUNCTIONCALL1 ( 6,	Pkg, DoRemove);
    REGISTERFUNCTIONCALL4 ( 7,	Pkg, FilterPackages);
    REGISTERFUNCTIONCALL  ( 8,	Pkg, GetAdditionalLocales);
    REGISTERFUNCTIONCALL  ( 9,	Pkg, GetBackupPath);
    REGISTERFUNCTIONCALL  ( 10, Pkg, GetLocale);
    REGISTERFUNCTIONCALL2 ( 11,	Pkg, GetPackages);
    REGISTERFUNCTIONCALL2 ( 12,	Pkg, GetSelections);
    REGISTERFUNCTIONCALL  ( 13,	Pkg, InstSysMode);
    REGISTERFUNCTIONCALL1 ( 14,	Pkg, IsAvailable);
    REGISTERFUNCTIONCALL  ( 15,	Pkg, IsManualSelection);
    REGISTERFUNCTIONCALL1 ( 16,	Pkg, IsProvided);
    REGISTERFUNCTIONCALL1 ( 17,	Pkg, IsSelected);
    REGISTERFUNCTIONCALL  ( 18,	Pkg, PkgAnyToDelete);
    REGISTERFUNCTIONCALL1 ( 19,	Pkg, PkgCommit);
    REGISTERFUNCTIONCALL1 ( 20,	Pkg, PkgDelete);
    REGISTERFUNCTIONCALL1 ( 21,	Pkg, PkgGroup);
    REGISTERFUNCTIONCALL1 ( 22,	Pkg, PkgInstall);
    REGISTERFUNCTIONCALL  ( 23,	Pkg, PkgMediaNames);
    REGISTERFUNCTIONCALL  ( 24,	Pkg, PkgMediaSizes);
    REGISTERFUNCTIONCALL1 ( 25,	Pkg, PkgNeutral);
    REGISTERFUNCTIONCALL1 ( 26,	Pkg, PkgSize);
    REGISTERFUNCTIONCALL1 ( 27,	Pkg, PkgSolve);
    REGISTERFUNCTIONCALL  ( 28,	Pkg, PkgSolveErrors);
    REGISTERFUNCTIONCALL1 ( 29,	Pkg, PkgSrcInstall);
    REGISTERFUNCTIONCALL1 ( 30,	Pkg, PkgSummary);
    REGISTERFUNCTIONCALL1 ( 31,	Pkg, PkgVersion);
    REGISTERFUNCTIONCALL  ( 32,	Pkg, PkgReset);
    REGISTERFUNCTIONCALL1 ( 33,	Pkg, RestoreState);
    REGISTERFUNCTIONCALL  ( 34,	Pkg, SaveState);
    REGISTERFUNCTIONCALL3 ( 35,	Pkg, SelectionContent);
    REGISTERFUNCTIONCALL1 ( 36,	Pkg, SelectionData);
    REGISTERFUNCTIONCALL1 ( 37,	Pkg, SetAdditionalLocales);
    REGISTERFUNCTIONCALL1 ( 38,	Pkg, SetBackupPath);
    REGISTERFUNCTIONCALL1 ( 39,	Pkg, SetLocale);
    REGISTERFUNCTIONCALL1 ( 40,	Pkg, SetSelection);
    REGISTERFUNCTIONCALL1 ( 41,	Pkg, SourceCacheCopyTo);
    REGISTERFUNCTIONCALL2 ( 42,	Pkg, SourceChangeUrl);
    REGISTERFUNCTIONCALL2 ( 43,	Pkg, SourceCreate);
    REGISTERFUNCTIONCALL1 ( 44,	Pkg, SourceDelete);
    REGISTERFUNCTIONCALL1 ( 45,	Pkg, SourceFinish);
    REGISTERFUNCTIONCALL  ( 46,	Pkg, SourceFinishAll);
    REGISTERFUNCTIONCALL1 ( 47,	Pkg, SourceGeneralData);
    REGISTERFUNCTIONCALL1 ( 48,	Pkg, SourceGetCurrent);
    REGISTERFUNCTIONCALL1 ( 49,	Pkg, SourceInstallOrder);
    REGISTERFUNCTIONCALL1 ( 50,	Pkg, SourceLowerPriority);
    REGISTERFUNCTIONCALL1 ( 51,	Pkg, SourceMediaData);
    REGISTERFUNCTIONCALL1 ( 52,	Pkg, SourceProduct);
    REGISTERFUNCTIONCALL1 ( 53,	Pkg, SourceProductData);
    REGISTERFUNCTIONCALL3 ( 54,	Pkg, SourceProvideDir);
    REGISTERFUNCTIONCALL3 ( 55,	Pkg, SourceProvideFile);
    REGISTERFUNCTIONCALL1 ( 56,	Pkg, SourceRaisePriority);
    REGISTERFUNCTIONCALL  ( 57,	Pkg, SourceSaveRanks);
    REGISTERFUNCTIONCALL2 ( 58,	Pkg, SourceSetEnabled);
    REGISTERFUNCTIONCALL1 ( 59,	Pkg, SourceSetRamCache);
    REGISTERFUNCTIONCALL1 ( 60,	Pkg, SourceStartCache);
    REGISTERFUNCTIONCALL1 ( 61,	Pkg, SourceStartManager);
    REGISTERFUNCTIONCALL1 ( 62,	Pkg, TargetInitDU);
    REGISTERFUNCTIONCALL1 ( 63,	Pkg, TargetBlockSize);
    REGISTERFUNCTIONCALL1 ( 64,	Pkg, TargetCapacity);
    REGISTERFUNCTIONCALL1 ( 65,	Pkg, TargetFileHasOwner);
    REGISTERFUNCTIONCALL  ( 66,	Pkg, TargetFinish);
    REGISTERFUNCTIONCALL  ( 67,	Pkg, TargetGetDU);
    REGISTERFUNCTIONCALL2 ( 68,	Pkg, TargetInit);
    REGISTERFUNCTIONCALL1 ( 69,	Pkg, TargetInstall);
    REGISTERFUNCTIONCALL1 ( 70,	Pkg, TargetLogfile);
    REGISTERFUNCTIONCALL  ( 71,	Pkg, TargetProducts);
    REGISTERFUNCTIONCALL  ( 72,	Pkg, TargetRebuildDB);
    REGISTERFUNCTIONCALL1 ( 73,	Pkg, TargetRemove);
    REGISTERFUNCTIONCALL1 ( 74,	Pkg, TargetUpdateInf);
    REGISTERFUNCTIONCALL1 ( 75,	Pkg, TargetUsed);
    REGISTERFUNCTIONCALL  ( 76,	Pkg, YouAttachSource);
    REGISTERFUNCTIONCALL  ( 77,	Pkg, YouDisconnect);
    REGISTERFUNCTIONCALL  ( 78,	Pkg, YouFinish);
    REGISTERFUNCTIONCALL1 ( 79,	Pkg, YouFirstPatch);
    REGISTERFUNCTIONCALL2 ( 80,	Pkg, YouGetCurrentPatch);
    REGISTERFUNCTIONCALL  ( 81,	Pkg, YouGetDirectory);
    REGISTERFUNCTIONCALL  ( 82,	Pkg, YouGetPackages);
    REGISTERFUNCTIONCALL3 ( 83,	Pkg, YouGetPatches);
    REGISTERFUNCTIONCALL  ( 84,	Pkg, YouInstallCurrentPatch);
    REGISTERFUNCTIONCALL  ( 85,	Pkg, YouInstallPatches);
    REGISTERFUNCTIONCALL  ( 86,	Pkg, YouNextPatch);
    REGISTERFUNCTIONCALL  ( 87,	Pkg, YouRemovePackages);
    REGISTERFUNCTIONCALL  ( 88,	Pkg, YouSelectPatches);
    REGISTERFUNCTIONCALL  ( 89,	Pkg, YouStatus);
    REGISTERFUNCTIONCALL1 ( 90,	Pkg, PkgUpdateAll);
    REGISTERFUNCTIONCALL  ( 91,	Pkg, LastError);
    REGISTERFUNCTIONCALL  ( 92,	Pkg, LastErrorId);
    REGISTERFUNCTIONCALL  ( 93,	Pkg, LastErrorDetails);

    // callback registration routines
    REGISTERFUNCTIONCALL1 ( 94,		Pkg, CallbackMediaChange);
    REGISTERFUNCTIONCALL1 ( 95,		Pkg, CallbackSourceChange);
    REGISTERFUNCTIONCALL1 ( 96,		Pkg, CallbackStartProvide);
    REGISTERFUNCTIONCALL1 ( 97,		Pkg, CallbackProgressProvide);
    REGISTERFUNCTIONCALL1 ( 98,		Pkg, CallbackDoneProvide);
    REGISTERFUNCTIONCALL1 ( 99,		Pkg, CallbackStartPackage);
    REGISTERFUNCTIONCALL1 ( 100,	Pkg, CallbackProgressPackage);
    REGISTERFUNCTIONCALL1 ( 101,	Pkg, CallbackDonePackage);
    REGISTERFUNCTIONCALL1 ( 102,	Pkg, CallbackProgressRebuildDB);
    REGISTERFUNCTIONCALL1 ( 103,	Pkg, CallbackYouProgress);
    REGISTERFUNCTIONCALL1 ( 104,	Pkg, CallbackYouPatchProgress);
    REGISTERFUNCTIONCALL1 ( 105,	Pkg, CallbackYouExecuteYcpScript);
    REGISTERFUNCTIONCALL1 ( 106,	Pkg, CallbackYouScriptProgress);
    REGISTERFUNCTIONCALL1 ( 107,	Pkg, CallbackStartConvertDb);
    REGISTERFUNCTIONCALL1 ( 108,	Pkg, CallbackProgressConvertDb);
    REGISTERFUNCTIONCALL1 ( 109,	Pkg, CallbackNotifyConvertDb);
    REGISTERFUNCTIONCALL1 ( 110,	Pkg, CallbackStopConvertDb);
    
    REGISTERFUNCTIONCALL1 ( 111,	Pkg, PkgLocation);
    REGISTERFUNCTIONCALL1 ( 112,	Pkg, YouSetServer);
    REGISTERFUNCTIONCALL  ( 113,	Pkg, YouGetUserPassword);
    REGISTERFUNCTIONCALL3 ( 114,	Pkg, YouSetUserPassword);
}

///////////////////////////////////////////////////////////////////
//
//
//     METHOD NAME : PkgModuleFunctions::pkgError
//     METHOD TYPE : YCPValue
//
YCPValue PkgModuleFunctions::pkgError( PMError err_r, const YCPValue & ret_r )
{
  _last_error = err_r;
  return ret_r;
}


// ------------------------------------------------------------------
// general

/**
 * @builtin Pkg::InstSysMode () -> void
 *
 * Set packagemanager to "inst-sys" mode
 * - dont use local caches (ramdisk!)
 *
 * !!!!!!!!!! CAUTION !!!!!!!!!!!
 * Can only be called ONCE
 * MUST be called before any other function
 * !!!!!!!!!! CAUTION !!!!!!!!!!!
 */
YCPValue
PkgModuleFunctions::InstSysMode ()
{
    _y2pm.setNotRunningFromSystem();
    return YCPVoid();
}


/**
 * @builtin Pkg::SetLocale (string locale) -> void
 *
 * set the given locale as the "preferred" locale
 */
YCPValue
PkgModuleFunctions::SetLocale (const YCPString &locale)
{
    LangCode langcode(locale->value());
    _y2pm.setPreferredLocale (langcode);
    return YCPVoid();
}

/**
 * @builtin Pkg::GetLocale () -> string locale
 *
 * get the currently preferred locale
 */
YCPValue
PkgModuleFunctions::GetLocale ()
{
    return YCPString ((const std::string &)(_y2pm.getPreferredLocale()));
}


/**
 * @builtin Pkg::SetAdditionalLocales (list of string) -> void
 *
 * set list of
 */
YCPValue
PkgModuleFunctions::SetAdditionalLocales (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isList()))
    {
	return YCPError ("Bad args to Pkg::SetAdditionalLocales");
    }
    std::list<LangCode> langcodelist;
    int i = 0;
    YCPList langycplist = args->value(0)->asList();
    while (i < langycplist->size())
    {
	if (langycplist->value (i)->isString())
	{
	    langcodelist.push_back (LangCode (langycplist->value (i)->asString()->value()));
	}
	else
	{
	    y2error ("Pkg::SetAdditionalLocales ([...,%s,...]) not string", langycplist->value (i)->toString().c_str());
	}
	i++;
    }
    _y2pm.setRequestedLocales (langcodelist);
    return YCPVoid();
}

/**
 * @builtin Pkg::GetAdditionalLocales -> list
 *
 * return list of additional locales
 */
YCPValue
PkgModuleFunctions::GetAdditionalLocales ()
{
    YCPList langycplist;
    const std::list<LangCode> langcodelist = _y2pm.getRequestedLocales();
    for (std::list<LangCode>::const_iterator it = langcodelist.begin();
	 it != langcodelist.end(); ++it)
    {
	langycplist->add (YCPString ((const std::string &)(*it)));
    }
    return langycplist;
}


/**
 * @builtin Pkg::LastError
 *
 * get current error as string
 */
YCPValue
PkgModuleFunctions::LastError ()
{
    return YCPString (_last_error.errstr());
}

/**
 * @builtin Pkg::LastErrorDetails
 *
 * get current error details as string
 */
YCPValue
PkgModuleFunctions::LastErrorDetails ()
{
    return YCPString (_last_error.details());
}

/**
 * @builtin Pkg::LastErrorId
 *
 * get current error as id string
 */
YCPValue
PkgModuleFunctions::LastErrorId ()
{
    int errorId = _last_error;
    switch ( errorId ) {
        case PMError::E_ok:
            return YCPString( "ok" );
        case InstSrcError::E_isrc_cache_duplicate:
            return YCPString( "instsrc_duplicate" );
        default:
            return YCPString( "error" );
    }
}


/** ------------------------
 * Convert InstSrcDescr to product info YCPMap:
 * <TABLE>
 * <TR><TD>$[<TD>"product"             <TD>: YCPString (name' 'version)
 * <TR><TD>,<TD>"vendor"               <TD>: YCPString
 * <TR><TD>,<TD>"requires"             <TD>: YCPString
 * <TR><TD>,<TD>"name"                 <TD>: YCPString
 * <TR><TD>,<TD>"version"              <TD>: YCPString
 * <TR><TD>,<TD>"flags"                        <TD>: YCPString
 * <TR><TD>,<TD>"relnotesurl"          <TD>: YCPString
 * <TR><TD>,<TD>"distproduct"          <TD>: YCPString
 * <TR><TD>,<TD>"distversion"          <TD>: YCPString
 * <TR><TD>,<TD>"baseproduct"          <TD>: YCPString
 * <TR><TD>,<TD>"baseversion"          <TD>: YCPString
 * <TR><TD>];
 * </TABLE>
 */

YCPMap
PkgModuleFunctions::Descr2Map (constInstSrcDescrPtr descr)
{
    YCPMap map;

    map->add (YCPString ("product"), YCPString ((const std::string &)(descr->content_product().name) + " " + descr->content_product().edition.version()));
    map->add (YCPString ("vendor"), YCPString (descr->content_vendor()));
    map->add (YCPString ("requires"), YCPString (descr->content_requires().asString()));

    // for installation/modules/Product.ycp
    map->add (YCPString ("name"), YCPString ((const std::string &)(descr->content_product().name)));
    map->add (YCPString ("version"), YCPString (descr->content_product().edition.version()));
    map->add (YCPString ("flags"), YCPString (descr->content_flags()));
    map->add (YCPString ("relnotesurl"), YCPString (descr->content_relnotesurl()));

    // vendor already in map

    map->add (YCPString ("distproduct"), YCPString ((const std::string &)(descr->content_distproduct().name)));
    map->add (YCPString ("distversion"), YCPString ((const std::string &)(descr->content_distproduct().edition.version())));

    map->add (YCPString ("baseproduct"), YCPString ((const std::string &)(descr->content_baseproduct().name)));
    map->add (YCPString ("baseversion"), YCPString ((const std::string &)(descr->content_baseproduct().edition.version())));
    return map;
}


