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
#include <PkgModuleCallbacks.h>

#include <y2pm/InstSrcDescr.h>
#include <y2pm/InstSrcManager.h>
#include <y2pm/InstSrcDescr.h>
#include <ycp/YCPError.h>
#include <ycp/YCPInteger.h>
#include <ycp/YCPString.h>
#include <ycp/YCPBoolean.h>
#include <ycp/YCPMap.h>

/**
 * Constructor.
 */
PkgModuleFunctions::PkgModuleFunctions (YCPInterpreter *& wfmInterpreter)
    : _callbackHandler( *new CallbackHandler( wfmInterpreter ) )
{
  _y2pm.packageManager();
  _y2pm.selectionManager();
}

/**
 * Destructor.
 */
PkgModuleFunctions::~PkgModuleFunctions ()
{
    SourceFinishAll (YCPList());
    TargetFinish (YCPList());
    delete &_callbackHandler;
}

///////////////////////////////////////////////////////////////////
//
//
//	METHOD NAME : PkgModuleFunctions::pkgError
//	METHOD TYPE : YCPValue
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
PkgModuleFunctions::InstSysMode (YCPList args)
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
PkgModuleFunctions::SetLocale (YCPList args)
{
    if ((args->size() != 1)
	|| !(args->value(0)->isString()))
    {
	return YCPError ("Bad args to Pkg::SetLocale");
    }
    LangCode langcode(args->value(0)->asString()->value());
    _y2pm.setPreferredLocale (langcode);
    return YCPVoid();
}

/**
 * @builtin Pkg::GetLocale () -> string locale
 *
 * get the currently preferred locale
 */
YCPValue
PkgModuleFunctions::GetLocale (YCPList args)
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
PkgModuleFunctions::GetAdditionalLocales (YCPList args)
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
PkgModuleFunctions::LastError (YCPList args)
{
    return YCPString (_last_error.errstr());
}

/**
 * @builtin Pkg::LastErrorDetails
 *
 * get current error details as string
 */
YCPValue
PkgModuleFunctions::LastErrorDetails (YCPList args)
{
    return YCPString (_last_error.details());
}

/**
 * @builtin Pkg::LastErrorId
 *
 * get current error as id string
 */
YCPValue
PkgModuleFunctions::LastErrorId (YCPList args)
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
 * <TR><TD>$[<TD>"product"		<TD>: YCPString (name' 'version)
 * <TR><TD>,<TD>"vendor"		<TD>: YCPString
 * <TR><TD>,<TD>"requires"		<TD>: YCPString
 * <TR><TD>,<TD>"name"			<TD>: YCPString
 * <TR><TD>,<TD>"version"		<TD>: YCPString
 * <TR><TD>,<TD>"flags"			<TD>: YCPString
 * <TR><TD>,<TD>"relnotesurl"		<TD>: YCPString
 * <TR><TD>,<TD>"distproduct"		<TD>: YCPString
 * <TR><TD>,<TD>"distversion"		<TD>: YCPString
 * <TR><TD>,<TD>"baseproduct"		<TD>: YCPString
 * <TR><TD>,<TD>"baseversion"		<TD>: YCPString
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


